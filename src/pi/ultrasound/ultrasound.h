
#ifndef _ULTRASOUND_H_
#define _ULTRASOUND_H_

#include <stdio.h>
#include <unistd.h>

#include <pigpio.h>

#include "../constants.h"

/** @brief  number of sensors */
#define ULTSND_SENSOR_COUNT 6
/** @brief  sensor reading update rate */
#define ULTSND_SAMPLE_PERIOD_MS 100

uint32_t tickToUm(uint32_t ticks);
uint32_t cmToTick(uint32_t cm);
uint32_t sonarReadUm(uint32_t sensor);
int sonarTrigger(uint32_t sensor);
int sonarPollStart();
int sonarPollStop();
int sonarStart();
void sonarStop();

#endif //_ULTRASOUND_H_
