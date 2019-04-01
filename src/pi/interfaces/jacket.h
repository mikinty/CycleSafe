
#ifndef _JACKET_H_
#define _JACKET_H_

#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <bluetooth/bluetooth.h>
#include <bluetooth/rfcomm.h>

#include "../constants.h"
#include "jacket_protocol.h"

#define JACKET_BT_ADDR "98:D3:61:FD:51:31"

void jacketSet(uint32_t mask);
void jacketUnset(uint32_t mask);
int jacketUpdate();
void jacketDisconnect();
int jacketConnect();

#endif //_JACKET_H_
