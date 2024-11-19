#!/bin/bash
i_______icounter=0;

execute(){
	file=$1
	while IFS= read -r line
	do
  		echo "$line" > /tmp/tmp.bash
		echo "bash> $line" >> /tmp/output
		. /tmp/tmp.bash 2>&1 >> /tmp/output 2>>/tmp/output
	done < "$file"
}

while [ 1 ]
do
	#. /tmp/input.$i_______icounter >> /tmp/output 2 >> /tmp/output
	execute /tmp/input.$i_______icounter
	i_______icounter=$((i_______icounter+1))
	test -f /tmp/input.$i_______icounter
	val=$?
	while [ $val -ne 0 ]
	do
		test -f /tmp/input.$i_______icounter
		val=$?
		sleep 0.5
	done
done
