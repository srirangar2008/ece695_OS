#ifndef __DFS_SHARED__
#define __DFS_SHARED__

typedef struct dfs_superblock {
  // STUDENT: put superblock internals here
  int valid;
  uint32 fs_blocksize;
  uint32 num_fs_blocks;
  uint32 start_array_inodes;
  uint32 number_inodes;
  uint32 start_free_block_vector;
} dfs_superblock;

#define DFS_BLOCKSIZE 512  // Must be an integer multiple of the disk blocksize
#define MAX_INODES 192


typedef struct dfs_block {
  char data[DFS_BLOCKSIZE];
} dfs_block;

typedef struct dfs_inode {
  // STUDENT: put inode structure internals here
  // IMPORTANT: sizeof(dfs_inode) MUST return 96 in order to fit in enough
  // inodes in the filesystem (and to make your life easier).  To do this, 
  // adjust the maximumm length of the filename until the size of the overall inode 
  // is 96 bytes.
  int blocksUsed;
  int lastWrittenBlockNumber;
  int used; //used = 0 -> free, used - 1 -> in use.
  uint32 filesize;
  char filename[32];
  int directAddress[10];
  int singleIndirectBlockNum;
  int DoubleIndirectBlockNum;
} dfs_inode;

#define DFS_MAX_FILESYSTEM_SIZE 0x4000000  // 64MB

#define NUM_FS_BLOCKS (DFS_MAX_FILESYSTEM_SIZE/DFS_BLOCKSIZE)
#define NUM_FBV (NUM_FS_BLOCKS / 32)


#define DFS_FAIL -1
#define DFS_SUCCESS 1



#endif
