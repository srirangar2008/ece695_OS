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
    int num_h2o_molecules;
    int num_n2_molecules;
    int i = 0;
    int num_no2_molecule_produced = 0;
    
    int h2o_required = 0;
  if (argc != 5) { 
    Printf("Usage: "); Printf(argv[0]); Printf(" <handle_to_shared_memory_page> <handle_to_page_mapped_semaphore> <number of n2 molecules> <number of h2o molecules>\n"); 
    Exit();
  } 

  // Convert the command-line strings into integers for use as handles
  h_mem = dstrtol(argv[1], NULL, 10); // The "10" means base 10
  s_procs_completed = dstrtol(argv[2], NULL, 10);
  num_n2_molecules = dstrtol(argv[3],NULL,10);
  num_h2o_molecules = dstrtol(argv[4],NULL,10);
  //Printf("s_procs_completed = %d\n",s_procs_completed);
  h2o_required = num_h2o_molecules / 2;

  // Map shared memory page into this process's memory space
  if ((kryp = (krypton *)shmat(h_mem)) == NULL) {
    Printf("Could not map the virtual address to the memory in "); Printf(argv[0]); Printf(", exiting...\n");
    Exit();
  }

  //Printf("Strating the creationg of %d N2 molecules now.\n",num_h2o_molecules);
  
 
  // Now print a message to show that everything worked
  //Printf("spawn_me: This is one of the %d instances you created.  ", mc->numprocs);
  //Printf("spawn_me: Missile code is: %c\n", mc->really_important_char);
  //Printf("producer %d acquiring the lock\n",pid);
  //Printf("Entering while loop.\n");
  if(num_h2o_molecules < 2)
  {
    Printf("Not enough H2O molecules to produce NO2.\n");
  }
  else if(num_n2_molecules < 1)
  {
    Printf("Not enough NO2 molecules to produce NO2.\n");
  }
  else
  {
    while(1)
    {
      int j = 0;
      sem_wait(kryp->molecule_o2);
      sem_wait(kryp->atom_n);
      //Printf("Got N and O2  molecules.\n");
      sem_signal(kryp->molecule_no2);
      
      num_no2_molecule_produced += 1;
      Printf("An NO2 molecule is created.\n");
      
      
      //for(j = 0; j < 1000; j++);
      //Printf("NO2 : h2o required = %d\n",h2o_required);
      //Printf("No2 : NO2 produced = %d\n",num_no2_molecule_produced);
      if(num_no2_molecule_produced == h2o_required || num_no2_molecule_produced == 2*num_n2_molecules)
        break;
    
    
    }
  }
  
  

  // Signal the semaphore to tell the original process that we're done
  //Printf("Produced %d NO2 molecules\n", num_no2_molecule_produced);
  if(sem_signal(s_procs_completed) != SYNC_SUCCESS) {
    Printf("Bad semaphore s_procs_completed (%d) in ", s_procs_completed); Printf(argv[0]); Printf(", exiting...\n");
    Exit();
  }
  
}
