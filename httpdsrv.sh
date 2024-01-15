#!/bin/bash

export LD_LIBRARY_PATH=`pwd`/../lib/
# httpdsrv port  thresds  dosthreshold(disabled) sslport
./httpdsrv 15000 10 10000 16000
