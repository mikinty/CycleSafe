
#ifndef _LIDAR_H_
#define _LIDAR_H_

#include <byteswap.h>
#include <stdio.h>

#include <pigpio.h>

struct __lidar_dev_t;
typedef struct __lidar_dev_t lidar_dev_t;

/** @brief  Get distance in cm */
uint16_t lidarDistGet(lidar_dev_t *dev);

/** @brief  Get velocity in mm/s */
int32_t lidarVelGet(lidar_dev_t *dev);

/** @brief  Get time to impact in ms */
uint32_t lidarTimeToImpactGet(lidar_dev_t *dev);

int lidarUpdate(lidar_dev_t *dev);

void lidarClose(lidar_dev_t *dev);

lidar_dev_t *lidarInit();

#endif //_LIDAR_H_
