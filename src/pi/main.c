
#include <pigpio.h>

#include "constants.h"
#include "interfaces/jacket.h"
#include "lidar/lidar.h"
#include "ultrasound/ultrasound.h"

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

  status = sonarStart();
  if (status < 0) {
    ERRP("sonarStart() failed.\n");
    jacketDisconnect();
    gpioTerminate();
    return status;
  }

  frontLidar = lidarStart(LIDAR_HP_ID);
  if (status < 0) {
    ERRP("lidarStart() failed.\n");
    sonarStop();
    jacketDisconnect();
    gpioTerminate();
    return status;
  }

  status = sonarPollStart();
  if (status < 0) {
    ERRP("sonarPollStart() failed.\n");
    lidarClose(frontLidar);
    sonarStop();
    jacketDisconnect();
    gpioTerminate();
    return status;
  }


}

int close() {
  sonarPollStop();
  lidarClose(frontLidar);
  sonarStop();
  jacketDisconnect();
  gpioTerminate();
}

int main() {

  init();

  while (1) {

    lidarUpdate(frontLidar);
    if (lidarTimeToImpactGet(frontLidar) < 3 * MSEC_PER_SEC) {
      jacketSet(JKP_MASK_BUZZ_L | JKP_MASK_BUZZ_R);
    }
    else {
      jacketUnset(JKP_MASK_BUZZ_L | JKP_MASK_BUZZ_R);
    }
    jacketUpdate();

  }

  return 0;

}
