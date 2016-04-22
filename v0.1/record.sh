#!/bin/sh

day=$(date +%d)
month=$(date +%m)
year=$(date +%Y)

path="/mnt/streamfile/"${year}"-"${month}"-"${day}"/"

foldernum=$(ls -l "/mnt/streamfile" | grep "^d" | wc -l)
n=$foldernum
if [ -d ${path} ]; then
	n=`expr $n - 1`
fi

dir=$(ls -l /mnt/streamfile/ |awk '/^d/ {print $NF}') 
for i in $dir 
do
    tempath="/mnt/streamfile/"$i
    if [ $n -gt 4 ]; then
	rm -rf $tempath
	echo "tempath="$tempath", n="$n
    fi 
    n=`expr $n - 1`
#    echo $i 
done  

if [ -d ${path} ]; then
	mkdir 777 ${path}
fi

#/mnt/record/record-0.1 165 "rtsp://admin:extremevision201@192.168.1.165:554/mpeg4/ch1/main/av_stream" /mnt /usr/local/bin/ffmpeg "b=512" "f=5" "s=640x360" &
#sleep 1
/mnt/record/record-0.1 164 "rtsp://admin:extremevision201@192.168.1.164:554/mpeg4/ch1/main/av_stream" /mnt /usr/local/bin/ffmpeg "b=512" "f=5" "s=640x360" &
