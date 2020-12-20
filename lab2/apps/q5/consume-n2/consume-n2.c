#include "lab2-api.h"
#include "usertraps.h"
#include "misc.h"

#include "spawn.h"

void main (int argc, char *argv[])
{
  //missile_code *mc; 
  krypton *kryp;
  uint32 h_mem;            // Handle to the shared memory page
  sem_t s_procs_completed; // Semaphore to signal the original process that we're done
  int lock_status;
  int pid = Getpid();
  const char word [BUFFER_SIZE]= "Hello World";
  int charCounter = 0;
  int sem_wait_status;
  int sem_signal_status;
  char charInput;
    int inserted = 0;
    int available = 1;
    int num_n2_molecules;
    int i = 0;
    int num_n_atom_produced = 0;

  if (argc != 4) { 
    Printf("Usage: "); Printf(argv[0]); Printf(" <handle_to_shared_memory_page> <handle_to_page_mapped_semaphore> <number of n2 molecules>\n"); 
    Exit();
  } 

  // Convert the command-line strings into integers for use as handles
  h_mem = dstrtol(argv[1], NULL, 10); // The "10" means base 10
  s_procs_completed = dstrtol(argv[2], NULL, 10);
  num_n2_molecules = dstrtol(argv[3],NULL,10);
  //Printf("s_procs_completed = %d\n",s_procs_completed);
  

  // Map shared memory page into this process's memory space
  if ((kryp = (krypton *)shmat(h_mem)) == NULL) {
    Printf("Could not map the virtual address to the memory in "); Printf(argv[0]); Printf(", exiting...\n");
    Exit();
  }

  //Printf("Strating the creationg of %d N2 molecules now.\n",num_n2_molecules);
  
 
  // Now print a message to show that everything worked
  //Printf("spawn_me: This is one of the %d instances you created.  ", mc->numprocs);
  //Printf("spawn_me: Missile code is: %c\n", mc->really_important_char);
  //Printf("producer %d acquiring the lock\n",pid);
  //Printf("Entering while loop.\n");
  if(num_n2_molecules < 1)
  {
    Printf("Not enough N2 molecules.\n");
  }
  else
  {
    while(1)
    {
      int j = 0;
      sem_wait(kryp->molecule_n2);
      //Printf("Got a N2 molecule.\n");
      sem_signal(kryp->atom_n);
      num_n_atom_produced += 1;
      Printf("An N molecule is created.\n");
      sem_signal(kryp->atom_n);
      num_n_atom_produced += 1;
      Printf("An N molecule is created.\n");
      
      //for(j = 0; j < 1000; j++);
      if(num_n_atom_produced == 2*num_n2_molecules)
        break;
    
    
    }

  }
  
  
  // Signal the semaphore to tell the original process that we're done
  //Printf("Produced %d N atoms\n", num_n_atom_produced);
  if(sem_signal(s_procs_completed) != SYNC_SUCCESS) {
    Printf("Bad semaphore s_procs_completed (%d) in ", s_procs_completed); Printf(argv[0]); Printf(", exiting...\n");
    Exit();
  }
  
}
