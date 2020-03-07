#define _XOPEN_SOURCE 500

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include "ext2_fs.h"
#include <time.h>
#include <stdint.h>

int fd;
const int SUPERBLOCK_VALUE = 1024;
struct ext2_super_block super_block;
struct ext2_group_desc* groups;

void printerror(char *mess)
{
    fprintf(stderr, mess, sizeof(mess));
    exit(1);
}

void gettime(unsigned int time, char* buffer)
{
    time_t buf_time = (time_t) time;
    struct tm res = *gmtime(&buf_time);
    strftime(buffer, 80, "%m/%d/%y %H:%M:%S", &res);
}

int main(int argc, char* argv[])
{
    int ret_result = 0;
    unsigned int block_size;
    int true_block_num;
    int bit, number;

    if (argc != 2)
        printerror("Incorrect syntax, use: ./lab3a filename\n");

    fd = open(argv[1], O_RDONLY);

    if (fd < 0)
        printerror("Error opening file\n");



    //super block output
    ret_result = pread(fd, &super_block, sizeof(super_block), SUPERBLOCK_VALUE);
    if (ret_result < 0)
        printerror("Error reading superblock");
    block_size = EXT2_MIN_BLOCK_SIZE << super_block.s_log_block_size;

    printf("SUPERBLOCK,%u,%u,%u,%u,%u,%u,%u\n", super_block.s_blocks_count,
        super_block.s_inodes_count, block_size, 
        super_block.s_inode_size, super_block.s_blocks_per_group, 
        super_block.s_inodes_per_group, super_block.s_first_ino);



    //print group info
    int x;
    groups = malloc(sizeof(struct ext2_group_desc));
    if (groups == NULL)
        printerror("Error allocating memory for structure\n");
    if (block_size == 1024)
        x = 2;
    else
        x = 1;
    int offset_tmp = block_size * x; //* sizeof(struct ext2_group_desc);
    ret_result = pread(fd, &groups[0], sizeof(struct ext2_group_desc), offset_tmp);
    if (ret_result < 0)
        printerror("Error reading groups\n");

    if (super_block.s_blocks_per_group > super_block.s_blocks_count)
        true_block_num = super_block.s_blocks_count;
    else
        true_block_num = super_block.s_blocks_per_group;
    printf("GROUP,%u,%u,%u,%u,%u,%u,%u,%u\n", 0, true_block_num, super_block.s_inodes_per_group,
        groups[0].bg_free_blocks_count, groups[0].bg_free_inodes_count,
        groups[0].bg_block_bitmap, groups[0].bg_inode_bitmap, groups[0].bg_inode_table);

    //bitmap
    char* bitmap = malloc(block_size);
    offset_tmp = groups[0].bg_block_bitmap * block_size;
    ret_result = pread(fd, bitmap, block_size, offset_tmp);
    if (ret_result < 0)
        printerror("Error reading bitmap\n");
    unsigned int i;
    int j;
    for (i = 0; i < block_size; i++)
    {
        for (j = 0; j < 8; j++)
        {
            bit = bitmap[i] & (1 << j);
            number = 0 * super_block.s_blocks_per_group + i * 8 + j + 1;
            if (!bit)
            {
                printf("BFREE,%u\n", number);
            }
        }
    }

    //inode bitmaps
    offset_tmp = groups[0].bg_inode_bitmap * block_size;
    ret_result = pread(fd, bitmap, block_size, offset_tmp);
    if (ret_result < 0)
        printerror("Error reading inode bitmaps\n");
    for (i = 0; i < block_size; i++)
    {
        for (j = 0; j < 8; j++)
        {
            bit = bitmap[i] & (0x01 << j);
            number = 0 * super_block.s_inodes_per_group + i * 8 + j + 1;
            if (!bit)
            {
                printf("IFREE,%u\n", number);
            }
        }
    }

    //inode tables
    offset_tmp = groups[0].bg_inode_table * block_size;
    struct ext2_inode* table = malloc(super_block.s_inodes_per_group * sizeof(struct ext2_inode));
    if (table == NULL)
        printerror("Error allocating memory\n");
    ret_result = pread(fd, table, super_block.s_inodes_per_group * sizeof(struct ext2_inode), offset_tmp);
    if (ret_result < 0)
        printerror("Error reading inode tables\n");
    
    for (i = 0; i < super_block.s_inodes_per_group; i++)
    {
        int n = 0 * super_block.s_inodes_per_group + i + 1;
        struct ext2_inode c = table[i];
        if (c.i_links_count != 0 && c.i_mode != 0)
        {
            char type;
            unsigned int form = c.i_mode & 0xF000;
            if (form == 0x8000)
                type = 'f';
            else if (form == 0x4000)
                type = 'd';
            else if (form == 0xA000)
                type = 'l';
            else
                type = '?';
            
            char ctime[30], mtime[30], atime[30];
            gettime(c.i_ctime, ctime);
            gettime(c.i_ctime, mtime);
            gettime(c.i_ctime, atime);

            printf("INODE,%d,%c,%o,%u,%u,%u,%s,%s,%s,%u,%u", n, type, (0xFFF & c.i_mode),
                c.i_uid, c.i_gid, c.i_links_count, ctime, mtime, atime, c.i_size, c.i_blocks);
            for (j = 0; j < 15; j++)
                printf(",%u",c.i_block[j]);
            printf("\n");

            if (c.i_block[EXT2_IND_BLOCK])
            {
                
            }
            
        }
    }

    
    
    exit(0);
}

