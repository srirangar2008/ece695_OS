#ifndef	_memory_h_
#define	_memory_h_

// Put all your #define's in memory_constants.h
#include "memory_constants.h"

extern int lastosaddress; // Defined in an assembly file
char PpageCounters[FREEMAP_BITS][32];
//--------------------------------------------------------
// Existing function prototypes:
//--------------------------------------------------------

int MemoryGetSize();
void MemoryModuleInit();
uint32 MemoryTranslateUserToSystem (PCB *pcb, uint32 addr);
int MemoryMoveBetweenSpaces (PCB *pcb, unsigned char *system, unsigned char *user, int n, int dir);
int MemoryCopySystemToUser (PCB *pcb, unsigned char *from, unsigned char *to, int n);
int MemoryCopyUserToSystem (PCB *pcb, unsigned char *from, unsigned char *to, int n);


//---------------------------------------------------------
// Put your function prototypes here
//---------------------------------------------------------
void PrintPPageCounters();
int MemoryPageFaultHandler(PCB *pcb);
void ROPAccessHandler(PCB *pcb);
void MemoryFreeL2Table(uint32*);

#endif	// _memory_h_
