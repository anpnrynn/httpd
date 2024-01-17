#!/bin/bash
#Copyright Anoop Kumar Narayanan <anoop.kumar.narayanan@gmail.com> , LICENSE - GPLv2 / GPLv3

i=0
while [[ $i -le 100 ]] 
do
	j=0
	while [[ $j -le 100 ]]
	do
		wget -O staticdown.$i.$j.1  http://127.0.0.1:15000/nandihills.jpg &
		wget -O staticdown.$i.$j.2  http://127.0.0.1:15000/login.html &
		wget -O staticdown.$i.$j.3  http://127.0.0.1:15000/image.jpg &
		wget -O staticdown.$i.$j.4  http://[::1]:15000/nandihills.jpg &
		wget -O staticdown.$i.$j.5  http://[::1]:15000/login.html &
		wget -O staticdown.$i.$j.6  http://[::1]:15000/image.jpg &
		wget --no-check-certificate -O staticdown.$i.$j.s1  https://127.0.0.1:16000/nandihills.jpg &
		wget --no-check-certificate -O staticdown.$i.$j.s2  https://127.0.0.1:16000/login.html &
		wget --no-check-certificate -O staticdown.$i.$j.s3  https://127.0.0.1:16000/image.jpg &
		wget --no-check-certificate -O staticdown.$i.$j.s4  https://[::1]:16000/nandihills.jpg &
		wget --no-check-certificate -O staticdown.$i.$j.s5  https://[::1]:16000/login.html &
		wget --no-check-certificate -O staticdown.$i.$j.s6  https://[::1]:16000/image.jpg &
		j=$((j+1))
	done
	count=`ps -ef |egrep "(curl|wget)" | wc -l`
	while [[ $count -gt 120 ]]
	do
		files=`ls -lh |grep "6.8M"  | awk -F" " '{print $9}'`
		rm -f $files
		count=`ps -ef |egrep "(curl|wget)" | wc -l`
		sleep 1
	done
	sleep 2
i=$((i+1))
done

while [[ 1 ]]
do
count=`ps -ef |grep "wget" | wc -l`
if [[ $count -lt 2 ]] 
then
	exit 0
fi
echo "waiting for the test to stop $count"
sleep 1
done
