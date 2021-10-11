#!/bin/bash

nowTime=`date +"%Y-%m-%d %H:%M:%S"`
#时间 主机名 OS版本 内核版本 运行时间 平均负载 磁盘总量 磁盘已用% 内存大小 内存已用% CPU温度 磁盘报警级别 内存报警级别 CPU报警级别
hostName=`(uname -n)`
OSVersion=`(uname -v | awk '{print $1}')`
kernelVersion=`(uname -r)`
runningTime=`(uptime | awk -F [" ",]+ '{print $4}')`
loadAVG=`(uptime | awk -F [" ",]+ '{printf("%s %s %s",$9, $10, $11)}')`
diskInfo=`df | awk 'BEGIN {diskSize=0;usedSpace=0;availSpace=0;} {if (NR > 1) {diskSize+=$2; usedSpace+=$3;}} END {printf ("%d %d", diskSize, usedSpace)}' | awk '{printf("%d %.2f", $1, $2/$1*100)}'`
MemInfo=(`free -m | head -2 | tail -1 | awk '{printf("%s %s", $2, $3)}' | awk '{printf("%s %.2f", $1, $2/$1*100)}'`)
temperature=0

if [[ temperature -le 50 ]] ; then
  CPUStat="normal";
elif [[ temperature -gt 50 && temperature -le 70 ]]; then
  CPUStat="note";
elif [[ temperature -gt 70 ]]; then
  CPUStat="warning"
fi

if [[ `echo ${MemInfo[1]} | awk '{print($1>70)?"0":"1"}'` ]] ; then
  MemStat="normal";
elif [[ `echo ${MemInfo[1]} | awk '{print($1>=70)?"0":"1"}'` && `echo ${MemInfo[1]} | awk '{print($1<=80)?"0":"1"}'` ]]; then
  MemStat="note";
elif [[ `echo ${MemInfo[1]} | awk '{print($1>80)?"0":"1"}'` ]]; then
  MemStat="warning"
fi

if [[ ${diskInfo[1]} -le 80 ]] ; then
  DiskStat="normal";
elif [[ ${diskInfo[1]} -gt 80 && ${diskInfo[1]} -le 90 ]]; then
  DiskStat="note";
elif [[ ${diskInfo[1]} -gt 90 ]]; then
  DiskStat="warning"
fi

echo $nowTime $hostName $OSVersion $kernelVersion $runningTime ${loadAVG[0]} ${loadAVG[1]} ${loadAVG[2]} $diskInfo% ${MemInfo[0]} ${MemInfo[1]}% $DiskStat $MemStat $CPUStat