#!/bin/sh
while [ 0 ]
do
	htpdate -s 180.97.33.108
	if [ ! $? -eq 0 ];then
		echo sync time error !!
	else
		echo sync date ok
		break
	fi
	sleep 2
done

