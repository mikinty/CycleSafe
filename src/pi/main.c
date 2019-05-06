
#include <pigpio.h>

#include "constants.h"
#include "interfaces/jacket.h"
#include "interfaces/phone.h"
#include "lidar/lidar.h"
#include "ultrasound/ultrasound.h"
#include "speed/speed.h"

typedef struct blindspot_struct {
  double time_tti_near_notif;
  double time_tti_far_notif;
  double time_tti_near_alert;
  double time_tti_far_alert;
  double notif_expire;
  double alert_expire;
} blindspot_t;

blindspot_t bs;

typedef struct braking_struct {
  double time_on;
  int accel_zero;
  int prev_speed;
  double prev_time;
} braking_t;

braking_t brakes;

lidar_dev_t *frontLidar;
lidar_dev_t *farLidar;
lidar_dev_t *nearLidar;

int isActiveUltSnd;
int isActiveSpeed;
int isActiveJacket;
int isActivePhone;

int init() {

  int status;

  status = gpioInitialise();
  if (status < 0) {
    ERRP("gpioInitialise() failed.\n");
    return status;
  }
  INFOP("pigpio initialized.\n");

  INFOP("Resetting LIDAR...");
  gpioSetMode(PIN_RESET, PI_OUTPUT);
  gpioWrite(PIN_RESET, PI_OFF);
  gpioSleep(0, 0, 200000);
  gpioWrite(PIN_RESET, PI_ON);
  INFOP("success.\n");

  INFOP("Connecting to jacket...");
  do {
    isActiveJacket = jacketConnect();
    if (isActiveJacket == 0) break;
    ERRP("jacketConnect() failed %d.\n", isActiveJacket);
    gpioSleep(0, 2, 0);
  } while (isActiveJacket < 0);

  INFOP("connected.\n");
  INFOP("Jacket system check...");
  jacketCycle(400);
  INFOP("completed.\n");

  INFOP("Initializing speedometer...");
  isActiveSpeed = speedInit();
  if (isActiveSpeed < 0) {
    ERRP("speedInit() failed.\n");
  }
  else {
    INFOP("success.\n");
    status = speedRequest(SPEED_REQ_SPEED);
    if (status < 0) {
      ERRP("speedRequest(speed) error 0x%x\n", status);
    }
#ifndef TRAINER
    status = speedRequest(SPEED_REQ_ACCEL);
    if (status < 0) {
      ERRP("speedRequest(accel) error 0x%x\n", status);
    }
#endif
    gpioSleep(0, 0, 100000);
    int speed = speedRead();
    if (speed < 0) {
      ERRP("speedRead(speed) error 0x%x\n", speed);
    }
    INFOP("\tSpeed reading: %d\n", speed);
#ifndef TRAINER
    int accel = speedRead();
    if (accel < 0) {
      ERRP("speedRead(accel) error 0x%x\n", accel);
    }
    INFOP("\tAcceleration reading: %d\n", accel);
    brakes.accel_zero = accel;
#else
    INFOP("\tTrainer mode on! Accelerometer disabled.\n");
    brakes.prev_speed = speed;
    brakes.prev_time = time_time();
#endif
    brakes.time_on = 0.0;
  }

  INFOP("Initializing front LIDAR...");
  frontLidar = lidarInit(LIDAR_ID_SPMARK);
  if (frontLidar == NULL) {
    ERRP("lidarStart() failed.\n");
  }
  else {
    status = lidarAddressSet(frontLidar, LIDAR_I2C_ADDR_SPMARK);
    if (status < 0) {
      ERRP("lidarAddressSet() failed.\n");
      lidarClose(frontLidar);
      frontLidar = NULL;
    }
    else {
      INFOP("success.\n");
      status = lidarUpdate(frontLidar);
      if (status < 0) {
        ERRP("lidarUpdate(frontLidar) error 0x%x\n", status);
      }
      else {
        INFOP("\tReading: %d\n", lidarDistGet(frontLidar));
      }
    }
  }

  INFOP("Initializing rear far LIDAR...");
  farLidar = lidarInit(LIDAR_ID_HP);
  if (farLidar == NULL) {
    ERRP("lidarStart() failed.\n");
  }
  else {
    status = lidarAddressSet(farLidar, LIDAR_I2C_ADDR_HP);
    if (status < 0) {
      ERRP("lidarAddressSet() failed.\n");
      lidarClose(farLidar);
      farLidar = NULL;
    }
    else {
      INFOP("success.\n");
      status = lidarUpdate(farLidar);
      if (status < 0) {
        ERRP("lidarUpdate(farLidar) error 0x%x\n", status);
      }
      else {
        INFOP("\tReading: %d\n", lidarDistGet(farLidar));
      }
    }
  }

  INFOP("Initializing rear close LIDAR...");
  nearLidar = lidarInit(LIDAR_ID_SP);
  if (nearLidar == NULL) {
    ERRP("lidarStart() failed.\n");
  }
  else {
    status = lidarAddressSet(nearLidar, LIDAR_I2C_ADDR_SP);
    if (status < 0) {
      ERRP("lidarAddressSet() failed.\n");
      lidarClose(nearLidar);
      farLidar = NULL;
    }
    else {
      INFOP("success.\n");
      status = lidarUpdate(nearLidar);
      if (status < 0) {
        ERRP("lidarUpdate(nearLidar) error 0x%x\n", status);
      }
      else {
        INFOP("\tReading: %d\n", lidarDistGet(nearLidar));
      }
    }
  }

  INFOP("Initializing ultrasonic sensors...");

  sonarStart();
  status = sonarPollStart();
  if (status < 0) {
    ERRP("sonarPollStart() failed.\n");
  }
  else {
    INFOP("success.\n");
    gpioSleep(0, 0, 500000);
    int i;
    for (i = 0; i < ULTSND_SENSOR_COUNT; i++) {
      uint32_t dist = sonarReadUm(i);
      INFOP("\tSonar %d: Reading: %d\n", i, dist);
    }
  }

  INFOP("Initializing turn signals...");
  gpioSetMode(PIN_TURNSIG_L, PI_INPUT);
  gpioSetPullUpDown(PIN_TURNSIG_L, PI_PUD_UP);
  gpioGlitchFilter(PIN_TURNSIG_L, 10000);
  gpioSetMode(PIN_TURNSIG_R, PI_INPUT);
  gpioSetPullUpDown(PIN_TURNSIG_R, PI_PUD_UP);
  gpioGlitchFilter(PIN_TURNSIG_R, 10000);
  INFOP("success.\n");

  return 0;

}

void blindspotClear() {
  bs.alert_expire = 0.0;
  bs.notif_expire = 0.0;
}

void csClose() {
  sonarPollStop();
  lidarClose(nearLidar);
  lidarClose(farLidar);
  lidarClose(frontLidar);
  sonarStop();
  speedClose();
  jacketDisconnect();
  gpioWrite(PIN_RESET, PI_OFF);
  gpioTerminate();
}

int blindspotUpdate(int proxFlag) {

  int status;
  status = lidarUpdate(farLidar);
  if (status < 0) {
    ERRP("lidarUpdate(farLidar) error 0x%x\n", status);
  }

  status = lidarUpdate(nearLidar);
  if (status < 0) {
    ERRP("lidarUpdate(nearLidar) error 0x%x\n", status);
  }

  double time_curr = time_time();

  uint32_t far_tti = lidarTimeToImpactGetMs(farLidar);
  uint32_t near_tti = lidarTimeToImpactGetMs(nearLidar);

  double time_expire;

  int alert = 0, notif = 0, f_al = 0;

  if (near_tti < THRESH_BACK_TTI_ALERT_MSEC) {
    bs.time_tti_near_alert = time_curr;
    time_expire = ((double) near_tti) / 1000 + time_curr;
    printf("t=%.3f Near tti: %u\n", time_curr, near_tti);
    alert = 1;
  }
  else if (far_tti < THRESH_BACK_TTI_ALERT_MSEC) {
    bs.time_tti_far_alert = time_curr;
    time_expire = ((double) far_tti) / 1000 + time_curr;
    printf("t=%.3f Far tti: %u\n", time_curr, near_tti);
    alert = 1;
    f_al = 1;
  }

  if (alert) {
    if (bs.alert_expire < time_curr) {
      if (f_al) bs.alert_expire = time_curr + BS_FIRST_STEP_FAR_SEC;
      else bs.alert_expire = time_curr + BS_FIRST_STEP_NEAR_SEC;
      printf("t=%.3f: First step\n", time_curr);
    }
    else {
      bs.alert_expire += bs.alert_expire - time_curr;
      if (bs.alert_expire > time_expire) bs.alert_expire = time_expire;
    }
  }

  if (near_tti < THRESH_BACK_TTI_NOTIF_MSEC) {
    bs.time_tti_near_notif = time_curr;
    time_expire = ((double) near_tti) / 1000 + time_curr;
    notif = 1;
  }
  else if (far_tti < THRESH_BACK_TTI_NOTIF_MSEC) {
    bs.time_tti_far_notif = time_curr;
    time_expire = ((double) far_tti) / 1000 + time_curr;
    notif = 1;
  }

  if (!alert && notif) {
    if (bs.notif_expire < time_curr) {
      bs.notif_expire = time_curr + BS_FIRST_STEP_FAR_SEC;
    }
    else {
      bs.notif_expire += bs.notif_expire - time_curr;
      if (bs.notif_expire > time_expire) bs.notif_expire = time_expire;
    }
  }

  if (gpioRead(PIN_TURNSIG_L)) {
  // Turn signal not on, return
    return status;
  }

  if (proxFlag) {
    blindspotClear();
    jacketSet(JKP_MASK_BUZZ_L | JKP_MASK_VIB_L);
    UIP("Blind spot proximity alert!\n");
  }
  else if (bs.alert_expire > time_curr) {
    jacketSet(JKP_MASK_BUZZ_L);
    jacketSet(JKP_MASK_PROX_SL);
    UIP("Blind spot alert!\n");
  }
  else if (bs.notif_expire > time_curr) {
    jacketSet(JKP_MASK_VIB_L);
    jacketSet(JKP_MASK_PROX_SL);
    UIP("Blind spot notification.\n");
  }

  return status;

}

int brakingUpdate(int speed, int accel) {

  double time_curr = time_time();

#ifdef TRAINER
  (void) accel;
  int dv = speed - brakes.prev_speed;
  double dt = time_curr - brakes.prev_time;
  double a = ((double)dv) / dt;
  if (a < -500.0) brakes.time_on = time_curr;
#else
  if (speed < THRESH_BRAKE_SPEED_MM_PER_SEC || accel > brakes.accel_zero + THRESH_BRAKE_DECCELERATION_STEPS) {
    brakes.time_on = time_curr;
  }
#endif

  if (time_curr - brakes.time_on < BR_PERSIST_SEC) {
    jacketSet(JKP_MASK_BRAKE);
    UIP("Braking active.\n");
  }
  else {
    jacketUnset(JKP_MASK_BRAKE);
  }
  return 0;

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
    lidarDistGet(frontLidar);
    // DBGP("Vel:%d, Dist:%d, TTI:%d\n", lidarVelGet(frontLidar), lidarDistGet(frontLidar), tti);
#ifdef TRAINER
    if (tti < THRESH_FRONT_TTI_MSEC) {
      jacketSet(JKP_MASK_BUZZ_R);
      // jacketSet(JKP_MASK_PROX_SR);
      UIP("Collision alert!\n");
    }
    else {
      jacketUnset(JKP_MASK_BUZZ_R);
      // jacketUnset(JKP_MASK_PROX_SR);
    }
#else
    jacketUnset(JKP_MASK_BUZZ_R);
    if (tti < THRESH_FRONT_TTI_MSEC) {
      if (lidarDistGet(frontLidar) * MM_PER_CM * MM_PER_M <
          THRESH_FRONT_TTI_MSEC * speed * speed) {
      // This second check is basically comparing the time to impact assuming the obstacle
      // is stationary and using the bike's speed, and checking if it meets the threshold.
      // The threshold however is weighted by the speed, so at lower speeds it is harder to activate.
      // The unit for the speed weight is m/s, so moving at 1m/s will require the TTI to be the
      // unwieghted threshold tti.
        jacketSet(JKP_MASK_BUZZ_R);
        // jacketSet(JKP_MASK_PROX_SR);
        UIP("Collision alert!\n");
      }
    }
#endif

    jacketUnset(JKP_MASK_PROX_SL);
    int i;
    int proxFlag = 0;
    for (i = 0; i < ULTSND_SENSOR_COUNT; i++) {
      if (i == 1 || i == 2) continue;
      uint32_t dist = sonarReadUm(i);
      if (dist > ULTSND_MIN_DIST_UM && dist < THRESH_PROX_UM) {
        // DBGP("Sonar %d, %d\n", i, dist);
        jacketSet(JKP_MASK_PROX_SL);
        UIP("Proximity sensors active.\n");
        proxFlag = 1;
        break;
      }
    }

    jacketUnset(JKP_MASK_BUZZ_L | JKP_MASK_VIB_L);
    blindspotUpdate(proxFlag);

    if (!gpioRead(PIN_TURNSIG_L)) {
      if (blinkerStart(JKP_MASK_TURNSIG_L, 1) == 0) UIP("Left turn signal active.\n");
    }
    else {
      if (blinkerStop(JKP_MASK_TURNSIG_L) == 0) UIP("Left turn signal off.\n");
    }

    if (!gpioRead(PIN_TURNSIG_R)) {
      if (blinkerStart(JKP_MASK_TURNSIG_R, 1) == 0) UIP("Right turn signal active.\n");
    }
    else {
      if (blinkerStop(JKP_MASK_TURNSIG_R) == 0) UIP("Right turn signal off.\n");
    }

    if (!gpioRead(PIN_TURNSIG_L) && !gpioRead(PIN_TURNSIG_R)) {
      UIP("Temporary pause.\n");
      jacketUnset(0xFFFFFFFF);
      jacketUpdate();
      gpioSleep(0, 5, 0);
      if (!gpioRead(PIN_TURNSIG_L) && !gpioRead(PIN_TURNSIG_R)) {
        UIP("Shutting down...\n");
        csClose();
        return 0;
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

    // DBGP("Speed: %d, Accel: %d\n", speed, accel);

    brakingUpdate(speed, accel);

    status = jacketUpdate();
    if (status < 0) {
      ERRP("Jacket update failed\n");
    }
    gpioSleep(0, 0, SYSTEM_UPDATE_PERIOD_MIN_USEC);

  }

  return 0;

}
