#include "ostraps.h"
#include "dlxos.h"
#include "traps.h"
#include "queue.h"
#include "disk.h"
#include "dfs.h"
#include "synch.h"
#include "filesys.h"
#include "files.h"

static dfs_inode inodes[MAX_INODES]; // all inodes
static dfs_superblock sb; // superblock
static uint32 fbv[NUM_FBV]; // Free block vector

static uint32 negativeone = 0xFFFFFFFF;
static inline uint32 invert(uint32 n) { return n ^ negativeone; }

// You have already been told about the most likely places where you should use locks. You may use 
// additional locks if it is really necessary.
Lock dfs_lock;
lock_t lock_handle;

Lock inode_lock;
lock_t inode_lock_handle;

// STUDENT: put your file system level functions below.
// Some skeletons are provided. You can implement additional functions.

///////////////////////////////////////////////////////////////////
// Non-inode functions first
///////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------
// DfsModuleInit is called at boot time to initialize things and
// open the file system for use.
//-----------------------------------------------------------------

void DfsModuleInit() {
// You essentially set the file system as invalid and then open 
// using DfsOpenFileSystem().
int ret;
 
// Ranga : Setting the filesystem as invalid.
    //Check the return values of reads and writes.
    #if 1
    DfsInvalidate();
    ret = DfsOpenFileSystem();
    if(ret == DFS_FAIL)
    {
        dbprintf('r',"The superblock value is incosistent. \n");
       // GracefulExit ();
    }
    #endif
    
}

//-----------------------------------------------------------------
// DfsInavlidate marks the current version of the filesystem in
// memory as invalid.  This is really only useful when formatting
// the disk, to prevent the current memory version from overwriting
// what you already have on the disk when the OS exits.
//-----------------------------------------------------------------

void DfsInvalidate() {
// This is just a one-line function which sets the valid bit of the 
// superblock to 0.
    //sb.valid = 1;
    sb.valid = 0;   
}

//-------------------------------------------------------------------
// DfsOpenFileSystem loads the file system metadata from the disk
// into memory.  Returns DFS_SUCCESS on success, and DFS_FAIL on 
// failure.
//-------------------------------------------------------------------

int DfsOpenFileSystem() {
    disk_block db;
    dfs_block dfsb;
    int ret;
    int fsfd;
    int i = 0;
    int currentPhysicalBlock = 2;
    int bytesRead = 0;
    int bytesToBeRead = 0;
    int diskblocksize = DiskBytesPerBlock();
    int inodeBlock = 0;
    int pos = 0, j = 0;
    int residue;
//Basic steps:
// Check that filesystem is not already open


//dbprintf('r',"fsfd = %d\n",fsfd);
//exit();

//Init the lock here.
lock_handle = LockInit(&dfs_lock);
inode_lock_handle = LockInit(&inode_lock);
//Init the lock for files here.
file_lock_handle = LockInit(&file_lock);

/*if(sb.valid !=  1)
{
    return DFS_FAIL;
}*/

// Read superblock from disk.  Note this is using the disk read rather 
// than the DFS read function because the DFS read requires a valid 
// filesystem in memory already, and the filesystem cannot be valid 
// until we read the superblock. Also, we don't know the block size 
// until we read the superblock, either.
ret = DiskReadBlock(1,&db);
if(ret < 0)
{
    printf("Looks like the image is not there. Try running fdisk to create it.\n");
    return DFS_FAIL;
}

//exit();
// Copy the data from the block we just read into the superblock in memory
bcopy(db.data,(char*)&sb,sizeof(sb));
PrintSuperBlock(&sb);

if(sb.valid !=  1)
{
    return DFS_FAIL;
}



// All other blocks are sized by virtual block size:
// Read inodes
// Read free block vector
// bzero(db.data,diskblocksize);
ret = DiskReadBlock(currentPhysicalBlock,&db);
for(i=0;i<MAX_INODES;i++)
{   
    //dbprintf('r',"Inode number = %d\n",i);
    pos = bytesRead % diskblocksize;
    bytesToBeRead += 96;
    //dbprintf('r',"Outside the if : Current Block = %d, inode Block = %d\n", currentPhysicalBlock,inodeBlock);
    //dbprintf('r',"Outside the if : pos = %d, bytesToBeREad = %d, BytesRead = %d\n",pos,bytesToBeRead,bytesRead);
    if((bytesToBeRead / diskblocksize) > inodeBlock)
    {

        residue = bytesToBeRead % diskblocksize;
        bcopy(db.data + pos,(char*)&inodes[i],96 - residue);
        currentPhysicalBlock += 1;
        inodeBlock += 1;
        bzero(db.data,diskblocksize);
        ret = DiskReadBlock(currentPhysicalBlock,&db);
        bcopy(db.data,(char*)&inodes[i] + 96 - residue,residue);
        //dbprintf('r',"Inside the if : pos = %d, bytesToBeREad = %d, BytesRead = %d, residue = %d bytesinpreviousblock  = %d \n"\
            ,pos,bytesToBeRead,bytesRead,residue,96-residue);
    }
    else
    {
        bcopy(db.data + pos,(char*)&inodes[i],96);
    }
    //PrintInode(inodes[i]);
    bytesRead += 96;
    //dbprintf('r',"At the End : pos = %d, bytesToBeREad = %d, BytesRead = %d\n",pos,bytesToBeRead,bytesRead);
    //dbprintf('r',"At the end : Current Block = %d, inode Block = %d\n", currentPhysicalBlock,inodeBlock);
    //GracefulExit();
    //dbprintf('r',"Bytes read = %d\n",bytesRead);
    //dbprintf('r',"The indoe.use in inode[%d] = %d\n",i,inodes[i].used);
    //dbprintf('r',"The filename in the inode = ");
    //dbprintf('r',inodes[i].filename);
    
    //dbprintf('r',"\n");
}
dbprintf('r',"Reading FBV now.\n");
//Reading FBV
bzero(db.data,diskblocksize);
//ret = DiskReadBlock(74,&db);
//PrintDiskBlock(&db);

//dbprintf('r',"Number of entries in fbv = %d\n",(sb.num_fs_blocks) / 32);
//dbprintf('r',"NUM_FBV = %d\n",NUM_FBV);
i = 0;
bytesRead = 0;
while(bytesRead < sizeof(fbv))
//for(i = 0; i < NUM_FBV / sb.fs_blocksize; i++)
{

        ret = DiskReadBlock(currentPhysicalBlock,&db);
        currentPhysicalBlock += 1;
        bcopy(db.data,dfsb.data,diskblocksize);
        ret = DiskReadBlock(currentPhysicalBlock,&db);
        currentPhysicalBlock += 1;
        bcopy(db.data,dfsb.data + diskblocksize,diskblocksize);
        bcopy(dfsb.data,(char*)&fbv + (i*sb.fs_blocksize),sb.fs_blocksize);
        //PrintDiskBlock(&dfsb);
        //dbprintf('r',"sizeof fbv = %d\n",sizeof(fbv));
        //dbprintf('r',"i = %d, i * sb.fs_blocksize = %d\n",i,i*sb.fs_blocksize);
        i = i + 1;
        bytesRead += sb.fs_blocksize;
        
}
//dbprintf('r',"Current Physical Block = %d\n",currentPhysicalBlock);
//PrintFBV();
//exit();
// Change superblock to be invalid, write back to disk, then change 
// it back to be valid in memory
sb.valid = 0;
bzero(db.data,diskblocksize);
bcopy((char*)&sb,(char*)db.data,sizeof(sb));
DiskWriteBlock(1,&db);
sb.valid = 1;
dbprintf('r',"DFSOpenFileSystem : Done!\n");
}

void PrintFBV()
{
  int i = 0;
  for(i = 0;i < sizeof(fbv) / 4;i++)
  {
  if(fbv[i] > 0)
  {
    dbprintf('r',"DFSOpenFS() : fbv[%d] = 0x%x\n",i,fbv[i]);
    //Exit();
  }
  else
  {
    dbprintf('r',"DFSOpenFS() : fbv[%d] = 0x%x\n",i,fbv[i]);
  }
  }
  
}

void PrintSuperBlock(dfs_superblock* s)
{
  dbprintf('r',"DfsModuleInit() : Address of superblock = 0x%x\n",s);
  dbprintf('r',"DfsModuleInit() : Address of valid = 0x%x, Valid = %d\n",&s->valid,s->valid);
  dbprintf('r',"DfsModuleInit() : Address of number_inodes = 0x%x, number_inodes = %d\n",&s->number_inodes,s->number_inodes);
  dbprintf('r',"DfsModuleInit() : Address of start_array_inodes = 0x%x, start_array_inodes = %d\n",&s->start_array_inodes,s->start_array_inodes);
  dbprintf('r',"DfsModuleInit() : Address of num_fs_blocks = 0x%x, num_fs_blocks = %d\n",&s->num_fs_blocks,s->num_fs_blocks);
  dbprintf('r',"DfsModuleInit() : Address of start_free_block_vector = 0x%x, start_free_block_vector = %d\n",&s->start_free_block_vector,s->start_free_block_vector);
  dbprintf('r',"DfsModuleInit() : Address of fs_blocksize = 0x%x, fs_blocksize = %d\n",&s->fs_blocksize,s->fs_blocksize);
}

void PrintInode(dfs_inode d_i)
{
  
  dbprintf('r',"\nSize of inode->used  = %d, inode->used = %d\n",sizeof(d_i.used),d_i.used);
  dbprintf('r',"Size of inode->filesize  = %d, inode->filesize  = %d\n",sizeof(d_i.filesize),d_i.filesize);
  dbprintf('r',"Size of inode->filename  = %d, inode->filename  = ",sizeof(d_i.filename));
  dbprintf('r',d_i.filename);
  dbprintf('r',"\n");
  dbprintf('r',"Size of inode->directAddress  = %d, inode->directAddress  = 0x%x\n",sizeof(d_i.directAddress),&d_i.directAddress);
  dbprintf('r',"Size of inode->singleIndirectBlockNum  = %d, inode->singleIndirectBlockNum = %d\n",sizeof(d_i.singleIndirectBlockNum),d_i.singleIndirectBlockNum);
  dbprintf('r',"Size of inode->DoubleIndirectBlockNum  = %d, inode->DoubleIndirectBlockNum  = %d\n",sizeof(d_i.DoubleIndirectBlockNum),d_i.DoubleIndirectBlockNum);
  dbprintf('r',"Total size of inode = %d\n", sizeof(d_i));

}


//-------------------------------------------------------------------
// DfsCloseFileSystem writes the current memory version of the
// filesystem metadata to the disk, and invalidates the memory's 
// version.
//-------------------------------------------------------------------

int DfsCloseFileSystem() 
{
    #if 1
    int ret, ret1, ret2;
    dfs_block d;
    int i;
    int pos = 0, bytesWritten = 0, bytesTobeWritten = 0;
    int inodeBlock = 0;
    int currentBlock = 2;
    int bytesLeft = 0;
    int diskblocksize = DiskBytesPerBlock();
    char buffer[256];
    //testvar
    char test[100] = "My Name is Ranga. This is only some test data being written.\n";
    char test_2[100] = "My Name is Ranga. Writing over existing data.\n";
    int block;
    //Writing only the superblock for now.
    
    if(sb.valid != 1)
    {
        dbprintf('r',"DfsCloseFS() : FileSystem not open. Ending now.\n");
        return DFS_FAIL;
    }
    
    //sb.valid  = 1;
    //PrintSuperBlock(&sb);
    bzero(d.data,DiskBytesPerBlock());
    dbprintf('r',"Closing filesystem.\n");
    bcopy((char*)&sb,d.data,sizeof(sb));
    DiskWriteBlock(1,&d);

    //Writing the inodes.
    for(i = 0; i < MAX_INODES; i++)
    {
    
    pos = bytesWritten % diskblocksize;
    bytesTobeWritten += sizeof(dfs_inode);
    //dbprintf('r',"pos = %d, byteWritten = %d, bytesTobeWritten = %d, inodeBlock = %d, currentBlock = %d\n",pos,bytesWritten,bytesTobeWritten\
                                                                  ,inodeBlock,currentBlock);
    //dbprintf('r',"inodes[%d].used = %d\n",i,inodes[i].used);
    //inodes[i].used = 0xff - i;
    if((bytesTobeWritten / diskblocksize) > inodeBlock)
    {
      bytesLeft = bytesTobeWritten % sizeof(dfs_inode);
      bcopy((char*)&inodes[i],(char*)d.data + pos,sizeof(dfs_inode) - bytesLeft);
      DiskWriteBlock(currentBlock,&d);
      currentBlock += 1;
      inodeBlock += 1;
      pos = 0;
      bzero(d.data,diskblocksize);
      bcopy((char*)&inodes[i],(char*)d.data + pos,bytesLeft);
      //DiskWriteBlock(currentBlock,&d);
    }
    else
    {
      //Printf("pos = %d\n, byteWritten = %d\n, bytesTobeWritten = %d\n",pos,bytesWritten,bytesTobeWritten);
      bcopy((char*)&inodes[i],(char*)d.data + pos,sizeof(dfs_inode)); 
      DiskWriteBlock(currentBlock,&d);
    }
   
    
    //FdiskWriteBlock(2,&d);
   bytesWritten += sizeof(dfs_inode);
  }
    //ret = DiskReadBlock(2,&d);
    //PrintDiskBlock(&d);
    dbprintf('r',"CurrentBlock = %d\n",currentBlock);
     
    bytesWritten = 0;
    i = 0;
  while(bytesWritten < sizeof(fbv))
  { 

    //dbprintf('r',"0x%x\n",fbv);
    //dbprintf('r',"i = %d, i*diskblocksize - %d\n",i,i * diskblocksize);
    bcopy((char*)&fbv[i*32],d.data,diskblocksize);
    DiskWriteBlock(currentBlock,&d);
    //Printf("Value = 0x%x, Address = 0x%x ,i = %d\n",fbv_ptr[i],&fbv_ptr[i],i);
    currentBlock += 1;
    bytesWritten += diskblocksize;
    i = i + 1;
    //dbprintf('r',"bytesWritten = %d\n",bytesWritten);
    
  }
  dbprintf('r',"Current physical block = %d\n",currentBlock);
  //PrintFBV();
dbprintf('r',"FileSystem CLosed.\n");
#endif
//testblock

return DFS_SUCCESS;

}

void PrintDiskBlock(dfs_block *d)
{
    int i = 0;
    for(i = 0; i < sizeof(d->data); i++)
    {
        dbprintf('r',"d->data[%d] = 0x%x\n",i,d->data[i]);
    }
}
//-----------------------------------------------------------------
// DfsAllocateBlock allocates a DFS block for use. Remember to use 
// locks where necessary.
//-----------------------------------------------------------------

uint32 DfsAllocateBlock() {
// Check that file system has been validly loaded into memory
// Find the first free block using the free block vector (FBV), mark it in use
// Return handle to block
    int i = 0;
    int index = 0;
    int pos = 0;
    dfs_block dfsb;
    if(sb.valid != 1)
    {
        dbprintf('r',"FileSystem not loaded into memory. Exiting...\n");
        return DFS_FAIL;
    }
    for(i = 0; i < NUM_FBV; i++)
    {
        dbprintf('r',"DfsAllocateBlock() : fbv[%d] = 0x%x\n",i,fbv[i]);
        if(fbv[i] > 0)
        {
            break;
        }
    }
    if(i == NUM_FBV)
    {
        dbprintf('r',"No more disk space.\n");
        return DFS_FAIL;
    }
    for(index = 0x01,pos=0;pos<32;index = index << 1, pos++)
    {
        if(index & fbv[i])
        {
            lock_handle = LockAcquire(&dfs_lock);
            fbv[i] = fbv[i] << 1;
            lock_handle = LockRelease(&dfs_lock);
            //dbprintf(r',"DfsAllocateBlock() : index = 0x%x, pos = %d, fbv[%d] = 0x%x\n",index,pos,i,fbv[i]);
            break;
        }
    }
    dbprintf('r',"DfsAllocateBlock() : Allocating fs block = %d\n",(i*32) + pos);
    dbprintf('r',"DfsAllocateBlock() : Zeroing out the memory of that block.\n");
    bzero(dfsb.data,DFS_BLOCKSIZE);
    DfsWriteBlock((i*32) + pos,dfsb.data);
    return (i*32) + pos;


}


//-----------------------------------------------------------------
// DfsFreeBlock deallocates a DFS block.
//-----------------------------------------------------------------

int DfsFreeBlock(uint32 blocknum) {
    int fbv_num;
    int fbv_offset;
    int  i = 0;
    dfs_block d;
    dbprintf('r',"DFsFreeBlock() : Block to be freed = %d\n",blocknum);
    if(sb.valid != 1)
    {
        dbprintf('r',"Filesystem not opened.Exiting ..");
        return DFS_FAIL;
    }
    
    fbv_num = blocknum/32;
    fbv_offset = blocknum % 32;
    dbprintf('r',"Befroe Free : fbv[%d] = 0x%x\n",fbv_num,fbv[fbv_num]);
    dbprintf('r',"DfsFreeBlock() : fbv_num = %d, fbv_offset = %d\n",fbv_num,fbv_offset);
    bzero(d.data,DFS_BLOCKSIZE);
    DfsWriteBlock(blocknum,&d);
    lock_handle = LockAcquire(&dfs_lock);
    fbv[fbv_num] = (1 << fbv_offset) | fbv[fbv_num];
    lock_handle = LockRelease(&dfs_lock);
    dbprintf('r',"After Free : fbv[%d] = 0x%x\n",fbv_num,fbv[fbv_num]);
    dbprintf('r',"Block %d freed.\n",blocknum);
    return DFS_SUCCESS;
    
}


//-----------------------------------------------------------------
// DfsReadBlock reads an allocated DFS block from the disk
// (which could span multiple physical disk blocks).  The block
// must be allocated in order to read from it.  Returns DFS_FAIL
// on failure, and the number of bytes read on success.  
//-----------------------------------------------------------------

int DfsReadBlock(uint32 blocknum, dfs_block *b) {
    //Check Reads and Writes.
    int ret;
    int i = 0;
    disk_block db;
    int pos = 0;
    int bytesRead = 0;
    int multiple = DFS_BLOCKSIZE / DISK_BLOCKSIZE;

    if(sb.valid !=  1)  
    {
        printf("Filesystem not open.\n");
        return DFS_FAIL;
    }
    dbprintf('r',"DfsReadBlock() : Have to read fs blocknum = %d\n",blocknum);
    dbprintf('r',"DfsReadBlock() : This translates to physical blocks = %d, %d\n",blocknum * 2, (blocknum * 2) +1);
    for(i = 0; i < multiple; i++)
    {
        pos = i * DISK_BLOCKSIZE;
        dbprintf('r',"DfsReadBlokc() : Reading Physical Block = %d\n",(blocknum * multiple) + i);
        DiskReadBlock((blocknum * multiple) + i,&db);
        bcopy(db.data,b->data + pos,DISK_BLOCKSIZE);
        dbprintf('r',"pos = %d\n",pos);
        bytesRead += DISK_BLOCKSIZE;
    }
    //PrintDiskBlock(b);
    dbprintf('r',"DfsReadBlock() : Bytes Read = %d\n",bytesRead);
    return bytesRead;


}


//-----------------------------------------------------------------
// DfsWriteBlock writes to an allocated DFS block on the disk
// (which could span multiple physical disk blocks).  The block
// must be allocated in order to write to it.  Returns DFS_FAIL
// on failure, and the number of bytes written on success.  
//-----------------------------------------------------------------

int DfsWriteBlock(uint32 blocknum, dfs_block *b){
    int ret;
    disk_block db;
    int i = 0;
    int bytesWritten = 0;
    int multiple = DFS_BLOCKSIZE / DISK_BLOCKSIZE;

    if(sb.valid !=  1)  
    {
        printf("Filesystem not open.\n");
        return DFS_FAIL;
    }
    for(i = 0; i < multiple; i++)
    {
        bzero(db.data,DISK_BLOCKSIZE);
        bcopy(b->data + (i*DISK_BLOCKSIZE),db.data,DISK_BLOCKSIZE);
        dbprintf('r',"DfsWriteBlock() : Writing physical block num = %d\n",(blocknum *multiple) + i);
        DiskWriteBlock((blocknum *multiple) + i,&db);
        bytesWritten += dstrlen(db.data);
        dbprintf('r',"DfsWriteBlock() : bytesWritten = %d\n",bytesWritten);
    }
    return bytesWritten;
}


////////////////////////////////////////////////////////////////////////////////
// Inode-based functions
////////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------
// DfsInodeFilenameExists looks through all the inuse inodes for 
// the given filename. If the filename is found, return the handle 
// of the inode. If it is not found, return DFS_FAIL.
//-----------------------------------------------------------------

uint32 DfsInodeFilenameExists(char *filename) 
{
    int i;
    int ret = -2;
    dbprintf('r',"DfsInodeFilenameExists() : Checking if filename exists.\n");
    if(sb.valid != 1)
    {
        dbprintf('r',"DFSInodeFilenameExists(): Filesystem not open. Exiting...\n");
        exit();
        return DFS_FAIL;
    }
    for(i=0; i < MAX_INODES;i++)
    {
        if(inodes[i].used == 1)
        {
            if(dstrncmp(filename,inodes[i].filename,dstrlen(filename)) == 0)
            {
                dbprintf('r',"DFSIndoeFilenameExists() : filename matches at inode i.\n");
                dbprintf('r',"Filename = ");
                dbprintf('r',filename);
                dbprintf('r',"\nFilename from inode = ");
                dbprintf('r',inodes[i].filename);
                ret = i;
                break;
            }
        }
        
    }
    dbprintf('r',"DfsInodeFilenaameexists() : ret = %d\n",ret);
    return ret;
}


//-----------------------------------------------------------------
// DfsInodeOpen: search the list of all inuse inodes for the 
// specified filename. If the filename exists, return the handle 
// of the inode. If it does not, allocate a new inode for this 
// filename and return its handle. Return DFS_FAIL on failure. 
// Remember to use locks whenever you allocate a new inode.
//-----------------------------------------------------------------

uint32 DfsInodeOpen(char *filename) {
    int i = 0;
    int ret = DfsInodeFilenameExists(filename);
    int inodeHandle = -1;
    dbprintf('r',"Entered DfsInodeOPen()\n");
    dbprintf('r',"To create a file name with name = ");
    dbprintf('r',filename);
    dbprintf('r',"\n");
    if(sb.valid != 1)
    {
        dbprintf('r',"DfsInodeOpen(): Filesystem not open. Exiting...\n");
        exit();
        return DFS_FAIL;
    }
    if(ret == -2)
    {
        dbprintf('r',"DFSInodeOPen() : A new file with ");
        dbprintf('r',filename);
        dbprintf('r'," is being created.");
        for(i = 0; i < MAX_INODES; i++)
        {
            if(inodes[i].used == 0)
            {
                inode_lock_handle = LockAcquire(&inode_lock);
                inodes[i].used = 1;
                inode_lock_handle = LockRelease(&inode_lock);
                dbprintf('r',"strlen of filename = %d\n",dstrlen(filename));
                bcopy(filename,inodes[i].filename,dstrlen(filename));
                dbprintf('r',"Filename = ");
                dbprintf('r',filename);
                dbprintf('r',"\nFilename from inode = ");
                dbprintf('r',inodes[i].filename);
                inodes[i].filesize = 0;
                inodes[i].blocksUsed = 0;
                inodeHandle = i;
                
                PrintInode(inodes[i]);                
                break;
            }
            if(i == MAX_INODES)
            {
                dbprintf('r',"Max number of inodes already used. Cannot allocate anymore.\n");
                exit();
                return DFS_FAIL;
            }      
        }
    }
    else
    {
        inodeHandle = ret;
    }
    dbprintf('r',"DfsInodeOpen() : returned inode handle = %d\n",inodeHandle);
    return inodeHandle;

}


//-----------------------------------------------------------------
// DfsInodeDelete de-allocates any data blocks used by this inode, 
// including the indirect addressing block if necessary, then mark 
// the inode as no longer in use. Use locks when modifying the 
// "inuse" flag in an inode.Return DFS_FAIL on failure, and 
// DFS_SUCCESS on success.
//-----------------------------------------------------------------

int DfsInodeDelete(uint32 handle) {
    int i = 0, j = 0;
    int block_num = 0, block_num_2 = 0;
    dfs_block dfsb, dfsb_2;
    bzero(dfsb.data,DFS_BLOCKSIZE);
    if(sb.valid == 0)
    {
        dbprintf('r',"Valid FS not open. Exiting...\n");
        return DFS_FAIL;
    }
    if(inodes[handle].used == 0)
    {
        dbprintf('r',"The inode is not in use. Failure.\n");
        return DFS_FAIL;
    }
    dbprintf('r',"DfsIndoeDelete() : Received inode handle = %d\n",handle);
    dbprintf('r',"DfsInodeDelete() : Clearing the direct blocks.\n");
    for(i = 0; i < sizeof(inodes[i].directAddress) / sizeof(uint32); i++)
    {
        if(inodes[handle].directAddress[i] != 0)
        {
            //dbprintf('r',"DfsInodeDelete() : Freeing block = ")
            DfsFreeBlock(inodes[handle].directAddress[i]);
        }
    }
    dbprintf('r',"DfsInodeDelete() : Clearing single indirect blocks.\n");
    if(inodes[handle].singleIndirectBlockNum != 0)
    {
        dbprintf('r',"Blocknumber being pointed by singleindirectblock = %d\n",inodes[handle].singleIndirectBlockNum);
        DfsReadBlock(inodes[handle].singleIndirectBlockNum,&dfsb);
        for(i = 0; i < DFS_BLOCKSIZE / 4; i++)
        {
            bcopy(dfsb.data + (i*4),(char*)&block_num,4);
            //PrintDiskBlock(&dfsb);
            if(block_num != 0)
            {
                dbprintf('r',"DfsInodeDelete() : Theblock number beind deleted  = %d\n",block_num);
                //exit();

                DfsFreeBlock(block_num);
                
                //exit();
            }
        }
        //Check ths.
        dbprintf('r',"The singleindirectblock num = %d\n",inodes[handle].singleIndirectBlockNum);
        DfsFreeBlock(inodes[handle].singleIndirectBlockNum);
    }

    dbprintf('r',"DfsInodeDelete() : Clearing second indirect blocks.\n");
    bzero(dfsb.data,DFS_BLOCKSIZE);
    if(inodes[handle].DoubleIndirectBlockNum != NULL)
    {
        dbprintf('r',"DfsInodeDelete() : DoubleIndirectBlockNum = %d\n",inodes[handle].DoubleIndirectBlockNum);
        DfsReadBlock(inodes[handle].DoubleIndirectBlockNum, &dfsb);
        for(i = 0; i < DFS_BLOCKSIZE / 4;i++)
        {
            bcopy(dfsb.data + (i*4),(char*)&block_num,sizeof(int));
            dbprintf('r',"DfsInodeDelete() : block_num = %d\n",block_num);
            if(block_num != 0)
            {
                bzero(dfsb_2.data,DFS_BLOCKSIZE);
                DfsReadBlock(block_num,&dfsb_2);
                for(j = 0; j < DFS_BLOCKSIZE / 4; j++)
                {
                    bcopy(dfsb_2.data + (j*4),(char*)&block_num_2,sizeof(int));
                    dbprintf('r',"DfsInodeDelete() : block_num_2 = %d\n",block_num_2);
                    if(block_num_2 != 0)
                    {
                        //dbprintf('r',"DfsInodeDelete() : block_num_2 = %d\n",block_num_2);
                        DfsFreeBlock(block_num_2);
                    }
                    
                }
                dbprintf('r',"DfsInodeDelete() : deleting block_num = %d\n",block_num);
                //exit();
                DfsFreeBlock(block_num);
            }
            
        }
        DfsFreeBlock(inodes[handle].DoubleIndirectBlockNum);
    }
    inode_lock_handle = LockAcquire(&inode_lock);
    inodes[handle].used = 0;
    inodes[handle].filesize = 0;
    bzero(inodes[handle].filename,sizeof(inodes[handle].filename));
    bzero((char*)inodes[handle].directAddress,sizeof(inodes[handle].directAddress));
    inodes[handle].singleIndirectBlockNum = 0;
    inodes[handle].DoubleIndirectBlockNum = 0;
    inodes[handle].blocksUsed = 0;
    inode_lock_handle = LockRelease(&inode_lock);
    return DFS_SUCCESS;

}



//-----------------------------------------------------------------
// DfsInodeReadBytes reads num_bytes from the file represented by 
// the inode handle, starting at virtual byte start_byte, copying 
// the data to the address pointed to by mem. Return DFS_FAIL on 
// failure, and the number of bytes read on success.
//-----------------------------------------------------------------

int DfsInodeReadBytes(uint32 handle, void *mem, int start_byte, int num_bytes) 
{
    int i = 0,j = 0;
    int bytesRead = 0;
    int pos = 0;
    int ret;
    dfs_block dfsb;
    dfs_block dfs_l1, dfs_l2;
    int block_num;
    int offset;
    int bytesToBeRead;
    int SecondIndirectBlockNum;
    int maxdirectAddressBytes = (sizeof(inodes[handle].directAddress) / sizeof(uint32)) * DFS_BLOCKSIZE;
    uint32 maxSingleDirectAddressBytes = maxdirectAddressBytes + (DFS_BLOCKSIZE / sizeof(int) * DFS_BLOCKSIZE);
    uint32 maxSecondaryDirectAddressBytes = maxSingleDirectAddressBytes + (DFS_BLOCKSIZE/sizeof(int) * DFS_BLOCKSIZE/sizeof(int) * DFS_BLOCKSIZE);
    dbprintf('r',"Handle = %d\n",handle);
    dbprintf('r',"maxdirectAddressBytes = %d\nmaxSingleDirectAddressBytes = %d\naxSecondaryDirectAddressBytes = %d\n",\
            maxdirectAddressBytes,maxSingleDirectAddressBytes,maxSecondaryDirectAddressBytes);
    
    //Just for test purposes. 
    //inodes[handle].filesize = DFS_BLOCKSIZE;


    if(start_byte + num_bytes > inodes[handle].filesize)
    {
        dbprintf('r',"Trying to read more than the filesize.\n");
        num_bytes = DfsInodeFilesize(handle) - start_byte;
        //return DFS_FAIL;
    }
    if(start_byte + num_bytes <= maxdirectAddressBytes)
    {
        dbprintf('r',"DfsInodeReadBytes() : num_bytes = %d\n",num_bytes);
        while(bytesRead < num_bytes)
        {
            bzero(dfsb.data,DFS_BLOCKSIZE);
            bytesToBeRead = num_bytes - bytesRead;
            block_num = (start_byte + bytesRead) / DFS_BLOCKSIZE; //This identifies the ith element of directaddress
            offset = (start_byte + bytesRead) % DFS_BLOCKSIZE;
            /*if((offset == 0) && (inodes[handle].blocksUsed != 0))
            {
                block_num += 1;
            }*/
            dbprintf('r',"DfsInodeREadBytes() : bytestoberead = %d, bytesRead = %d, offset = %d, block_num = %d\n",bytesToBeRead,\
                                                bytesRead, offset,block_num);
            
            if(bytesToBeRead < DFS_BLOCKSIZE)
            {
                DfsReadBlock(inodes[handle].directAddress[block_num],&dfsb);
                bcopy(dfsb.data + offset,(char*)mem + bytesRead,bytesToBeRead);
                bytesRead += bytesToBeRead;
                dbprintf('r',"DfsInodeREadBytes() : DataRead = \n");
                dbprintf('r',dfsb.data);
            }
            else
            {
                DfsReadBlock(inodes[handle].directAddress[block_num],&dfsb);
                bcopy(dfsb.data + offset,(char*)mem + bytesRead,DFS_BLOCKSIZE-offset);
                bytesRead += DFS_BLOCKSIZE-offset;
            }
            dbprintf('r',"DfsReadBlockData() : bytesRread = %d\n",bytesRead);
            
        }
    }
    else if(start_byte + num_bytes <= maxSingleDirectAddressBytes)
    {
        dbprintf('r',"Entered the singleindirect if loop.\n");
        dbprintf('r',"DfsIndoeReadBytes() : num_bytes = %d\n",num_bytes);
        pos = start_byte;
        while(pos < maxdirectAddressBytes)
        {
            bzero(dfsb.data,DFS_BLOCKSIZE);
            bytesToBeRead = num_bytes - bytesRead;
            block_num = (start_byte + bytesRead) / DFS_BLOCKSIZE; //This identifies the ith element of directaddress
            offset = (start_byte + bytesRead) % DFS_BLOCKSIZE;
            dbprintf('r',"DfsInodeREadBytes() : bytestoberead = %d, bytesRead = %d, offset = %d\n",bytesToBeRead,\
                                                bytesRead, offset);
            if(bytesToBeRead < DFS_BLOCKSIZE)
            {
                DfsReadBlock(inodes[handle].directAddress[block_num],&dfsb);
                bcopy(dfsb.data + offset,(char*)mem + bytesRead,bytesToBeRead);
                bytesRead += bytesToBeRead;
                pos += bytesToBeRead;
                dbprintf('r',"DfsInodeREadBytes() : DataRead = \n");
                dbprintf('r',dfsb.data);
            }
            else
            {
                DfsReadBlock(inodes[handle].directAddress[block_num],&dfsb);
                bcopy(dfsb.data + offset,(char*)mem + bytesRead,DFS_BLOCKSIZE-offset);
                bytesRead += DFS_BLOCKSIZE-offset;
                pos += DFS_BLOCKSIZE-offset;
            }
            dbprintf('r',"DfsInodeReadBytes() : pos = %d\n",pos);
        }
        ret = DfsReadBlock(inodes[handle].singleIndirectBlockNum,&dfs_l1);
        dbprintf('r',"DfsIndoeREadBytes() : inodes[handle].singleIndirectBlock = %d\n",inodes[handle].singleIndirectBlockNum);
        dbprintf('r',"DfsInodeREadBytes() : Entered the sngle indirect case.\n");
        dbprintf('r',"DfsInodeReadButes() : dfs_l1 -> [3] = %d [7] = %d [11] =%d [15] = %d [19] = %d [23] = %d [27] = %d [31] = %d\n",dfs_l1.data[3],dfs_l1.data[7],dfs_l1.data[11],dfs_l1.data[15],\
                        dfs_l1.data[19],dfs_l1.data[23],dfs_l1.data[27],dfs_l1.data[31]);

        
        for(i=0; i < DFS_BLOCKSIZE / sizeof(int); i++)
        {
            bytesToBeRead = num_bytes - bytesRead;
            bzero(dfsb.data,DFS_BLOCKSIZE);
            
            //block_num = (start_byte + bytesRead) / DFS_BLOCKSIZE; //This identifies the ith element of directaddress
            offset = (start_byte + bytesRead) % DFS_BLOCKSIZE;
            dbprintf('r',"DfsInodeREadBytes() : bytestoberead = %d, bytesRead = %d, offset = %d\n",bytesToBeRead,\
                                                bytesRead, offset);
            if(bytesRead >= num_bytes)
            {
                dbprintf('r',"Read = %d bytes. Ending now.\n",bytesRead);
                break;
            }
            else
            {
                bcopy(dfs_l1.data + (i*4),(char*)&block_num,sizeof(int));
                DfsReadBlock(block_num,&dfsb);
                if(block_num == 0)
                {
                    dbprintf('r',"No more data to be read in the file. block num = %d.\n",block_num);
                    PrintDiskBlock(&dfsb);
                    return DFS_FAIL;
                }
                if(bytesToBeRead < DFS_BLOCKSIZE)
                {
                    
                    bcopy(dfsb.data + offset,(char*)mem + bytesRead,bytesToBeRead);
                    
                    dbprintf('r',"DfsIndoeTeadBytes() : FS block num being read = %d\n",block_num);
                    //dbprintf('r',"DfsInodeREadBytes() : DataRead = \n");
                    //dbprintf('r',dfsb.data);
                    dbprintf('r',"mem + bytes REad = \n");
                    dbprintf('r',mem+bytesRead);
                    dbprintf('r',"\n");
                    bytesRead += bytesToBeRead;
                    pos += bytesToBeRead;
                }
                else
                {
                    bcopy(dfsb.data + offset,(char*)mem + bytesRead,DFS_BLOCKSIZE-offset);
                    dbprintf('r',"DfsIndoeTeadBytes() : FS block num being read = %d\n",block_num);
                    //dbprintf('r',"DfsInodeREadBytes() : DataRead = \n");
                    //dbprintf('r',dfsb.data);
                    dbprintf('r',"mem + bytes REad = \n");
                    dbprintf('r',mem+bytesRead);
                    dbprintf('r',"\n");
                    bytesRead += DFS_BLOCKSIZE-offset;
                    pos += DFS_BLOCKSIZE-offset;
                }
                dbprintf('r',"At the end of DfsInodeREadBytes() : bytestoberead = %d, bytesRead = %d, offset = %d\n",bytesToBeRead,\
                                                bytesRead, offset);
                dbprintf('r',"mem = ");
                dbprintf('r',mem);
                dbprintf('r',"\nstrlen(mem) = %d\n",dstrlen(mem));
            }
        }    
    }
    else
    {
        dbprintf('r',"Entered the singleindirect if loop.\n");
        dbprintf('r',"DfsIndoeReadBytes() : num_bytes = %d\n",num_bytes);
        pos = start_byte;
        while(pos < maxdirectAddressBytes)
        {
            bzero(dfsb.data,DFS_BLOCKSIZE);
            bytesToBeRead = num_bytes - bytesRead;
            block_num = (start_byte + bytesRead) / DFS_BLOCKSIZE; //This identifies the ith element of directaddress
            offset = (start_byte + bytesRead) % DFS_BLOCKSIZE;
            dbprintf('r',"DfsInodeREadBytes() : bytestoberead = %d, bytesRead = %d, offset = %d\n",bytesToBeRead,\
                                                bytesRead, offset);
            if(bytesToBeRead < DFS_BLOCKSIZE)
            {
                DfsReadBlock(inodes[handle].directAddress[block_num],&dfsb);
                bcopy(dfsb.data + offset,(char*)mem + bytesRead,bytesToBeRead);
                bytesRead += bytesToBeRead;
                pos += bytesToBeRead;
                dbprintf('r',"DfsInodeREadBytes() : DataRead = \n");
                dbprintf('r',dfsb.data);
            }
            else
            {
                DfsReadBlock(inodes[handle].directAddress[block_num],&dfsb);
                bcopy(dfsb.data + offset,(char*)mem + bytesRead,DFS_BLOCKSIZE-offset);
                bytesRead += DFS_BLOCKSIZE-offset;
                pos += DFS_BLOCKSIZE-offset;
            }
            dbprintf('r',"DfsInodeReadBytes() : pos = %d\n",pos);
        }
        ret = DfsReadBlock(inodes[handle].singleIndirectBlockNum,&dfs_l1);
        dbprintf('r',"DfsIndoeREadBytes() : inodes[handle].singleIndirectBlock = %d\n",inodes[handle].singleIndirectBlockNum);
        dbprintf('r',"DfsInodeREadBytes() : Entered the sngle indirect case.\n");
        dbprintf('r',"DfsInodeReadButes() : dfs_l1 -> [3] = %d [7] = %d [11] =%d [15] = %d [19] = %d [23] = %d [27] = %d [31] = %d\n",dfs_l1.data[3],dfs_l1.data[7],dfs_l1.data[11],dfs_l1.data[15],\
                        dfs_l1.data[19],dfs_l1.data[23],dfs_l1.data[27],dfs_l1.data[31]);

        while(pos < maxSingleDirectAddressBytes)
        {
        for(i=0; i < DFS_BLOCKSIZE / sizeof(int); i++)
        {
            bytesToBeRead = num_bytes - bytesRead;
            bzero(dfsb.data,DFS_BLOCKSIZE);
            
            //block_num = (start_byte + bytesRead) / DFS_BLOCKSIZE; //This identifies the ith element of directaddress
            offset = (start_byte + bytesRead) % DFS_BLOCKSIZE;
            dbprintf('r',"DfsInodeREadBytes() : bytestoberead = %d, bytesRead = %d, offset = %d\n",bytesToBeRead,\
                                                bytesRead, offset);
            if(bytesRead >= num_bytes)
            {
                dbprintf('r',"Read = %d bytes. Ending now.\n",bytesRead);
                break;
            }
            else
            {
                bcopy(dfs_l1.data + (i*4),(char*)&block_num,sizeof(int));
                DfsReadBlock(block_num,&dfsb);
                if(block_num == 0)
                {
                    dbprintf('r',"No more data to be read in the file. block num = %d.\n",block_num);
                    PrintDiskBlock(&dfsb);
                    return DFS_FAIL;
                }
                if(bytesToBeRead < DFS_BLOCKSIZE)
                {
                    
                    bcopy(dfsb.data + offset,(char*)mem + bytesRead,bytesToBeRead);
                    
                    dbprintf('r',"DfsIndoeTeadBytes() : FS block num being read = %d\n",block_num);
                    //dbprintf('r',"DfsInodeREadBytes() : DataRead = \n");
                    //dbprintf('r',dfsb.data);
                    dbprintf('r',"mem + bytes REad = \n");
                    dbprintf('r',mem+bytesRead);
                    dbprintf('r',"\n");
                    bytesRead += bytesToBeRead;
                    pos += bytesToBeRead;
                }
                else
                {
                    bcopy(dfsb.data + offset,(char*)mem + bytesRead,DFS_BLOCKSIZE-offset);
                    dbprintf('r',"DfsIndoeTeadBytes() : FS block num being read = %d\n",block_num);
                    //dbprintf('r',"DfsInodeREadBytes() : DataRead = \n");
                    //dbprintf('r',dfsb.data);
                    dbprintf('r',"mem + bytes REad = \n");
                    dbprintf('r',mem+bytesRead);
                    dbprintf('r',"\n");
                    bytesRead += DFS_BLOCKSIZE-offset;
                    pos += DFS_BLOCKSIZE-offset;
                }
                dbprintf('r',"At the end of DfsInodeREadBytes() : bytestoberead = %d, bytesRead = %d, offset = %d\n",bytesToBeRead,\
                                                bytesRead, offset);
                dbprintf('r',"mem = ");
                dbprintf('r',mem);
                dbprintf('r',"\nstrlen(mem) = %d\n",dstrlen(mem));
            }
        }
        }
        dbprintf('r',"Strarting the second direct addressing now.\n");
        //dbprintf('r',"bytesWritten = %d, pos = %d\n",bytesWritten,pos);

        block_num = (start_byte + bytesRead) / DFS_BLOCKSIZE; //This identifies the ith element of directaddress
        offset = (start_byte + bytesRead) % DFS_BLOCKSIZE;
        dbprintf('r',"DfsInodeWriteBytes() : Creating the double ndirect table.\n block_num = %d\n",block_num);
        dbprintf('r',"DfsInodeWriteBytes() : inodes[handle].DoubleIndirectBlockNum = %d\n",inodes[handle].DoubleIndirectBlockNum );
        
        ret = DfsReadBlock(inodes[handle].DoubleIndirectBlockNum, &dfs_l2);
        bytesToBeRead = num_bytes - bytesRead;

        //L2 block.

        for(i = 0; i < DFS_BLOCKSIZE / sizeof(int); i++)
        {
            if(bytesRead >= num_bytes)
                {
                    dbprintf('r',"DfsInodeWriteBytes() : Written bytes = %d\n Ending write.\n",bytesRead);
                    break;
                }  
                bzero(dfsb.data,DFS_BLOCKSIZE);
                block_num = ((start_byte + bytesRead) / DFS_BLOCKSIZE) + 1; //because already one block number is allocated to store indirect address.
                offset = (start_byte + bytesRead) % DFS_BLOCKSIZE;
                bcopy((char*)&SecondIndirectBlockNum,dfs_l2.data + (i*4),sizeof(int));
                //dbprintf("DfsInodeWriteBytes() : Read indirect blocknum = %d\n",IndirectBlockNum);
                if(SecondIndirectBlockNum != 0)
                    {
                        continue;
                    }
                else
                {

                    bzero(dfs_l1.data,DFS_BLOCKSIZE);
                    for(j = 0; j < DFS_BLOCKSIZE / sizeof(int); j++)
                    {
                        bytesToBeRead = num_bytes - bytesRead;
                        bzero(dfsb.data,DFS_BLOCKSIZE);
                
                        //block_num = (start_byte + bytesRead) / DFS_BLOCKSIZE; //This identifies the ith element of directaddress
                        offset = (start_byte + bytesRead) % DFS_BLOCKSIZE;
                        dbprintf('r',"DfsInodeREadBytes() : bytestoberead = %d, bytesRead = %d, offset = %d\n",bytesToBeRead,\
                                                    bytesRead, offset);
                        if(bytesRead >= num_bytes)
                        {
                            dbprintf('r',"Read = %d bytes. Ending now.\n",bytesRead);
                            break;
                        }
                        else
                        {
                    bcopy((char*)&block_num,dfs_l1.data + (j*4),sizeof(int));
                    DfsReadBlock(block_num,&dfsb);
                    if(block_num == 0)
                    {
                        dbprintf('r',"No more data to be read in the file. block num = %d.\n",block_num);
                        PrintDiskBlock(&dfsb);
                        return DFS_FAIL;
                    }
                    if(bytesToBeRead < DFS_BLOCKSIZE)
                    {
                        
                        bcopy(dfsb.data + offset,(char*)mem + bytesRead,bytesToBeRead);
                        
                        dbprintf('r',"DfsIndoeTeadBytes() : FS block num being read = %d\n",block_num);
                        //dbprintf('r',"DfsInodeREadBytes() : DataRead = \n");
                        //dbprintf('r',dfsb.data);
                        dbprintf('r',"mem + bytes REad = \n");
                        dbprintf('r',mem+bytesRead);
                        dbprintf('r',"\n");
                        bytesRead += bytesToBeRead;
                        pos += bytesToBeRead;
                    }
                    else
                    {
                        bcopy(dfsb.data + offset,(char*)mem + bytesRead,DFS_BLOCKSIZE-offset);
                        dbprintf('r',"DfsIndoeTeadBytes() : FS block num being read = %d\n",block_num);
                        //dbprintf('r',"DfsInodeREadBytes() : DataRead = \n");
                        //dbprintf('r',dfsb.data);
                        dbprintf('r',"mem + bytes REad = \n");
                        dbprintf('r',mem+bytesRead);
                        dbprintf('r',"\n");
                        bytesRead += DFS_BLOCKSIZE-offset;
                        pos += DFS_BLOCKSIZE-offset;
                    }
                    dbprintf('r',"At the end of DfsInodeREadBytes() : bytestoberead = %d, bytesRead = %d, offset = %d\n",bytesToBeRead,\
                                                    bytesRead, offset);
                    dbprintf('r',"mem = ");
                    dbprintf('r',mem);
                    dbprintf('r',"\nstrlen(mem) = %d\n",dstrlen(mem));
                }
                    
                }

            
            
        }


        dbprintf('r',"To habdle the second indirect case\n");
        }
    }
    
    return bytesRead;

}



//-----------------------------------------------------------------
// DfsInodeWriteBytes writes num_bytes from the memory pointed to 
// by mem to the file represented by the inode handle, starting at 
// virtual byte start_byte. Note that if you are only writing part 
// of a given file system block, you'll need to read that block 
// from the disk first. Return DFS_FAIL on failure and the number 
// of bytes written on success.
//-----------------------------------------------------------------

int DfsInodeWriteBytes(uint32 handle, void *mem, int start_byte, int num_bytes) 
{
    int i = 0, j = 0;
    int bytesWritten = 0;
    int bytesToBeWritten = num_bytes;
    int maxdirectAddressBytes = (sizeof(inodes[handle].directAddress) / sizeof(uint32)) * DFS_BLOCKSIZE;
    int maxSingleDirectAddressBytes = maxdirectAddressBytes +(DFS_BLOCKSIZE/sizeof(int) * DFS_BLOCKSIZE);
    uint32 maxSecondaryDirectAddressBytes = maxSingleDirectAddressBytes + ((DFS_BLOCKSIZE / sizeof(int)) * DFS_BLOCKSIZE/sizeof(int) * DFS_BLOCKSIZE);
    int offset = 0;
    int block_num = 0;
    dfs_block dfsb;
    int pos = start_byte;
    int increaseCount = 0;
    int ret;
    dfs_block dfs_l1,dfs_l2;
    int IndirectBlockNum = 0;
    int SecondIndirectBlockNum = 0;
    int l1VirtualBlock;
    int l2VirtualBlock;
    if(inodes[i].filesize == 0)
    {
        increaseCount = 1;
    }
    else if((start_byte + num_bytes > inodes[handle].filesize) && ((inodes[handle].filesize + num_bytes - start_byte) > DFS_BLOCKSIZE))
    {
        increaseCount = 1;
    }
    else
    {
        increaseCount = 0;
    }
    dbprintf('r',"DfsWriteBlock() : num_bytes = %d\n", num_bytes);
    

    if(start_byte + num_bytes <= maxdirectAddressBytes)
    {
        while(bytesWritten < num_bytes)
        {
            bzero(dfsb.data,DFS_BLOCKSIZE);
            bytesToBeWritten = num_bytes - bytesWritten;
            
            block_num = (start_byte + bytesWritten) / DFS_BLOCKSIZE; //This identifies the ith element of directaddress
            
            offset = (start_byte + bytesWritten) % DFS_BLOCKSIZE;
            
            if(inodes[handle].directAddress[block_num] == 0)
            {
                inodes[handle].directAddress[block_num] = DfsInodeAllocateVirtualBlock(handle, block_num);
                dbprintf('r',"DfsInodeWriteBytes() : physical block retunred = %d\n",inodes[handle].directAddress[block_num]);
                //inodes[handle].blocksUsed += 1;
                inodes[handle].lastWrittenBlockNumber = inodes[handle].directAddress[block_num];
            }
            dbprintf('r',"DfsInodeWriteBytes() : bytestobewritten = %d, bytesWritten = %d, offset = %d, block_num = %d\n",bytesToBeWritten,\
                                                bytesWritten, offset, block_num);
            if(bytesToBeWritten < DFS_BLOCKSIZE)
            {
                dbprintf('r',"DfsInodeWriteBytes() : Reading blck from directAddress.\n");
                DfsReadBlock(inodes[handle].directAddress[block_num],&dfsb);
                bcopy((char*)mem + bytesWritten,(char*)dfsb.data + offset,bytesToBeWritten);
                DfsWriteBlock(inodes[handle].directAddress[block_num],&dfsb);
                bytesWritten += bytesToBeWritten;
                /*if(increaseCount)
                {
                    inodes[handle].blocksUsed += 1;
                    inodes[handle].lastWrittenBlockNumber = inodes[handle].directAddress[block_num];
                }*/
                dbprintf('r',"DfsInodeWriteBytes() : DataWritten = ");
                dbprintf('r',dfsb.data);
                dbprintf('r',"\n");
                //exit();
            }
            else
            {
                dbprintf('r',"DfsInodeWriteBytes() : Reading blck from directAddress.\n");
                DfsReadBlock(inodes[handle].directAddress[block_num],&dfsb);
                bcopy((char*)mem + bytesWritten,(char*)dfsb.data + offset,DFS_BLOCKSIZE-offset);
                DfsWriteBlock(inodes[handle].directAddress[block_num],&dfsb);
                bytesWritten += DFS_BLOCKSIZE-offset;
                /*if(increaseCount)
                {
                    inodes[handle].blocksUsed += 1;
                    inodes[handle].lastWrittenBlockNumber = inodes[handle].directAddress[block_num];
                }*/
                dbprintf('r',"DfsInodeWriteBytes() : DataWritten = ");
                dbprintf('r',dfsb.data);
                dbprintf('r',"\n");
            }
            dbprintf('r',"DfsInodeWriteBytes() : bytesWritten = %d and num_bytes = %d\n",bytesWritten,num_bytes);
            dbprintf('r',"DfsInodeWriteBytes() : bytestobewritten = %d, bytesWritten = %d, offset = %d, block_num = %d\n",bytesToBeWritten,\
                                                bytesWritten, offset, block_num);
            dbprintf('r',"(if(bytesWritten < num_bytes) = %d\n",(bytesWritten < num_bytes));
            //exit();
            if(bytesWritten >= num_bytes)
            {
                break;
            }
        }
    }

    else if(start_byte + num_bytes <=  maxSingleDirectAddressBytes)
    {
        dbprintf('r',"DfsInodeWriteBytes() : ENtered the loop where maxSingleDirectADdress is there.\n");
        
        pos = start_byte;
        while(pos < maxdirectAddressBytes)
        {
            bzero(dfsb.data,DFS_BLOCKSIZE);
            bytesToBeWritten = num_bytes - bytesWritten;
            block_num = (start_byte + bytesWritten) / DFS_BLOCKSIZE; //This identifies the ith element of directaddress
            
            offset = (start_byte + bytesWritten) % DFS_BLOCKSIZE;
            
            if(inodes[handle].directAddress[block_num] == 0)
            {
                inodes[handle].directAddress[block_num] = DfsInodeAllocateVirtualBlock(handle, block_num);
                //inodes[handle].blocksUsed += 1;
                inodes[handle].lastWrittenBlockNumber = inodes[handle].directAddress[block_num];
            }
            dbprintf('r',"DfsInodeWriteBytes() : bytestobewritten = %d, bytesWritten = %d, offset = %d, block_num = %d\n",bytesToBeWritten,\
                                                bytesWritten, offset, block_num);
            if(bytesToBeWritten < DFS_BLOCKSIZE)
            {
                dbprintf('r',"DfsInodeWriteBytes() : Reading blck from directAddress in single indirect.\n");
                DfsReadBlock(inodes[handle].directAddress[block_num],&dfsb);
                bcopy((char*)mem + bytesWritten,(char*)dfsb.data + offset,bytesToBeWritten);
                DfsWriteBlock(inodes[handle].directAddress[block_num],&dfsb);
                bytesWritten += bytesToBeWritten;
                /*if(increaseCount)
                {
                    inodes[handle].blocksUsed += 1;
                    inodes[handle].lastWrittenBlockNumber = inodes[handle].directAddress[block_num];
                }*/
                dbprintf('r',"DfsInodeWriteBytes() : DataWritten = ");
                dbprintf('r',dfsb.data);
                dbprintf('r',"\n");
                pos += bytesToBeWritten;
            }
            else
            {
                dbprintf('r',"DfsInodeWriteBytes() : Reading blck from directAddress in single indirect.\n");
                DfsReadBlock(inodes[handle].directAddress[block_num],&dfsb);
                bcopy((char*)mem + bytesWritten,(char*)dfsb.data + offset,DFS_BLOCKSIZE-offset);
                DfsWriteBlock(inodes[handle].directAddress[block_num],&dfsb);
                bytesWritten += DFS_BLOCKSIZE-offset;
                /*if(increaseCount)
                {
                    inodes[handle].blocksUsed += 1;
                    inodes[handle].lastWrittenBlockNumber = inodes[handle].directAddress[block_num];
                }*/
                dbprintf('r',"DfsInodeWriteBytes() : DataWritten = ");
                dbprintf('r',dfsb.data);
                dbprintf('r',"\n");
                pos += DFS_BLOCKSIZE-offset;
            }
            dbprintf('r',"DfsInodeWriteBytes() : bytesWritten = %d\n",bytesWritten);
            
        }
        //REading the [hysical block containg the sigle indirect address]
        block_num = (start_byte + bytesWritten) / DFS_BLOCKSIZE; //This identifies the ith element of directaddress
        offset = (start_byte + bytesWritten) % DFS_BLOCKSIZE;
        dbprintf('r',"DfsInodeWriteBytes() : Creating the single ndirect table.\n block_num = %d\n",block_num);
        dbprintf('r',"DfsInodeWriteBytes() : inodes[handle].singleIndirectBlockNum = %d\n",inodes[handle].singleIndirectBlockNum );
        
        if(inodes[handle].singleIndirectBlockNum == 0)
        {
            dbprintf('r',"DfsInodeWriteBytes() : Entered the if\n");
            inodes[handle].singleIndirectBlockNum = DfsInodeAllocateVirtualBlock(handle,block_num);
            dbprintf('r',"DfsInodeWriteBytes() : Retunred block number = %d\n",inodes[handle].singleIndirectBlockNum );
            dbprintf('r',"DfsInodeWriteBytes() : inodes[handle].singleIndirectBlockNum = %d\n",inodes[handle].singleIndirectBlockNum );

        }
        dbprintf('r',"DfsInodeWriteBytes() : Reading single indirect blck  in single indirect.\n");
        ret = DfsReadBlock(inodes[handle].singleIndirectBlockNum, &dfs_l1);
        //dbprintf('r',"")
        //exit(); 
        bytesToBeWritten = num_bytes - bytesWritten;
                          
        for(i = 0; i < DFS_BLOCKSIZE / sizeof(int); i++)
        {
            if(bytesWritten >= num_bytes)
            {
                dbprintf('r',"DfsInodeWriteBytes() : Written bytes = %d\n Ending write.\n",bytesWritten);
                break;
            }  
            bzero(dfsb.data,DFS_BLOCKSIZE);
            block_num = ((start_byte + bytesWritten) / DFS_BLOCKSIZE) + 1; //because already one block number is allocated to store indirect address.
            offset = (start_byte + bytesWritten) % DFS_BLOCKSIZE;
            bcopy(dfs_l1.data + (i*4),(char*)&IndirectBlockNum,sizeof(int));
            dbprintf('r',"DfsInodeWriteBytes() : Read indirect blocknum = %d\n",IndirectBlockNum);
         //exit();  
            if(IndirectBlockNum != 0)
            {
                continue;
            }
            else
            {
                l1VirtualBlock = (int)DfsInodeAllocateVirtualBlock( handle,block_num);
                bcopy((char*)&l1VirtualBlock,dfs_l1.data + (i*4),sizeof(int));
               // *(dfs_l1.data + (i*4)) = (int)DfsInodeAllocateVirtualBlock(handle,block_num);
                dbprintf('r',"DfsInodeWriteBytes() : bytestobewritten = %d, bytesWritten = %d, offset = %d, block_num = %d\n",bytesToBeWritten,\
                                                bytesWritten, offset, block_num);
                dbprintf('r',"DfsInodeWriteBytes() : BLock Allocated : %d\n",*(dfs_l1.data + (i*4)));
                DfsWriteBlock(inodes[handle].singleIndirectBlockNum,&dfs_l1);
               //PrintDiskBlock(&dfs_l1); 
                //exit();
                if(bytesToBeWritten < DFS_BLOCKSIZE)
                {
                    dbprintf('r',"DfsInodeWriteBytes() : Reading virtua blck from singleindirectAddress in single indirect.\n");
                 DfsReadBlock(l1VirtualBlock,&dfsb);
                bcopy((char*)mem + bytesWritten,(char*)dfsb.data + offset,bytesToBeWritten);
                DfsWriteBlock(l1VirtualBlock,&dfsb);
                bytesWritten += bytesToBeWritten;
                /*if(increaseCount)
                {
                    inodes[handle].blocksUsed += 1;
                    inodes[handle].lastWrittenBlockNumber = inodes[handle].directAddress[block_num];
                }*/
                //dbprintf('r',"DfsInodeWriteBytes() : DataWritten = ");
                dbprintf('r',dfsb.data);
                dbprintf('r',"\n");
                }
                else
                {
                    dbprintf('r',"DfsInodeWriteBytes() : Reading virtua blck from singleindirectAddress in single indirect.\n");
                DfsReadBlock(l1VirtualBlock,&dfsb);
                bcopy((char*)mem + bytesWritten,(char*)dfsb.data + offset,DFS_BLOCKSIZE-offset);
                DfsWriteBlock(l1VirtualBlock,&dfsb);
                bytesWritten += DFS_BLOCKSIZE-offset;
                /*if(increaseCount)
                {
                    inodes[handle].blocksUsed += 1;
                    inodes[handle].lastWrittenBlockNumber = inodes[handle].directAddress[block_num];
                }*/
                //dbprintf('r',"DfsInodeWriteBytes() : DataWritten = ");
                dbprintf('r',dfsb.data);
                dbprintf('r',"\n");
                }
                dbprintf('r',"DfsInodeWriteBytes() : bytesWritten = %d\n",bytesWritten);
                dbprintf('r',"dfs_l1 -> [3] = %d [7] = %d [11] =%d [15] = %d [19] = %d [23] = %d [27] = %d [31] = %d\n",dfs_l1.data[3],dfs_l1.data[7],dfs_l1.data[11],dfs_l1.data[15],\
                        dfs_l1.data[19],dfs_l1.data[23],dfs_l1.data[27],dfs_l1.data[31]);
            }
        }

    }      
    else
    {
        dbprintf('r',"DfsInodeWriteBytes() : ENtered the loop where maxDoubleInidrectAddressing is there.\n");
        dbprintf('r',"DfsInodeWriteBytes() : Start Byte = %d , num_bytes = %d\n",start_byte,num_bytes);
        pos = start_byte;
        while(pos < maxdirectAddressBytes)
        {
            bzero(dfsb.data,DFS_BLOCKSIZE);
            bytesToBeWritten = num_bytes - bytesWritten;
            block_num = (start_byte + bytesWritten) / DFS_BLOCKSIZE; //This identifies the ith element of directaddress
            
            offset = (start_byte + bytesWritten) % DFS_BLOCKSIZE;
            
            if(inodes[handle].directAddress[block_num] == 0)
            {
                inodes[handle].directAddress[block_num] = DfsInodeAllocateVirtualBlock(handle, block_num);
                //inodes[handle].blocksUsed += 1;
                inodes[handle].lastWrittenBlockNumber = inodes[handle].directAddress[block_num];
            }
            dbprintf('r',"DfsInodeWriteBytes() : bytestobewritten = %d, bytesWritten = %d, offset = %d, block_num = %d\n",bytesToBeWritten,\
                                                bytesWritten, offset, block_num);
            if(bytesToBeWritten < DFS_BLOCKSIZE)
            {
                dbprintf('r',"DfsInodeWriteBytes() : Reading ndirectAddress in double indirect.\n");
                DfsReadBlock(inodes[handle].directAddress[block_num],&dfsb);
                bcopy((char*)mem + bytesWritten,(char*)dfsb.data + offset,bytesToBeWritten);
                DfsWriteBlock(inodes[handle].directAddress[block_num],&dfsb);
                bytesWritten += bytesToBeWritten;
                /*if(increaseCount)
                {
                    inodes[handle].blocksUsed += 1;
                    inodes[handle].lastWrittenBlockNumber = inodes[handle].directAddress[block_num];
                }*/
                dbprintf('r',"DfsInodeWriteBytes() : DataWritten = ");
                dbprintf('r',dfsb.data);
                dbprintf('r',"\n");
                pos += bytesToBeWritten;
            }
            else
            {
                dbprintf('r',"DfsInodeWriteBytes() : Reading ndirectAddress in double indirect.\n");
                DfsReadBlock(inodes[handle].directAddress[block_num],&dfsb);
                bcopy((char*)mem + bytesWritten,(char*)dfsb.data + offset,DFS_BLOCKSIZE-offset);
                DfsWriteBlock(inodes[handle].directAddress[block_num],&dfsb);
                bytesWritten += DFS_BLOCKSIZE-offset;
                /*if(increaseCount)
                {
                    inodes[handle].blocksUsed += 1;
                    inodes[handle].lastWrittenBlockNumber = inodes[handle].directAddress[block_num];
                }*/
                dbprintf('r',"DfsInodeWriteBytes() : DataWritten = ");
                dbprintf('r',dfsb.data);
                dbprintf('r',"\n");
                pos += DFS_BLOCKSIZE-offset;
            }
            dbprintf('r',"DfsInodeWriteBytes() : bytesWritten = %d\n",bytesWritten);
            
        }

        while(pos < maxSingleDirectAddressBytes)
        {
            //REading the [hysical block containg the sigle indirect address]
            block_num = (start_byte + bytesWritten) / DFS_BLOCKSIZE; //This identifies the ith element of directaddress
            offset = (start_byte + bytesWritten) % DFS_BLOCKSIZE;
            dbprintf('r',"DfsInodeWriteBytes() : Creating the single ndirect table.\n block_num = %d\n",block_num);
            dbprintf('r',"DfsInodeWriteBytes() : inodes[handle].singleIndirectBlockNum = %d\n",inodes[handle].singleIndirectBlockNum );
            
            if(inodes[handle].singleIndirectBlockNum == 0)
            {
                dbprintf('r',"DfsInodeWriteBytes() : Entered the if\n");
                inodes[handle].singleIndirectBlockNum = DfsInodeAllocateVirtualBlock(handle,block_num);
                dbprintf('r',"DfsInodeWriteBytes() : Retunred block number = %d\n",inodes[handle].singleIndirectBlockNum );
                dbprintf('r',"DfsInodeWriteBytes() : inodes[handle].singleIndirectBlockNum = %d\n",inodes[handle].singleIndirectBlockNum );

            }
            dbprintf('r',"DfsInodeWriteBytes() : Reading singleindirectAddress in single indirect.\n");
            ret = DfsReadBlock(inodes[handle].singleIndirectBlockNum, &dfs_l1);
            //dbprintf('r',"")
            //exit(); 
            bytesToBeWritten = num_bytes - bytesWritten;
                            
            for(i = 0; i < DFS_BLOCKSIZE / sizeof(int); i++)
            {
                if(bytesWritten >= num_bytes)
                {
                    dbprintf('r',"DfsInodeWriteBytes() : Written bytes = %d\n Ending write.\n",bytesWritten);
                    break;
                }  
                bzero(dfsb.data,DFS_BLOCKSIZE);
                block_num = ((start_byte + bytesWritten) / DFS_BLOCKSIZE) + 1; //because already one block number is allocated to store indirect address.
                offset = (start_byte + bytesWritten) % DFS_BLOCKSIZE;
                bcopy(dfs_l1.data + (i*4),(char*)&IndirectBlockNum,sizeof(int));
                dbprintf("DfsInodeWriteBytes() : Read indirect blocknum = %d\n",IndirectBlockNum);
                //exit();  
                if(IndirectBlockNum != 0)
                {
                    continue;
                }
                else
                {
                    l1VirtualBlock = (int)DfsInodeAllocateVirtualBlock( handle,block_num);
                    bcopy((char*)&l1VirtualBlock,dfs_l1.data + (i*4),sizeof(int));
                // *(dfs_l1.data + (i*4)) = (int)DfsInodeAllocateVirtualBlock(handle,block_num);
                    dbprintf('r',"DfsInodeWriteBytes() : bytestobewritten = %d, bytesWritten = %d, offset = %d, block_num = %d\n",bytesToBeWritten,\
                                                    bytesWritten, offset, block_num);
                    dbprintf('r',"DfsInodeWriteBytes() : BLock Allocated : %d\n",*(dfs_l1.data + (i*4)));
                    DfsWriteBlock(inodes[handle].singleIndirectBlockNum,&dfs_l1);
                //PrintDiskBlock(&dfs_l1); 
                    //exit();
                    if(bytesToBeWritten < DFS_BLOCKSIZE)
                    {
                        dbprintf('r',"DfsInodeWriteBytes() : Reading singleindirectAddress in single indirect.\n");
                    DfsReadBlock(l1VirtualBlock,&dfsb);
                    bcopy((char*)mem + bytesWritten,(char*)dfsb.data + offset,bytesToBeWritten);
                    DfsWriteBlock(l1VirtualBlock,&dfsb);
                    bytesWritten += bytesToBeWritten;
                    pos += bytesToBeWritten;
                    /*if(increaseCount)
                    {
                        inodes[handle].blocksUsed += 1;
                        inodes[handle].lastWrittenBlockNumber = inodes[handle].directAddress[block_num];
                    }*/
                    //dbprintf('r',"DfsInodeWriteBytes() : DataWritten = ");
                    dbprintf('r',dfsb.data);
                    dbprintf('r',"\n");
                    }
                    else
                    {

                    dbprintf('r',"DfsInodeWriteBytes() : Reading singleindirectAddress in single indirect.\n");
                    DfsReadBlock(l1VirtualBlock,&dfsb);
                    bcopy((char*)mem + bytesWritten,(char*)dfsb.data + offset,DFS_BLOCKSIZE-offset);
                    DfsWriteBlock(l1VirtualBlock,&dfsb);
                    bytesWritten += DFS_BLOCKSIZE-offset;
                    pos += DFS_BLOCKSIZE-offset;
                    /*if(increaseCount)
                    {
                        inodes[handle].blocksUsed += 1;
                        inodes[handle].lastWrittenBlockNumber = inodes[handle].directAddress[block_num];
                    }*/
                    //dbprintf('r',"DfsInodeWriteBytes() : DataWritten = ");
                    dbprintf('r',dfsb.data);
                    dbprintf('r',"\n");
                    }
                    dbprintf('r',"DfsInodeWriteBytes() : bytesWritten = %d\n",bytesWritten);
                    dbprintf('r',"dfs_l1 -> [3] = %d [7] = %d [11] =%d [15] = %d [19] = %d [23] = %d [27] = %d [31] = %d\n",dfs_l1.data[3],dfs_l1.data[7],dfs_l1.data[11],dfs_l1.data[15],\
                            dfs_l1.data[19],dfs_l1.data[23],dfs_l1.data[27],dfs_l1.data[31]);
                }
            }
        }
        dbprintf('r',"Strarting the second direct addressing now.\n");
        dbprintf('r',"bytesWritten = %d, pos = %d\n",bytesWritten,pos);

        block_num = (start_byte + bytesWritten) / DFS_BLOCKSIZE; //This identifies the ith element of directaddress
        offset = (start_byte + bytesWritten) % DFS_BLOCKSIZE;
        dbprintf('r',"DfsInodeWriteBytes() : Creating the double ndirect table.\n block_num = %d\n",block_num);
        dbprintf('r',"DfsInodeWriteBytes() : inodes[handle].DoubleIndirectBlockNum = %d\n",inodes[handle].DoubleIndirectBlockNum );
        
        if(inodes[handle].DoubleIndirectBlockNum == 0)
        {
            dbprintf('r',"DfsInodeWriteBytes() : Entered the if\n");
            inodes[handle].DoubleIndirectBlockNum = DfsInodeAllocateVirtualBlock(handle,block_num);
            dbprintf('r',"DfsInodeWriteBytes() : Retunred block number = %d\n",inodes[handle].DoubleIndirectBlockNum );
            dbprintf('r',"DfsInodeWriteBytes() : inodes[handle].DoubleIndirectBlockNum = %d\n",inodes[handle].DoubleIndirectBlockNum );

        }
        dbprintf('r',"DfsInodeWriteBytes() : Reading doubleindirectAddress in double indirect.\n");
        ret = DfsReadBlock(inodes[handle].DoubleIndirectBlockNum, &dfs_l2);
        bytesToBeWritten = num_bytes - bytesWritten;

        //L2 block.

        for(i = 0; i < DFS_BLOCKSIZE / sizeof(int); i++)
        {
            if(bytesWritten >= num_bytes)
                {
                    dbprintf('r',"DfsInodeWriteBytes() : Written bytes = %d\n Ending write.\n",bytesWritten);
                    break;
                }  
                bzero(dfsb.data,DFS_BLOCKSIZE);
                block_num = ((start_byte + bytesWritten) / DFS_BLOCKSIZE); //because already one block number is allocated to store indirect address.
                offset = (start_byte + bytesWritten) % DFS_BLOCKSIZE;
                bcopy(dfs_l2.data + (i*4),(char*)&SecondIndirectBlockNum,sizeof(int));
                dbprintf('r',"DfsInodeWriteBytes() : Read indirect blocknum = %d\n",SecondIndirectBlockNum);
                
                if(SecondIndirectBlockNum != 0)
                    {
                        DfsReadBlock(SecondIndirectBlockNum,&dfs_l1);
                        goto l2read;
                        //continue;
                    }
                else
                {
                
                    l2VirtualBlock = (int)DfsInodeAllocateVirtualBlock( handle,block_num);
                    bcopy((char*)&l2VirtualBlock,dfs_l2.data + (i*4),sizeof(int));
                    // *(dfs_l1.data + (i*4)) = (int)DfsInodeAllocateVirtualBlock(handle,block_num);
                    dbprintf('r',"DfsInodeWriteBytes() : bytestobewritten = %d, bytesWritten = %d, offset = %d, block_num = %d\n",bytesToBeWritten,\
                                                        bytesWritten, offset, block_num);
                    dbprintf('r',"DfsInodeWriteBytes() : BLock Allocated : %d\n",(int)*(dfs_l2.data + (i*4)));
                    DfsWriteBlock(inodes[handle].DoubleIndirectBlockNum,&dfs_l2);
                    dbprintf('r',"l2.data \n");
                    PrintDiskBlock(&dfs_l2); 
                    //exit();
                    bzero(dfs_l1.data,DFS_BLOCKSIZE);
                    
        l2read :        

                
                for(j = 0; j < DFS_BLOCKSIZE / sizeof(int); j++)
                {
                    dbprintf('r',"DfsInodeWriteBytes() : Reading doubleindirectAddress in double indirect.\n");
                    //DfsReadBlock(dfs_l2.data + (i*4),&dfs_l1);
                    if(bytesWritten >= num_bytes)
                    {
                        dbprintf('r',"DfsInodeWriteBytes() : Written bytes = %d\n Ending write.\n",bytesWritten);
                        break;
                    }    

                    dbprintf('r',"DfsndoeWriteBytes() : SecondIndirectBlockNum = %d\n",SecondIndirectBlockNum);
                    //DfsReadBlock(SecondIndirectBlockNum,&dfs_l1);
                    bzero(dfsb.data,DFS_BLOCKSIZE);
                    block_num = ((start_byte + bytesWritten) / DFS_BLOCKSIZE); //because already one block number is allocated to store indirect address.
                    offset = (start_byte + bytesWritten) % DFS_BLOCKSIZE;
                    bcopy(dfs_l1.data + (j*4),(char*)&IndirectBlockNum,sizeof(int));
                    dbprintf("DfsInodeWriteBytes() : Read indirect blocknum = %d\n",IndirectBlockNum);
                    //exit();  
                    //dbprintf('r',"l1.data\n");
                    //PrintDiskBlock(&dfs_l1); 
                    if(IndirectBlockNum != 0)
                    {
                        dbprintf('r',"Indirect block num is present.\n");
                        dbprintf('r',"Indirect blocknum read = %d, j = %d\n",IndirectBlockNum,j);
                        continue;
                    }
                    else
                    {
                        l1VirtualBlock = (int)DfsInodeAllocateVirtualBlock( handle,block_num);
                        dbprintf('r',"l1Virtual BLock = %d\n",l1VirtualBlock);
                        bcopy((char*)&l1VirtualBlock,dfs_l1.data + (j*4),sizeof(int));
                    // *(dfs_l1.data + (i*4)) = (int)DfsInodeAllocateVirtualBlock(handle,block_num);
                        dbprintf('r',"DfsInodeWriteBytes() : bytestobewritten = %d, bytesWritten = %d, offset = %d, block_num = %d\n",bytesToBeWritten,\
                                                        bytesWritten, offset, block_num);
                        dbprintf('r',"DfsInodeWriteBytes() : BLock Allocated : %d\n",(int)*(dfs_l1.data + (j*4)));
                        DfsWriteBlock(SecondIndirectBlockNum,&dfs_l1);
                        //PrintDiskBlock(&dfs_l1); 
                        dbprintf('r',"The secondindirectblock read = %d\n",SecondIndirectBlockNum);
                        dbprintf('r',"The l1virtual block = %d\n",l1VirtualBlock);
                        
                        if(bytesToBeWritten < DFS_BLOCKSIZE)
                        {
                            dbprintf('r',"DfsInodeWriteBytes() : Reading singleindirectAddress in double indirect.\n");
                            DfsReadBlock(l1VirtualBlock,&dfsb);
                            bcopy((char*)mem + bytesWritten,(char*)dfsb.data + offset,bytesToBeWritten);
                            DfsWriteBlock(l1VirtualBlock,&dfsb);
                            bytesWritten += bytesToBeWritten;
                            pos += bytesToBeWritten;
                        /*if(increaseCount)
                        {
                            inodes[handle].blocksUsed += 1;
                            inodes[handle].lastWrittenBlockNumber = inodes[handle].directAddress[block_num];
                        }*/
                        //dbprintf('r',"DfsInodeWriteBytes() : DataWritten = ");
                        dbprintf('r',dfsb.data);
                        dbprintf('r',"\n");
                    }
                    else
                    {
                        dbprintf('r',"DfsInodeWriteBytes() : Reading singleindirectAddress in double indirect.\n");
                        DfsReadBlock(l1VirtualBlock,&dfsb);
                        bcopy((char*)mem + bytesWritten,(char*)dfsb.data + offset,DFS_BLOCKSIZE-offset);
                        DfsWriteBlock(l1VirtualBlock,&dfsb);    
                        bytesWritten += DFS_BLOCKSIZE-offset;
                        pos += DFS_BLOCKSIZE-offset;
                        /*if(increaseCount)
                        {
                            inodes[handle].blocksUsed += 1;
                            inodes[handle].lastWrittenBlockNumber = inodes[handle].directAddress[block_num];
                        }*/
                        //dbprintf('r',"DfsInodeWriteBytes() : DataWritten = ");
                        dbprintf('r',dfsb.data);
                        dbprintf('r',"\n");

                    }
                    /*if(start_byte > 75000)
                        exit();*/
                    dbprintf('r',"DfsInodeWriteBytes() : bytesWritten = %d\n",bytesWritten);
                    dbprintf('r',"dfs_l1 -> [3] = %d [7] = %d [11] =%d [15] = %d [19] = %d [23] = %d [27] = %d [31] = %d\n",dfs_l1.data[3],dfs_l1.data[7],dfs_l1.data[11],dfs_l1.data[15],\
                            dfs_l1.data[19],dfs_l1.data[23],dfs_l1.data[27],dfs_l1.data[31]);
                }
            }
                
            }

            
            
        }


        

        //to do writes for double indirect.
    }    
    inodes[handle].filesize = DfsInodeFilesize(handle);
    dbprintf('r',"DfsInodeWriteBytes() : Inode-%d, filesize = %d\n",handle,inodes[handle].filesize);
    
    return bytesWritten;
           
}
    
    

    


//-----------------------------------------------------------------
// DfsInodeFilesize simply returns the size of an inode's file. 
// This is defined as the maximum virtual byte number that has 
// been written to the inode thus far. Return DFS_FAIL on failure.
//-----------------------------------------------------------------

uint32 DfsInodeFilesize(uint32 handle) {
    int filesize = 0;
    dfs_block d;
    if(inodes[handle].blocksUsed == 0)
    {
        filesize = 0;
    }
    else
    {
        dbprintf('r',"DfsInodeFilesize() : %d\n",inodes[handle].blocksUsed);
        filesize = (inodes[handle].blocksUsed - 1) * DFS_BLOCKSIZE;
        DfsReadBlock(inodes[handle].lastWrittenBlockNumber,&d);
        filesize += dstrlen(d.data);
        
    }
    
    dbprintf('r',"DfsInodeFilesize() : DfsIndoeFileSize = %d\n",filesize);
    return filesize;

}


//-----------------------------------------------------------------
// DfsInodeAllocateVirtualBlock allocates a new filesystem block 
// for the given inode, storing its blocknumber at index 
// virtual_blocknumber in the translation table. If the 
// virtual_blocknumber resides in the indirect address space, and 
// there is not an allocated indirect addressing table, allocate it. 
// Return DFS_FAIL on failure, and the newly allocated file system 
// block number on success.
//-----------------------------------------------------------------

uint32 DfsInodeAllocateVirtualBlock(uint32 handle, uint32 virtual_blocknum) {
    int i = 0;
    int sizeDirectTable = 10;
    int sizeSingleIndirectTable = DFS_BLOCKSIZE / sizeof(int);
    int sizeDoubleIndirectTable = (DFS_BLOCKSIZE / sizeof(int))*(DFS_BLOCKSIZE / sizeof(int));
    dfs_block dfsb;
    int ret = 0;
    int offset_l1table;
    dbprintf('r',"DfsInodeAllocateVirtualBlock() : Virtual block num = %d\n",virtual_blocknum);
    if(virtual_blocknum < 10)
    {
        ret = DfsAllocateBlock();
        
        inodes[handle].directAddress[virtual_blocknum] = ret;
        dbprintf('r',"DfsInodeAllocateVirtualBlock() : Allocated block = %d at inodes[%d].directAddress[%d] = %d\n",virtual_blocknum,\
                                                handle,virtual_blocknum, inodes[handle].directAddress[virtual_blocknum]);
    }
    else if(virtual_blocknum < sizeDirectTable + sizeSingleIndirectTable)
    {
        /*if(inodes[handle].singleIndirectBlockNum == 0)
        {
             inodes[handle].singleIndirectBlockNum = DfsAllocateBlock();
        }*/
        //DfsReadBlock(inodes[handle].singleIndirectBlockNum,&dfsb);
        dbprintf('r',"DfsInodeAllocateVirtualBlock() : Allocated block = %d at inodes[%d].directAddress[%d] = %d\n",virtual_blocknum,\
                                                handle,virtual_blocknum, inodes[handle].directAddress[virtual_blocknum]);
        ret = DfsAllocateBlock();
        /*offset_l1table = virtual_blocknum - sizeSingleIndirectTable;
        bcopy((char*)&ret,dfsb.data + (offset_l1table * 4),sizeof(int));
        DfsWriteBlock(inodes[handle].singleIndirectBlockNum,&dfsb);*/
    }
    else
    {
        dbprintf('r',"Implement the second level table here.\n");
        dbprintf('r',"DfsInodeAllocateVirtualBlock() : Allocated block = %d at inodes[%d].directAddress[%d] = %d\n",virtual_blocknum,\
                                                handle,virtual_blocknum, inodes[handle].directAddress[virtual_blocknum]);
        ret = DfsAllocateBlock();
    }
    inodes[handle].blocksUsed += 1;
    inodes[handle].lastWrittenBlockNumber = ret;
    dbprintf('r',"VirtualBlockTrnsaltion() : The physical block for this %d virtual block = %d\n",virtual_blocknum,ret);
    return ret;

}



//-----------------------------------------------------------------
// DfsInodeTranslateVirtualToFilesys translates the 
// virtual_blocknum to the corresponding file system block using 
// the inode identified by handle. Return DFS_FAIL on failure.
//-----------------------------------------------------------------

uint32 DfsInodeTranslateVirtualToFilesys(uint32 handle, uint32 virtual_blocknum) {
    int ret;
    int sizeDirectTable = 10;
    int sizeSingleIndirectTable = DFS_BLOCKSIZE / sizeof(int);
    int sizeDoubleIndirectTable = (DFS_BLOCKSIZE / sizeof(int)) * (DFS_BLOCKSIZE / sizeof(int));
    dfs_block dfsb;
    int offset_l1table;
    if(virtual_blocknum < 10)
    {
        //ret = DfsAllocateBlock();
        ret = inodes[handle].directAddress[virtual_blocknum]; 
        //inodes[handle].directAddress[virtual_blocknum] = ret;
        dbprintf('r',"DfsInodeAllocateVirtualBlock() : Allocated block = %d at inodes[%d].directAddress[%d] = %d\n",virtual_blocknum,\
                                                handle,virtual_blocknum, inodes[handle].directAddress[virtual_blocknum]);
    }
    else if(virtual_blocknum < sizeDirectTable + sizeSingleIndirectTable)
    {
       
        DfsReadBlock(inodes[handle].singleIndirectBlockNum,&dfsb);
        //ret = DfsAllocateBlock();
        offset_l1table = virtual_blocknum - sizeSingleIndirectTable;
        bcopy((char*)&ret,dfsb.data + (offset_l1table * 4),sizeof(int));
        //DfsWriteBlock(inodes[handle].singleIndirectBlockNum,&dfsb);
    }
    else
    {
        dbprintf('r',"Implement the second level table here.\n");
    }
    return ret;
}
