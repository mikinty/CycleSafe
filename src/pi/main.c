
#include <pigpio.h>

#include "constants.h"
#include "interfaces/jacket.h"
#include "lidar/lidar.h"
#include "ultrasound/ultrasound.h"
#include "speed/speed.h"

/*
typedef struct blindspot_struct {
  double time_far_first;
  uint16_t dist_far_first;
  int vel_far_max;
  double tti_far;
  double time_near_expect;
  double time_near_first;
  uint16_t dist_near_first;
  int vel_near_max;
  double tti_near;
} blindspot_t;

blindspot_t bs;
*/

typedef struct blindspot_struct {
  double time_tti_near_notif;
  double time_tti_far_notif;
  double time_tti_near_alert;
  double time_tti_far_alert;
  double notif_expire;
  double alert_expire;
} blindspot_t;


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
  jacketCycle(250);
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
    printf("\tSpeed reading: %d\n", speed);
#ifndef TRAINER
    int accel = speedRead();
    if (accel < 0) {
      ERRP("speedRead(accel) error 0x%x\n", accel);
    }
    printf("\tAcceleration reading: %d\n", accel);
    brakes.accel_zero = accel;
#else
    printf("\tTrainer mode on! Accelerometer disabled.\n");
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
/*
void blindspotOldClear() {
  bs.time_far_first = 0.0;
  bs.dist_far_first = THRESH_BACK_FAR_MAX_DIST_CM;
  bs.tti_far = TTI_MAGIC_DOUBLE;
  bs.time_near_expect = 0.0;
  bs.time_near_first = 0.0;
  bs.dist_near_first = THRESH_BACK_NEAR_MAX_DIST_CM;
  bs.tti_near = TTI_MAGIC_DOUBLE;
}
*/
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
/*
int blindspotUpdateOld(int proxFlag) {

  double tti = TTI_MAGIC_DOUBLE;
  uint16_t dist_near, dist_far;
  int32_t vel_near, vel_far;
  int status;

  status = lidarUpdate(nearLidar);
  if (status < 0) {
    ERRP("lidarUpdate(nearLidar) error 0x%x\n", status);
  }

  status = lidarUpdate(farLidar);
  if (status < 0) {
    ERRP("lidarUpdate(farLidar) error 0x%x\n", status);
  }
  
  dist_near = lidarDistGet(nearLidar);
  dist_far = lidarDistGet(farLidar);
  double curr_time = time_time();
  
  if (proxFlag) {
    blindspotClear();
    if (gpioRead(PIN_TURNSIG_L)) {
    // Turn signal not on, return
  
      jacketUnset(JKP_MASK_BUZZ_L | JKP_MASK_VIB_L | JKP_MASK_VIB_R);
      return status;
    }
    else {
      jacketSet(JKP_MASK_BUZZ_L | JKP_MASK_VIB_L | JKP_MASK_VIB_R);
      return status;
    }
  }
  
  if (dist_near <= THRESH_BACK_NEAR_MAX_DIST_CM) {
      
    printf("Near dist: %d\n", dist_near);
    // First time, update first
    if (bs.time_near_first == 0.0) {
      bs.dist_near_first = dist_near;
      bs.time_near_first = curr_time;
    }
    // This could technically go into the first as well
    bs.time_near_expect = 0.0;
    bs.tti_far = TTI_MAGIC_DOUBLE;
    
    vel_near = lidarVelGet(nearLidar);
    if (vel_near > bs.vel_near_max) {
      bs.vel_near_max = vel_near;
      printf("Near vel: %d\n", vel_near);
    }
    
    if (bs.vel_near_max > 0.001) bs.tti_near = bs.time_near_first + (MM_PER_CM * (bs.dist_near_first / bs.vel_near_max));
    
  }
  else {
    if (bs.time_near_first != 0.0) {
      //INFOP("Near reset\n");
    }
    if (bs.time_near_expect != 0.0) {
      if (curr_time > bs.time_near_expect) {
        // The car is slowing down, change the tti
        double dt = curr_time - bs.time_far_first;
        if (dt > 0.001) {
          double vel = (bs.dist_far_first - THRESH_BACK_NEAR_MAX_DIST_CM) / dt;
          bs.tti_far = THRESH_BACK_NEAR_MAX_DIST_CM / vel;
        }
      }
    }
    
  }
  
  
  if (dist_far < THRESH_BACK_FAR_MAX_DIST_CM) {
  
      
    printf("far dist: %d\n", dist_far);
    if (bs.time_far_first == 0.0) {
      bs.dist_far_first = dist_far;
      bs.time_far_first = curr_time;
    }
    
    vel_far = lidarVelGet(farLidar);
    if (vel_far > bs.vel_far_max) bs.vel_far_max = vel_far;
  
  
    if (bs.vel_far_max > 0.001) {
      bs.time_near_expect = bs.time_far_first + (MM_PER_CM * (bs.dist_far_first - THRESH_BACK_NEAR_MAX_DIST_CM) / bs.vel_far_max);
      bs.tti_far = bs.time_far_first + (MM_PER_CM * bs.dist_far_first / bs.vel_far_max);
    }
    
  }
  
  else {
    
    if (bs.time_far_first != 0.0) {
      //INFOP("Far reset\n");
    }
  }
  
  jacketUnset(JKP_MASK_BUZZ_L | JKP_MASK_VIB_L | JKP_MASK_VIB_R);
  
  if (gpioRead(PIN_TURNSIG_L)) {
    // Turn signal not on, return
    return status;
  }
  
  if (bs.tti_near != TTI_MAGIC_DOUBLE) {
    tti = bs.tti_near;
  }
  else if (bs.tti_far != TTI_MAGIC_DOUBLE) {
    tti = bs.tti_far;
  }
  else tti = TTI_MAGIC_DOUBLE;
  
  
  if (tti * MSEC_PER_SEC < THRESH_BACK_TTI_ALERT_MSEC) {
    jacketSet(JKP_MASK_BUZZ_L);
    // printf("Vel:%d, Dist:%d, TTI:%d\n", lidarVelGet(nearLidar), lidarDistGet(Lidar), tti);
  }
  if (tti * MSEC_PER_SEC < THRESH_BACK_TTI_NOTIF_MSEC) {
    jacketSet(JKP_MASK_VIB_L | JKP_MASK_VIB_R);
    // printf("Vel:%d, Dist:%d, TTI:%d\n", lidarVelGet(frontLidar), lidarDistGet(frontLidar), tti);
  }
  
  return status;
  
}
*/
int blindSpotUpdate(int proxFlag) {

  status = lidarUpdate(farLidar);
  if (status < 0) {
    ERRP("lidarUpdate(farLidar) error 0x%x\n", status);
  }
  
  status = lidarUpdate(nearLidar);
  if (status < 0) {
    ERRP("lidarUpdate(nearLidar) error 0x%x\n", status);
  }
  
  double time_curr = time_time();
  
  int far_tti = lidarTimeToImpactGetMs(farLidar);
  int near_tti = lidarTimeToImpactGetMs(nearLidar);
  
  double time_expire;
  
  int alert = 0, notif = 0;
    
  if (near_tti < THRESH_BACK_TTI_ALERT_MSEC) {
    bs.time_tti_near_alert = time_curr;
    time_expire = ((double) near_tti) / 1000 + time_curr;
    alert = 1;
  }
  else if (far_tti < THRESH_BACK_TTI_ALERT_MSEC) {
    bs.time_tti_far_alert = time_curr;
    time_expire = ((double) far_tti) / 1000 + time_curr;
    alert = 1;
  }
  
  if (alert) {
    if (bs.alert_expire < time_curr) {
      bs.alert_expire = time_curr + BS_FIRST_STEP_SEC;
    }
    else {
      bs.alert_expire += bs.alert_expire - time_curr
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
      bs.notif_expire = time_curr + BS_FIRST_STEP_SEC;
    }
    else {
      bs.notif_expire += bs.notif_expire - time_curr
      if (bs.notif_expire > time_expire) bs.notif_expire = time_expire;
    }
  }
  
  if (gpioRead(PIN_TURNSIG_L)) {
  // Turn signal not on, return  
    jacketUnset(JKP_MASK_BUZZ_L | JKP_MASK_VIB_L | JKP_MASK_PROX_SL);
    return status;
  }
  
  if (proxFlag) {
    // blindspotClear();
    jacketSet(JKP_MASK_BUZZ_L | JKP_MASK_VIB_L | JKP_MASK_VIB_R);
  }
  else if (bs.alert_expire > time_curr) {
    jacketSet(JKP_MASK_BUZZ_L);
    jacketSet(JKP_MASK_PROX_SL);
  }
  else if (bs.notif.expire > time_curr) {
    jacketSet(JKP_MASK_VIB_L);
    jacketSet(JKP_MASK_PROX_SL);
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
  }
  else {
    jacketUnset(JKP_MASK_BRAKE);
  }

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
    // printf("Vel:%d, Dist:%d, TTI:%d\n", lidarVelGet(frontLidar), lidarDistGet(frontLidar), tti);
#ifdef TRAINER
    if (tti < THRESH_FRONT_TTI_MSEC) {
      jacketSet(JKP_MASK_BUZZ_R);
      // jacketSet(JKP_MASK_PROX_SR);
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
        INFOP("Sonar %d, %d\n", i, dist);
        jacketSet(JKP_MASK_PROX_SL);
        proxFlag = 1;
        break;
      }
    }
    
    blindspotUpdate(proxFlag);

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

    //printf("Speed: %d, Accel: %d\n", speed, accel);
    
    brakingUpdate(speed, accel);

    status = jacketUpdate();
    if (status < 0) {
      ERRP("Jacket update failed\n");
    }
    gpioSleep(0, 0, 10000);

  }

  return 0;

}
