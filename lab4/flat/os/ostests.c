#include "ostraps.h"
#include "dlxos.h"
#include "traps.h"
#include "disk.h"
#include "dfs.h"
#include "files.h"

void RunOSTests() {
  // STUDENT: run any os-level tests here
/*The tests to be run to check driver level functionality.
  1. DfsInodeOpen has to be verified.
  2. DfsIndoeWriteBytes : 
    a. Aligned bytes to address onlythe direct addressing.
    b. Unaligned bytes to address the direct addressing.
    c. Aligned bytes to address the single indirect addressing.
    d. Unaligned bytes to address the single indirect addressing.
    e. Aligned and unaligned bytes for second level indirect addressing.
  3. DfsInodeReadBytes : 
    a. Test whatever is written in the above testcases.
  4. Write multiple files and see if the consistency is maintained.
  5. Delete all inodes to see if the FS returns back to the original state.

*/

  int ret,ret_1;
  int i = 0;
  dfs_block d;
  int block_num = 0;
  int fd;
  int inodeHandle;
  int nbytes;
  int total_bytes = 0;
  char integer[1024] = "Project Excalibur was a Lawrence Livermore National Laboratory (LLNL)\n\
  Cold War-era research program to develop an X-ray laser as a ballistic missile defense (BMD)\n\
  for the United States.The concept involved packing large numbers of expendable X-ray lasers\n\
  around a nuclear device.When the device detonated, the X-rays released by the bomb would be\n\
  focused by the lasers,each of which would be aimed at a target missile.[2]\n \
  When detonated in space, the lack of atmosphere to block the X-rays allowedyet\n. \
  Project Excalibur was a Lawrence Livermore National Laboratory (LLNL)\n\
  Cold War-era research program to develop an X-ray laser as a ballistic missile defense (BMD)\n\
  for the United States.The concept involved packing large numbers of expendable X-ray lasers\n\
  around a nuclear device.When the device detonated, the X-rays released by the bomb would be\n\
  focused by the lasers,each of which would be aimed at a target missile.[2]\n";

  char buffer[15000];
  char test[100] = "This is the first line which im testing.\n";
  char test_2[100] = "This is the second line im testing.\n";

  printf("dtrlen of integer buffer = %d\n",dstrlen(integer));
  
  printf("Running OS Tests now.\n");
  #if 1
  printf("*************************************************\n");
  printf("Test 1 : Writing and reading aligned bytes \n");
  printf("*************************************************\n");

  inodeHandle = DfsInodeOpen("Test1.txt");
  if(inodeHandle == DFS_FAIL)
  {
    printf("Not able to allocate inodes.\n");
    exit();
  }
  printf("The returned inodeHandle = %d\n",inodeHandle);
  printf("Writing only 512 bytes.\n");
  if((nbytes = DfsInodeWriteBytes(inodeHandle,integer,0,512)) < 0)
  {
    printf("Write failed.\n");
    exit();
  }
  printf("Reading whatever is written within the file.\n");
  if((nbytes = DfsInodeReadBytes(inodeHandle,buffer,0,512)) < 0)
  {
    printf("Read failed.\n");
    exit();
  }
  printf("Buffer = ");
  printf(buffer);
  printf("\n");
  if((ret = DfsInodeDelete(inodeHandle)) < 0)
  {
    printf("Inode Delete failed.\n");
    exit();
  }
  printf("Inode with handle = %d\n successfully deleted.\n",inodeHandle);
  printf("********************Test 1 ended*********************\n");  

  printf("*************************************************\n");
  printf("Test 2 : Writing and reading unaligned bytes \n");
  printf("*************************************************\n");

  inodeHandle = DfsInodeOpen("Test1.txt");
  if(inodeHandle == DFS_FAIL)
  {
    printf("Not able to allocate inodes.\n");
    exit();
  }
  printf("The returned inodeHandle = %d\n",inodeHandle);
  printf("Writing only 768 bytes.\n");
  if((nbytes = DfsInodeWriteBytes(inodeHandle,integer,0,768)) < 0)
  {
    printf("Write failed.\n");
    exit();
  }
  printf("Reading whatever is written within the file.\n");
  if((nbytes = DfsInodeReadBytes(inodeHandle,buffer,0,768)) < 0)
  {
    printf("Read failed.\n");
    exit();
  }
  printf("Buffer = ");
  printf(buffer);
  printf("\n");
  if((ret = DfsInodeDelete(inodeHandle)) < 0)
  {
    printf("Inode Delete failed.\n");
    exit();
  }
  printf("Inode with handle = %d\n successfully deleted.\n",inodeHandle);
  printf("********************Test 2 ended*********************\n");  


  printf("*************************************************\n");
  printf("Test 3 : Writing and reading aligned bytes for single direct addressing \n");
  printf("*************************************************\n");

  inodeHandle = DfsInodeOpen("Test1.txt");
  if(inodeHandle == DFS_FAIL)
  {
    printf("Not able to allocate inodes.\n");
    exit();
  }
  printf("The returned inodeHandle = %d\n",inodeHandle);
  printf("Writing only 512*15 bytes.\n");
  for(i = 0; i < 15; i++)
  {
    if((nbytes = DfsInodeWriteBytes(inodeHandle,integer,i*512,512)) < 0)
    {
      printf("Write failed.\n");
      exit();
    }
  }

  printf("Reading whatever is written within the file.\n");
  if((nbytes = DfsInodeReadBytes(inodeHandle,buffer,0,(512*15))) < 0)
  {
    printf("Read failed.\n");
    exit();
  }
  printf("App() : Read bytes = %d\n",nbytes);
  printf("Buffer = ");
  printf(buffer);
  printf("Buffer len = %d\n",dstrlen(buffer));
  printf("\n");
  if((ret = DfsInodeDelete(inodeHandle)) < 0)
  {
    printf("Inode Delete failed.\n");
    exit();
  }
  printf("Inode with handle = %d\n successfully deleted.\n",inodeHandle);
  printf("********************Test 3 ended*********************\n");  


printf("*************************************************\n");
  printf("Test 4 : Writing and reading unaligned bytes for single direct addressing \n");
  printf("*************************************************\n");

  inodeHandle = DfsInodeOpen("Test2.txt");
  if(inodeHandle == DFS_FAIL)
  {
    printf("Not able to allocate inodes.\n");
    exit();
  }
  printf("The returned inodeHandle = %d\n",inodeHandle);
  printf("Writing only 512*15 bytes.\n");
  for(i = 0; i < 15; i++)
  {
    if((nbytes = DfsInodeWriteBytes(inodeHandle,integer,i*512,512)) < 0)
    {
      printf("Write failed.\n");
      exit();
    }
  }
  if((nbytes = DfsInodeWriteBytes(inodeHandle,integer,i*512,408)) < 0)
    {
      printf("Write failed.\n");
      exit();
    }
  printf("Reading whatever is written within the file.\n");
  if((nbytes = DfsInodeReadBytes(inodeHandle,buffer,0,(512*15 + 10))) < 0)
  {
    printf("Read failed.\n");
    exit();
  }
  printf("App() : Read bytes = %d\n",nbytes);
  printf("Buffer = ");
  printf(buffer);
  printf("Buffer len = %d\n",dstrlen(buffer));
  printf("\n");
  if((ret = DfsInodeDelete(inodeHandle)) < 0)
  {
    printf("Inode Delete failed.\n");
    exit();
  }
  printf("Inode with handle = %d\n successfully deleted.\n",inodeHandle);
  printf("********************Test 4 ended*********************\n");  
  #endif


printf("*************************************************\n");
  printf("Test 5 : Writing and reading unaligned bytes for second direct addressing \n");
  printf("*************************************************\n");

  inodeHandle = DfsInodeOpen("Test2.txt");
  if(inodeHandle == DFS_FAIL)
  {
    printf("Not able to allocate inodes.\n");
    exit();
  }
  printf("The returned inodeHandle = %d\n",inodeHandle);
  printf("Writing only 512*160 + 408 bytes.\n");
  for(i = 0; i < 160; i++)
  {
    if((nbytes = DfsInodeWriteBytes(inodeHandle,integer,i*512,512)) < 0)
    {
      printf("Write failed.\n");
      exit();
    }
  }
  if((nbytes = DfsInodeWriteBytes(inodeHandle,integer,i*512,408)) < 0)
    {
      printf("Write failed.\n");
      exit();
    }
  printf("Reading whatever is written within the file.\n");
  for(i = 0; i < 161; i++)
  {
    if((nbytes = DfsInodeReadBytes(inodeHandle,buffer,0,(512))) < 0)
    {
      printf("Write failed.\n");
      
      exit();
    }
      //printf("Read bytes = %d\n",nbytes);
      //printf("Buffer = ");
      //printf(buffer);
      //printf("Buffer len = %d\n",dstrlen(buffer));
      //printf("\n");
      total_bytes += nbytes;
  }
  printf("Total bytes read = %d\n",total_bytes);


 
  if((ret = DfsInodeDelete(inodeHandle)) < 0)
  {
    printf("Inode Delete failed.\n");
    exit();
  }
  printf("Inode with handle = %d\n successfully deleted.\n",inodeHandle);
  printf("********************Test 5 ended*********************\n");  


  printf("*************************************************\n");
  printf("Test 6 : Writing and reading aligned bytes for single direct addressing and checking consistency in FS.\n");
  printf("*************************************************\n");

  inodeHandle = DfsInodeOpen("Test1.txt");
  if(inodeHandle == DFS_FAIL)
  {
    printf("Not able to allocate inodes.\n");
    exit();
  }
  printf("The returned inodeHandle = %d\n",inodeHandle);
  printf("Writing only 512*15 bytes.\n");
  for(i = 0; i < 15; i++)
  {
    if((nbytes = DfsInodeWriteBytes(inodeHandle,integer,i*512,512)) < 0)
    {
      printf("Write failed.\n");
      exit();
    }
  }


  printf("Reading whatever is written within the file.\n");
  if((nbytes = DfsInodeReadBytes(inodeHandle,buffer,0,(512*15))) < 0)
  {
    printf("Read failed.\n");
    exit();
  }
  printf("App() : Read bytes = %d\n",nbytes);
  printf("Buffer = ");
  printf(buffer);
  printf("Buffer len = %d\n",dstrlen(buffer));
  printf("\n");

  if((ret = DfsInodeDelete(inodeHandle)) < 0)
  {
    printf("Inode Delete failed.\n");
    exit();
  }
  printf("Inode with handle = %d\n successfully deleted.\n",inodeHandle);
  printf("********************Test 6 ended*********************\n");  

  
  /*fd = FileOpen("test1.txt","rw");
  printf("The returned fd = %d\n",fd);
  fd = FileOpen("test1.txt","r");
  if(fd == -1)
  {
    exit();
  }
  FileClose(fd);
  
 
  

  //ret = DfsAllocateBlock();
  //DfsFreeBlock(ret);
  /*ret = DfsReadBlock(37,&d);
  bzero(d.data,DFS_BLOCKSIZE);
  dstrncpy(d.data,test,dstrlen(test));
  ret = DfsWriteBlock(80,&d);
  ret = DfsReadBlock(80,&d);
  bzero(d.data,DFS_BLOCKSIZE);
  ret = DfsWriteBlock(80,&d); 
  ret = DfsReadBlock(80,&d);*/
/*fd = FileOpen("test2.txt","rw");
//ret = DfsInodeOpen("bigFile1.txt");
i = FileWrite(fd,integer,20);
i = FileSeek(fd,20,FILE_SEEK_CUR);
i = FileWrite(fd,integer,20);
i = FileSeek(fd,20,FILE_SEEK_CUR);
FileClose(fd);

fd = FileOpen("test2.txt","rw");
//ret = DfsInodeOpen("bigFile1.txt");
i = FileRead(fd,buffer,80);
printf("buffer = ");
printf(buffer);
printf("\n");
FileClose(fd);
FileDelete("test2.txt");*/


//ret_1 = DfsInodeOpen("bigFile2.txt");

//ret = DfsInodeOpen("BiggerFile1.txt");
//exit();
/*DfsInodeAllocateVirtualBlock(ret, 0);
DfsInodeWriteBytes(ret,test,0,dstrlen(test));
DfsInodeReadBytes(ret,buffer,0,dstrlen(test));
printf("ostests() : Buffer : ");
printf(buffer);
printf("\n");
DfsInodeWriteBytes(ret,test_2,dstrlen(test),dstrlen(test_2));
DfsInodeReadBytes(ret,buffer,0,200);
printf("ostest() : Buffer : ");
printf(buffer);
printf("\n");*/

#if 0
for(i=0;i<2;i++)
{
    DfsInodeWriteBytes(ret,test,DfsInodeFilesize(ret),dstrlen(test));
    DfsInodeReadBytes(ret,buffer,0,DfsInodeFilesize(ret));
    printf("ostests() : Reading fs block number = %d\n",block_num);
    printf("ostests() : Read from hello.txt = ");
    printf(buffer);
    printf("\n");
    DfsInodeWriteBytes(ret,test_2,DfsInodeFilesize(ret),dstrlen(test_2));
    DfsInodeReadBytes(ret,buffer,0,DfsInodeFilesize(ret));
    printf("ostests() : Reading fs block number = %d\n",block_num);
    printf("ostests() : Read from hello.txt = ");
    printf(buffer);
    printf("\n");
    
}
  DfsInodeWriteBytes(ret,integer,DfsInodeFilesize(ret),dstrlen(integer));
  DfsInodeReadBytes(ret,buffer,dstrlen(test) + dstrlen(test_2),1024);
  printf("ostests() : Reading fs block number after loop = %d\n",block_num);
  printf("ostests() : Read from hello.txt = ");
  printf(buffer);
  printf("\n");
  printf("Before ending.\n");
  #endif
//inodes[ret].singleIndirectBlockNum = DfsAllocateBlock();

#if 0
for(i = 0; i < 16; i++)
{
  DfsInodeWriteBytes(ret,integer,i*512,512);
  printf("Exiting os traps.\n");
  //exit();
}
for(i = 0; i < 24; i++)
{
  DfsInodeWriteBytes(ret_1,integer,i*512,512);
  printf("Exiting os traps.\n");
  //exit();
}
DfsInodeReadBytes(ret,buffer,0,512*16);
  //printf("ostests() : Reading fs block number after loop = %d\n",block_num);
  printf("ostests() : Read from hello.txt = ");
  printf(buffer);
  printf("\n");
  printf("Before ending.\n");
  printf("strlen(buffer+ = %d\n",dstrlen(buffer));

DfsInodeReadBytes(ret_1,buffer,0,512*24);
  //printf("ostests() : Reading fs block number after loop = %d\n",block_num);
  printf("ostests() : Read from hello.txt = ");
  printf(buffer);
  printf("\n");
  printf("Before ending.\n");
  printf("strlen(buffer+ = %d\n",dstrlen(buffer));
#endif
/*bzero(d.data,DFS_BLOCKSIZE);
for(i = 0; i < 500; i++)
{
    block = DfsAllocateBlock();
    bcopy((char*)&block,d.data + (i*4),sizeof(int));
    DfsWriteBlock(inodes[ret].singleIndirectBlockNum,&d);
}*/

//ret1 = DfsInodeOpen("hello_1.txt");
//ret2 = DfsInodeOpen("hello_2.txt");
//DfsInodeDelete(ret2);
//DfsInodeDelete(ret1);

//DfsInodeDelete(ret);
//ret = DfsInodeOpen("hello.txt");

}

