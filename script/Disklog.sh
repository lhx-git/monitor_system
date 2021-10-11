#!/bin/bash

#diskSize, usedSpace, availSpace, utilization
NowTime=`date +"%Y-%m-%d %H:%M:%S"`

echo -n $NowTime" "
df  | awk 'BEGIN {diskSize=0;usedSpace=0;availSpace=0;} {if (NR > 1) {diskSize+=$2; usedSpace+=$3; availSpace+=$4;}} END {printf ("%d %d %d", diskSize, usedSpace, availSpace)}' | awk '{printf("%d %d %d %.2f%%\n", $1, $2, $3, $2/$1*100)}'