
#include <pigpio.h>

#include "constants.h"
#include "interfaces/jacket.h"
#include "lidar/lidar.h"
#include "ultrasound/ultrasound.h"
#include "speed/speed.h"

lidar_dev_t *frontLidar;
lidar_dev_t *farLidar;
lidar_dev_t *nearLidar;

int init() {

  int status;

  status = gpioInitialise();
  if (status < 0) {
    ERRP("gpioInitialise() failed.\n");
    return status;
  }

  status = jacketConnect();
  if (status < 0) {
    ERRP("jacketConnect() failed.\n");
    gpioTerminate();
    return status;
  }

  status = speedInit();
  if (status < 0) {
    ERRP("speedInit() failed.\n");
    jacketDisconnect();
    gpioTerminate();
    return status;
  }

  sonarStart();

  frontLidar = lidarInit(LIDAR_HP_ID);
  if (status < 0) {
    ERRP("lidarStart() failed.\n");
    sonarStop();
    speedClose();
    jacketDisconnect();
    gpioTerminate();
    return status;
  }

  status = sonarPollStart();
  if (status < 0) {
    ERRP("sonarPollStart() failed.\n");
    lidarClose(frontLidar);
    sonarStop();
    speedClose();
    jacketDisconnect();
    gpioTerminate();
    return status;
  }

  return 0;

}

void csClose() {
  sonarPollStop();
  lidarClose(frontLidar);
  sonarStop();
  speedClose();
  jacketDisconnect();
  gpioTerminate();
}

int main() {

  init();
  int prevSpeed = 0, speed = 0;
  while (1) {

    lidarUpdate(frontLidar);
    if (lidarTimeToImpactGetMs(frontLidar) < THRESH_FRONT_TTI_MSEC) {
      jacketSet(JKP_MASK_BUZZ_L | JKP_MASK_BUZZ_R);
    }
    else {
      jacketUnset(JKP_MASK_BUZZ_L | JKP_MASK_BUZZ_R);
    }

    jacketUnset(JKP_MASK_PROX_SL);
    int i;
    for (i = 0; i < ULTSND_SENSOR_COUNT; i++) {
      uint32_t dist = sonarReadUm(i);
      if (dist > ULTSND_MIN_DIST_UM && dist < THRESH_PROX_UM) {
        jacketSet(JKP_MASK_PROX_SL);
        break;
      }
    }

    if (speed == 0 || speed + THRESH_DECCELERATION_BRAKE_MM_PER_SEC_2 < prevSpeed) {
      jacketSet(JKP_MASK_BRAKE);
    }
    else {
      jacketUnset(JKP_MASK_BRAKE);
    }
    prevSpeed = speed;
    jacketUpdate();

  }

  return 0;

}
