#!/bin/bash

file_path=../../../files/$1
ctw=../../ctw/ctw
ppm=../../code/ppmd-i1/PPMd.exe
encoder=../hpzip.opt
decoder=../hpunzip.opt
tmpfile=/tmp/tabulator_tmp

bpcs="2 k "
count=0

echo ",,Time,Space,Dec. Time,OK"
cd $file_path

for i in *; do
   	base=`basename $i`
	echo -n $base
	echo -n ",ppm"

	rm -f xxx.pmd
 	time=`(/usr/bin/time -f%E $ppm e -m256 -o16 -fxxx.pmd $base > /dev/null) 2>&1`
    echo -n ",$time"

    size=`du -b xxx.pmd | awk '{print $1}'`
    echo -n ",$size"

	rm $i
   	decTime=`(/usr/bin/time -f%E $ppm d xxx.pmd > /dev/null) 2>&1`
	echo -n ",$decTime"

    sizeOrig=`du -b $i | awk '{print $1}'`
	echo -n ","
	bpc=`dc -e "2 k $size 8 * $sizeOrig / p"`
	bpcs+="$bpc ";
	if [[ $count -gt 0 ]]; then
		bpcs+="+ ";
	fi
	echo $bpc;
	count=$(($count+1));
done

bpcs+="$count / p";
echo "$bpcs" | dc;
echo

rm -f xxx.pmd
cd - > /dev/null
 

