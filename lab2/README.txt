A build and run shell script named build_test.sh is available in the lab2 folder. It can be used for this lab.

Note : The script calls mainframer.sh internally. To successfully execute build and run, mainframer.sh should be available in the environment.

How to build and run the solution : 

os: 
	build : ./build_test.sh os

q2:
	build : ./build_test.sh q2
	run : ./build_test.sh run q2

q3:
	build : ./build_test.sh q3
	run : ./build_test.sh run q3

q4: 
	build : ./build_test.sh q4
	run : ./build_test.sh run q4

q5:
	build: ./build_test.sh q5
	run : ./build_test.sh run q5

clean all :
	command : ./build_test.sh clean
