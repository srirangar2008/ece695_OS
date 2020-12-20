default:
	cd makeprocs; make
	cd inject-n2; make
	cd inject-h2o; make
	cd consume-n2; make
	cd consume-h2o; make
	cd consume-n-o2; make

clean:
	cd makeprocs; make clean
	cd inject-n2; make clean
	cd inject-h2o; make clean
	cd consume-n2; make clean
	cd consume-h2o; make clean
	cd consume-n-o2; make clean

run:
	cd ../../bin; dlxsim -x os.dlx.obj -a -u makeprocs.dlx.obj 10 10; ee469_fixterminal
