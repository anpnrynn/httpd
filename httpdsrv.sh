#!/bin/bash

export LD_LIBRARY_PATH=`pwd`/../lib/
# httpdsrv port  thresds  dosthreshold
./httpdsrv 15000 10 100
