#ifndef EXT2_H
#define EXT2_H

#include <stdint.h>

#define SUPERBLOCK_OFFSET 1024
#define SUPERBLOCK_SIZE 1024

#pragma pack(push, 1)
typedef struct ext2_super_block_t {
    uint32_t s_inodes_count; // Offset 0: Total number of inodes in file system
    uint32_t s_blocks_count; // Offset 4: Total number of blocks in file system
    uint32_t s_r_blocks_count; // Offset 8: Number of blocks reserved for superuser
    uint32_t s_free_blocks_count; // Offset 12: Total number of unallocated blocks
    uint32_t s_free_inodes_count; // Offset 16: Total number of unallocated inodes
    uint32_t s_first_data_block; // Offset 20: Block number of the block containing the superblock
    uint32_t s_log_block_size; // Offset 24: log2 (block size) - 10
    uint32_t s_log_frag_size; // Offset 28: log2 (fragment size) - 10
    uint32_t s_blocks_per_group; // Offset 32: Number of blocks in each block group
    uint32_t s_frags_per_group; // Offset 36: Number of fragments in each block group
    uint32_t s_inodes_per_group; // Offset 40: Number of inodes in each block group
    uint32_t s_mtime; // Offset 44: Last mount time (in POSIX time)
    uint32_t s_wtime; // Offset 48: Last written time (in POSIX time)
    uint16_t s_mnt_count; // Offset 52: Number of times the volume has been mounted since its last consistency check
    uint16_t s_max_mnt_count; // Offset 54: Number of mounts allowed before a consistency check
    uint16_t s_magic; // Offset 56: Ext2 signature (0xEF53)
    uint16_t s_state; // Offset 58: File system state
    uint16_t s_errors; // Offset 60: What to do when an error is detected
    uint16_t s_minor_rev_level; // Offset 62: Minor portion of version
    uint32_t s_lastcheck; // Offset 64: POSIX time of last consistency check (fsck)
    uint32_t s_checkinterval; // Offset 68: Interval (in POSIX time) between forced consistency checks
    uint32_t s_creator_os; // Offset 72: Operating system ID
    uint32_t s_rev_level; // Offset 76: Major portion of version (Revision level)
    uint16_t s_def_resuid; // Offset 80: User ID that can use reserved blocks
    uint16_t s_def_resgid; // Offset 82: Group ID that can use reserved blocks
} ext2_super_block;

void convert_superblock_endianness(ext2_super_block *sb);

typedef struct ext2_extended_super_block_t {
    uint32_t s_first_ino; // Offset 84: First non-reserved inode in file system
    uint16_t s_inode_size; // Offset 88: Size of each inode structure in bytes
    uint16_t s_block_group_nr; // Offset 90: Block group that this superblock is part of
    uint32_t s_feature_compat; // Offset 92: Optional features present
    uint32_t s_feature_incompat; // Offset 96: Required features present
    uint32_t s_feature_ro_compat; // Offset 100: Features that if not supported, mount read-only
    uint8_t s_uuid[16]; // Offset 104: File system ID (UUID)
    char s_volume_name[16]; // Offset 120: Volume name (C-style string)
    char s_last_mounted[64]; // Offset 136: Path volume was last mounted to (C-style string)
    uint32_t s_algorithm_usage_bitmap; // Offset 200: Compression algorithms used
    uint8_t s_prealloc_blocks; // Offset 204: Number of blocks to preallocate for files
    uint8_t s_prealloc_dir_blocks; // Offset 205: Number of blocks to preallocate for directories
    uint16_t s_reserved_gdt_blocks; // Offset 206: (Unused)
    uint8_t s_journal_uuid[16]; // Offset 208: Journal ID (UUID)
    uint32_t s_journal_inum; // Offset 224: Journal inode
    uint32_t s_journal_dev; // Offset 228: Journal device
    uint32_t s_last_orphan; // Offset 232: Head of orphan inode list (ends byte 235)
} ext2_extended_super_block;

void convert_extended_superblock_endianness(ext2_extended_super_block *esb);

#define BLOCK_GROUP_DESCRIPTOR_SIZE sizeof(ext2_block_group_descriptor)

typedef struct ext2_block_group_descriptor_t {
    uint32_t bg_block_bitmap; // Offset 0: Block address of block usage bitmap
    uint32_t bg_inode_bitmap; // Offset 4: Block address of inode usage bitmap
    uint32_t bg_inode_table; // Offset 8: Starting block address of inode table
    uint16_t bg_free_blocks_count; // Offset 12: Number of unallocated blocks in group
    uint16_t bg_free_inodes_count; // Offset 14: Number of unallocated inodes in group
    uint16_t bg_used_dirs_count; // Offset 16: Number of directories in group
    uint8_t bg_reserved[14]; // Offset 18: Unused (padding to 32 bytes)
} ext2_block_group_descriptor;

void convert_group_descriptor_endianness(ext2_block_group_descriptor *gd);

#define INODE_SIZE sizeof(ext2_inode)
#define N_BLOCKS 15 // 12 direct + 1 singly + 1 doubly + 1 triply indirect
#define N_DIRECT_BLOCK_POINTERS 12

typedef struct ext2_inode_t {
    uint16_t i_mode; // Offset 0: Type and Permissions
    uint16_t i_uid; // Offset 2: User ID
    uint32_t i_size_lo; // Offset 4: Lower 32 bits of size in bytes
    uint32_t i_atime; // Offset 8: Last Access Time (in POSIX time)
    uint32_t i_ctime; // Offset 12: Creation Time (in POSIX time)
    uint32_t i_mtime; // Offset 16: Last Modification time (in POSIX time)
    uint32_t i_dtime; // Offset 20: Deletion time (in POSIX time)
    uint16_t i_gid; // Offset 24: Group ID
    uint16_t i_links_count; // Offset 26: Count of hard links
    uint32_t i_blocks; // Offset 28: Count of disk sectors (not Ext2 blocks)
    uint32_t i_flags; // Offset 32: Flags
    uint32_t i_osd1; // Offset 36: Operating System Specific value #1 (e.g., Hurd translator)
    uint32_t i_block[N_BLOCKS];
    // Offset 40: Direct Block Pointers (i_block[0:12]),
    //            Singly Indirect Block Pointer (i_block[12]),
    //            Doubly Indirect Block Pointer (i_block[13]),
    //            Triply Indirect Block Pointer (i_block[14])
    uint32_t i_generation; // Offset 100: Generation number (Primarily used for NFS)
    uint32_t i_file_acl; // Offset 104: Extended attribute block (File ACL) or reserved (Ext2 ver 0)
    uint32_t i_size_high; // Offset 108: Upper 32 bits of file size (if feature bit set) or Directory ACL
    //            (if a directory and feature bit set), or reserved (Ext2 ver 0)
    uint32_t i_faddr; // Offset 112: Block address of fragment
    uint8_t i_osd2[12]; // Offset 116: Operating System Specific Value #2
} ext2_inode;
#pragma pack(pop)

void convert_inode_endianness(ext2_inode *inode);
#endif //EXT2_H
