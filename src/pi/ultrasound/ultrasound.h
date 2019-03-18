
#ifndef _ULTRASOUND_H_
#define _ULTRASOUND_H_

#include <stdio.h>
#include <unistd.h>

#include <pigpio.h>

uint32_t tickToUm(uint32_t ticks);
uint32_t cmToTick(uint32_t cm);
uint32_t sonarReadUm(uint32_t sensor);
int sonarTrigger(uint32_t sensor);
int sonarPollStart();
int sonarPollStop();
int sonarStart();
void sonarStop();

#endif //_ULTRASOUND_H_
