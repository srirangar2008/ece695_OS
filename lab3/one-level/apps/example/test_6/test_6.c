
#include "usertraps.h"
#include "misc.h"
#include "tests.h"

#define HELLO_WORLD "hello_world.dlx.obj"

#define SIZE_IN_MB(x) x*1024*1024
#define PAGE_SIZE 4*1024

void delay(unsigned int n)
{
  int  i = 0;
  for(i = 0; i<n;i++);
  //for(i = 0; i<n;i++);
  //for(i = 0; i<n;i++);
}

void main(int argc, char *argv[])
{

    sem_t s_procs_completed; // Semaphore to signal the original process that we're done
    char s_procs_completed_str[10];
    int pid = getpid();
    if (argc != 2) { 
        Printf("Usage: %s <handle_to_procs_completed_semaphore>\n"); 
        Exit();
    } 


  // Convert the command-line strings into integers for use as handles
    s_procs_completed = dstrtol(argv[1], NULL, 10);

    Printf("test6(%d) : Multiple process test!!!\n",getpid());
    Printf("test6(%d) : Started process with pid = %d in test6()\n",pid,pid);
    delay(500000);
    Printf("test6(%d) : done.\n",getpid());


    if(sem_signal(s_procs_completed) != SYNC_SUCCESS) {
    Printf("hello_world (%d): Bad semaphore s_procs_completed (%d)!\n", getpid(), s_procs_completed);
    Exit();
    }
    
}
