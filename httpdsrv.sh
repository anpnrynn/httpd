#!/bin/bash

export LD_LIBRARY_PATH=`pwd`/../lib/
# httpdsrv port  thresds  dosthresholdA sslport
./httpdsrv 15000 10 100 16000
