
#ifndef _PHONE_H_
#define _PHONE_H_

#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <bluetooth/bluetooth.h>
#include <bluetooth/rfcomm.h>

#define ADP_INTERSECT_APPROACH 221

#define ADP_DBG_BRAKE 111
#define ADP_DBG_TURNSIG_L 121
#define ADP_DBG_TURNSIG_R 131
#define ADP_DBG_PROX_L 141
#define ADP_DBG_PROX_R 151
#define ADP_DBG_BUZZ_L 181
#define ADP_DBG_BUZZ_R 191
#define ADP_DBG_VIB_L 201
#define ADP_DBG_VIB_R 211

#define ADP_CTRL_WHEELSIZE 50
#define ADP_CTRL_LED_BRIGHTNESS 60
#define ADP_CTRL_COLLISION_THRESH 70
#define ADP_CTRL_PROX_THRESH 80
#define ADP_CTRL_BRAKE_DECCELERATION_THRESH 90
#define ADP_CTRL_SPEED_SET 40
#define ADP_SET_BRAKE_WHEN_STOPPED 241

#define ADP_SYSTEM_TEST 243


char phoneRead();
int phoneInit();
int phoneClose();

#endif
