#!/bin/bash

nowTime=`date +"%Y-%m-%d %H:%M:%S"`
loadAVG=(`cat /proc/loadavg | awk '{printf("%s %s %s", $1, $2, $3)}'`)
# 温度获取不到  /sys/class/thermal/thermal_zone0/temp
temp=0
utilization=`cat /proc/stat | head -n 1 | awk '{printf ("%d %d", $4, ($1+$2+$3+$4+$5+$6+$7+$8+$9+$10))}' | awk '{printf("%.2f", $1/$2*100)}'`
if [[ $temp -le 50 ]] ; then
  stat="normal";
elif [[ $temp -gt 50 && $temp -le 70 ]]; then
  stat="note";
elif [[ $temp -gt 70 ]]; then
  stat="warning"
fi

echo $nowTime ${loadAVG[0]} ${loadAVG[1]} ${loadAVG[2]} $utilization $temp $stat