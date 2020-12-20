
#include "usertraps.h"
#include "misc.h"
#include "tests.h"

#define SIZE_IN_MB(x) x*1024*1024
#define PAGE_SIZE 4*1024

int factorial(int n)
{
    //int array[1024];
    //Printf("n = %d\n",n);
    if( n == 0 || n == 1)
        return 1;
    //Printf("Finished one iteration.\n");
    return n * factorial(n-1);
}

int overflowTest()
{
    static int i;
    int j = 0;
    int reg[PAGE_SIZE];
    if(i == 2)
        return 1;
    for(j = 1; j < PAGE_SIZE; j = j << 1)
    {
        reg[j] = j;
//        Printf("reg[%d] = %d\n",j,reg[j]);
    }
    i = i + 1;
    return overflowTest();
    
}

void main(int argc, char *argv[])
{

    sem_t s_procs_completed; // Semaphore to signal the original process that we're done
    char str[20] = "Hello World";
    int *ptr;
    int ret = 0;
    
    if (argc != 2) { 
        Printf("Usage: %s <handle_to_procs_completed_semaphore>\n"); 
        Exit();
    } 

  // Convert the command-line strings into integers for use as handles
    s_procs_completed = dstrtol(argv[1], NULL, 10);
    
   
    
    //int* ptr = (int*)malloc(sizeof(int) * SIZE_IN_MB(2));
    
    
    Printf("Test4() : Causing the user function stack to grow more than 1 page.\n");
    //ret = factorial(250);
    ret = overflowTest();    
  //  Printf("ret = %d\n",ret);



    if(sem_signal(s_procs_completed) != SYNC_SUCCESS) {
    Printf("hello_world (%d): Bad semaphore s_procs_completed (%d)!\n", getpid(), s_procs_completed);
    Exit();
    }
    
}
