
CC=gcc
CFLAGS=-Wall -pthread
LIBS=-lpigpio -lrt -lbluetooth
OBJS=ultrasound/ultrasound.o speed/speed.o lidar/lidar.o interfaces/jacket.o

default: ${OBJS}
	${CC} ${CFLAGS} ${LIBS} ${OBJS} main.c -o cycle
	
%.o: %.c
	${CC} ${CFLAGS} ${LIBS} -c $^ -o $@ -D FULL_BUILD
