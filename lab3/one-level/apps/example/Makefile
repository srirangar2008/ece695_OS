default:
	cd makeprocs; make
	cd hello_world; make
	cd test_1; make
	cd test_2; make
	cd test_3; make
	cd test_4; make
	cd test_5; make
	cd test_6; make
	

clean:
	cd makeprocs; make clean
	cd hello_world; make clean
	cd test_1; make clean
	cd test_2; make clean
	cd test_3; make clean
	cd test_4; make clean
	cd test_5; make clean
	cd test_6; make clean
	

run:
	#cd ../../bin; dlxsim -D m -x os.dlx.obj -a -D mpi -u makeprocs.dlx.obj 2; ee469_fixterminal
	cd ../../bin; dlxsim -x os.dlx.obj -a -u makeprocs.dlx.obj 1; ee469_fixterminal
	sleep 5
	cd ../../bin; dlxsim -x os.dlx.obj -a -u makeprocs.dlx.obj 2; ee469_fixterminal
	sleep 5
	cd ../../bin; dlxsim -x os.dlx.obj -a -u makeprocs.dlx.obj 3; ee469_fixterminal
	sleep 5
	cd ../../bin; dlxsim -x os.dlx.obj -a -u makeprocs.dlx.obj 4; ee469_fixterminal
	sleep 5
	cd ../../bin; dlxsim -x os.dlx.obj -a -u makeprocs.dlx.obj 5; ee469_fixterminal
	sleep 5
	cd ../../bin; dlxsim -x os.dlx.obj -a -u makeprocs.dlx.obj 6; ee469_fixterminal
	sleep 5
	
