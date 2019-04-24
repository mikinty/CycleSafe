
#include <pigpio.h>

#include "constants.h"
#include "interfaces/jacket.h"
#include "lidar/lidar.h"
#include "ultrasound/ultrasound.h"
#include "speed/speed.h"

lidar_dev_t *frontLidar;
lidar_dev_t *farLidar;
lidar_dev_t *nearLidar;

int accel_zero;

int init() {

  int status;

  status = gpioInitialise();
  if (status < 0) {
    ERRP("gpioInitialise() failed.\n");
    return status;
  }
  INFOP("pigpio initialized.\n");

  gpioSetMode(PIN_RESET, PI_OUTPUT);
  gpioWrite(PIN_RESET, PI_OFF);
  gpioSleep(0, 1, 0);
  gpioWrite(PIN_RESET, PI_ON);

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

  gpioSetMode(PIN_TURNSIG_L, PI_INPUT);
  gpioSetPullUpDown(PIN_TURNSIG_L, PI_PUD_UP);
  gpioSetMode(PIN_TURNSIG_R, PI_INPUT);
  gpioSetPullUpDown(PIN_TURNSIG_R, PI_PUD_UP);

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
  int speed = 0;
  int accel = 0;
  
  printf("=== System Test ===\n");
  printf("LIDAR (front):\n");
  status = lidarUpdate(frontLidar);
  if (status < 0) {
    ERRP("lidarUpdate(frontLidar) error 0x%x\n", status);
  }
  printf("Reading: %d\n", lidarDistGet(frontLidar));
  
  printf("LIDAR (far back):\n");
  status = lidarUpdate(farLidar);
  if (status < 0) {
    ERRP("lidarUpdate(farLidar) error 0x%x\n", status);
  }
  printf("Reading: %d\n", lidarDistGet(farLidar));
  
  printf("LIDAR (near back):\n");
  status = lidarUpdate(nearLidar);
  if (status < 0) {
    ERRP("lidarUpdate(nearLidar) error 0x%x\n", status);
  }
  printf("Reading: %d\n", lidarDistGet(nearLidar));
  
  printf("Sonars\n");
  int i;
  for (i = 0; i < ULTSND_SENSOR_COUNT; i++) {
    uint32_t dist = sonarReadUm(i);
    printf("Sonar %d: Reading: %d\n", i, dist);
  }
  
  status = speedRequest(SPEED_REQ_SPEED);
  if (status < 0) {
    ERRP("speedRequest(speed) error 0x%x\n", status);
  }
  status = speedRequest(SPEED_REQ_ACCEL);
  if (status < 0) {
    ERRP("speedRequest(accel) error 0x%x\n", status);
  }

  gpioSleep(0, 0, 100000);

  speed = speedRead();
  if (speed < 0) {
    ERRP("speedRead(speed) error 0x%x\n", speed);
    speed = 0; 
  }
  printf("Speed reading: %d\n", speed);
  accel = speedRead();
  if (accel < 0) {
    ERRP("speedRead(accel) error 0x%x\n", accel);
    accel = 0;
  }
  printf("Acceleration reading: %d\n", accel);
  accel_zero = accel;
  
  while (1) {

    status = speedRequest(SPEED_REQ_SPEED);
    if (status < 0) {
      ERRP("speedRequest(speed) error 0x%x\n", status);
    }
    status = speedRequest(SPEED_REQ_ACCEL);
    if (status < 0) {
      ERRP("speedRequest(accel) error 0x%x\n", status);
    }

    uint32_t tti;

    status = lidarUpdate(frontLidar);
    if (status < 0) {
      ERRP("lidarUpdate(frontLidar) error 0x%x\n", status);
    }
    tti = lidarTimeToImpactGetMs(frontLidar);
    // printf("Vel:%d, Dist:%d, TTI:%d\n", lidarVelGet(frontLidar), lidarDistGet(frontLidar), tti);
    if (tti < THRESH_FRONT_TTI_MSEC) {
      jacketSet(JKP_MASK_BUZZ_R);
    }
    else {
      jacketUnset(JKP_MASK_BUZZ_R);
    }

    lidarUpdate(nearLidar);
    if (status < 0) {
      ERRP("lidarUpdate(nearLidar) error 0x%x\n", status);
    }
    tti = lidarTimeToImpactGetMs(nearLidar);
    if (tti < THRESH_FRONT_TTI_MSEC) {
      jacketSet(JKP_MASK_BUZZ_L);
      // printf("Vel:%d, Dist:%d, TTI:%d\n", lidarVelGet(nearLidar), lidarDistGet(Lidar), tti);
    }
    else {
      jacketUnset(JKP_MASK_BUZZ_L);
    }
    
    lidarUpdate(farLidar);
    if (status < 0) {
      ERRP("lidarUpdate(farLidar) error 0x%x\n", status);
    }
    tti = lidarTimeToImpactGetMs(farLidar);
    if (tti < THRESH_FRONT_TTI_MSEC) {
      jacketSet(JKP_MASK_VIB_R);
      // printf("Vel:%d, Dist:%d, TTI:%d\n", lidarVelGet(frontLidar), lidarDistGet(frontLidar), tti);
    }
    else {
      jacketUnset(JKP_MASK_VIB_R);
    }

    jacketUnset(JKP_MASK_PROX_SL);
    jacketUnset(JKP_MASK_VIB_L);
    int i;
    for (i = 0; i < ULTSND_SENSOR_COUNT; i++) {
      uint32_t dist = sonarReadUm(i);
      if (dist > ULTSND_MIN_DIST_UM && dist < THRESH_PROX_UM) {
        jacketSet(JKP_MASK_PROX_SL);
        jacketSet(JKP_MASK_VIB_L);

        break;
      }
    }

    if (!gpioRead(PIN_TURNSIG_L)) jacketSet(JKP_MASK_TURNSIG_L);
    else jacketUnset(JKP_MASK_TURNSIG_L);
    if (!gpioRead(PIN_TURNSIG_R)) jacketSet(JKP_MASK_TURNSIG_R);
    else jacketUnset(JKP_MASK_TURNSIG_R);
    
    if (!gpioRead(PIN_TURNSIG_L) && !gpioRead(PIN_TURNSIG_R)) {
      jacketUnset(0xFFFFFFFF);
      jacketUpdate();
      gpioSleep(0, 5, 0);
      if (!gpioRead(PIN_TURNSIG_L) && !gpioRead(PIN_TURNSIG_R)) {
        csClose();
      }
    }

    //while (!speedAvailable());
    speed = speedRead();
    if (speed < 0) {
      ERRP("speedRead(speed) error 0x%x\n", speed);
      speed = 0;
    }
    accel = speedRead();
    if (accel < 0) {
      ERRP("speedRead(accel) error 0x%x\n", accel);
      accel = 0;
    }

    //printf("Speed: %d, Accel: %d\n", speed, accel);

    // TODO put back zero speed
    if (speed < THRESH_BRAKE_SPEED_MM_PER_SEC || accel > accel_zero + 1) {
      jacketSet(JKP_MASK_BRAKE);
    }
    else {
      jacketUnset(JKP_MASK_BRAKE);
    }

    status = jacketUpdate();
    if (status < 0) {
      ERRP("Jacket update failed\n");
    }
    gpioSleep(0, 0, 10000);

  }

  return 0;

}
