default:
	cd makeprocs; make
	
clean:
	cd makeprocs; make clean
	#cd tests/test1; make clean

run:
	#cd ../../bin; dlxsim -D m -x os.dlx.obj -a -D mpi -u makeprocs.dlx.obj 2; ee469_fixterminal
	cd ../../bin; dlxsim -x os.dlx.obj -a -u makeprocs.dlx.obj 1; ee469_fixterminal
	
