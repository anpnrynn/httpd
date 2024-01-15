#!/bin/bash

i=0
while [[ $i -le 100 ]] 
do
	j=0
	while [[ $j -le 20 ]]
	do
		curl -v -k --retry-delay 10 --retry-all-errors --retry 16 -F key1=value1 -F upload=@/tmp/var/www/Pages/nandihills.jpg -o 3typesv6.$i.$j.1 https://127.0.0.1:16000/p3.xyz &
		wget --no-check-certificate -O 3typesv6.$i.$j.2  https://127.0.0.1:16000/nandihills.jpg &
		wget --no-check-certificate -O 3typesv6.$i.$j.3  https://127.0.0.1:16000/p3.xyz &
		wget --no-check-certificate --O 3typesv6.$i.$j.3a  https://127.0.0.1:16000/notthere.xyz &
		wget --no-check-certificate --O 3typesv6.$i.$j.3b  https://127.0.0.1:16000/notthere.txt &
		wget --no-check-certificate --O 3typesv6.$i.$j.3c  https://127.0.0.1:16000/login.xyz &
		curl -v -k --retry-delay 10 --retry-all-errors --retry 16 -F key1=value1 -F upload=@/tmp/var/www/Pages/nandihills.jpg -o 3typesv6.$i.$j.4 https://[::1]:16000/p3.xyz &
		wget --no-check-certificate -O 3typesv6.$i.$j.5  https://[::1]:16000/nandihills.jpg &
		wget --no-check-certificate -O 3typesv6.$i.$j.6  https://[::1]:16000/p3.xyz &
		wget --no-check-certificate -O 3types.$i.$j.6a  https://[::1]:16000/notthere.xyz &
		wget --no-check-certificate -O 3types.$i.$j.6b  https://[::1]:16000/notthere.txt &
		wget --no-check-certificate -O 3types.$i.$j.6c  https://[::1]:16000/login.xyz &
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
