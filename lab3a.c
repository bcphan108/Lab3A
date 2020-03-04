#define _XOPEN_SOURCE 500

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include "ext2_fs.h"

int fd;
const int SUPERBLOCK_VALUE = 1024;
struct ext2_super_block super_block;
struct ext2_group_desc* groups;

void printerror(char *mess)
{
    fprintf(stderr, mess, sizeof(mess));
    exit(1);
}

int main(int argc, char* argv[])
{
    int ret_result = 0;
    unsigned int block_size;
    int true_block_num;

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

    char* bitmap = malloc(block_size * sizeof(char));
    offset_tmp = groups[0].bg_block_bitmap * block_size;
    ret_result = pread(fd, bitmap, block_size, offset_tmp);
    if (ret_result < 0)
        printerror("Error reading bitmap\n");
    unsigned int i;
    int j;
    char byte;
    for (i = 0; i < block_size; i++)
    {
        byte = bitmap[i];
        for (j = 0; j < 8; j++)
        {
            int bit = byte & (1 << j);
            int number = 0 * super_block.s_blocks_per_group + i * 8 + j + 1;
            if (!bit)
            {
                printf("BFREE,%u\n", number);
            }
        }
    }


    exit(0);
}

