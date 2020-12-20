#include "usertraps.h"
#include "misc.h"

#include "fdisk.h"

dfs_superblock sb;
dfs_inode inodes[DFS_INODE_MAX_NUM];
uint32 fbv[DFS_FBV_MAX_NUM_WORDS];

int diskblocksize = 0; // These are global in order to speed things up
int disksize = 0;      // (i.e. fewer traps to OS to get the same number)

int FdiskWriteBlock(uint32 blocknum, dfs_block *b); //You can use your own function. This function 
//calls disk_write_block() to write physical blocks to disk

void main (int argc, char *argv[])
{
	// STUDENT: put your code here. Follow the guidelines below. They are just the main steps. 
	// You need to think of the finer details. You can use bzero() to zero out bytes in memory
  int fs_block_size;
  int ret;
  int i = 0;
  dfs_block d;
  char dummyFile[20] = "RangaTestFile";
  int j = 0;
  int bytesWritten = 0;
  int bytesTobeWritten = 0;
  int currentBlock = 2;
  int inodeBlock = 0;
  int bytesLeft;
  int pos = 0;
  int fbv_num;
  int fbv_offset;
  char pos_c;
  uint32* fbv_ptr = fbv; 

  //Initializations and argc check
  if(argc != 2)
  {
    Printf("usage = fdisk.dlx.obj <fs_block_size> \n");
    Exit();
  }
  fs_block_size = dstrtol(argv[1],NULL,10);
  // Need to invalidate filesystem before writing to it to make sure that the OS
  // doesn't wipe out what we do here with the old version in memory
  // You can use dfs_invalidate(); but it will be implemented in Problem 2. You can just do 
  //sb.valid = 0;

  dfs_invalidate(); 
  disksize = disk_size(); 
  Printf("The total disk size = %d\n",disksize);
  diskblocksize = disk_blocksize(); 
  Printf("The diskblock size = %d\n",diskblocksize);
  //num_filesystem_blocks = disksize / fs_block_size;

  // Make sure the disk exists before doing anything else
  ret = disk_create();
  if(ret == DFS_FAIL)
  {
    Printf("Exiting.\n");
    //Exit();
  }

  j = 2;
  // Write all inodes as not in use and empty (all zeros)
  for(i = 0; i < FDISK_NUM_INODES; i++)
  {
    inodes[i].used = 0;
    inodes[i].filesize = 0;
    inodes[i].DoubleIndirectBlockNum = NULL;
    inodes[i].singleIndirectBlockNum = NULL;
    inodes[i].blocksUsed = 0;
    inodes[i].lastWrittenBlockNumber = 0;
    bzero(inodes[i].filename,sizeof(inodes[i].filename));
    bzero((char*)inodes[i].directAddress,sizeof(inodes[i].directAddress));
    //inodes[i].filename = NULL;
    //inodes[i].directAddress = NULL;
    //inodes[i].inode_num = i;
    //bcopy(dummyFile,(char*)&inodes[i].filename,dstrlen(dummyFile));
    pos = bytesWritten % diskblocksize;
    bytesTobeWritten += sizeof(dfs_inode);
    Printf("Fdsik : Size of dfs_inode = %d\n",sizeof(dfs_inode));
    //Exit();
    Printf("fdisk : pos = %d, byteWritten = %d, bytesTobeWritten = %d, inodeBlock = %d, currentBlock = %d\n",pos,bytesWritten,bytesTobeWritten\
                                                                  ,inodeBlock,currentBlock);
    Printf("fdisk : inodes[%d].used = %d\n",i,inodes[i].used);
    //PrintInode(inodes[i]);
    //Exit();
    if((bytesTobeWritten / diskblocksize) > inodeBlock)
    {
      bytesLeft = bytesTobeWritten % sizeof(dfs_inode);
      bcopy((char*)&inodes[i],(char*)d.data + pos,sizeof(dfs_inode) - bytesLeft);
      FdiskWriteBlock(currentBlock,&d);
      currentBlock += 1;
      inodeBlock += 1;
      pos = 0;
      bzero(d.data,diskblocksize);
      bcopy((char*)&inodes[i],(char*)d.data + pos,bytesLeft);
      FdiskWriteBlock(currentBlock,&d);
    }
    else
    {
      //Printf("pos = %d\n, byteWritten = %d\n, bytesTobeWritten = %d\n",pos,bytesWritten,bytesTobeWritten);
      bcopy((char*)&inodes[i],(char*)d.data + pos,sizeof(dfs_inode)); 
      FdiskWriteBlock(currentBlock,&d);
    }
    
    
    //FdiskWriteBlock(2,&d);
   bytesWritten += sizeof(dfs_inode);
  }
  
  //bcopy((char*)&inodes[0],(char*)d.data,sizeof(dfs_inode));
  //bcopy((char*)&inodes[0],(char*)d.data + sizeof(dfs_inode),sizeof(dfs_inode));
  //FdiskWriteBlock(2,&d);
  Printf("i = %d\n",i);
  //PrintInode(inodes[0]);
  Printf("Total Bytes written = %d\n",bytesWritten);

  // Next, setup free block vector (fbv) and write free block vector to the disk

  // Finally, setup superblock as valid filesystem and write superblock and boot record to disk: 
  // boot record is all zeros in the first physical block, and superblock structure goes into the second physical block


  //To do the fbv here.
  Printf("Number of fbv vectors = %d, total no of disk bloxks = %d\n",DFS_FBV_MAX_NUM_WORDS,DFS_FBV_MAX_NUM_WORDS * 32);
  Printf("CurrentBlock = %d\n",currentBlock);
  for(i = 0; i < DFS_FBV_MAX_NUM_WORDS; i++)
  {
    fbv[i] = 0xffffffff;
  }
  fbv_num = currentBlock / 32;
  fbv_offset = currentBlock % 32;
  for(i = 0; i < fbv_num; i++)
  {
    fbv[i] = 0;
  }
  fbv[i] = (-1 << fbv_offset) & fbv[i];
  //PrintFBV();
  pos_c = 0;
  bzero(d.data,diskblocksize);
  Printf("DFS_FBV_MAX_NUM_WORDS = %d\n",DFS_FBV_MAX_NUM_WORDS);
  Printf("sizeof fbv = %d\n",sizeof(fbv));
  //Exit();
    bytesWritten = 0;
    i = 0;
  //for(i = 0; i < DFS_FBV_MAX_NUM_WORDS / diskblocksize;i++)
  while(bytesWritten < sizeof(fbv))
  { 

    Printf("0x%x\n",fbv);
    Printf("i = %d, i*diskblocksize - %d\n",i,i * diskblocksize);
    bcopy((char*)&fbv[i*32],d.data,diskblocksize);
    FdiskWriteBlock(currentBlock,&d);
    //Printf("Value = 0x%x, Address = 0x%x ,i = %d\n",fbv_ptr[i],&fbv_ptr[i],i);
    currentBlock += 1;
    bytesWritten += diskblocksize;
    i = i + 1;
    Printf("bytesWritten = %d\n",bytesWritten);
    
  }
  Printf("Current physical block = %d\n",currentBlock);
  //PrintFBV();
  Printf("Zeroing out all other memory.\n");

  //while(currentBlock < DISK_SIZE/diskblocksize)
  while(currentBlock < 200)
  {
    Printf("%d\n",currentBlock);
    bzero(d.data,diskblocksize);
    FdiskWriteBlock(currentBlock,&d);
    currentBlock += 1;
  }

  sb.valid = 1;
  sb.number_inodes = FDISK_NUM_INODES;
  sb.start_array_inodes = 1;
  sb.num_fs_blocks = disksize / fs_block_size;
  sb.start_free_block_vector = 37;
  sb.fs_blocksize = fs_block_size;
  bcopy((char*)&sb,(char*)d.data,sizeof(sb));
  //PrintSuperBlock(&sb);
  //PrintInode();
  //Exit();
  //Exit();
  FdiskWriteBlock(1,&d);
  //Exit();
  bzero(d.data,diskblocksize);
  FdiskWriteBlock(0,&d);
  //Printf(currentBlock);
  Printf("fdisk (%d): Formatted DFS disk for %d bytes.\n", getpid(), disksize);
  //Exit();
}

int FdiskWriteBlock(uint32 blocknum, dfs_block *b) {
  // STUDENT: put your code here
  char i = 0;
  int ret;
  ret = disk_write_block(blocknum, b->data);
  return ret;
}

/*void PrintSuperBlock(dfs_superblock* s)
{
  Printf("Address of superblock = 0x%x\n",s);
  Printf("Address of valid = 0x%x, Valid = %d\n",&s->valid,s->valid);
  Printf("Address of number_inodes = 0x%x, number_inodes = %d\n",&s->number_inodes,s->number_inodes);
  Printf("Address of start_array_inodes = 0x%x, start_array_inodes = %d\n",&s->start_array_inodes,s->start_array_inodes);
  Printf("Address of num_fs_blocks = 0x%x, num_fs_blocks = %d\n",&s->num_fs_blocks,s->num_fs_blocks);
  Printf("Address of start_free_block_vector = 0x%x, start_free_block_vector = %d\n",&s->start_free_block_vector,s->start_free_block_vector);
  Printf("Address of fs_blocksize = 0x%x, fs_blocksize = %d\n",&s->fs_blocksize,s->fs_blocksize);
}*/

void PrintInode(dfs_inode d_i)
{
  
  Printf("Size of inode->used  = %d, inode->used = %d\n",sizeof(d_i.used),d_i.used);
  Printf("Size of inode->filesize  = %d, inode->filesize  = %d\n",sizeof(d_i.filesize),d_i.filesize);
  Printf("Size of inode->filename  = %d, inode->filename  = ",sizeof(d_i.filename));
  Printf(d_i.filename);
  Printf("\n");
  Printf("Size of inode->directAddress  = %d, inode->directAddress  = %d\n",sizeof(d_i.directAddress),d_i.directAddress);
  Printf("Size of inode->singleIndirectBlockNum  = %d, inode->singleIndirectBlockNum = %d\n",sizeof(d_i.singleIndirectBlockNum),d_i.singleIndirectBlockNum);
  Printf("Size of inode->DoubleIndirectBlockNum  = %d, inode->DoubleIndirectBlockNum  = %d\n",sizeof(d_i.DoubleIndirectBlockNum),d_i.DoubleIndirectBlockNum);
  Printf("Total size of inode = %d\n", sizeof(d_i));

}

void PrintFBV()
{
  int i = 0;
  for(i = 0;i < DFS_FBV_MAX_NUM_WORDS;i++)
  {
  if(fbv[i] > 0)
  {
    Printf("fdisk.c : fbv[%d] = 0x%x\n",i,fbv[i]);
    //Exit();
  }
  else
  {
    Printf("fdisk.c : fbv[%d] = 0x%x\n",i,fbv[i]);
  }
  }
  
}
