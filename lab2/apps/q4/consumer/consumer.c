#include "lab2-api.h"
#include "usertraps.h"
#include "misc.h"

#include "spawn.h"

void main (int argc, char *argv[])
{
  cBuf* cBufPtr;       // Used to access missile codes in shared memory page
  uint32 h_mem;            // Handle to the shared memory page
  sem_t s_procs_completed; // Semaphore to signal the original process that we're done
  int pid = Getpid();
  int lock_status;
  int itemsConsumed = 0;
  char temp[BUFFER_SIZE];
   int consumed = 0;
    char charConsumed;

  if (argc != 3) { 
    Printf("Usage: "); Printf(argv[0]); Printf(" <handle_to_shared_memory_page> <handle_to_page_mapped_semaphore>\n"); 
    Exit();
  } 
  

  // Convert the command-line strings into integers for use as handles
  h_mem = dstrtol(argv[1], NULL, 10); // The "10" means base 10
  s_procs_completed = dstrtol(argv[2], NULL, 10);
  //Printf("s_procs_completed = %d\n",s_procs_completed);
  

  // Map shared memory page into this process's memory space
  if ((cBufPtr = (cBuf *)shmat(h_mem)) == NULL) {
    Printf("Could not map the virtual address to the memory in "); Printf(argv[0]); Printf(", exiting...\n");
    Exit();
  }

  
 
  // Now print a message to show that everything worked
  //Printf("spawn_me: This is one of the %d instances you created.  ", mc->numprocs);
  //Printf("spawn_me: Missile code is: %c\n", mc->really_important_char);
  
  while(itemsConsumed != dstrlen("Hello World"))
  {
   
      //sem_wait_status = sem_wait(cBufPtr->s_fullslots);
        lock_status = lock_acquire(cBufPtr->lock);
        
        if(cBufPtr->head == cBufPtr->tail)
        {
          //Printf("Buffer empty..\n");
          //Printf("Consumer calling cond_wait()\n");
          cond_wait(cBufPtr->c_notEmpty);
        }
        else
        {
          temp[itemsConsumed] = cBufPtr->buf[cBufPtr->tail];
        
          charConsumed = cBufPtr->buf[cBufPtr->tail];
          cBufPtr->tail = (cBufPtr->tail + 1) % BUFFER_SIZE;
          //Printf("COnsumer calling cond_signal()\n");
          
          consumed  = 1;
          //Printf("consumer %d : The value of tail = %d The value of head = %d\n",pid,cBufPtr->tail, cBufPtr->head);
          //Printf("consumer %d : The lock status is now %d\n",pid,lock_status);
          

          Printf("Consumer %d : removed %c\n",pid,charConsumed);
          
          itemsConsumed += 1;
          cond_signal(cBufPtr->c_notFull);
  //        lock_status = lock_release(cBufPtr->lock);
        }
      //sem_signal_status = sem_signal(cBufPtr->s_emptyslots);
    
          lock_status = lock_release(cBufPtr->lock);
    if(itemsConsumed == dstrlen("Hello World"))
      break;
  }
  // Signal the semaphore to tell the original process that we're done
  //Printf("consumer %d is complete.\n",pid);
  //Printf("Consumer %d consumed -> ");Printf(temp);Printf("\n");
  if(sem_signal(s_procs_completed) != SYNC_SUCCESS) {
    Printf("Bad semaphore s_procs_completed (%d) in ", s_procs_completed); Printf(argv[0]); Printf(", exiting...\n");
    Exit();
  }
}
