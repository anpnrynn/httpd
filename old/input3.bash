#!/bin/bash
i_______icounter=0;

touch /tmp/input-$$..test.$$

execute(){
	file=$1
	while IFS= read -r line
	do
  		echo "$line" > /tmp/tmp-$$.bash
		echo "bash> $line"
		. /tmp/tmp-$$.bash 
	done < "$file"
}

while [ 1 ]
do
	#. /tmp/input-$$.$i_______icounter >> /tmp/output 2 >> /tmp/output
	test -f /tmp/input-$$.$i_______icounter
	val=$?
	while [ $val -ne 0 ]
	do
		test -f /tmp/input-$$.$i_______icounter
		val=$?
		sleep 0.5
	done
	execute /tmp/input-$$.$i_______icounter
	rm -f /tmp/input-$$.$i_______icounter
	sleep 1
	touch /tmp/output-$$.$i_______icounter
	i_______icounter=$((i_______icounter+1))
done
