#include "lab2-api.h"
#include "usertraps.h"
#include "misc.h"

#include "spawn.h"

void main (int argc, char *argv[])
{
  //Printf("Entered the mainfunction of makeprocs.c\n");
  int numprocs = 0;               // Used to store number of processes to create
  int i;                          // Loop index variable
  //missile_code *mc;      
  cBuf* cBufPtr;         // Used to get address of shared memory page
  uint32 h_mem;                   // Used to hold handle to shared memory page
  sem_t s_procs_completed;        // Semaphore used to wait until all spawned processes have completed
  char h_mem_str[10];             // Used as command-line argument to pass mem_handle to new processes
  char s_procs_completed_str[10]; // Used as command-line argument to pass page_mapped handle to new processes

  int j = 0;
  char* temp;
  const char word[BUFFER_SIZE-1] = "Hello World";
//  Printf("Entered the mainfunction of makeprocs.c\n");
  if (argc != 2) {
    Printf("Usage: "); Printf(argv[0]); Printf(" <number of processes to create>\n");
    Exit();
  }
  
    
  // Convert string from ascii command line argument to integer number
  numprocs = dstrtol(argv[1], NULL, 10) ; // the "10" means base 10
 // Printf("Creating %d processes\n", numprocs);

  // Allocate space for a shared memory page, which is exactly 64KB
  // Note that it doesn't matter how much memory we actually need: we 
  // always get 64KB
  if ((h_mem = shmget()) == 0) {
    Printf("ERROR: could not allocate shared memory page in "); Printf(argv[0]); Printf(", exiting...\n");
    Exit();
  }

  // Map shared memory page into this process's memory space
  if ((cBufPtr = (cBuf *)shmat(h_mem)) == NULL) {
    Printf("Could not map the shared page to virtual address in "); Printf(argv[0]); Printf(", exiting..\n");
    Exit();
  }

  // Put some values in the shared memory, to be read by other processes
  //mc->numprocs = numprocs;
  //mc->really_important_char = 'A';

  cBufPtr->head = 0;
  cBufPtr->tail = 0;
  //temp = dstrncpy(cBufPtr->buf,word,dstrlen(word));
  //Printf("Temp = %c\n",temp[0]);
  //Printf("cBufPtr->buf = %c\n",cBufPtr->buf[0]);
  cBufPtr->lock = lock_create();


  // Create semaphore to not exit this process until all other processes 
  // have signalled that they are complete.  To do this, we will initialize
  // the semaphore to (-1) * (number of signals), where "number of signals"
  // should be equal to the number of processes we're spawning - 1.  Once 
  // each of the processes has signaled, the semaphore should be back to
  // zero and the final sem_wait below will return.
  //Printf("%d\n",-1 * (numprocs - 1));
  
  if ((s_procs_completed = sem_create(-1 * ((numprocs*2)-1)) == SYNC_FAIL)) {
    Printf("Bad sem_create in "); Printf(argv[0]); Printf("\n");
    Exit();
  }

  // Setup the command-line arguments for the new process.  We're going to
  // pass the handles to the shared memory page and the semaphore as strings
  // on the command line, so we must first convert them from ints to strings.
  ditoa(h_mem, h_mem_str);
  ditoa(s_procs_completed, s_procs_completed_str);

  // Now we can create the processes.  Note that you MUST end your call to
  // process_create with a NULL argument so that the operating system
  // knows how many arguments you are sending.
  for(i=0; i<numprocs; i++) {
    process_create(PRODUCER, h_mem_str, s_procs_completed_str, NULL);
   // Printf("Process %d producer created\n", i);
    process_create(CONSUMER, h_mem_str, s_procs_completed_str, NULL);
   // Printf("Process %d consumer created\n", i);
  }

  // And finally, wait until all spawned processes have finished.
  if(sem_wait(s_procs_completed) != SYNC_SUCCESS) {
    Printf("Bad semaphore s_procs_completed (%d) in ", s_procs_completed); Printf(argv[0]); Printf("\n");
    Exit();
  }
  Printf("All other processes completed, exiting main process.\n");
}
