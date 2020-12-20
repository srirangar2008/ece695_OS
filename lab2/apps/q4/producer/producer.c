#include "lab2-api.h"
#include "usertraps.h"
#include "misc.h"

#include "spawn.h"

void main (int argc, char *argv[])
{
  cBuf* cBufPtr;       // Used to access missile codes in shared memory page
  uint32 h_mem;            // Handle to the shared memory page
  sem_t s_procs_completed; // Semaphore to signal the original process that we're done
  int lock_status;
  int pid = Getpid();
  const char word [BUFFER_SIZE]= "Hello World";
  int charCounter = 0;
  char charInput;
   int inserted = 0;

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
  //Printf("producer %d acquiring the lock\n",pid);
  
  while(charCounter != dstrlen("Hello World"))
  {
      charInput = word[charCounter];
      //Printf("Producer = %d : CharInput = %c\n",pid,charInput);
      //if(((cBufPtr->head + 1) % BUFFER_SIZE) != cBufPtr->tail)
      //{
      //sem_wait_status = sem_wait(cBufPtr->s_emptyslots);
      
        lock_status = lock_acquire(cBufPtr->lock);
        if(((cBufPtr->head + 1) % BUFFER_SIZE) == cBufPtr->tail)
        {
          cond_wait(cBufPtr->c_notFull);
        }
        else
        {
          cBufPtr->buf[cBufPtr->head] = charInput;
          //Printf("producer %d : The value of tail = %d The value of head = %d\n",pid,cBufPtr->tail, cBufPtr->head);
          cBufPtr->head = (cBufPtr->head + 1) % BUFFER_SIZE;
          
          inserted = 1;
          
          Printf("Producer %d : inserted %c\n",pid,charInput);
          charCounter += 1;
        //sem_signal_status = sem_signal(cBufPtr->s_fullslots);
          cond_signal(cBufPtr->c_notEmpty);
//          cond_broadcast(cBufPtr->c_notEmpty);
          //lock_status = lock_release(cBufPtr->lock);
        }
        
    lock_status = lock_release(cBufPtr->lock);
        
    if (charCounter == dstrlen("Hello World"))
      break;
    
    
  }

  // Signal the semaphore to tell the original process that we're done
//  Printf("producer %d is complete.\n", pid);
  if(sem_signal(s_procs_completed) != SYNC_SUCCESS) {
    Printf("Bad semaphore s_procs_completed (%d) in ", s_procs_completed); Printf(argv[0]); Printf(", exiting...\n");
    Exit();
  }
  
}
