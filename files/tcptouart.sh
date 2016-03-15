#!/bin/sh
#uart_name=$(find /dev -name ttyUSB*)
#interface
#net=wlan0
#nip=192.168.1.108

#net=wwan0
#nip="dhcp"

net=eth0
nip='192.168.2.1'

uart_name=''
uart_type_list="ch34 cp21"
getUartName(){
	logstr=$(dmesg | grep $1 |  awk 'BEGIN {res=0;} {if(NR==FNR){res=$NF;}} END{print res}')
	#tmp1=${logstr##[0.9]}
	tmp1=$(expr substr "$logstr" 1 6)
	if [ 'ttyUSBx' = $tmp1'x' ];then
		uart_name="/dev/$logstr"
	else
		uart_name=''
	fi
}
try_getUart(){
	for i in $uart_type_list
	do
		getUartName $i
		if [ ! -z $uart_name ];then
			echo find $i uart in $uart_name
			return 0
		fi
	done
}
setdate(){
	htpdate -s 180.97.33.108
	if [ ! $? -eq 0 ];then
		echo sync time error !!
	fi
}
htpdate.sh &
while [ 0 ]
do
	try_getUart
	if [ -z $uart_name ];then
		echo no find uart , sleep 3s
		sleep 3
		continue
	else
		echo use uart $uart_name
	fi

	ip=$(ifconfig $net  | grep 'inet addr')
	ip=${ip#*'inet addr:'}
	ip=${ip%' Bcast:'*}
	ip=${ip%' '}
	if [ -z $ip ];then
		echo no ip
	else
		#setdate
		echo $ip
		if [ "$nip" = "dhcp" ] || [ "$ip" = "$nip" ];then
        		echo "start tcpuart server, in $ip"
        		/bin/tcptouart $ip 6666 $uart_name 57600 $1
		else
			echo can not connect ip $nip
		fi
	fi
	sleep 3
done
