#include "usertraps.h"
#include "files_shared.h"

void run_file_tests()
{
  
  int fd;
  int nbytes;
  int ret;
  char buffer[15000];
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

  Printf("-----------------------------------------------------------------\n");
  Printf("-----------------------------------------------------------------\n");
  Printf("*****************Running the file tests now.*********************\n");
  Printf("-----------------------------------------------------------------\n");
  Printf("-----------------------------------------------------------------\n");
  Printf("\n");
  Printf("*********************************************************************\n");
  Printf("*********** Test1 : writing within direct addressing.****************\n");
  Printf("*********************************************************************\n");
  fd = file_open("Ranga_test.txt","rw");
  Printf("Handle retuenred = %d\n",fd);
  nbytes = file_write(fd,integer,512);
  if(nbytes == -1)
  {
    Printf("Write failed.\n");
    Exit();
  }
  Printf("Number of bytes written = %d\n",nbytes);
  ret = file_seek(fd,0,FILE_SEEK_SET);
  Printf("Fileposition after file_seek = %d\n", ret);
  nbytes = file_read(fd,buffer,512);
  Printf("Number of bytes read = %d\n",nbytes);
  Printf("Buffer = ");
  //Printf(buffer);
  Printf("\n");
  if(dstrncmp(buffer,integer,512) == 0)
  {
    Printf("It is found that the bytes read and the bytes written are matching.\n");
    Printf("The tests passed.\n");
  }
  ret = file_close(fd);
  Printf("ret after file_close = %d\n", ret);
  Printf("*********************************************************************\n");
  Printf("************           Test1 ended          *************************\n");
  Printf("*********************************************************************\n");

  Printf("*********************************************************************\n");
  Printf("*********** Test2 : Opening a written file in read only mode..****************\n");
  Printf("*********** Test2 : Trying to write to that file.             ****************\n");
  Printf("*********** Test2 : Trying to delete the opened file without file_close*******\n");
  Printf("*********************************************************************\n");

  fd = file_open("Ranga_test.txt","r");
  Printf("Handle retuenred = %d\n",fd);
  ret = file_seek(fd,0,FILE_SEEK_END);
  Printf("filepos = %d\n",ret);
  nbytes = file_write(fd,integer,100);


  ret = file_delete("Ranga_test.txt");
  Printf("ret after file_delete = %d\n", ret);

  Printf("*********************************************************************\n");
  Printf("************           Test2 ended          *************************\n");
  Printf("*********************************************************************\n");


  Printf("*********************************************************************\n");
  Printf("***********Test3 : Closing the file and deleting it.*****************\n");
  Printf("*********************************************************************\n");
  Printf("Closing the file.\n");
  ret = file_close(fd);
  Printf("Deleting the file.\n");
  ret = file_delete("Ranga_test.txt");
  Printf("*********************************************************************\n");
  Printf("************           Test3 ended          *************************\n");
  Printf("*********************************************************************\n");
}

void main (int argc, char *argv[])
{
  run_os_tests();

  run_file_tests();
  
}
