//
//	memory.c
//
//	Routines for dealing with memory management.

//static char rcsid[] = "$Id: memory.c,v 1.1 2000/09/20 01:50:19 elm Exp elm $";

#include "ostraps.h"
#include "dlxos.h"
#include "process.h"
#include "memory.h"
#include "queue.h"



//const int num_pages = MEM_MAX_SIZE / MEM_PAGESIZE;

static uint32 freemap[FREEMAP_BITS];
static uint32 pagestart;
static int nfreepages;
static int freemapmax;
//USed to maintain the counter values per page.
extern char PpageCounters[FREEMAP_BITS][32];


void PrintPPageCounters();
//----------------------------------------------------------------------
//
//	This silliness is required because the compiler believes that
//	it can invert a number by subtracting it from zero and subtracting
//	an additional 1.  This works unless you try to negate 0x80000000,
//	which causes an overflow when subtracted from 0.  Simply
//	trying to do an XOR with 0xffffffff results in the same code
//	being emitted.
//
//----------------------------------------------------------------------
static int negativeone = 0xFFFFFFFF;
static inline uint32 invert (uint32 n) {
  return (n ^ negativeone);
}

//----------------------------------------------------------------------
//
//	MemoryGetSize
//
//	Return the total size of memory in the simulator.  This is
//	available by reading a special location.
//
//----------------------------------------------------------------------
int MemoryGetSize() {
  return (*((int *)DLX_MEMSIZE_ADDRESS));
}


//----------------------------------------------------------------------
//
//	MemoryInitModule
//
//	Initialize the memory module of the operating system.
//      Basically just need to setup the freemap for pages, and mark
//      the ones in use by the operating system as "VALID", and mark
//      all the rest as not in use.
//
//----------------------------------------------------------------------
void MemoryModuleInit() {
  uint32 os_numpages;
  uint32 offset_32bit;
  uint32 num_32bit;
  int i = 0, j =0;
  
  MemoryInfo();
  dbprintf('m',"Entered Memory Module Init.\n");
  for(i = 0; i < FREEMAP_BITS; i++)
  {
    freemap[i] = -1;
  }
  dbprintf('r', "Setting the counter values for pages\n");
  for(i = 0; i < FREEMAP_BITS; i++)
  {
    for(j = 0; j < 32; j++)
    {
      PpageCounters[i][j] = 0;
    }
  }
  

  freemapmax = MemoryGetSize() / MEM_PAGESIZE;
  dbprintf('m', "MemoryModuleInit() : Current Memory available = %d\n",MemoryGetSize());
  dbprintf('m', "MemoryModuleInit() : Number of pages available = %d\n",freemapmax);
  nfreepages = freemapmax;
  dbprintf('m', "MemoryModuleInit() : OS last address = %p(Hex) , in dec=%d\n",lastosaddress,lastosaddress);
  os_numpages = (lastosaddress + 1) / MEM_PAGESIZE;
  
  if(lastosaddress % MEM_PAGESIZE)
  {
    os_numpages += 1;
  }
  dbprintf('m', "MemoryModuleInit() : os_numpages = %d\n",os_numpages);
  offset_32bit = os_numpages % 32;
  num_32bit = os_numpages / 32;
  for(i = 0; i < num_32bit; i++)
  {
    freemap[i] = 0;
  }

  freemap[num_32bit] = negativeone << offset_32bit;
  nfreepages = nfreepages - ((num_32bit * 32) + offset_32bit);
  dbprintf('m', "MemoryModuleInit() : Free pages available = %d\n",nfreepages);
  for(i = 0; i < num_32bit; i++)
  {
    for(j = 0; j < 32; j++)
    {
      PpageCounters[i][j] = 1;
    }
  }
  for(i = 0; i < offset_32bit; i++)
  {
    PpageCounters[num_32bit][i] = 1;
  }
  //PrintPPageCounters();
}


//----------------------------------------------------------------------
//
// MemoryTranslateUserToSystem
//
//	Translate a user address (in the process referenced by pcb)
//	into an OS (physical) address.  Return the physical address.
//
//----------------------------------------------------------------------
uint32 MemoryTranslateUserToSystem (PCB *pcb, uint32 addr) {
  uint32 pagenum = addr >> (MEM_L1FIELD_FIRST_BITNUM);
  uint32 offset = addr & (MEM_ADDRESS_OFFSET_MASK);
  uint32 physicalAddress = ((pcb->pagetable[pagenum]) & ~(MEM_ADDRESS_OFFSET_MASK)) | offset;
  dbprintf('m',"MemoryTranslate() : vaddr = 0x%x\n",addr);
  
  dbprintf('m',"MemoryTranslate() : pagenum = %d\n",pagenum);
  
  dbprintf('m',"MemoryTranslate() : offset = 0x%x\n",offset);
  
  dbprintf('m',"MemoryTranslate() : physicalAddress = 0x%x\n",physicalAddress);
  return physicalAddress;
}


//----------------------------------------------------------------------
//
//	MemoryMoveBetweenSpaces
//
//	Copy data between user and system spaces.  This is done page by
//	page by:
//	* Translating the user address into system space.
//	* Copying all of the data in that page
//	* Repeating until all of the data is copied.
//	A positive direction means the copy goes from system to user
//	space; negative direction means the copy goes from user to system
//	space.
//
//	This routine returns the number of bytes copied.  Note that this
//	may be less than the number requested if there were unmapped pages
//	in the user range.  If this happens, the copy stops at the
//	first unmapped address.
//
//----------------------------------------------------------------------
int MemoryMoveBetweenSpaces (PCB *pcb, unsigned char *system, unsigned char *user, int n, int dir) {
  unsigned char *curUser;         // Holds current physical address representing user-space virtual address
  int		bytesCopied = 0;  // Running counter
  int		bytesToCopy;      // Used to compute number of bytes left in page to be copied

  while (n > 0) {
    // Translate current user page to system address.  If this fails, return
    // the number of bytes copied so far.
    curUser = (unsigned char *)MemoryTranslateUserToSystem (pcb, (uint32)user);

    // If we could not translate address, exit now
    if (curUser == (unsigned char *)0) break;

    // Calculate the number of bytes to copy this time.  If we have more bytes
    // to copy than there are left in the current page, we'll have to just copy to the
    // end of the page and then go through the loop again with the next page.
    // In other words, "bytesToCopy" is the minimum of the bytes left on this page 
    // and the total number of bytes left to copy ("n").

    // First, compute number of bytes left in this page.  This is just
    // the total size of a page minus the current offset part of the physical
    // address.  MEM_PAGESIZE should be the size (in bytes) of 1 page of memory.
    // MEM_ADDRESS_OFFSET_MASK should be the bit mask required to get just the
    // "offset" portion of an address.
    bytesToCopy = MEM_PAGESIZE - ((uint32)curUser & MEM_ADDRESS_OFFSET_MASK);
    
    // Now find minimum of bytes in this page vs. total bytes left to copy
    if (bytesToCopy > n) {
      bytesToCopy = n;
    }

    // Perform the copy.
    if (dir >= 0) {
      bcopy (system, curUser, bytesToCopy);
    } else {
      bcopy (curUser, system, bytesToCopy);
    }

    // Keep track of bytes copied and adjust addresses appropriately.
    n -= bytesToCopy;           // Total number of bytes left to copy
    bytesCopied += bytesToCopy; // Total number of bytes copied thus far
    system += bytesToCopy;      // Current address in system space to copy next bytes from/into
    user += bytesToCopy;        // Current virtual address in user space to copy next bytes from/into
  }
  return (bytesCopied);
}

//----------------------------------------------------------------------
//
//	These two routines copy data between user and system spaces.
//	They call a common routine to do the copying; the only difference
//	between the calls is the actual call to do the copying.  Everything
//	else is identical.
//
//----------------------------------------------------------------------
int MemoryCopySystemToUser (PCB *pcb, unsigned char *from,unsigned char *to, int n) {
  return (MemoryMoveBetweenSpaces (pcb, from, to, n, 1));
}

int MemoryCopyUserToSystem (PCB *pcb, unsigned char *from,unsigned char *to, int n) {
  return (MemoryMoveBetweenSpaces (pcb, to, from, n, -1));
}

//---------------------------------------------------------------------
// MemoryPageFaultHandler is called in traps.c whenever a page fault 
// (better known as a "seg fault" occurs.  If the address that was
// being accessed is on the stack, we need to allocate a new page 
// for the stack.  If it is not on the stack, then this is a legitimate
// seg fault and we should kill the process.  Returns MEM_SUCCESS
// on success, and kills the current process on failure.  Note that
// fault_address is the beginning of the page of the virtual address that 
// caused the page fault, i.e. it is the vaddr with the offset zero-ed
// out.
//
// Note: The existing code is incomplete and only for reference. 
// Feel free to edit.
//---------------------------------------------------------------------
int MemoryPageFaultHandler(PCB *pcb) {
  
  uint32 addr = pcb->currentSavedFrame[PROCESS_STACK_FAULT];
  uint32 ppagenum;
  uint32 stackPointerPage;
  printf("Entered the MemPageFaultHandler\n");
  printf("The fault address = 0x%x\n",addr);
  printf("StackPointer = 0x%x\n",pcb->currentSavedFrame[PROCESS_STACK_USER_STACKPOINTER]);
  printf("Stack pointer page = 0x%x\n",(pcb->currentSavedFrame[PROCESS_STACK_USER_STACKPOINTER] >> 12) << 12);
  stackPointerPage = (pcb->currentSavedFrame[PROCESS_STACK_USER_STACKPOINTER] >> 12) << 12;
  //printf("PageTableHandler() :pcb->pagetable[((addr + MEM_PAGESIZE)>> 12)] = 0x%x\n",pcb->pagetable[((addr + MEM_PAGESIZE)>> 12)]);

  if (addr == stackPointerPage)
  {
    ppagenum = MemoryAllocPage(); 
    pcb->pagetable[(addr >> 12)] = MemorySetupPte(ppagenum); 
    printf("Allocated a page with vpagenum = 0x%x\n",addr>>12);
    printf("Allocated page base address = 0x%x\n",pcb->pagetable[(addr >> 12)]);
    dbprintf('m', "Returning from page fault handler\n"); 
    return MEM_SUCCESS; 
  } 
  
  // segfault if the faulting address is not part of the stack
  if (addr < pcb->currentSavedFrame[PROCESS_STACK_USER_STACKPOINTER]) { 
     printf("addr = %x\nsp = %x\n", addr, pcb->currentSavedFrame[PROCESS_STACK_USER_STACKPOINTER]); 
     printf("FATAL ERROR (%d): segmentation fault at page address %x\n", findpid(pcb), addr); 
     ProcessKill(); 
     return MEM_FAIL; 
  } 

  //if (pcb->pagetable[((addr + MEM_PAGESIZE)>> 12)] != 0)
  //{
    ppagenum = MemoryAllocPage(); 
    pcb->pagetable[(addr >> 12)] = MemorySetupPte(ppagenum); 
    printf("Allocated a page with vpagenum = 0x%x\n",addr>>12);
    dbprintf('m', "Returning from page fault handler\n"); 
    return MEM_SUCCESS; 
  //}

   
  //return MEM_FAIL; 
}


//---------------------------------------------------------------------
// You may need to implement the following functions and access them from process.c
// Feel free to edit/remove them
//---------------------------------------------------------------------

int MemoryAllocPage(void) {
  int pagenum;
  int i = 0;
  int index;
  int pos = 0;
  dbprintf('m', "func : MemoryAllocPage() entered.\n");
  if (nfreepages == 0)
    return 0;
  for(i = 0; i < FREEMAP_BITS; i++)
  {
    //dbprintf('r', "MemoryAllocPage() : index in freemap[%d] = %d\n",i,freemap[i]);
    dbprintf('m', "MemoryAllocPage() : freemap[%d] = %x\n",i,freemap[i]); 
    if(freemap[i] > 0)
      
      break;
  }
  if(i == FREEMAP_BITS)
  {
    printf("MemoryAllocPage() : No memory available.\n");
    return 0;
  }
  
  dbprintf('r', "MemoryAllocPage() : freemap[%d] = %x\n",i,freemap[i]); 
  for(index = 0x01, pos = 0; index != -1; index = index << 1, pos++)
  {
    dbprintf('r', "MemoryAllocPage() : index = 0x%x and freemap[%d] = 0x%x\n",index,i,freemap[i]);
    
    if(index & freemap[i])
    {
      
      //pos+1 beacuse, pos would be the first non zero entry.
      //To ensure that entry is 0, we have to increment pos by 1.
      //freemap[i] = freemap[i] & (-1 << pos+1);
      freemap[i] = freemap[i] << 1;
      //dbprintf('r',"MemoryAllocPage() : mask = 0x%x\n",(-1 << pos+1));
      dbprintf('r',"MemoryAllocPage() : pos = %d , index = %x\n",pos,index);
      dbprintf('r',"MemoryAllocPage() : freemap[%d] = %x\n",i,freemap[i]);
      break;
    }
  }
  nfreepages -= 1; 
  dbprintf('r', "PageStartAddress = 0x%x\n", ((i*32) + pos) * MEM_PAGESIZE );
  dbprintf('r',"Successfully allocated 4KB of memory with pagenumber = %d.\n",(i*32) + pos);
  dbprintf('r', "Returned page number = %d\n",(i*32) + pos);
  
  PpageCounters[i][pos] += 1;
  dbprintf('r',"PpageCounter[%d][%d] = %d\n",i,pos,PpageCounters[i][pos]);
  //PrintPPageCounters();
  return (i*32) + pos;
}


uint32 MemorySetupPte (uint32 page) {
  dbprintf('m', "SetupPte() : return = %x\n",((page * MEM_PAGESIZE) | MEM_PTE_VALID));
  return ((page * MEM_PAGESIZE) | MEM_PTE_VALID);
}


void MemoryFreePage(uint32 page) {
  uint32 numPage = page / 32;
  uint32 offset = page % 32;
  dbprintf('r',"MemoryFreePage() : Page to be freed = %d\n",page);
  
  //printf("MemoryFreePage() : Freeing page number = %d\n";
  dbprintf('r',"Before : freemap[%d] = %x\n",numPage,freemap[numPage]);
  freemap[numPage] = (1 << offset) | freemap[numPage] ;
  dbprintf('r',"After : freemap[%d] = %x\n",numPage,freemap[numPage]);
  nfreepages += 1;
  dbprintf('r',"nFreepages = %d\n",nfreepages);
  PpageCounters[numPage][offset] -= 1;
  dbprintf('r',"PpageCounters[numPage][offset] = %d\n",PpageCounters[numPage][offset]);
}

void MemoryInfo()
{
  dbprintf('r', "Max Virtal Memory Address = %x\n",MEM_MAX_VIRTUAL_ADDRESS);
  dbprintf('r', "Max mem size = %d, in Hex = %x\n",MEM_MAX_SIZE,MEM_MAX_SIZE);
  dbprintf('r', "Mem page size = %d, in Hex = %x\n",MEM_PAGESIZE,MEM_PAGESIZE);
  dbprintf('r', "Page Table Size = %d, in Hex = %x\n",MEM_L1TABLE_SIZE, MEM_L1TABLE_SIZE);
  dbprintf('r', "Mex pages = %d, in Hex = %x\n",MEM_MAX_PAGES,MEM_MAX_PAGES);
  dbprintf('r', "Address offset mask = 0x%x\n",MEM_ADDRESS_OFFSET_MASK);
  dbprintf('r',"Required number of entries in freemap = %d\n",FREEMAP_BITS);
  //exit();
}

void PrintPPageCounters()
{
  int i = 0, j = 0;
  for(i = 0; i < FREEMAP_BITS; i++)
  {
    for(j = 0; j < 32; j++)
    {
      dbprintf('r',"PpageCounters[%d][%d] = %d\n",i,j,PpageCounters[i][j]);
      printf("PpageCounters[%d][%d] = %d\n",i,j,PpageCounters[i][j]);
    }
  }
  
}

void ROPAccessHandler(PCB *pcb)
{
  int count;
  int retVal;
  int offset, pagerow;
  int i = 0;
  int newpagetable;
  int pageframe = (pcb->pagetable[pcb->currentSavedFrame[PROCESS_STACK_FAULT] >> MEM_L1FIELD_FIRST_BITNUM]) >> MEM_L1FIELD_FIRST_BITNUM; 
  printf("The page which caused the exception is 0x%x\n",pcb->currentSavedFrame[PROCESS_STACK_FAULT]);
  printf("The shared page = 0x%x\n",pcb->pagetable[pcb->currentSavedFrame[PROCESS_STACK_FAULT] >> MEM_L1FIELD_FIRST_BITNUM] );
  printf("The shared frame =%d\n",pageframe);
  pagerow = pageframe / 32;
  offset = pageframe % 32;
  count = PpageCounters[pagerow][offset];
  printf("PpageCounters[%d][%d] = %d\n",pagerow,offset,count);
  if(count > 1)
  {
    printf("The shared page is shared bw %d processes.\n");
    retVal = MemoryAllocPage();
    if(retVal == 0)
    {
      printf("ERR : ROPACCESSHandler : Memory Alloc() failed.\n");
      exit();
    }
    else
    {
      newpagetable = MemorySetupPte(retVal);
      printf("Copying the entire page byte by byte\n");
      //copying the items
      bcopy((char*)((pcb->pagetable[pcb->currentSavedFrame[PROCESS_STACK_FAULT] >> MEM_L1FIELD_FIRST_BITNUM]) & MEM_PTE_MASK), \
                    (char*)(newpagetable & MEM_PTE_MASK),MEM_PAGESIZE);
      pcb->pagetable[pcb->currentSavedFrame[PROCESS_STACK_FAULT] >> MEM_L1FIELD_FIRST_BITNUM] =  \
                    (pcb->pagetable[pcb->currentSavedFrame[PROCESS_STACK_FAULT] >> MEM_L1FIELD_FIRST_BITNUM] & ~MEM_PTE_READONLY);
      printf("old pagetable entry = 0x%x\n",pcb->pagetable[pcb->currentSavedFrame[PROCESS_STACK_FAULT] >> MEM_L1FIELD_FIRST_BITNUM]);
      pcb->pagetable[pcb->currentSavedFrame[PROCESS_STACK_FAULT] >> MEM_L1FIELD_FIRST_BITNUM] = newpagetable;
      printf("new pagetable entry = 0x%x\n",pcb->pagetable[pcb->currentSavedFrame[PROCESS_STACK_FAULT] >> MEM_L1FIELD_FIRST_BITNUM]);
      
      PpageCounters[pagerow][offset] -= 1;
      printf("PpageCounters[%d][%d] = %d\n",pagerow,offset,PpageCounters[pagerow][offset]);
    }
    
  }
  if(count == 1)
  {
    //Setting to readwrite, no need to copy.
     pcb->pagetable[pcb->currentSavedFrame[PROCESS_STACK_FAULT] >> MEM_L1FIELD_FIRST_BITNUM] =  \
                    (pcb->pagetable[pcb->currentSavedFrame[PROCESS_STACK_FAULT] >> MEM_L1FIELD_FIRST_BITNUM] & ~MEM_PTE_READONLY);
  }
  
  
  //exit();
}

