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

void printerror(char *mess)
{
    fprintf(stderr, mess, sizeof(mess));
    exit(1);
}

int main(int argc, char* argv[])
{
    int ret_result = 0;

    if (argc != 2)
        printerror("Incorrect syntax, use: ./lab3a filename\n");

    fd = open(argv[1], O_RDONLY);

    if (fd < 0)
        printerror("Error opening file\n");

    ret_result = pread(fd, &super_block, sizeof(super_block), SUPERBLOCK_VALUE);
    if (ret_result < 0)
        printerror("Error reading superblock");

    //super block output
    printf("SUPERBLOCK,%u,%u,%u,%u,%u,%u,%u\n", super_block.s_blocks_count,
        super_block.s_inodes_count, SUPERBLOCK_VALUE, super_block.s_inode_size,
        super_block.s_blocks_per_group, super_block.s_inodes_per_group, 
        super_block.s_first_ino);


    exit(0);
}

