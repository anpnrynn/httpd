#!/bin/bash

i=0
while [[ $i -le 100 ]] 
do
	j=0
	while [[ $j -le 100 ]]
	do
		curl -v --retry-delay 10 --retry-all-errors --retry 16 -F key1=value1 -F upload=@/tmp/var/www/Pages/nandihills.jpg -o plugupout.$i.$j http://127.0.0.1:15000/p3.xyz &
		j=$((j+1))
	done
	count=`ps -ef |grep "curl -v" | wc -l`
	while [[ $count -gt 300 ]]
	do
		count=`ps -ef |grep "curl -v" | wc -l`
		sleep 1
	done
	sleep 2
i=$((i+1))
done

while [[ 1 ]]
do
count=`ps -ef |grep "curl -v" | wc -l`
if [[ $count -lt 2 ]] 
then
	exit 0
fi
echo "waiting for test to stop $count"
sleep 1
done
