#include <endian.h>

#include "ext2.h"

void convert_superblock_endianness(ext2_super_block *sb) {
    sb->s_inodes_count = le32toh(sb->s_inodes_count);
    sb->s_blocks_count = le32toh(sb->s_blocks_count);
    sb->s_r_blocks_count = le32toh(sb->s_r_blocks_count);
    sb->s_free_blocks_count = le32toh(sb->s_free_blocks_count);
    sb->s_free_inodes_count = le32toh(sb->s_free_inodes_count);
    sb->s_first_data_block = le32toh(sb->s_first_data_block);
    sb->s_log_block_size = le32toh(sb->s_log_block_size);
    sb->s_log_frag_size = le32toh(sb->s_log_frag_size);
    sb->s_blocks_per_group = le32toh(sb->s_blocks_per_group);
    sb->s_frags_per_group = le32toh(sb->s_frags_per_group);
    sb->s_inodes_per_group = le32toh(sb->s_inodes_per_group);
    sb->s_mtime = le32toh(sb->s_mtime);
    sb->s_wtime = le32toh(sb->s_wtime);
    sb->s_mnt_count = le16toh(sb->s_mnt_count);
    sb->s_max_mnt_count = le16toh(sb->s_max_mnt_count);
    sb->s_magic = le16toh(sb->s_magic);
    sb->s_state = le16toh(sb->s_state);
    sb->s_errors = le16toh(sb->s_errors);
    sb->s_minor_rev_level = le16toh(sb->s_minor_rev_level);
    sb->s_lastcheck = le32toh(sb->s_lastcheck);
    sb->s_checkinterval = le32toh(sb->s_checkinterval);
    sb->s_creator_os = le32toh(sb->s_creator_os);
    sb->s_rev_level = le32toh(sb->s_rev_level);
    sb->s_def_resuid = le16toh(sb->s_def_resuid);
    sb->s_def_resgid = le16toh(sb->s_def_resgid);
}

void convert_extended_superblock_endianness(ext2_extended_super_block *esb) {
    esb->s_first_ino = le32toh(esb->s_first_ino);
    esb->s_inode_size = le16toh(esb->s_inode_size);
    esb->s_block_group_nr = le16toh(esb->s_block_group_nr);
    esb->s_feature_compat = le32toh(esb->s_feature_compat);
    esb->s_feature_incompat = le32toh(esb->s_feature_incompat);
    esb->s_feature_ro_compat = le32toh(esb->s_feature_ro_compat);
    // s_uuid - no conversion
    // s_volume_name - no conversion
    // s_last_mounted - no conversion
    esb->s_algorithm_usage_bitmap = le32toh(esb->s_algorithm_usage_bitmap);
    // s_prealloc_blocks - no conversion
    // s_prealloc_dir_blocks - no conversion
    esb->s_reserved_gdt_blocks = le16toh(esb->s_reserved_gdt_blocks);
    // journal_uuid - no conversion
    esb->s_journal_inum = le32toh(esb->s_journal_inum);
    esb->s_journal_dev = le32toh(esb->s_journal_dev);
    esb->s_last_orphan = le32toh(esb->s_last_orphan);
}

void convert_group_descriptor_endianness(ext2_block_group_descriptor *gd) {
    gd->bg_block_bitmap = le32toh(gd->bg_block_bitmap);
    gd->bg_inode_bitmap = le32toh(gd->bg_inode_bitmap);
    gd->bg_inode_table = le32toh(gd->bg_inode_table);
    gd->bg_free_blocks_count = le16toh(gd->bg_free_blocks_count);
    gd->bg_free_inodes_count = le16toh(gd->bg_free_inodes_count);
    gd->bg_used_dirs_count = le16toh(gd->bg_used_dirs_count);
    // bg_pad - no conversion
    // bg_reserved - no conversion
}

void convert_inode_endianness(ext2_inode *inode) {
    inode->i_mode = le16toh(inode->i_mode);
    inode->i_uid = le16toh(inode->i_uid);
    inode->i_size_lo = le32toh(inode->i_size_lo);
    inode->i_atime = le32toh(inode->i_atime);
    inode->i_ctime = le32toh(inode->i_ctime);
    inode->i_mtime = le32toh(inode->i_mtime);
    inode->i_dtime = le32toh(inode->i_dtime);
    inode->i_gid = le16toh(inode->i_gid);
    inode->i_links_count = le16toh(inode->i_links_count);
    inode->i_blocks = le32toh(inode->i_blocks);
    inode->i_flags = le32toh(inode->i_flags);
    inode->i_osd1 = le32toh(inode->i_osd1);
    for (int i = 0; i < N_BLOCKS; ++i) {
        inode->i_block[i] = le32toh(inode->i_block[i]);
    }
    inode->i_generation = le32toh(inode->i_generation);
    inode->i_file_acl = le32toh(inode->i_file_acl);
    inode->i_size_high = le32toh(inode->i_size_high);
    inode->i_faddr = le32toh(inode->i_faddr);
    // i_osd2 - no conversion
}
