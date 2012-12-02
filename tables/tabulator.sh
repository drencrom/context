#!/bin/bash

file_path=../../../files
ctw=../../../../ctw/ctw
ppm=../../code/ppmd-i1-jorge/PPMd.exe
paq=../../paq/zpaq102
unpaq=../../paq/unzpaq102
paqfile=../../paq/max.cfg
encoder=../hpzip.opt
decoder=../hpunzip.opt
tmpfile=/tmp/tabulator_tmp

sizes=( 100k 600k 1m )
#sizes=( 100k 600k 1m 2m 2.5m 4m )
data_files=( lexgrog fcpci.ko xine trident_dri.so libkabc_groupwise.so.1.0.0 libevll.so )
text_files=( texto.txt book2 bible.txt 2donq10.txt world192.txt bible.txt )


echo "DATA"
echo ",,Time,Space,Dec. Time,OK"

for ((i=0; i<${#sizes[*]}; i++)); do
	echo -n "${sizes[$i]}"
	echo -n ",Kurtz"

        time=`(/usr/bin/time -f%E $encoder $file_path/${sizes[$i]}/${data_files[$i]}  > /dev/null) 2>&1`
        echo -n ",$time"

        size=`du -b $file_path/${sizes[$i]}/${data_files[$i]}.hpz | awk '{print $1}'`
        echo -n ",$size"

       	decTime=`(/usr/bin/time -f%E $decoder $file_path/${sizes[$i]}/${data_files[$i]}.hpz $tmpfile > /dev/null) 2>&1`
	echo -n ",$decTime"

	diff $file_path/${sizes[$i]}/${data_files[$i]} $tmpfile > /dev/null 	
	if [ "$?" = "0" ]; then
		echo -n ",Yes"
	else
		echo -n ",No"
	fi
	echo

# 	echo -n "${sizes[$i]}"
# 	echo -n ",bzip"
# 	time=`(/usr/bin/time -f%E bzip2 -kf $file_path/${sizes[$i]}/${data_files[$i]} > /dev/null) 2>&1`
# 	echo -n ",$time"
#	size=`du -b $file_path/${sizes[$i]}/${data_files[$i]}.bz2 | awk '{print $1}'`
#	echo -n ",$size"
#	decTime=`(/usr/bin/time -f%E bunzip2 -kf $file_path/${sizes[$i]}/${data_files[$i]}.bz2 > /dev/null) 2>&1`
# 	echo -n ",$decTime"
# 	echo
#
# 	echo -n "${sizes[$i]}"
# 	echo -n ",ctw"
# 	time=`(/usr/bin/time -f%E $ctw e -y $file_path/${sizes[$i]}/${data_files[$i]} > /dev/null) 2>&1`
# 	echo -n ",$time"                      
#	size=`du -b $file_path/${sizes[$i]}/${data_files[$i]}.ctw | awk '{print $1}'`
#	echo -n ",$size"
#	decTime=`(/usr/bin/time -f%E $ctw d -y $file_path/${sizes[$i]}/${data_files[$i]}.ctw > /dev/null) 2>&1`
# 	echo -n ",$decTime"
# 	echo
#
# 	echo -n "${sizes[$i]}"
# 	echo -n ",ppm"
# 	cd $file_path/${sizes[$i]}
# 	rm -f xxx.pmd
# 	time=`(/usr/bin/time -f%E $ppm e -m265 -o16 -fxxx.pmd ${data_files[$i]} > /dev/null) 2>&1`
# 	echo -n ",$time"                      
#        size=`du -b xxx.pmd | awk '{print $1}'`
#        echo -n ",$size"
#	rm -f ${data_files[$i]}
#        decTime=`(/usr/bin/time -f%E $ppm d xxx.pmd > /dev/null) 2>&1`
# 	echo -n ",$decTime"
# 	cd - > /dev/null
# 	echo
#
# 	echo -n "${sizes[$i]}"
#	echo -n ",paq"
# 	time=`(/usr/bin/time -f%E $paq c$paqfile $file_path/${sizes[$i]}/${data_files[$i]}.paq $file_path/${sizes[$i]}/${data_files[$i]} > /dev/null) 2>&1`
# 	echo -n ",$time"
#	size=`du -b $file_path/${sizes[$i]}/${data_files[$i]}.paq | awk '{print $1}'`
#	echo -n ",$size"
#	decTime=`(/usr/bin/time -f%E $unpaq x $file_path/${sizes[$i]}/${data_files[$i]}.paq > /dev/null) 2>&1`
# 	echo -n ",$decTime"
# 	echo

done
echo

echo "TEXT"
echo ",,Time,Space,OK"

for ((i=0; i<${#sizes[*]}; i++)); do
	echo -n "${sizes[$i]}"
	echo -n ",Kurtz"

        time=`(/usr/bin/time -f%E $encoder $file_path/${sizes[$i]}/${text_files[$i]} > /dev/null) 2>&1`
        echo -n ",$time"

        size=`du -b $file_path/${sizes[$i]}/${text_files[$i]}.hpz | awk '{print $1}'`
        echo -n ",$size"

       	decTime=`(/usr/bin/time -f%E $decoder $file_path/${sizes[$i]}/${text_files[$i]}.hpz $tmpfile > /dev/null) 2>& 1`
	echo -n ",$decTime"

	diff $file_path/${sizes[$i]}/${text_files[$i]} $tmpfile > /dev/null 	
	if [ "$?" = "0" ]; then
		echo -n ",Yes"
	else
		echo -n ",No"
	fi
	echo

# 	echo -n "${sizes[$i]}"
# 	echo -n ",bzip"
# 	time=`(/usr/bin/time -f%E bzip2 -kf $file_path/${sizes[$i]}/${text_files[$i]} > /dev/null) 2>&1`
# 	echo -n ",$time"
#	size=`du -b $file_path/${sizes[$i]}/${text_files[$i]}.bz2 | awk '{print $1}'`
#	echo -n ",$size"
#   	decTime=`(/usr/bin/time -f%E bunzip2 -kf $file_path/${sizes[$i]}/${text_files[$i]}.bz2 > /dev/null) 2>&1`
# 	echo -n ",$decTime"
# 	echo
#
#	echo -n "${sizes[$i]}"
#	echo -n ",ctw"
#	time=`(/usr/bin/time -f%E $ctw e -y $file_path/${sizes[$i]}/${text_files[$i]} > /dev/null) 2>&1`
#	echo -n ",$time"                      
#        size=`du -b $file_path/${sizes[$i]}/${text_files[$i]}.ctw | awk '{print $1}'`
#        echo -n ",$size"
#       	decTime=`(/usr/bin/time -f%E $ctw d -y $file_path/${sizes[$i]}/${text_files[$i]}.ctw > /dev/null) 2>&1`
#	echo -n ",$decTime"
#	echo
#
# 	echo -n "${sizes[$i]}"
#        echo -n ",ppm"
#        cd $file_path/${sizes[$i]}
# 	rm -f xxx.pmd
#        time=`(/usr/bin/time -f%E $ppm e -m265 -o16 -fxxx.pmd ${text_files[$i]} > /dev/null) 2>&1`
#        echo -n ",$time"
#        size=`du -b xxx.pmd | awk '{print $1}'`
#        echo -n ",$size"
#	rm -f ${text_files[$i]}
#        decTime=`(/usr/bin/time -f%E $ppm d xxx.pmd > /dev/null) 2>&1`
#        echo -n ",$decTime"
#        cd - > /dev/null
#        echo
#
# 	echo -n "${sizes[$i]}"
#	echo -n ",paq"
# 	time=`(/usr/bin/time -f%E $paq c$paqfile $file_path/${sizes[$i]}/${text_files[$i]}.paq $file_path/${sizes[$i]}/${text_files[$i]} > /dev/null) 2>&1`
# 	echo -n ",$time"
#	size=`du -b $file_path/${sizes[$i]}/${text_files[$i]}.paq | awk '{print $1}'`
#	echo -n ",$size"
#	decTime=`(/usr/bin/time -f%E $unpaq x $file_path/${sizes[$i]}/${text_files[$i]}.paq > /dev/null) 2>&1`
# 	echo -n ",$decTime"
# 	echo
done

rm -f $tmpfile


