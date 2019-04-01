
#include "speed.h"

int speed_fd = -1;


int speedAvailable() {

  int status;
  int result;
  status = ioctl(speed_fd, FIONREAD, &result);
  if (status == -1) return 0;

  return result;
}

int speedRead() {

  int result;

  int status = read(speed_fd, &result, sizeof(int));
  if (status < 0) {
    ERRP("read() failed %d.\n", errno);
    return status;
  }
  if (status != sizeof(int)) {
    ERRP("read() only %d bytes.\n", status);
    return -1;
  }

  return result;

}

int speedRequest() {

  int status;

  /*status = tcflush(speed_fd, TCIFLUSH);
  if (status < 0) {
    ERRP("tcflush() failed %d.\n", errno);
    return status;
  }*/

  char c = SPEED_REQ_SPEED;
  status = write(speed_fd, &c, 1);
  if (status != 1) {
    ERRP("write() failed %d.\n", errno);
    return -1;
  }
  return 0;

}

int speedInit() {
  speed_fd = open(SPEED_USB_PORT, O_RDWR | O_NOCTTY | O_NONBLOCK);
  if (speed_fd < 0) {
    ERRP("open() failed %d.\n", errno);
    return -1;
  }

  int status;
  struct termios opt;

  status = tcgetattr(speed_fd, &opt);
  if (status < 0) {
    ERRP("tcgetattr() failed %d.\n", errno);
    return status;
  }

  cfmakeraw(&opt);

  status = cfsetspeed(&opt, SPEED_BAUD_RATE_SPEED_T); // set baud rate
  if (status < 0) {
    ERRP("cfsetspeed() failed %d.\n", errno);
    return status;
  }

  opt.c_cc [VMIN]  = 0;
  opt.c_cc [VTIME] = 0;

  // Set to 8N1
  status = tcflush(speed_fd, TCIFLUSH);
  if (status < 0) {
    ERRP("tcflush() failed %d.\n", errno);
    return status;
  }

  status = tcsetattr(speed_fd, TCSANOW, &opt);
  if (status < 0) {
    ERRP("tcsetattr() failed %d.\n", errno);
    return status;
  }

  return 0;

}

void speedClose() {
  if (speed_fd >= 0) close(speed_fd);
}

int speedTest() {

  int status;
  int spd;

  status = speedInit();
  if (status < 0) {
    printf("speedInit() error %d\n", status);
    return status;
  }

  int i;
  for (i = 0; i < 60; i++) {
    status = speedRequest();
    if (status < 0) {
      printf("speedRequest() error %d\n", status);
      //speedClose();
      //return status;
    }

    //while (!speedAvailable());
    spd = speedRead();
    if (spd < 0) {
      printf("speedRead() error %d\n", spd);
      //speedClose();
      //return spd;
    }

    printf("%d mm/s\n", spd);
    sleep(1);

  }

  speedClose();
  return 0;

}

int main() {
  return speedTest();
}
