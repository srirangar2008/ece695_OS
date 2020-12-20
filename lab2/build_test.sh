#! /bin/bash

echo $1
OS="os"
Q2="q2"
Q3="q3"
Q4="q4"
Q5="q5"
CLEAN="clean"
RUN="run"
function domake()
{
	echo "Making $1"
	mainframer.sh "cd $1 && make"
}

function doclean()
{
	echo "Make clean $1"
	mainframer.sh "cd $1 && make clean"
}

case $1 in	

	$OS)
	doclean $1
	domake $1
	#mainframer.sh 'cd os && make clean'
	#mainframer.sh 'cd os && make'
	;;
	
	$Q2 | $Q3 | $Q4 | $Q5)
	doclean "apps/$1"
	domake "apps/$1"
	;;
	
	$CLEAN)
	doclean $OS
	doclean "apps/$Q2"
	doclean "apps/$Q3"
	doclean "apps/$Q4"
	doclean "apps/$Q5"
	;;

	$RUN)
	cd "apps/$2"; make run; cd -
	;;

	*)
	echo "default case"
	;;
esac

echo " done "
