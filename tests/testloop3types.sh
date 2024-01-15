#!/bin/bash

i=0
while [[ $i -le 100 ]] 
do
	j=0
	while [[ $j -le 20 ]]
	do
		curl -v --retry-delay 10 --retry-all-errors --retry 16 -F key1=value1 -F upload=@/tmp/var/www/Pages/nandihills.jpg -o 3types.$i.$j.1 http://127.0.0.1:15000/p3.xyz &
		wget -O 3types.$i.$j.2  http://127.0.0.1:15000/nandihills.jpg &
		wget -O 3types.$i.$j.3  http://127.0.0.1:15000/p3.xyz &
		wget -O 3types.$i.$j.3a  http://127.0.0.1:15000/notthere.xyz &
		wget -O 3types.$i.$j.3b  http://127.0.0.1:15000/notthere.txt &
		wget -O 3types.$i.$j.3c  http://127.0.0.1:15000/login.xyz &
		curl -v --retry-delay 10 --retry-all-errors --retry 16 -F key1=value1 -F upload=@/tmp/var/www/Pages/nandihills.jpg -o 3types.$i.$j.4 http://[::1]:15000/p3.xyz &
		wget -O 3types.$i.$j.5  http://[::1]:15000/nandihills.jpg &
		wget -O 3types.$i.$j.6  http://[::1]:15000/p3.xyz &
		wget -O 3types.$i.$j.6a  http://[::1]:15000/notthere.xyz &
		wget -O 3types.$i.$j.6b  http://[::1]:15000/notthere.txt &
		wget -O 3types.$i.$j.6c  http://[::1]:15000/login.xyz &
		j=$((j+1))
	done
	count=`ps -ef |egrep "(curl|wget)" | wc -l`
	while [[ $count -gt 120 ]]
	do
		count=`ps -ef |egrep "(curl|wget)" | wc -l`
		sleep 1
	done
	sleep 2
i=$((i+1))
done

while [[ 1 ]]
do
	count=`ps -ef |egrep "(curl|wget)" | wc -l`
if [[ $count -lt 2 ]] 
then
	exit 0
fi
echo "waiting for test to stop $count"
sleep 1
done
