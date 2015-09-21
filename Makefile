ARCH=arm
#CROSS_COMPILE= /home/mx6/rasberrypi/tools/arm-bcm2708/arm-bcm2708hardfp-linux-gnueabi/bin/arm-bcm2708hardfp-linux-gnueabi-
CROSS_COMPILE=
CC=$(CROSS_COMPILE)gcc

tcptouart: main.o
	$(CC) $(LDFLAGS) main.o -o tcptouart -lpthread
main.o: main.c
	$(CC) $(CFLAGS) -c main.c -I mavlink/include/mavlink/v1.0

install:
	sudo cp ./files/tcptouartd /etc/init.d/
	sudo cp ./files/tcptouart.sh /bin/tcptouart.sh
	sudo cp ./tcptouart /bin/tcptouart
	sudo chmod 0777 /bin/tcptouart
	sudo chmod 0777 /bin/tcptouart.sh
	sudo update-rc.d tcptouartd defaults
uninstall:
	sudo update-rc.d -f tcptouartd remove
clean:
	rm *.o tcptouart
