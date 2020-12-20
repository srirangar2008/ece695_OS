
#include "usertraps.h"
#include "misc.h"
#include "tests.h"

#define SIZE_IN_MB(x) x*1024*1024
#define PAGE_SIZE 4*1024

void main(int argc, char *argv[])
{

    sem_t s_procs_completed; // Semaphore to signal the original process that we're done
    char str[20] = "Hello World";
    int *ptr;
    
    if (argc != 2) { 
        Printf("Usage: %s <handle_to_procs_completed_semaphore>\n"); 
        Exit();
    } 

  // Convert the command-line strings into integers for use as handles
    s_procs_completed = dstrtol(argv[1], NULL, 10);
    
   
    
    //int* ptr = (int*)malloc(sizeof(int) * SIZE_IN_MB(2));
    Printf("The address of ptr = %d\n",ptr);
    
    Printf("Test2() : Accessing memory beyond the max virtual address.\n");
    *(ptr + SIZE_IN_MB(1) + 4) = 10;

    if(sem_signal(s_procs_completed) != SYNC_SUCCESS) {
    Printf("hello_world (%d): Bad semaphore s_procs_completed (%d)!\n", getpid(), s_procs_completed);
    Exit();
    }
    
}