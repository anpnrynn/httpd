#!/bin/bash

i=0
while [[ $i -le 100 ]] 
do
wget http://127.0.0.1:15000/nandihills.jpg &
i=$((i+1))
done

sleep 120

rm -f *.jpg*
