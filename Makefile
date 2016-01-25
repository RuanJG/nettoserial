ARCH=arm
#CROSS_COMPILE= /home/mx6/rasberrypi/tools/arm-bcm2708/arm-bcm2708hardfp-linux-gnueabi/bin/arm-bcm2708hardfp-linux-gnueabi-
CROSS_COMPILE=
CC=$(CROSS_COMPILE)gcc

srcs:=main.c o2o_service.c
tcptouart: $(srcs)
	$(CC) $(LDFLAGS) $(srcs) -o tcptouart -lpthread -I mavlink/include/mavlink/v1.0
#main.o: main.c
#	$(CC) $(CFLAGS) -c main.c -I mavlink/include/mavlink/v1.0

install:
	sudo cp ./files/tcptouartd /etc/init.d/
	sudo cp ./files/camera_port_forward.rule /etc/camera_port_forward.rule
	sudo cp ./files/tcptouart.sh /bin/tcptouart.sh
	sudo cp ./files/o2oservice.sh /bin/o2oservice.sh
	sudo cp ./files/htpdate.sh /bin/htpdate.sh
	sudo cp ./tcptouart /bin/tcptouart
	sudo cp ./files/htpdate /bin/htpdate
	sudo cp ./files/iptables-port-forward /etc/network/if-pre-up.d/iptables-port-forward
	sudo chmod 0777 /etc/network/if-pre-up.d/iptables-port-forward
	sudo chmod 0777 /bin/tcptouart
	sudo chmod 0777 /bin/tcptouart.sh
	sudo chmod 0777 /bin/o2oservice.sh
	sudo chmod 0777 /bin/htpdate
	sudo update-rc.d tcptouartd defaults
uninstall:
	sudo update-rc.d -f tcptouartd remove
clean:
	rm *.o tcptouart

