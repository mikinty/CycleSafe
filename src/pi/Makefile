
CC=gcc
CFLAGS=-Wall -pthread
LIBS=-lpigpio -lrt -lbluetooth
OBJS=ultrasound/ultrasound.o speed/speed.o lidar/lidar.o interfaces/jacket.o interfaces.phone.o

default: clean all

.PHONY: clean
clean:
	rm -rf ${OBJS}

all: ${OBJS}
	${CC} ${CFLAGS} ${LIBS} ${OBJS} main.c -o cycle

%.o: %.c
	${CC} ${CFLAGS} ${LIBS} -c $^ -o $@ -D FULL_BUILD

nojacket: CFLAGS += -D DEBUG_NO_CONNECT
nojacket: default

trainer: CFLAGS += -D TRAINER
trainer: default
