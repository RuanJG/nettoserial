ARCH=arm
#CROSS_COMPILE= /home/mx6/rasberrypi/tools/arm-bcm2708/arm-bcm2708hardfp-linux-gnueabi/bin/arm-bcm2708hardfp-linux-gnueabi-
CROSS_COMPILE=
CC=$(CROSS_COMPILE)gcc

tcptouart: main.o
	$(CC) $(LDFLAGS) main.o -o tcptouart -lpthread
main.o: main.c
	$(CC) $(CFLAGS) -c main.c -I mavlink/include/mavlink/v1.0
clean:
	rm *.o tcptouart
