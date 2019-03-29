
#include <pigpio.h>

#include "constants.h"
#include "interfaces/jacket.h"
#include "lidar/lidar.h"
#include "ultrasound/ultrasound.h"



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

  status = lidarStart();
  if (status < 0) {
    ERRP("sonarStart() failed.\n");
    sonarStop();
    jacketDisconnect();
    gpioTerminate();
    return status;
  }

}

int main() {

  while (1) {

  }




  if gpioStatus

}
