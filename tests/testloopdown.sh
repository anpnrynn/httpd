#!/bin/bash

i=0
while [[ $i -le 100 ]] 
do
	j=0
	while [[ $j -le 100 ]]
	do
		wget -O staticdown.$i.$j  http://127.0.0.1:15000/nandihills.jpg &
		j=$((j+1))
	done
	sleep 2
i=$((i+1))
done

while [[ 1 ]]
do
count=`ps -ef |grep "wget http" | wc -l`
if [[ $count -lt 2 ]] 
then
	exit 0
fi
echo "waiting for the test to stop $count"
sleep 1
done
