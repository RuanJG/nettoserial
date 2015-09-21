#!/bin/sh
uart_name='/dev/ttyUSB0'

while [ 0 ]
do
	ip=$(ifconfig wwan0  | grep 'inet addr')
	ip=${ip#*'inet addr:'}
	ip=${ip%' Bcast:'*}
	if [ "$ip"'x' = 'x' ];then
		echo can not find ip in br-lan
	else
        	echo "start tcpuart server, in $ip"
        	/bin/tcptouart $ip 6666 $uart_name 57600
	fi
	sleep 3
done
