
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
  INFOP("pigpio initialized.\n");

  status = jacketConnect();
  if (status < 0) {
    ERRP("jacketConnect() failed.\n");
    gpioTerminate();
    return status;
  }
  INFOP("Jacket connected.\n");

  status = speedInit();
  if (status < 0) {
    ERRP("speedInit() failed.\n");
    jacketDisconnect();
    gpioTerminate();
    return status;
  }
  INFOP("Speedometer initialized.\n");

  sonarStart();

  frontLidar = lidarInit(LIDAR_ID_SP);
  if (frontLidar == NULL) {
    ERRP("lidarStart() failed.\n");
    sonarStop();
    speedClose();
    jacketDisconnect();
    gpioTerminate();
    return status;
  }
  status = lidarAddressSet(frontLidar, 0x6A);
  if (status < 0) {
    ERRP("lidarAddressSet() failed.\n");
    lidarClose(frontLidar);
    sonarStop();
    speedClose();
    jacketDisconnect();
    gpioTerminate();
    return status;
  }
  INFOP("LIDAR (front) initialized.\n");
/*
  farLidar = lidarInit(LIDAR_ID_HP);
  if (farLidar == NULL) {
    ERRP("lidarStart() failed.\n");
    lidarClose(frontLidar);
    sonarStop();
    speedClose();
    jacketDisconnect();
    gpioTerminate();
    return status;
  }
  status = lidarAddressSet(farLidar, 0x6C);
  if (status < 0) {
    ERRP("lidarAddressSet() failed.\n");
    lidarClose(farLidar);
    lidarClose(frontLidar);
    sonarStop();
    speedClose();
    jacketDisconnect();
    gpioTerminate();
    return status;
  }
  INFOP("LIDAR (far) initialized.\n");
*/
  nearLidar = lidarInit(LIDAR_ID_SPMARK);
  if (nearLidar == NULL) {
    ERRP("lidarStart() failed.\n");
    lidarClose(farLidar);
    lidarClose(frontLidar);
    sonarStop();
    speedClose();
    jacketDisconnect();
    gpioTerminate();
    return status;
  }
  status = lidarAddressSet(nearLidar, 0x6E);
  if (status < 0) {
    ERRP("lidarAddressSet() failed.\n");
    lidarClose(nearLidar);
    lidarClose(farLidar);
    lidarClose(frontLidar);
    sonarStop();
    speedClose();
    jacketDisconnect();
    gpioTerminate();
    return status;
  }
  INFOP("LIDAR (near) initialized.\n");

  status = sonarPollStart();
  if (status < 0) {
    ERRP("sonarPollStart() failed.\n");
    lidarClose(nearLidar);
    lidarClose(farLidar);
    lidarClose(frontLidar);
    sonarStop();
    speedClose();
    jacketDisconnect();
    gpioTerminate();
    return status;
  }
  INFOP("sonar initialized.\n");

  gpioSetMode(5, PI_INPUT);
  gpioSetPullUpDown(5, PI_PUD_UP); 
  gpioSetMode(6, PI_INPUT);
  gpioSetPullUpDown(6, PI_PUD_UP); 

  return 0;

}

void csClose() {
  sonarPollStop();
  lidarClose(nearLidar);
  lidarClose(farLidar);
  lidarClose(frontLidar);
  sonarStop();
  speedClose();
  jacketDisconnect();
  gpioTerminate();
}

int main() {

  int status;
  status = init();
  if (status < 0) {  
    ERRP("Init failed\n");
    return -1;
  }
  int prevSpeed = 0, speed = 0;
  
  while (1) {
    
    status = speedRequest();
    if (status < 0) {
      printf("speedRequest() error %d\n", status);
    }

    uint32_t tti;

    lidarUpdate(frontLidar);
    tti = lidarTimeToImpactGetMs(frontLidar);
    if (tti < THRESH_FRONT_TTI_MSEC) {
      jacketSet(JKP_MASK_BUZZ_L | JKP_MASK_BUZZ_R);
      // printf("Vel:%d, Dist:%d, TTI:%d\n", lidarVelGet(frontLidar), lidarDistGet(frontLidar), tti); 
    }
    else {
      jacketUnset(JKP_MASK_BUZZ_L | JKP_MASK_BUZZ_R);
    }
    
    lidarUpdate(nearLidar);
    tti = lidarTimeToImpactGetMs(nearLidar);
    if (tti < THRESH_FRONT_TTI_MSEC) {
      jacketSet(0x1000);
      // printf("Vel:%d, Dist:%d, TTI:%d\n", lidarVelGet(frontLidar), lidarDistGet(frontLidar), tti); 
    }
    else {
      jacketUnset(0x1000);
    }

    jacketUnset(JKP_MASK_PROX_SL);
    jacketUnset(JKP_MASK_VIB_L);
    jacketUnset(JKP_MASK_VIB_R);
    int i;
    for (i = 0; i < ULTSND_SENSOR_COUNT; i++) {
      uint32_t dist = sonarReadUm(i);
      if (dist > ULTSND_MIN_DIST_UM && dist < THRESH_PROX_UM) {
        jacketSet(JKP_MASK_PROX_SL);
        jacketSet(JKP_MASK_VIB_L);
        jacketSet(JKP_MASK_VIB_R);
        
        break;
      }
    }

    //while (!speedAvailable());
    speed = speedRead();
    if (speed < 0) {
      printf("speedRead() error %d\n", speed);
      speed = 0;
    }

    // TODO put back zero speed
    if (speed == 0 || speed + THRESH_DECCELERATION_BRAKE_MM_PER_SEC_2 < prevSpeed) {
      jacketSet(JKP_MASK_BRAKE);
    }
    else {
      jacketUnset(JKP_MASK_BRAKE);
    }
    prevSpeed = speed;
    
    if (!gpioRead(5)) jacketSet(JKP_MASK_TURNSIG_L);
    else jacketUnset(JKP_MASK_TURNSIG_L);
    if (!gpioRead(6)) jacketSet(JKP_MASK_TURNSIG_R);
    else jacketUnset(JKP_MASK_TURNSIG_R);
    
    status = jacketUpdate();
    if (status < 0) {
      ERRP("Jacket update failed\n");
    }
    gpioSleep(0, 0, 10000);

  }

  return 0;

}
