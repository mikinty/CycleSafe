
#ifndef _JACKET_H_
#define _JACKET_H_

#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include <sys/socket.h>
#include <bluetooth/bluetooth.h>
#include <bluetooth/rfcomm.h>
#include <pigpio.h>

#include "../constants.h"
#include "jacket_protocol.h"

#define JACKET_BT_ADDR "98:D3:61:FD:51:31"

typedef struct blinker_struct {
  int mask;
  double t_on;
  double t_off;
  int slow;
  struct blinker_struct *next;
} blinker_t;

int blinkerStart(uint32_t mask, int slow);
int blinkerStop(uint32_t mask);
void blinkerUpdate();
void blinkerStopAll();

void jacketSet(uint32_t mask);
void jacketUnset(uint32_t mask);
int jacketUpdate();
void jacketDisconnect();
int jacketConnect();
int jacketCycle(int sleepMs);

#endif //_JACKET_H_
