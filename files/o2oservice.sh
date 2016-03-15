#!/bin/sh 
net='wwan0'
date=''
time=''
copter_name='4g_copter'
while [ 0 ]
do
	ip=$(ifconfig $net | grep 'inet addr')
	ip=${ip#*'inet addr:'}
	ip=${ip%' Bcast:'*}
	ip=${ip%' '}
	if [ "$ip"'x' = 'x' ];then
		echo can not find ip in br-lan
		ip='0.0.0.0'
	else
        	#echo "start o2oserver, in $ip"
		date=$(date -d today +"%Y-%m-%d")
		time=$(date -d today +"%H:%M:%S")
		curl "http://www.o2oc.cn/UAV/RuanJG/123?id=$copter_name&ip=$ip&iol=1&mac=10:22:33:44:55:66&olt=$date%20$time"
	fi
	sleep 3
done
