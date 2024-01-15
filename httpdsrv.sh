#!/bin/bash
#Copyright Anoop Kumar Narayanan <anoop.kumar.narayanan@gmail.com> , LICENSE - GPLv2 / GPLv3

export LD_LIBRARY_PATH=`pwd`/../lib/
# httpdsrv port threads dosthreshold(large value like 10000 means disabled) sslport loglevel
./httpdsrv 15000 10 10000 16000 1 > httpd.log 2>&1
