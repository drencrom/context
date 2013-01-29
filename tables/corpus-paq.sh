#!/bin/bash

file_path=../../../files/$1
ctw=../../ctw/ctw
ppm=../../code/ppmd-i1-jorge/PPMd.exe
paq=../../paq/zpaq102
unpaq=../../paq/unzpaq102
paqfile=../../paq/max.cfg
encoder=../hpzip.opt
decoder=../hpunzip.opt
tmpfile=/tmp/tabulator_tmp

bpcs="2 k "
count=0

echo ",,Time,Space,Dec. Time,OK"

for i in "$file_path"/*; do
	echo -n `basename $i`
	echo -n ",ctw"

	time=`(/usr/bin/time -f%E $paq c$paqfile $i.paq $i > /dev/null) 2>&1`
    echo -n ",$time"

    size=`du -b $i.paq | awk '{print $1}'`
    echo -n ",$size"

	decTime=`(/usr/bin/time -f%E $unpaq x $i.paq > /dev/null) 2>&1`
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

rm -f $file_path/*.ctw
rm -f $file_path/*.d
