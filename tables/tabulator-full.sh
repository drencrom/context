#!/bin/bash

file_path=../../../files
ctw=../../32/ctw/ctw
ppm=../../code/32/ppmd-i1/PPMd.exe
paq=../../32/paq/zpaq102
unpaq=../../32/paq/unzpaq102
paqfile=../../32/paq/max.cfg
encoder=../context.opt
decoder=../uncontext.opt
tmpfile=/tmp/tabulator_tmp

sizes=( 50k 100k 600k 1m )
#sizes=( 50k 100k 600k 1m 2m )
data_files=( libfltk_images.so.1.1 timezones.png object.o user32.dll.so trident_dri.so )
text_files=( overlay_spanish.txt super.c smsserver.log steamui_spanish.txt 2donq10.txt )


echo "DATA"
echo ",,Time,Space,Dec. Time,OK"

for ((i=0; i<${#sizes[*]}; i++)); do
	echo -n "${sizes[$i]}"
	echo -n ",Kurtz"

        time=`(/usr/bin/time -f%E $encoder $file_path/${sizes[$i]}/${data_files[$i]}  > /dev/null) 2>&1`
        echo -n ",$time"

        size=`du -b $file_path/${sizes[$i]}/${data_files[$i]}.ctx | awk '{print $1}'`
        echo -n ",$size"

       	decTime=`(/usr/bin/time -f%E $decoder $file_path/${sizes[$i]}/${data_files[$i]}.ctx $tmpfile > /dev/null) 2>&1`
	echo -n ",$decTime"

	diff $file_path/${sizes[$i]}/${data_files[$i]} $tmpfile > /dev/null 	
	if [ "$?" = "0" ]; then
		echo -n ",Yes"
	else
		echo -n ",No"
	fi
	echo

 	echo -n "${sizes[$i]}"
 	echo -n ",bzip"
 	time=`(/usr/bin/time -f%E bzip2 -kf $file_path/${sizes[$i]}/${data_files[$i]} > /dev/null) 2>&1`
 	echo -n ",$time"
	size=`du -b $file_path/${sizes[$i]}/${data_files[$i]}.bz2 | awk '{print $1}'`
	echo -n ",$size"
	decTime=`(/usr/bin/time -f%E bunzip2 -kf $file_path/${sizes[$i]}/${data_files[$i]}.bz2 > /dev/null) 2>&1`
 	echo -n ",$decTime"
 	echo

 	echo -n "${sizes[$i]}"
 	echo -n ",ctw"
 	time=`(/usr/bin/time -f%E $ctw e -y $file_path/${sizes[$i]}/${data_files[$i]} > /dev/null) 2>&1`
 	echo -n ",$time"                      
	size=`du -b $file_path/${sizes[$i]}/${data_files[$i]}.ctw | awk '{print $1}'`
	echo -n ",$size"
	decTime=`(/usr/bin/time -f%E $ctw d -y $file_path/${sizes[$i]}/${data_files[$i]}.ctw > /dev/null) 2>&1`
 	echo -n ",$decTime"
 	echo

 	echo -n "${sizes[$i]}"
 	echo -n ",ppm"
 	cd $file_path/${sizes[$i]}
 	rm -f xxx.pmd
 	time=`(/usr/bin/time -f%E $ppm e -m265 -o16 -fxxx.pmd ${data_files[$i]} > /dev/null) 2>&1`
 	echo -n ",$time"                      
        size=`du -b xxx.pmd | awk '{print $1}'`
        echo -n ",$size"
	rm -f ${data_files[$i]}
        decTime=`(/usr/bin/time -f%E $ppm d xxx.pmd > /dev/null) 2>&1`
 	echo -n ",$decTime"
 	cd - > /dev/null
 	echo

 	echo -n "${sizes[$i]}"
	echo -n ",paq"
 	time=`(/usr/bin/time -f%E $paq c$paqfile $file_path/${sizes[$i]}/${data_files[$i]}.paq $file_path/${sizes[$i]}/${data_files[$i]} > /dev/null) 2>&1`
 	echo -n ",$time"
	size=`du -b $file_path/${sizes[$i]}/${data_files[$i]}.paq | awk '{print $1}'`
	echo -n ",$size"
	decTime=`(/usr/bin/time -f%E $unpaq x $file_path/${sizes[$i]}/${data_files[$i]}.paq > /dev/null) 2>&1`
 	echo -n ",$decTime"
 	echo

done
echo

echo "TEXT"
echo ",,Time,Space,OK"

for ((i=0; i<${#sizes[*]}; i++)); do
	echo -n "${sizes[$i]}"
	echo -n ",Kurtz"

        time=`(/usr/bin/time -f%E $encoder $file_path/${sizes[$i]}/${text_files[$i]} > /dev/null) 2>&1`
        echo -n ",$time"

        size=`du -b $file_path/${sizes[$i]}/${text_files[$i]}.ctx | awk '{print $1}'`
        echo -n ",$size"

       	decTime=`(/usr/bin/time -f%E $decoder $file_path/${sizes[$i]}/${text_files[$i]}.ctx $tmpfile > /dev/null) 2>& 1`
	echo -n ",$decTime"

	diff $file_path/${sizes[$i]}/${text_files[$i]} $tmpfile > /dev/null 	
	if [ "$?" = "0" ]; then
		echo -n ",Yes"
	else
		echo -n ",No"
	fi
	echo

 	echo -n "${sizes[$i]}"
 	echo -n ",bzip"
 	time=`(/usr/bin/time -f%E bzip2 -kf $file_path/${sizes[$i]}/${text_files[$i]} > /dev/null) 2>&1`
 	echo -n ",$time"
	size=`du -b $file_path/${sizes[$i]}/${text_files[$i]}.bz2 | awk '{print $1}'`
	echo -n ",$size"
   	decTime=`(/usr/bin/time -f%E bunzip2 -kf $file_path/${sizes[$i]}/${text_files[$i]}.bz2 > /dev/null) 2>&1`
 	echo -n ",$decTime"
 	echo

	echo -n "${sizes[$i]}"
	echo -n ",ctw"
	time=`(/usr/bin/time -f%E $ctw e -y $file_path/${sizes[$i]}/${text_files[$i]} > /dev/null) 2>&1`
	echo -n ",$time"                      
        size=`du -b $file_path/${sizes[$i]}/${text_files[$i]}.ctw | awk '{print $1}'`
        echo -n ",$size"
       	decTime=`(/usr/bin/time -f%E $ctw d -y $file_path/${sizes[$i]}/${text_files[$i]}.ctw > /dev/null) 2>&1`
	echo -n ",$decTime"
	echo

 	echo -n "${sizes[$i]}"
        echo -n ",ppm"
        cd $file_path/${sizes[$i]}
 	rm -f xxx.pmd
        time=`(/usr/bin/time -f%E $ppm e -m265 -o16 -fxxx.pmd ${text_files[$i]} > /dev/null) 2>&1`
        echo -n ",$time"
        size=`du -b xxx.pmd | awk '{print $1}'`
        echo -n ",$size"
	rm -f ${text_files[$i]}
        decTime=`(/usr/bin/time -f%E $ppm d xxx.pmd > /dev/null) 2>&1`
        echo -n ",$decTime"
        cd - > /dev/null
        echo

 	echo -n "${sizes[$i]}"
	echo -n ",paq"
 	time=`(/usr/bin/time -f%E $paq c$paqfile $file_path/${sizes[$i]}/${text_files[$i]}.paq $file_path/${sizes[$i]}/${text_files[$i]} > /dev/null) 2>&1`
 	echo -n ",$time"
	size=`du -b $file_path/${sizes[$i]}/${text_files[$i]}.paq | awk '{print $1}'`
	echo -n ",$size"
	decTime=`(/usr/bin/time -f%E $unpaq x $file_path/${sizes[$i]}/${text_files[$i]}.paq > /dev/null) 2>&1`
 	echo -n ",$decTime"
 	echo
done

rm -f $tmpfile


