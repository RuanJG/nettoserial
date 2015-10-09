#!/bin/sh
uart_name=$(find /dev -name ttyUSB*)
#interface
net=wlan0
nip=192.168.1.108
while [ 0 ]
do
	ip=$(ifconfig $net  | grep 'inet addr')
	ip=${ip#*'inet addr:'}
	ip=${ip%' Bcast:'*}
	ip=${ip%' '}
	if [ -z $ip ];then
		echo no ip
	else
		echo $ip
		if [ "$ip" = "$nip" ];then
        		echo "start tcpuart server, in $ip"
        		/bin/tcptouart $ip 6666 $uart_name 57600
		else
			echo can not connect ip $nip
		fi
	fi
	sleep 3
done
