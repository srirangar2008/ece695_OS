#ifndef	_memory_constants_h_
#define	_memory_constants_h_

//------------------------------------------------
// #define's that you are given:
//------------------------------------------------

// We can read this address in I/O space to figure out how much memory
// is available on the system.
#define	DLX_MEMSIZE_ADDRESS	0xffff0000

// Return values for success and failure of functions
#define MEM_SUCCESS 1
#define MEM_FAIL -1

//--------------------------------------------------------
// Put your constant definitions related to memory here.
// Be sure to prepend any constant names with "MEM_" so 
// that the grader knows they are defined in this file.
// Feel free to edit the constants as per your needs.
//--------------------------------------------------------

// 4KB pages, so offset requires 12 bits (positions 11 to 0)
// Least significant bit of page number is at position ??
// 32 bit VA and 4KB pages = 2^12 => 32-12 = 20, 2^20 pages. offset = 11 to 0

#define MEM_L1FIELD_FIRST_BITNUM 12
#define MEM_L2FIELD_FIRST_BITNUM MEM_L1FIELD_FIRST_BITNUM

/* // 1024KB virtual memory size.  so max address is 1<<20 - 1 */
#define MEM_MAX_VIRTUAL_ADDRESS 0xFFFFF

#define MEM_PTE_READONLY 0x4
#define MEM_PTE_DIRTY 0x2
#define MEM_PTE_VALID 0x1

#define MEM_MAX_SIZE (0x1 << 21)

#define MEM_PAGESIZE (0x1 << MEM_L1FIELD_FIRST_BITNUM) 
#define MEM_L1TABLE_SIZE ((MEM_MAX_VIRTUAL_ADDRESS + 1) >> MEM_L1FIELD_FIRST_BITNUM)
#define MEM_ADDRESS_OFFSET_MASK (MEM_PAGESIZE - 1) 

#define MEM_MAX_PHYS_MEM (0x1 << 21)
#define MEM_MAX_PAGES (MEM_MAX_PHYS_MEM / MEM_PAGESIZE)

#define MEM_PTE_MASK ~(MEM_PTE_READONLY | MEM_PTE_DIRTY | MEM_PTE_VALID)

#define NUM_PAGES (MEM_MAX_SIZE / MEM_PAGESIZE) 
#define FREEMAP_BITS (NUM_PAGES/32)

#endif	// _memory_constants_h_
