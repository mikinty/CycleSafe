
#ifndef _SPEED_H_
#define _SPEED_H_

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <unistd.h>

#include "../constants.h"
#include "speed_protocol.h"

int speedRead(); // mm/sec
int speedRequest(char c);
int speedInit();
void speedClose();

#endif //_SPEED_H_
