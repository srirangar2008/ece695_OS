default:
	cd fdisk; make

clean:
	cd fdisk; make clean

run:
	cd ../../os; make; cd ../bin; dlxsim -x os.dlx.obj -a -D r -u fdisk.dlx.obj 512; ee469_fixterminal
