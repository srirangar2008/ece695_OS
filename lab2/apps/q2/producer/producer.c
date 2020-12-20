#include "lab2-api.h"
#include "usertraps.h"
#include "misc.h"

#include "spawn.h"

void main (int argc, char *argv[])
{
  missile_code *mc; 
  cBuf* cBufPtr;       // Used to access missile codes in shared memory page
  uint32 h_mem;            // Handle to the shared memory page
  sem_t s_procs_completed; // Semaphore to signal the original process that we're done
  int lock_status;
  int pid = Getpid();
  const char word [BUFFER_SIZE - 1]= "Hello World";
  int charCounter = 0;

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
  
  while(1)
  {
    int inserted = 0;
    int available = 1;
    char charInput;
    while(!inserted)
    {
      
      
      //Printf("producer %d : The lock status is now %d\n",pid,lock_status);
      //Printf("producer %d : The value of tail = %d The value of head = %d\n",pid,cBufPtr->tail, cBufPtr->head);
      /*lock_status = lock_acquire(cBufPtr->lock);
        if (((cBufPtr->head + 1) % BUFFER_SIZE) == cBufPtr->tail)
        {
           available = 0;
        }
      lock_status = lock_release(cBufPtr->lock);*/
      charInput = word[charCounter];
      if(((cBufPtr->head + 1) % BUFFER_SIZE) != cBufPtr->tail)
      {
        lock_status = lock_acquire(cBufPtr->lock);
      
        
        
        //Printf("producer: My PID is %d\n", Getpid());
        //Printf("producer %d realeasing the lock\n",pid);
        inserted = 1;
        cBufPtr->buf[cBufPtr->head] = charInput;
        cBufPtr->head = (cBufPtr->head + 1) % BUFFER_SIZE;
        
        //Printf("producer %d: The lock status is now %d\n",pid,lock_status);
        lock_status = lock_release(cBufPtr->lock);
      }
      
      if(inserted)
      {
          
        Printf("Producer %d : inserted %c\n",pid,charInput);
        
        charCounter += 1;
        break;
      }
    }
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
