
#include "usertraps.h"
#include "misc.h"
#include "tests.h"

#define HELLO_WORLD "hello_world.dlx.obj"

#define SIZE_IN_MB(x) x*1024*1024
#define PAGE_SIZE 4*1024

void main(int argc, char *argv[])
{

    sem_t s_procs_completed; // Semaphore to signal the original process that we're done
    char str[20] = "Hello World";
    int *ptr;
    int ret = 0;
    sem_t s_procs_completed_helloworld;             // Semaphore used to wait until all spawned processes have completed
    char s_procs_completed_str_helloworld[10];      // Used as command-line argument to pass page_mapped handle to new processes
    int num_hello_world = 100;
    int i = 0;
    char s_procs_completed_str[10];
    
    if (argc != 2) { 
        Printf("Usage: %s <handle_to_procs_completed_semaphore>\n"); 
        Exit();
    } 
    if ((s_procs_completed_helloworld = sem_create(0)) == SYNC_FAIL) {
    Printf("makeprocs (%d): Bad sem_create\n", getpid());
    Exit();
  }
    //Printf("The sem ID created for helloworld in test5 = %d\n",s_procs_completed_helloworld);

  // Convert the command-line strings into integers for use as handles
    s_procs_completed = dstrtol(argv[1], NULL, 10);
    
    ditoa(s_procs_completed_helloworld, s_procs_completed_str_helloworld);
    //ditoa(s_procs_completed, s_procs_completed_str);
    
    //int* ptr = (int*)malloc(sizeof(int) * SIZE_IN_MB(2));
    Printf("Test5() : Creating 100 helloworld processes.\n");
    Printf("-------------------------------------------------------------------------------------\n");
    Printf("makeprocs (%d): Creating %d hello world's in a row, but only one runs at a time\n", getpid(), num_hello_world);
    for(i=0; i<num_hello_world; i++) {
        Printf("makeprocs (%d): Creating hello world #%d\n", getpid(), i);
        process_create(HELLO_WORLD, s_procs_completed_str_helloworld, NULL);
        if (sem_wait(s_procs_completed_helloworld) != SYNC_SUCCESS) {
        Printf("Bad semaphore s_procs_completed (%d) in %s\n", s_procs_completed_helloworld, argv[0]);
        Exit();
        }
        Printf("Completed %dth process.\n",i);
    }
    Printf("Test5() : done.\n");
    Printf("-------------------------------------------------------------------------------------\n");



    if(sem_signal(s_procs_completed) != SYNC_SUCCESS) {
    Printf("hello_world (%d): Bad semaphore s_procs_completed (%d)!\n", getpid(), s_procs_completed);
    Exit();
    }
    
}