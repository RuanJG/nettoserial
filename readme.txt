tcptouart: 在linux下做网络tcp转串口的透传程序，主要适配rassparry pi系统

编译安装：在代码根目录执行：
	make
	make install

main.c : 主程序代码文件
用法：tcptouart ip port /dev/ttyxxx baudrate  debug[0,1]
例如：
./tcptouart 192.168.2.1 6666 /dev/ttyUSB0 57600 1   


files/tcptouartd : tcptouart 程序的系统服务文件
files/tcptouart.sh : tcptouart 程序的执行脚本，由tcptouartd服务调用
files/o2oservice.sh : 上报本机ip信息到服务器的脚本程序，目前方案是用这个脚本来做上报4G状态信息，由tcptouartd服务调用
files/htpdate.sh : 在连接上4G网络后，更新本机时间与时区的脚本程序，由tcptouart.sh执行


网络转发功能：
files/iptables-port-forward
files/camera_port_forward.rule
在make install时会安装上转发的功能，这个可以不用理会，但如果要修改时，可以看下Makefile里是怎样配置 的，也可以修改camera_port_forward.rule
 
