#ifndef __FDISK_H__
#define __FDISK_H__

typedef unsigned int uint32;

#include "dfs_shared.h" // This gets us structures and #define's from main filesystem driver

#define FDISK_INODE_BLOCK_START 1 // Starts after super block (which is in file system block 0, physical block 1)
#define FDISK_INODE_NUM_BLOCKS 36 // Number of file system blocks to use for inodes
#define FDISK_NUM_INODES 192 //STUDENT: define this
#define FDISK_FBV_BLOCK_START 37//STUDENT: define this
#define FDISK_BOOT_FILESYSTEM_BLOCKNUM 0 // Where the boot record and superblock reside in the filesystem

#define DFS_INODE_MAX_NUM FDISK_NUM_INODES
#define DFS_FBV_MAX_NUM_WORDS (64*1024*1024 / (512 * 32))

#ifndef NULL
#define NULL (void *)0x0
#endif

//STUDENT: define additional parameters here, if any

#define DISK_SIZE (64*1024*1024)
#define DISK_BLOCK_SIZE 256
#define FS_BLOCKS (DISK_SIZE / 512) 

#endif
