#include <assert.h>
#include <fcntl.h>
#include <stdio.h>
#include <sys/stat.h>
#include <stdint.h>
#include <stdlib.h>
#include <linux/fs.h>
#include <sys/types.h>
#include <getopt.h>
#include <string.h>

#include "ext2.h"

static char *fs_image = NULL;
static uint32_t inode_number;
static int print_block_address = 0;

static void parse_args(int argc, char *argv[]) {
    int opt;
    char *inode_string = NULL;
    struct option long_options[] = {
        {"addr", no_argument, NULL, 'a'},
        {NULL, 0, NULL, 0}
    };

    while ((opt = getopt_long(argc, argv, "-a", long_options, NULL)) != -1) {
        switch (opt) {
            case 'a':
                print_block_address = 1;
                break;
            case 1:
                if (fs_image == NULL) {
                    fs_image = optarg;
                } else if (inode_string == NULL) {
                    inode_string = optarg;
                } else {
                    fprintf(stderr, "Unexpected argument '%s'\n", optarg);
                    exit(EXIT_FAILURE);
                }
                break;
            case '?':
                fprintf(stderr, "Unknown option\n");
                exit(EXIT_FAILURE);
            default:
                fprintf(stderr, "Unexpected error\n");
                exit(EXIT_FAILURE);
        }
    }

    if (fs_image == NULL || inode_string == NULL) {
        fprintf(stderr, "Missing filesystem image or inode number\n");
        fprintf(stderr, "Usage: %s filesystem_image inode_number [-a|--addr]\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    char *endptr;
    inode_number = strtol(inode_string, &endptr, 10);
    if (*endptr != '\0' || inode_string == endptr) {
        fprintf(stderr, "Error: Invalid inode number '%s'\n", inode_string);
        exit(EXIT_FAILURE);
    }
}

static FILE *fd;
static uint32_t visited_blocks = 0;
static int64_t block_size;

void *block_data = NULL;
int is_filled_with_zeros = 0;

uint64_t remained_file_size;

static int process_block(uint32_t block_address) {
    if (print_block_address) {
        printf("block %d:\t%u\n", visited_blocks, block_address);
        return 0;
    }

    uint64_t data_size = block_size;
    if (data_size > remained_file_size) {
        data_size = (int64_t) remained_file_size;
    }

    if (block_data == NULL) {
        block_data = malloc(block_size);
        if (!block_data) {
            perror("Failed to allocate memory for block of data");
            return -1;
        }
    }

    // Handle sparse files
    if (block_address == 0) {
        if (!is_filled_with_zeros) {
            memset(block_data, 0, data_size);
            is_filled_with_zeros = 1;
        }

        if (fwrite(block_data, data_size, 1, stdout) != 1) {
            perror("Failed to write block of data");
            return -1;
        }
        remained_file_size -= data_size;
        return 0;
    }

    if (fseek(fd, block_address * block_size, SEEK_SET) != 0) {
        perror("Failed to seek to block of data");
        return -1;
    }

    is_filled_with_zeros = 0;

    size_t res = fread(block_data, data_size, 1, fd);
    if (res != 1 && data_size > 0) {
        perror("Failed to read block of data");
        return -1;
    }

    if (res == 1 && fwrite(block_data, data_size, 1, stdout) != 1) {
        perror("Failed to write block of data");
        return -1;
    }
    remained_file_size -= data_size;
    return 0;
}

static int process_indirect_block(uint32_t block_num, int level, uint32_t block_count, uint32_t pointers_in_block) {
    if (visited_blocks >= block_count) {
        return 0;
    }

    uint32_t *block_data = malloc(block_size);
    if (!block_data) {
        perror("Failed to allocate memory for indirect block");
        return -1;
    }

    // Handle sparse files
    if (block_num == 0) {
        // In case of a null block pointer, just fill all the pointers with 0
        memset(block_data, 0, block_size);
    } else {
        if (fseek(fd, block_num * block_size, SEEK_SET) != 0) {
            perror("Failed to seek to indirect block");
            free(block_data);
            return -1;
        }

        if (fread(block_data, block_size, 1, fd) != 1) {
            perror("Failed to read indirect block");
            free(block_data);
            return -1;
        }

        for (uint32_t i = 0; i < pointers_in_block; i++) {
            block_data[i] = le32toh(block_data[i]);
        }
    }

    int result = 0;

    for (uint32_t i = 0; i < pointers_in_block && visited_blocks < block_count; i++) {
        if (level == 0) {
            ++visited_blocks;
            result = process_block(block_data[i]);
        } else {
            result = process_indirect_block(block_data[i], level - 1, block_count, pointers_in_block);
        }
        if (result < 0) {
            break;
        }
    }

    free(block_data);
    return result;
}

int main(int argc, char *argv[]) {
    parse_args(argc, argv);

    fd = fopen(argv[1], "rb");
    if (fd == NULL) {
        perror("Failed to open image of the filesystem");
        return 1;
    }

    if (fseek(fd, SUPERBLOCK_OFFSET, SEEK_SET) != 0) {
        perror("Failed to seek to superblock");
        fclose(fd);
        return 1;
    }

    ext2_super_block sb;
    if (fread(&sb, sizeof(sb), 1, fd) != 1) {
        perror("Failed to read superblock");
        fclose(fd);
        return 1;
    }
    convert_superblock_endianness(&sb);

    if (sb.s_magic != 0xEF53) {
        fprintf(stderr, "Failed to read superblock. Wrong magic number.\n");
        fclose(fd);
        return 1;
    }
    uint16_t inode_size = INODE_SIZE;
    if (sb.s_rev_level >= 1) {
        ext2_extended_super_block esb;
        if (fread(&esb, sizeof(esb), 1, fd) != 1) {
            perror("Failed to read extended fields of the superblock");
            fclose(fd);
            return 1;
        }
        convert_extended_superblock_endianness(&esb);
        inode_size = esb.s_inode_size;
    }

    block_size = 1024 << sb.s_log_block_size;

    const uint32_t block_groups_count = (sb.s_blocks_count + sb.s_blocks_per_group - 1) / sb.s_blocks_per_group;

    if (block_groups_count != (sb.s_inodes_count + sb.s_inodes_per_group - 1) / sb.s_inodes_per_group) {
        fprintf(stderr, "Failed to calculate the number of block groups. The superblock is corrupted.\n");
        return 1;
    }

    const uint32_t block_group = (inode_number - 1) / sb.s_inodes_per_group;
    const uint32_t block_group_descriptor_offset = (block_size == 1024 ? 2 : 1) * block_size;

    if (fseek(fd, block_group_descriptor_offset + block_group * BLOCK_GROUP_DESCRIPTOR_SIZE, SEEK_SET) != 0) {
        perror("Failed to seek to group descriptor");
        fclose(fd);
        return 1;
    }

    ext2_block_group_descriptor gd;
    if (fread(&gd, sizeof(gd), 1, fd) != 1) {
        perror("Failed to read group descriptor");
        fclose(fd);
        return 1;
    }
    convert_group_descriptor_endianness(&gd);

    const uint32_t inode_index_in_group = (inode_number - 1) % sb.s_inodes_per_group;

    const uint32_t absolute_inode_location = gd.bg_inode_table * block_size + inode_index_in_group * inode_size;

    if (fseek(fd, absolute_inode_location, SEEK_SET) != 0) {
        perror("Failed to seek to inode");
        fclose(fd);
        return 1;
    }

    ext2_inode inode;
    if (fread(&inode, sizeof(inode), 1, fd) != 1) {
        perror("Failed to read inode");
        fclose(fd);
        return 1;
    }
    convert_inode_endianness(&inode);

    const uint64_t file_size = (sb.s_rev_level > 0 ? (uint64_t) inode.i_size_high << 32ULL : 0)
                               + (uint64_t) inode.i_size_lo;

    const uint32_t block_count = (file_size + block_size - 1) / block_size;


    const uint32_t pointers_in_block = block_size / sizeof(uint32_t);
    visited_blocks = 0;

    block_data = malloc(block_size);
    if (!block_data) {
        perror("Failed to allocate memory for direct blocks");
        return -1;
    }

    remained_file_size = file_size;
    for (; visited_blocks < N_DIRECT_BLOCK_POINTERS && visited_blocks < block_count; ++visited_blocks) {
        if (process_block(inode.i_block[visited_blocks]) != 0) {
            fprintf(stderr, "Failed to process block %u\n", inode.i_block[visited_blocks]);
            free(block_data);
            fclose(fd);
            return 1;
        }
    }

    if (visited_blocks < block_count) {
        int result = process_indirect_block(inode.i_block[N_DIRECT_BLOCK_POINTERS], 0,
                                            block_count, pointers_in_block);
        if (result < 0) {
            fprintf(stderr, "Failed to process singly indirect blocks %u\n", inode.i_block[N_DIRECT_BLOCK_POINTERS]);
            free(block_data);
            fclose(fd);
            return 1;
        }
    }

    if (visited_blocks < block_count) {
        int result = process_indirect_block(inode.i_block[N_DIRECT_BLOCK_POINTERS + 1], 1,
                                            block_count, pointers_in_block);
        if (result < 0) {
            fprintf(stderr, "Failed to process doubly indirect blocks %u\n",
                    inode.i_block[N_DIRECT_BLOCK_POINTERS + 1]);
            free(block_data);
            fclose(fd);
            return 1;
        }
    }

    if (visited_blocks < block_count) {
        int result = process_indirect_block(inode.i_block[N_DIRECT_BLOCK_POINTERS + 2], 2,
                                            block_count, pointers_in_block);
        if (result < 0) {
            fprintf(stderr, "Failed to process triply indirect blocks %u\n",
                    inode.i_block[N_DIRECT_BLOCK_POINTERS + 2]);
            free(block_data);
            fclose(fd);
            return 1;
        }
    }

    free(block_data);
    fclose(fd);
    return 0;
}
