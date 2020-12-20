#This is the README for lab4.

How to build : 

For flat : 
    1. Building the OS :
        mainframer.sh "cd os && make clean && make"
    2. Building the fdisk : 
        mainframer.sh "cd apps/fdisk && make clean && make"
    3. Building the ostests : 
        mainframer.sh "cd apps/ostests && make clean && make"

    Note : The tests for verifying dfs functions and the file functions are written in apps/ostests/ostests.c . 
        The verification of dfs functions is implemented within the os at os/osests.c and is called by a trap from the userspace app 
        at apps/ostests/ostests.c. 
        The verification of userspace filesystem APIs are written in apps/ostests/ostests.c. 

    Running the programs : 
        1. Running fdisk :
            cd apps/fdisk; make run; cd -
        2. Running ostests : 
            cd apps/ostests; make run; cd -
    Note : The fdisk program can be run even without the actual disk image being present as well as after creating the disk image to 
    format it during subsequent runs.

For multilevel : 
    1. Building the OS :
        mainframer.sh "cd os && make clean && make"
    2. Building the fdisk : 
        mainframer.sh "cd apps/fdisk && make clean && make"

    Note : A few preliminary tests with minimal print information is available at apps/example/test/test.c. 

    Running the programs : 
        1. Running fdisk :
            cd apps/fdisk; make run; cd -
    Note : The fdisk program can be run even without the actual disk image being present as well as after creating the disk image to 
    format it during subsequent runs.

