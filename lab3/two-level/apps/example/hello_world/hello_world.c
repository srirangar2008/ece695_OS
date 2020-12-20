#include "usertraps.h"
#include "misc.h"

void main (int argc, char *argv[])
{
  sem_t s_procs_completed_helloworld; // Semaphore to signal the original process that we're done

  if (argc != 2) { 
    Printf("Usage: %s <handle_to_procs_completed_semaphore>\n"); 
    Exit();
  } 

  // Convert the command-line strings into integers for use as handles
  s_procs_completed_helloworld = dstrtol(argv[1], NULL, 10);
  //Printf("The sem_id recvd in helloworld = %d\n",s_procs_completed_helloworld);

  // Now print a message to show that everything worked
  Printf("hello_world (%d): Hello world!\n", getpid());

  // Signal the semaphore to tell the original process that we're done
  if(sem_signal(s_procs_completed_helloworld) != SYNC_SUCCESS) {
    Printf("hello_world (%d): Bad semaphore s_procs_completed (%d)!\n", getpid(), s_procs_completed_helloworld);
    Exit();
  }

  Printf("hello_world (%d): Done!\n", getpid());
}
