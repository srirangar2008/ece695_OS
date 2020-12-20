This is the README for problems 3 and 4 in lab3.

Files changed in the os:
        1. memory.c
        2. process.c
        3. traps.c
        4. memory.h
        5. process.h
        6. traps.h
        7. usertraps.s
        8. usertraps.h

Files added/modified in apps/example:
	1. makeprocs.c -> Both the tests which require us to fork and print pagetables and access a shared variable and then print the pagetables are implemented in this file. 

What the output looks like : 
	1. Once the fork is done, and before returning from ProcessRealFork(), the page tables of the parent and the child are printed.
	2. Once the control is back in the main function, as the pid variable is common to both, there is ROP trap generated.
	3. Within the parent and the child a static function common to both are accessed and another ROP trap is generated.
	4. Once the function is completed, the page tables of both the parent and the child are printed via a system call. 
	5. The ROP handler prints out the shared page table entry and also prints out the newly allocated page for the parent/child. 

Build and run steps :

Build :

OS:
        mainframer.sh "cd os && make"

APPS :
        mainframer.sh "cd apps/example && make"

RUN :
        cd apps/example ; make run ; cd -

