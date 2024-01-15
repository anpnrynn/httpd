#!/bin/bash
#Copyright Anoop Kumar Narayanan <anoop.kumar.narayanan@gmail.com> , LICENSE - GPLv2 / GPLv3

export LD_LIBRARY_PATH=`pwd`/../lib/
# httpdsrv port  thresds  dosthreshold(disabled) sslport
./httpdsrv 15000 10 10000 16000 1
