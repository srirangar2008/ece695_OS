#include "ostraps.h"
#include "dlxos.h"
#include "traps.h"
#include "queue.h"
#include "disk.h"
#include "dfs.h"
#include "synch.h"
#include "filesys.h"
#include "files.h"
#include "process.h"


static dfs_inode inodes[MAX_INODES]; // all inodes
static dfs_superblock sb; // superblock
static uint32 fbv[NUM_FBV]; // Free block vector

extern PCB *currentPCB;

static char str_string[72];
static char *str = str_string;


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
        //GracefulExit ();
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
dbprintf('r',"Size of dfs_inode = %d\n",sizeof(dfs_inode));





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

SetupRootDir();
}

int SetupRootDir()
{
  int j;
  int retval=0;
  dbprintf('r',"Setting up the root directory now.\n");
  if(inodes[0].used==0)
  {
    inodes[0].used = 1;
    inodes[0].filesize = 0;
    inodes[0].blocksUsed = 0;
    inodes[0].singleIndirectBlockNum = 0;
    for (j = 0; j < 10; j++)
    {
      inodes[0].directAddress[j] = 0;
    }
    inodes[0].permission = 077;		//All permissions to everyone
    inodes[0].type = DIR;
    inodes[0].ownerId = 0;
    inodes[0].directAddress[0] = DfsInodeAllocateVirtualBlock(0,0); //Allocating root inode with the first virtual block;
    inodes[0].dirEntries = 0;
    retval = 1;
  }
  else
  {
      dbprintf('r',"Root inode already exists.\n");
  }

  

  dbprintf('r',"Completed setting up the root inode.\n");
  dbprintf('r',"Root inode :");
  PrintInode(inodes[0]);


  return retval;
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
  int i = 0;
  dbprintf('r',"\nSize of inode->used  = %d, inode->used = %d\n",sizeof(d_i.used),d_i.used);
  dbprintf('r',"Size of inode->filesize  = %d, inode->filesize  = %d\n",sizeof(d_i.filesize),d_i.filesize);
  //dbprintf('r',"Size of inode->filename  = %d, inode->filename  = ",sizeof(d_i.filename));

  //dbprintf('r',d_i.filename);
  //dbprintf('r',"\n");
  dbprintf('r',"Size of inode->directAddress  = %d, inode->directAddress  = 0x%x\n",sizeof(d_i.directAddress),&d_i.directAddress);
  for(i = 0; i< 10 ; i++)
    {
        dbprintf('r',"directaddress[i] = %d\n",d_i.directAddress[i]);
    }
  dbprintf('b',"Size of inode->singleIndirectBlockNum  = %d, inode->singleIndirectBlockNum = %d\n",sizeof(d_i.singleIndirectBlockNum),d_i.singleIndirectBlockNum);
  dbprintf('b',"Size of inode->DoubleIndirectBlockNum  = %d, inode->DoubleIndirectBlockNum  = %d\n",sizeof(d_i.DoubleIndirectBlockNum),d_i.DoubleIndirectBlockNum);
  dbprintf('b',"Permissions of the File = 0x%x\n",d_i.permission);
  dbprintf('b',"The type of File(1 -File, 2 - Dir) = %d\n",d_i.type);
  dbprintf('b',"The ownder of the file = %d\n",d_i.ownerId);
  dbprintf('b',"Total size of inode = %d\n", sizeof(d_i));

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
    if(sb.valid != 1)
    {
        printf("ERROR : FileSystem not loaded into memory. Exiting...\n");
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
        printf("ERROR : No more disk space.\n");
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
        printf("ERROR : Filesystem not opened.Exiting ..");
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

//---------------------------------------------------------------------------
//	getOneName
//	
//	This function works much like strtok. The only difference is that the
//	field delimiter is fixed at '/', and you have to specify the
//	destination string where the token could be returned. The string dst
//	should have at least 31 bytes of space. This function also checks for
//	some error conditions such as extremely long filenames and successive
//	'/' characters. On success it returns the length of the string written
//	in dst (not to exceed 30). On error, it returns -1. If the string src
//	starts with a '/', the first '/' is ignored.
//
//	Example: Consider src = "/a/b/cd"
//		len = getOneName(dst, src); 
//			//returns dst = "a", len = 1;
//		len = getOneName(dst, NULL);
//			//returns dst = "b", len = 1;
//		len = getOneName(dst, NULL);
//			//returns dst = "cd", len = 2;
//		len = getOneName(dst, NULL);
//			//returns dst = "", len = 0;
//	
//	Note that the successive calls should be made with src = NULL. If src
//	is not NULL, the string passed is parsed from the beginnning.
//--------------------------------------------------------------------------
int 
getOneName(char *dst, char *src)
{
  int count = 0;
  

  if(src!=NULL)
  {
    if(*src=='/')
      src++;
    if(*src=='/')
      return -1;			//successive '/' not allowed
    str = src;	
  }
  for(;*str!='\0';str++)
  {
    dst[count] = *str;
    count++;
    if(*str=='/')
    {
      str++;
      count--;
      if(*str =='/')
      {
        return -1;			//successive '/' not allowed
      }
      break;
    }
    else
    {
      if(count==71) 
      {
       return -1;			//Filename too long
      }
    }
  }
  dst[count] = '\0';
  return count;
}

int isDir(int id)
{
  
  uint32 flmode;

  //node = getInodeAddress(id);

  if(id==NULL)
  {
    return 0;				//inode not valid
  }
  if(inodes[id].type == DIR)
  {
    return 1;
  }
  else
  {
    return 0;
  }
}

int checkIfValidInode_2(char* filename, int inode_entry)
{
    //Starting from the root always. 
    int i = 0;
    int j = 0;
    dfs_block dfsb;
    dirEntry dir;
    int ret = 0;
    int found = 0;
    dbprintf('r',"REading inode with handle = %d\n",inode_entry);
    bzero(dfsb.data,DFS_BLOCKSIZE);
    for(i = 0; i < MAX_INODES; i++)
    {
        //Read the data block of the inode.
        if(inodes[inode_entry].directAddress[i] != 0)
        {
            DfsReadBlock(inodes[inode_entry].directAddress[i],&dfsb);
            for(j = 0; j < 6; j++)
            {
                bcopy(dfsb.data + (j*76),(char*)&dir,sizeof(dirEntry));
                dbprintf('r',"dir.filename = %s\n",dir.filename);
                dbprintf('r',"inodeum = %d\n",dir.inodeNum);
                if(dstrlen(dir.filename) == 0)
                {
                    dbprintf('r',"No entries found.\n");
                    ret = inode_entry; // Create a new directory/file with this as the parent.
                }
                if(dstrncmp(dir.filename,filename,dstrlen(filename)) == 0)
                {
                    dbprintf('r',"Match found. Match from data block = %s at index = %d\n",dir.filename, j);
                    dbprintf('r',"Corressponding inode = %d\n",dir.inodeNum);
                    ret = dir.inodeNum;
                    found = 1;
                    break;
                }
            }
        }
        else
        {
            break;
        }

        if(found)
        {
            //  ret = inode_entry;
            break;
        }
        
    }
    if((i == MAX_INODES) || ((j == 6) &&(found == 0)))
    {
        ret = -10;
    }
    return ret;
}

//Storing only 6 entries per block. 
//The last 56 bytes is empty.

/*int checkIfValidInode(char* filename, int inode_entry)
{
    int i = 0;
    dfs_block dfsb;
    int j;
    dirEntry dir;
    int inodeHandle ;
    int found = 0;

    //First check in root node. Return the inode of dir.
    
    for(j = 0; j < 10; j++)
    {
        if(inodes[inode_entry].directAddress[j] != 0)
        {
            DfsReadBlock(inodes[inode_entry].directAddress[j],&dfsb);
            for(i = 0; i < 6; i++)
            {
                bcopy(dfsb.data + (i*sizeof(dirEntry)),(char*)&dir,sizeof(dirEntry));
                
                if(dstrncmp(filename,dir.filename,dstrlen(filename)) == 0)
                {
                    if(dstrlen(dir.filename) == 0)
                    {
                        dbprintf('r',"The file does not exist.\n");
                        inodeHandle = inode_entry;
                        break;
                    }
                    dbprintf('r',"strlen(dir.filename) = %d\n",dstrlen(dir.filename));
                    dbprintf('r',"Match for the filename found.\n");
                    bcopy(dfsb.data + (i*sizeof(dirEntry)) + sizeof(dir.filename),(char*)&inodeHandle,sizeof(dir.inodeNum));
                    dbprintf('r',"dir.inode  = %d, dir.filename = ",dir.inodeNum);
                    dbprintf('r',dir.filename);
                    dbprintf('r',"\n");
                    
                    found = 1;
                    break;
                }
                dbprintf('r',"dir.inode  = %d, dir.filename = ",dir.inodeNum);
                dbprintf('r',dir.filename);
                dbprintf('r',"\n");
                //dbprintf('r',"Not found in the first block. exiting.\n");
                //inodeHandle = -10;
            }
            if(found)
            {
                inodeHandle == -5;
                break;
            }
           
        }
        if(!found)
        {
            inodeHandle = inode_entry;
            break;
        }
    }
    /*if(j == 10)
    {
        dbprintf('r',"checkIfValidInode() : Not found.");
        inodeHandle = -10;
        /*if(inodes[inode_entry].singleIndirectBlockNum != 0)
        {
            dbprintf('r',"Implement this.\n");
        }
    }
    return inodeHandle;

}*/

//mk = 0; -> called from mkdir.
//rm = 1; ->called from rmdir.

int MakeInodeFromPath_2(char *path, int mode, int type, int rm_mk)
{
    //1. check if the path exists.
    char leafNode[72];
    char dst[31];
    dirEntry dir;
    dfs_block dfsb;
    int len;
    int inode_entry = 0;
    int temp;
    int exists = 0;
    int newInode;
    int block; 
    int create = 0;
    int offset = 0;
    bzero(dst,sizeof(dst));
    len = getOneName(dst,path);
    dbprintf('r',"Obtained path = %s\n",path);
    while(len != 0)
    {
        exists = 0;
        bzero(leafNode,sizeof(leafNode));
        dbprintf('r',"while loop 1() : len = %d, dst = %s\n",len,dst);
        bcopy(dst,leafNode,dstrlen(dst));
        //CHeck if dst exists.
        temp = inode_entry;
        inode_entry = checkIfValidInode_2(leafNode,inode_entry);
        dbprintf('r',"Inode Entry = %d\n",inode_entry);
        bzero(dst,sizeof(dst));
        len = getOneName(dst,NULL);
        dbprintf('r',"len = %d\n",len);
        
        dbprintf('r',"while loop 2() : len = %d, dst = %s\n",len,leafNode);
        //bzero(dst,sizeof(dst));

        if(rm_mk == 1 && (inode_entry == -10))
        {
            printf("Path does not exist.\n");
            return INVALID_PATH;
        }
        
        if((len != 0) && (inode_entry == -10) && (rm_mk == 0))
        {
           printf("ERROR : Path doen not exist.\n");
            return INVALID_PATH;
         }
         else if(inode_entry > 0)
         {
             exists = 1;
             dbprintf('r',"The parent directory exists.\n");
             if(inodes[inode_entry].type == DIR)
             {
                 dbprintf('r',"Permission = 0x%x and check = 0x%x, answer = 0x%x\n",inodes[inode_entry].permission, (UX | OX | UW | OW), \
                                (inodes[inode_entry].permission & (UX | OX | UW | OW)));
                if((inodes[inode_entry].permission & (UX | OX | UW | OW)) != (UX | OX | UW | OW))
                {
                    dbprintf('r',"0x%x",(UX | OX | UW | OW));
                    dbprintf('r',"Insufficient permissions.\n");
                    return PERMISSION_DENIED;
                }
             }
         }
         else
         {
             create = 1;
             dbprintf('r',"Create a new path with this indoe num as parent = %d",temp);
         }
         
         
         
         
    }
    if((len == 0) && (create == 1))
        {
            dbprintf('r',"Create a new file/directory and u[date this inode = %d\n",temp);
            //break;
        }
    else
        {
            dbprintf('r',"The path exists. Continuing..");
            exists = 1;
        }
        
    dbprintf('r',"len = %d, dst = %s\n",len,dst);
    dbprintf('r',"leafNode = %s\n",leafNode);
    if(exists)
    {
        printf("The path exists = %s.\n",path);
        /*if(inodes[inode_entry].type == FILE)
            return FILE_EXISTS;
        if(inodes[inode_entry].type == DIR)
            return DIRECTORY_EXISTS;*/
        
        return inode_entry;
    }
    /*if(inode_entry == -10)
    {
        return -1;
    }*/

    if(type == DIR)
    {
        bzero(dir.filename,sizeof(dir.filename));
        //Create inode.Update the block. 
        newInode = DfsInodeOpen(leafNode);
        block = inodes[temp].dirEntries / 6;
        offset = (inodes[temp].dirEntries % 6) * sizeof(dirEntry);
        dbprintf('r',"Writing back : block = %d, offset = %d\n",block,offset);
        if(inodes[temp].directAddress[block] == 0)
        {
            inodes[temp].directAddress[block] = DfsInodeAllocateVirtualBlock(temp,block);
        }
        bcopy(leafNode,(char*)dir.filename,dstrlen(leafNode));
        dir.inodeNum = newInode;
        dbprintf('r',"Before writing : dir.filename = %s, dir.numInode = %d\n",dir.filename,dir.inodeNum);
        DfsReadBlock(inodes[temp].directAddress[block],&dfsb);
        bcopy((char*)&dir,dfsb.data + offset,sizeof(dirEntry));
        DfsWriteBlock(inodes[temp].directAddress[block],&dfsb);
        inodes[temp].dirEntries += 1;
        inodes[newInode].type = DIR;
        inodes[newInode].ownerId = GetCurrentPid();
        inodes[newInode].permission = mode;
        PrintInode(inodes[newInode]);
    }
    if(type == FILE)
    {
        dbprintf('r',"Have to create Inode for a file\n");
        newInode = DfsInodeOpen(leafNode);
        block = inodes[temp].dirEntries / 6;
        offset = (inodes[temp].dirEntries % 6) * sizeof(dirEntry);
        dbprintf('r',"sizeof(dirEntry) = %d\n",sizeof(dirEntry));
        dbprintf('r',"Writing back : block = %d, offset = %d\n",block,offset);
        if(inodes[temp].directAddress[block] == 0)
        {
            inodes[temp].directAddress[block] = DfsInodeAllocateVirtualBlock(temp,block);
        }
        bcopy(leafNode,(char*)dir.filename,dstrlen(leafNode));
        dir.inodeNum = newInode;
        DfsReadBlock(inodes[temp].directAddress[block],&dfsb);
        bcopy((char*)&dir,dfsb.data + offset,sizeof(dirEntry));
        DfsWriteBlock(inodes[temp].directAddress[block],&dfsb);
        inodes[temp].dirEntries += 1;
        inodes[newInode].type = FILE;
        inodes[newInode].ownerId = GetCurrentPid();
        inodes[newInode].permission = mode;
        PrintInode(inodes[newInode]);
    }
    return newInode; 
}


//--------------------------------------------------------------------------
//	MakeInodeFromPath
//
//	Given a path, and inode description, create that inode and return the
//	inode identifier. If a file corresponding to the path already exists, 
//	it returns FILE_EXISTS. This function is used both for creating a file 
//	and creating a directory. If the path provided is not valid, then this
//	function returns INVALID_PATH. If any directory along the path does
//	not have EXECUTE permission, or the parent directory of the leaf does
//	not have write permission, this function returns PERMISSION_DENIED.
//--------------------------------------------------------------------------
/*int MakeInodeFromPath(char *path, int mode, int type)
{
  //This function is not mandatory, and you can skip writing it if you want.
  //But we strongly recommend writing this function as you can use this
  //function to create files as well as directories.
  
  //it may insert the inode number and file or directory name in global variables
    char dst[31];
    char leafNode[72];
    int len = 0;
    int inode_entry = 0;
    int level = 0;
    
    int inodeHandle;
    int i = 0;
    dfs_block dfsb;
    dirEntry dir;
    
    // dbprintf('r',"len = %d, dst = ",len);
    // dbprintf('r',dst);
    // dbprintf('r',"\n");
    // dbprintf('r',"str = ");
    // dbprintf('r',str);
    // dbprintf('r',"\n");
    int blocknum;
    int offset = 0;
    
    //Check if the inode exists and the path is correct.
    len  = getOneName(dst,path);
    while(len > 0)
    {
        bzero(leafNode,sizeof(leafNode));
        bcopy(dst,leafNode,dstrlen(dst));
        dbprintf('r',"MakeIndoeFromPath() : len = %d, dst = ",len);
        dbprintf('r',dst);
        dbprintf('r',"\n");
        inode_entry = checkIfValidInode(dst,inode_entry);
        dbprintf('r',"Inode Entry = %d\n",inode_entry);
        if(inode_entry == -5)
        {
            dbprintf('r',"Already the inode exists. Not creating a new one.\n");
            return FILE_FAIL;
        }
        else if((inode_entry < 0) && (level != 0))
        {
            dbprintf('r',"The entered directory path is not valid.\n");
            return FILE_FAIL;
            exit();
        }
        else
        {
            level += 1;
            dbprintf('r',"MakeIndoeFromPath() : str = ");
            dbprintf('r',str);
            dbprintf('r',"\n");
            dbprintf('r',"MakeIndoeFromPath() : leafNode = ");
            dbprintf('r',leafNode);
            dbprintf('r',"\n");
            len = getOneName(dst,NULL);
        }
        
    }
    // if(inode_entry > 0 && len == 0)
    // {
    //     dbprintf('r',"The directory required = %s already exists\n",leafNode);
    //     return FILE_FAIL;
    // }
    bzero(str_string,sizeof(str_string));
    str = str_string;
    dbprintf('r',"Have to create directory/file for the leaf node.\n");
    dbprintf('r',"LeafNode = ");
    dbprintf('r',leafNode);
    dbprintf('r',"\n");
    if(level == 1)
    {
        dbprintf('r',"The parent for this file/directory is the root.\n");
    }

    dbprintf('r'," MakeInodeFromPath() : A new dir/file with ");
    dbprintf('r',leafNode);
    dbprintf('r'," is being created.");
    for(i = 0; i < MAX_INODES; i++)
    {
        if(inodes[i].used == 0)
        {
            inode_lock_handle = LockAcquire(&inode_lock);
            inodes[i].used = 1;
            inode_lock_handle = LockRelease(&inode_lock);
            dbprintf('r',"strlen of filename = %d\n",dstrlen(leafNode));
            
            blocknum = (inodes[inode_entry].dirEntries / 6);

            dbprintf('r',"blockNum = %d\n",blocknum);
            dbprintf('r',"Filename = ");
            dbprintf('r',leafNode);
            if(inodes[inode_entry].directAddress[blocknum] == 0)
            {
                inodes[inode_entry].directAddress[blocknum] = DfsInodeAllocateVirtualBlock(i,blocknum);
            }
            dbprintf('r',"FS block = %d\n",inodes[inode_entry].directAddress[blocknum]);
            DfsReadBlock(inodes[inode_entry].directAddress[blocknum],&dfsb);
            while(offset < 6*76)
            {
                bcopy(dfsb.data + offset, (char*)&dir,sizeof(dirEntry));
                dbprintf('r',"strlen(dir.filename) = %d\n",dstrlen(dir.filename));
                if(dstrlen(dir.filename) == 0)
                {
                    dbprintf('r',"Entere the if.\n");
                    bcopy(leafNode,dir.filename,dstrlen(leafNode));
                    dir.inodeNum = i;
                    dbprintf('r',"dir.filename = %s, dir.inode = %d\n",dir.filename, dir.inodeNum);
                    bcopy((char*)&dir,dfsb.data + offset,sizeof(dirEntry));
                    //PrintDiskBlock(&dfsb);
                    DfsWriteBlock(inodes[inode_entry].directAddress[blocknum],&dfsb);
                    break;
                }
                offset += sizeof(dirEntry);
            }
            inodes[i].filesize = 0;
            inodes[i].blocksUsed = 0;
            inodeHandle = i;
            if(type == DIR)
            {
                inodes[i].type = DIR;
            }
            else{
                inodes[i].type = FILE;
            }
            inodes[i].permission = (unsigned char)mode;
            
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
    return inodeHandle;
    //exit();
}*/
  


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
    //ret = MakeInodeFromPath(filename,077,FILE);
    if(sb.valid != 1)
    {
        dbprintf('r',"DFSInodeFilenameExists(): Filesystem not open. Exiting...\n");
        exit();
        return DFS_FAIL;
    }
    /*for(i=0; i < MAX_INODES;i++)
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
        
    }*/
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
    //int ret = DfsInodeFilenameExists(filename);
    int ret = -2;
    int inodeHandle = -1;
    dbprintf('r',"Entered DfsInodeOPen()\n");
    dbprintf('r',"To create a file name with name = ");
    dbprintf('r',filename);
    dbprintf('r',"\n");
    if(sb.valid != 1)
    {
        printf("ERROR : DfsInodeOpen(): Filesystem not open. Exiting...\n");
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
                inodes[i].directAddress[0] = DfsInodeAllocateVirtualBlock(i,0);
                inodes[i].blocksUsed += 1;
                inodes[i].lastWrittenBlockNumber = inodes[i].directAddress[0];
                //PrintInode(inodes[i]);                
                break;
            }
            if(i == MAX_INODES)
            {
                printf("ERROR : Max number of inodes already used. Cannot allocate anymore.\n");
                //exit();
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
        printf("ERROR : Valid FS not open. Exiting...\n");
        return DFS_FAIL;
    }
    if(inodes[handle].used == 0)
    {
        printf("ERROR : The inode is not in use. Failure.\n");
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
        DfsReadBlock(inodes[handle].DoubleIndirectBlockNum, &dfsb);
        for(i = 0; i < DFS_BLOCKSIZE / 4;i++)
        {
            bcopy(dfsb.data + (i*4),(char*)&block_num,4);
            if(block_num != 0)
            {
                bzero(dfsb_2.data,DFS_BLOCKSIZE);
                DfsReadBlock(block_num,&dfsb_2);
                for(j = 0; j < DFS_BLOCKSIZE / 4; j++)
                {
                    bcopy(dfsb_2.data + (i*4),(char*)&block_num_2,4);
                    if(block_num_2 != 0)
                    {
                        DfsFreeBlock(block_num_2);
                    }
                }
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

int DfsInodeReadBytes(uint32 handle, void *mem, int start_byte, int num_bytes) {
    int i = 0;
    int bytesRead = 0;
    int pos = 0;
    int ret;
    dfs_block dfsb;
    dfs_block dfs_l1, dfs_l2;
    int block_num;
    int offset;
    int bytesToBeRead;
    int maxdirectAddressBytes = (sizeof(inodes[handle].directAddress) / sizeof(uint32)) * DFS_BLOCKSIZE;
    uint32 maxSingleDirectAddressBytes = maxdirectAddressBytes + (DFS_BLOCKSIZE * DFS_BLOCKSIZE);
    uint32 maxSecondaryDirectAddressBytes = maxSingleDirectAddressBytes + (DFS_BLOCKSIZE * DFS_BLOCKSIZE * DFS_BLOCKSIZE);
    dbprintf('r',"Handle = %d\n",handle);
    dbprintf('r',"maxdirectAddressBytes = %d\nmaxSingleDirectAddressBytes = %d\naxSecondaryDirectAddressBytes = %d\n",\
            maxdirectAddressBytes,maxSingleDirectAddressBytes,maxSecondaryDirectAddressBytes);
    
    //Just for test purposes. 
    //inodes[handle].filesize = DFS_BLOCKSIZE;
    dbprintf('r',"start byte = %d, num_bytes = %d\n",start_byte,num_bytes);


    /*if(start_byte + num_bytes > DfsInodeFilesize(handle))
    {
        dbprintf('r',"Trying to read more than the filesize.\n");
        num_bytes = DfsInodeFilesize(handle) - start_byte;
        //return DFS_FAIL;
    }*/
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
                bcopy(dfsb.data + offset,mem + bytesRead,bytesToBeRead);
                bytesRead += bytesToBeRead;
                dbprintf('r',"DfsInodeREadBytes() : DataRead = \n");
                dbprintf('r',dfsb.data);
                dbprintf('r',"from mem buffer  = %s\n",mem);
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
        dbprintf('r',"To habdle the second indirect case\n");
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
    int i = 0;
    int bytesWritten = 0;
    int bytesToBeWritten = num_bytes;
    int maxdirectAddressBytes = (sizeof(inodes[handle].directAddress) / sizeof(uint32)) * DFS_BLOCKSIZE;
    int maxSingleDirectAddressBytes = maxdirectAddressBytes +(DFS_BLOCKSIZE * DFS_BLOCKSIZE);
    uint32 maxSecondaryDirectAddressBytes = maxSingleDirectAddressBytes + (DFS_BLOCKSIZE * DFS_BLOCKSIZE * DFS_BLOCKSIZE);
    int offset = 0;
    int block_num = 0;
    dfs_block dfsb;
    int pos = start_byte;
    int increaseCount = 0;
    int ret;
    dfs_block dfs_l1;
    int IndirectBlockNum = 0;
    int l1VirtualBlock;
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
    int sizeSingleIndirectTable = 512;
    int sizeDoubleIndirectTable = 512*512;
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
        DfsReadBlock(inodes[handle].singleIndirectBlockNum,&dfsb);
        ret = DfsAllocateBlock();
        /*offset_l1table = virtual_blocknum - sizeSingleIndirectTable;
        bcopy((char*)&ret,dfsb.data + (offset_l1table * 4),sizeof(int));
        DfsWriteBlock(inodes[handle].singleIndirectBlockNum,&dfsb);*/
    }
    else
    {
        dbprintf('r',"Implement the second level table here.\n");
    }
    inodes[handle].blocksUsed += 1;
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
    int sizeSingleIndirectTable = 512;
    int sizeDoubleIndirectTable = 512*512;
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

int countchar(char* path, char* ch)
{
    int count = 0;
    int i = 0;
    int len = dstrlen(path);
    for(i = 0; i < len; i++)
    {
        if(path[i] == *ch)
        {
            count += 1;
        }
    }
    if(path[0] != *ch)
        count += 1;
    return count;
}

int getParentInode(char* path)
{
    char buf[72];
    char parentPath[72];
    int i = 0;
    int count = 0;
    int refCount = 0;
    int ret = 0;
    bzero(buf,sizeof(buf));
    bzero(parentPath,sizeof(parentPath));
    count = countchar(path,"/");
    dbprintf('r',"getParentInode() : Entered the getParentInode func.\n");
    dbprintf('r',"getParentInode() : path = %s\n",path);

    dbprintf('r',"Count = %d\n",count);
    if(dstrncmp("/",path,1) == 0)
    {
       dbprintf('r',"Directory from root.\n");
        bcopy(path,buf,dstrlen(path));
    }
    else
    {
         bcopy("/",buf,1);
        dbprintf('r',"buf[0] = %c\n",buf[0]);
        bcopy(path,buf + 1,dstrlen(path));
    }
    
    dbprintf('r',"buf = %s\n",buf);
    for(i = 0; i < dstrlen(path); i++)
    {
        dbprintf('r',"buf[%d] = %c\n",i,buf[i]);
        if(dstrncmp(buf + i,"/",1) == 0)
        {
            refCount += 1;
        }
        if(refCount == count)
        {
            bcopy(buf,parentPath,i);
            dbprintf('r',"ParentPath = %s\n",parentPath);
            break;
        }
    }
    dbprintf('r',"RefCOunt = %d\n",refCount);
    ret = MakeInodeFromPath_2(parentPath,0x3f,DIR,1);
    dbprintf('r',"Inode obtained = %d\n",ret);
    return ret;
}

int removeDirFile(char* path, int type)
{
    int ret;
    int i = 0;
    int parentInode;
    dfs_block dfsb;
    char leaf[72];
    char temp[72];
    bzero(leaf,sizeof(leaf));
    bcopy(path,temp,dstrlen(path));


    if((dstrncmp(path,"/",1) == 0) && (dstrlen(path) == 1))
    {
        printf("ERROR : Cannot delete the root node.\n");
        return PERMISSION_DENIED;
    }
    
    dbprintf('b',"Entered the removeDir function in dfs.c\n");
    parentInode = getParentInode(temp);
    ret = MakeInodeFromPath_2(path,0x3f,DIR,1);
    dbprintf('r',"ret = %d\n",ret);
    if(ret < 0)
    {
        return ret;
    }
    PrintInode(inodes[ret]);
    if(inodes[ret].type == FILE)
    {
        dbprintf('b',"Have to delete a file.\n");
    }
    else if(inodes[ret].type != DIR)
    {
        printf("ERROR : The path = %s is not a directory.\n",path);
        return FILE_FAIL;
    }
    else
    {
        dbprintf('b',"Have to delete a directory.\n");
    }
    
    dbprintf('r',"Obtained inode num = %d\n",ret);
    //Checking if the dir is empty.
    for(i = 0; i < 10;i++)
    {
        if(inodes[ret].directAddress[i] != 0 )
        {
        DfsReadBlock(inodes[ret].directAddress[i],&dfsb);
            if(dstrlen(dfsb.data) != 0)
            {
                dbprintf('r',"dfsb.data = %s\n",dfsb.data);
                dbprintf('r',"The dir is not empty.\n");
                break;
            }
        }
    }
    if( i != 10)
    {
        dbprintf('r',"Clear the contents before deleting the dir.\n");
        return FILE_FAIL;
    }
    else
    {
        DfsInodeDelete(ret);
        dbprintf('r',"Have to remove the directory/file entry from parent.\n");
        dbprintf('r',"Parent Inode = %d\n",parentInode);
        getLeafNode(path,leaf);
        dbprintf('r',"leaf = %s\n",leaf);
        removeFromParentInode(parentInode,leaf);
        //exit();
    }
    return FILE_SUCCESS;
}

int getLeafNode(char* path, char* leaf)
{
    int len; char dst[72]; char leafNode[72];
    bzero(dst,sizeof(dst));
    bzero(leaf,sizeof(leaf));
    len = getOneName(dst,path);
    while(len != 0)
    {
        bzero(leafNode,sizeof(leafNode));
        //dbprintf('r',"while loop 1() : len = %d, dst = %s\n",len,dst);
        bcopy(dst,leafNode,dstrlen(dst));
        //CHeck if dst exists.
        //temp = inode_entry;
        //inode_entry = checkIfValidInode_2(leafNode,inode_entry);
        //dbprintf('r',"Inode Entry = %d\n",inode_entry);

        
        //dbprintf('r',"while loop 2() : len = %d, dst = %s\n",len,dst);
        bzero(dst,sizeof(dst));
        len = getOneName(dst,NULL);
        /*if((len != 0) && (inode_entry == -10))
        {
           dbprintf('r',"Path doen not exist.\n");
            return -1;
         }
         else if(inode_entry > 0)
         {
             exists = 1;
             dbprintf('r',"The parent directory exists.\n");
         }
         else
         {
             create = 1;
             dbprintf('r',"Create a new path with this indoe num as parent = %d",temp);
         }*/
         
         
         
    }
    dbprintf('r',"LeafNode = %s\n",leafNode);
    bcopy(leafNode,leaf,dstrlen(leafNode));
    //exit();
    return 0;
}
int removeFromParentInode(int inode_parent, char* filename)
{
    int j;
    int found;
    dirEntry dir;
    dfs_block dfsb;
    int i;

    for(j = 0; j < 10; j++)
    {
        if(inodes[inode_parent].directAddress[j] != 0)
        {
            DfsReadBlock(inodes[inode_parent].directAddress[j],&dfsb);
            for(i = 0; i < 6; i++)
            {
                bcopy(dfsb.data + (i*sizeof(dirEntry)),(char*)&dir,sizeof(dirEntry));
                
                if(dstrncmp(filename,dir.filename,dstrlen(filename)) == 0)
                {
                    if(dstrlen(dir.filename) == 0)
                    {
                        dbprintf('r',"The file does not exist.\n");
                        //inodeHandle = inode_parent;
                       // break;
                    }
                    dbprintf('r',"dfsb.data before : %s\n",dfsb.data);
                    dbprintf('r',"strlen(dir.filename) = %d\n",dstrlen(dir.filename));
                    dbprintf('r',"Match for the filename found.\n");
                    //bcopy(dfsb.data + (i*sizeof(dirEntry)) + sizeof(dir.filename),(char*)&inodeHandle,sizeof(dir.inodeNum));
                    dbprintf('r',"dir.inode  = %d, dir.filename = ",dir.inodeNum);
                    dbprintf('r',dir.filename);
                    dbprintf('r',"\n");
                    bzero(dfsb.data + (i*sizeof(dirEntry)),sizeof(dirEntry));
                    dbprintf('r',"dfsb.data after : %s\n",dfsb.data);
                    DfsWriteBlock(inodes[inode_parent].directAddress[j],&dfsb);
                    found = 1;
                    //exit();
                    break;
                }
                dbprintf('r',"dir.inode  = %d, dir.filename = ",dir.inodeNum);
                dbprintf('r',dir.filename);
                dbprintf('r',"\n");
                //dbprintf('r',"Not found in the first block. exiting.\n");
                //inodeHandle = -10;
            }
         
           
        }
        
    }
    return 0;
}

dfs_inode* getInodeAddress(int inode_handle)
{
    
    if(inode_handle <= 0 || inode_handle >= MAX_INODES)
    {
        return NULL;
    }
    else
    {
        return &inodes[inode_handle];
    }
    
}

int copyInode(int source, int dest, char* newFile)
{
    int i = 0, j = 0;
    dfs_block dfsb;
    dirEntry dir;
    int parentInode = getParentInode(newFile);
    dbprintf('r',"Parent inode = %d\n",parentInode);
    //exit();
    dbprintf('r',"SOurce Inode before : \n");
    //PrintInode(inodes[source]);
    dbprintf('r',"Destinaton Inode before : \n");
    //PrintInode(inodes[dest]);

    for(i = 0; i < 10; i++)
    {
    if(inodes[parentInode].directAddress[i] != 0)
        {
            DfsReadBlock(inodes[parentInode].directAddress[i],&dfsb);
            for(j = 0; j < 6; j++)
            {
                bzero(dir.filename,sizeof(dir.filename));
                bcopy(dfsb.data + (j*76),(char*)&dir,sizeof(dirEntry));
                dbprintf('r',"dir.filename = %s\n",dir.filename);
                dbprintf('r',"inodeum = %d\n",dir.inodeNum);
                if(dstrlen(dir.filename) == 0)
                {
                    dbprintf('r',"No entries found.\n");
                    //ret = inode_entry; // Create a new directory/file with this as the parent.
                }
                if(dstrncmp(dir.filename,newFile,dstrlen(newFile)) == 0)
                {
                    dbprintf('r',"Match found. Match from data block = %s at index = %d\n",dir.filename, j);
                    dbprintf('r',"Corressponding inode = %d\n",dir.inodeNum);
                    dir.inodeNum = source;
                    dbprintf('r',"Changed inode = %d\n",dir.inodeNum);
                    bcopy((char*)&dir, dfsb.data + (j*76), sizeof(dirEntry));
                    DfsWriteBlock(inodes[parentInode].directAddress[i],&dfsb);
                    //PrintDiskBlock(inodes[parentInode].directAddress[i]);
                    //exit();
                    break;
                }
            }
        }
    }

    /*bcopy((char*)&inodes[source],(char*)&inodes[dest],sizeof(dfs_inode));

    printf("Source ndoe after : \n");
    PrintInode(inodes[source]);

    printf("Destination Inode after \n");
    PrintInode(inodes[dest]);*/

    return 0;

}
