This is the README file for problem 5 in lab3.

Files changed in the os:
        1. memory.c
        2. process.c
        3. traps.c
        4. memory.h
        5. process.h
        6. traps.h
        7. usertraps.s
        8. usertraps.h
        9. memory_constants.h

Files added/modified in apps/example :
        1. makeprocs.c -> Has the main code to select and run different testcases.
        2. test_1.c -> Print "Hello World" and exit.
        3. test_2.c -> Access memory beyond the max virtual address. The dlxsim exits after this fault.
        4. test_3.c -> Accessing memory inside the virtual page but outside the allocated memory. The dlxsim exits after a seg fault.
        5. test_4.c -> Calls a recursive function which accesses memory more than the size of a page. This creates a page fault and the page fault handler is invoked. The handler allocates a new page for the user stack and the process executes till completion.
        6. test_5.c -> This test launches 100 hello_world processes one after the other and executes till completion.
        7. test_6.c -> This test launches 30 processes parallelly and waits to ensure that at some point there are 30 processes in the system without causing any memory overflows. Executes till completion.

Regarding outputs :
        1. For test4, the contents specific to the pagehandler are printed on the console to show the faulted page and the allocated page by the OS along with the current StackPointer value.

Build and run steps :

Build :

OS:
        mainframer.sh "cd os && make"

APPS :
        mainframer.sh "cd apps/example && make"

RUN :
        cd apps/example ; make run ; cd -

