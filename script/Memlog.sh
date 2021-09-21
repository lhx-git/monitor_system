#!/bin/bash
function Usage() {
    echo "Usage : $0 DyAver"
}

if [[ $# -lt 1 ]]; then
    Usage
    exit 1
fi

# $1 is the dynamic average of the mem accupancy

DyAver=$1
MemValue=(`free -m | head -2 | tail -1 | awk '{printf("%s %s %s\n", $2, $3, $7)}'`)
MemUsed=`echo "scale=2;${MemValue[1]}*100/${MemValue[0]} " | bc`
NowDyAver=`echo "scale=2;0.8*${DyAver}+0.2*${MemUsed}" | bc`

NowTime=`date +"%Y-%m-%d %H:%M:%S"`

echo ${NowTime} ${MemValue[0]}M ${MemValue[1]}M ${MemUsed}% ${NowDyAver}%