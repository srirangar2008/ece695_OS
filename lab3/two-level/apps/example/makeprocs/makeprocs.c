#include "usertraps.h"
#include "misc.h"

#define HELLO_WORLD "hello_world.dlx.obj"
#define TEST_1 "test_1.dlx.obj"
#define TEST_2 "test_2.dlx.obj"
#define TEST_3 "test_3.dlx.obj"
#define TEST_4 "test_4.dlx.obj"
#define TEST_5 "test_5.dlx.obj"
#define TEST_6 "test_6.dlx.obj"

void delay(int n)
{
  int i = 0;
  for( i = 0; i < n; i++);
}

void test1()
{

    //sem_t s_procs_completed; // Semaphore to signal the original process that we're done
    char str[20] = "Hello World";
    
    /*if (argc != 2) { 
        Printf("Usage: %s <handle_to_procs_completed_semaphore>\n"); 
        Exit();
    } */
    Printf("Entered test1.\n");
  // Convert the command-line strings into integers for use as handles
    //s_procs_completed = dstrtol(argv[1], NULL, 10);
    
    //Printf("Test1 starting.\n");
    Printf("Test1() : "); Printf(str);Printf("\n");
    //Printf("Test1() : completed.\n");

    /*if(sem_signal(s_procs_completed) != SYNC_SUCCESS) {
    Printf("hello_world (%d): Bad semaphore s_procs_completed (%d)!\n", getpid(), s_procs_completed);
    Exit();
    }*/
    
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

  //Printf("Hello world.\n");
  //Exit(0);
  
  

  // Convert string from ascii command line argument to integer number
  //num_hello_world = dstrtol(argv[1], NULL, 10); // the "10" means base 10
  //Printf("makeprocs (%d): Creating %d hello_world processes\n", getpid(), num_hello_world);

  // Setup the command-line arguments for the new processes.  We're going to
  // pass the handles to the semaphore as strings
  // on the command line, so we must first convert them from ints to strings.
  ditoa(s_procs_completed, s_procs_completed_str);
  testcase_num = dstrtol(argv[1], NULL, 10); // the "10" means base 10

  if(testcase_num == 6)
  {
    if ((s_procs_completed = sem_create(-28)) == SYNC_FAIL) 
    {
    Printf("makeprocs (%d): Bad sem_create\n", getpid());
    Exit();
    }
  }
  else
  {
  // Create semaphore to not exit this process until all other processes 
  // have signalled that they are complete.
    if ((s_procs_completed = sem_create(0)) == SYNC_FAIL) {
      Printf("makeprocs (%d): Bad sem_create\n", getpid());
      Exit();
    }
  }



  


  switch(testcase_num)
  {

    case 1:
          //Create Hello World processes
          Printf("-------------------------------------------------------------------------------------\n");
          Printf("************************Test1 Starting.**********************************************\n");
          //Printf("makeprocs (%d): Creating %d hello world's in a row, but only one runs at a time\n", getpid(), num_hello_world);
          //for(i=0; i<num_hello_world; i++) {
          Printf("makeprocs (%d): Creating test process 1\n", getpid());
          //test1();
          process_create(TEST_1, s_procs_completed_str, NULL);
          Printf("Completed creation.\n");
          if (sem_wait(s_procs_completed) != SYNC_SUCCESS) {
            Printf("Bad semaphore s_procs_completed (%d) in %s\n", s_procs_completed, argv[0]);
            Exit();
          }
          break;

    case 2:
          Printf("-------------------------------------------------------------------------------------\n");
          Printf("************************Test2 Starting.**********************************************\n");
          Printf("makeprocs (%d): Creating test process 2 \n", getpid());
          process_create(TEST_2, s_procs_completed_str, NULL);
          if (sem_wait(s_procs_completed) != SYNC_SUCCESS) {
            Printf("Bad semaphore s_procs_completed (%d) in %s\n", s_procs_completed, argv[0]);
            Exit();
          }
          break;

    case 3:
          Printf("-------------------------------------------------------------------------------------\n");
          Printf("************************Test3 Starting.**********************************************\n");
          Printf("makeprocs (%d): Creating test process 3\n", getpid());
          process_create(TEST_3, s_procs_completed_str, NULL);
          if (sem_wait(s_procs_completed) != SYNC_SUCCESS) {
            Printf("Bad semaphore s_procs_completed (%d) in %s\n", s_procs_completed, argv[0]);
            Exit();
          }
          break;
    case 4:
          Printf("-------------------------------------------------------------------------------------\n");
          Printf("************************Test4 Starting.**********************************************\n");
          Printf("makeprocs (%d): Creating test process 4\n", getpid());
          process_create(TEST_4, s_procs_completed_str, NULL);
          if (sem_wait(s_procs_completed) != SYNC_SUCCESS) {
            Printf("Bad semaphore s_procs_completed (%d) in %s\n", s_procs_completed, argv[0]);
            Exit();
          }
          break;
  

    case 5:
          Printf("-------------------------------------------------------------------------------------\n");
          Printf("************************Test5 Starting.**********************************************\n");
          Printf("makeprocs (%d): Creating test process 5\n", getpid());
          process_create(TEST_5, s_procs_completed_str, NULL);
          if (sem_wait(s_procs_completed) != SYNC_SUCCESS) {
            Printf("Bad semaphore s_procs_completed (%d) in %s\n", s_procs_completed, argv[0]);
            Exit();
          }
          break;

    case 6:
          Printf("-------------------------------------------------------------------------------------\n");
          Printf("************************Test6 Starting.**********************************************\n");
          for(temp = 0; temp < 30; temp++)
          {
            process_create(TEST_6, s_procs_completed_str, NULL);
          }
          if (sem_wait(s_procs_completed) != SYNC_SUCCESS) {
            Printf("Bad semaphore s_procs_completed (%d) in %s\n", s_procs_completed, argv[0]);
            Exit();
          }
          break;


    default : 
          Printf("The number should be between 1 and 6\n");
          
          break;
  }

  
  Printf("makeprocs (%d): All other processes completed, exiting main process.\n", getpid());
  
  //delay(10000000);
}
