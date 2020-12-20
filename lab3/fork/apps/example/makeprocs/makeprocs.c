#include "usertraps.h"
#include "misc.h"




static int n = 0;

void delay(int n)
{
  int i = 0;
  for( i = 0; i < n; i++);
}

static void increment_counter(int entry)
{
    n = n + 1;
    if(entry == 0)
    {
        Printf("incremenet_counter() : from child, n = %d\n",n);
    }
    else
    {
        Printf("increment_counter() : from parent, n = %d\n",n);
    }
}  



void main (int argc, char *argv[])
{
  int testcase_num = 0;             // Used to store number of processes to create
  int i;                               // Loop index variable
  sem_t s_procs_completed;             // Semaphore used to wait until all spawned processes have completed
  char s_procs_completed_str[10];      // Used as command-line argument to pass page_mapped handle to new processes
  int temp = 0;
  int pid;
  if (argc != 2) {
    Printf("Usage: %s <test_case number>\n", argv[0]);
    Exit();
  }
  
  pid = fork();
  Printf("main() : pid = %d\n",pid);
  if(pid == 0)
  {
    Printf("Executing child.\n");
//    printPageTable();
    delay(1000000);
    temp = temp + 5;
    
    Printf("temp in child = %d\n",temp);
    increment_counter(0);
    delay(1000000);
    Printf("Child PageTable\n");
    printPageTable();
  }
  else
  {
    Printf("Executing Parent.\n");
//    printPageTable();
    temp = temp + 10;
    
    Printf("temp in parent = %d\n",temp);
    increment_counter(1);
    Printf("Parent pagetable.\n");
    printPageTable();

  }
}  
  

