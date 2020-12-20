#include "usertraps.h"
#include "misc.h"



void main (int argc, char *argv[])
{
  int ret;
  char readBuf[100];
  int fd;
  int mode = (UX | OX | UW | OW);
  char buffer[50] = "Hi my name is Ranga.\n";


  Printf("Hello World!\n");
  ret = mkdir("/test",mode);
  fd = file_open("/test/ranga.txt","rw");
  Printf("fd = %d\n",fd);
  
  ret = file_write(fd,buffer,30);
  file_close(fd);
  // if(ret == -1)
  // {
  //   Exit();
  // }
  // Printf("0x%x\n",UX | OX | UW | OW);
  // //Exit();
  ret = mkdir("/test/abc",mode);
  // //Printf("/test/abc\n");
  ret = mkdir("/test2/def",mode);
  fd = file_open("/test/ranga.txt","rw");
  file_write(fd,buffer,50);
  file_seek(fd,0,FILE_SEEK_SET);
  file_read(fd,readBuf,50);
  
  Printf("REadBuf = ");
  Printf(readBuf);
  file_close(fd);

  // //Printf("mkdir(test/abc/xyz,mode)\n");
  ret = mkdir("test/abc/xyz",mode);
  ret = mkdir("/test/abc/xyz/omnop",mode);
  ret = mkdir("/test/xyz/lmnop",mode);

  ret = rmdir("test/abc/xyz/omnop");
  ret = rmdir("test/abc/xyz");
  ret = rmdir("/test");
  ret = rmdir("/");
  ret = file_open("/test","rw");
  ret = rmdir("/test/abc");
  if(ret < 0)
    Exit();

  ret = file_link("/test/ranga.txt","hello.txt");
  fd = file_open("hello.txt","rw");
  Printf("ret = %d\n",ret);
  if(ret <= -1)
  {
    Printf("File Fail\n");
    Exit();
  }
  ret = file_read(fd, readBuf,30);
  ret = file_seek(fd, 0, FILE_SEEK_END);
  ret = file_write(fd, buffer, 30);
  ret = file_close(fd);

  Printf("ReadBuf = \n");
  Printf(readBuf);
  Printf("\n");

  fd = file_open("/test/ranga.txt","rw");
  ret = file_read(fd,readBuf, 60);
  Printf("ReadBuf = \n");
  Printf(readBuf);
  Printf("\n");
  ret = file_close(fd);


  ret = file_delete("/test/ranga.txt");
  ret = rmdir("/test");
 
  
  ret = rmdir("/test/abc");
  Printf("Removing the /test directory.\n");
  ret = rmdir("/test");
  ret = file_open("/test/ranga.txt","rw");
  // /*ret = mkdir("/test/test2/test3",0x3f);*/
  // ret = file_open("/test/test.txt","rw");
  // file_write(ret,buffer,40);
  // file_close("/test/test.txt");
  //Printf("main (%d): Formatted DFS disk for %d bytes.\n", getpid(), disksize);
}





