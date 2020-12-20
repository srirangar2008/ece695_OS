
#include "usertraps.h"
#include "misc.h"
#include "tests.h"

void main(int argc, char *argv[])
{

    sem_t s_procs_completed; // Semaphore to signal the original process that we're done
    char str[20] = "Hello World";
    
    if (argc != 2) { 
        Printf("Usage: %s <handle_to_procs_completed_semaphore>\n"); 
        Exit();
    } 

  // Convert the command-line strings into integers for use as handles
    s_procs_completed = dstrtol(argv[1], NULL, 10);
    
    //Printf("Test1 starting.\n");
    Printf("Test1() : "); Printf(str);Printf("\n");
    //Printf("Test1() : completed.\n");

    if(sem_signal(s_procs_completed) != SYNC_SUCCESS) {
    Printf("hello_world (%d): Bad semaphore s_procs_completed (%d)!\n", getpid(), s_procs_completed);
    Exit();
    }
    
}