
#include "jacket.h"

uint32_t jacket_state = 0;
int jacket_fd = -1;

void jacketSet(uint32_t mask) {
  jacket_state |= mask;
}

void jacketUnset(uint32_t mask) {
  jacket_state &= ~mask;
}

int jacketUpdate() {

  int status;
  status = write(jacket_fd, &jacket_state, sizeof(uint32_t));
  return status;

}

void jacketDisconnect() {
  if (jacket_fd >= 0) close(jacket_fd);
  jacket_fd = -1;
}

int jacketConnect() {

  struct sockaddr_rc addr;
  int status;

  // allocate a socket
  jacket_fd = socket(AF_BLUETOOTH, SOCK_STREAM, BTPROTO_RFCOMM);

  // set the connection parameters (who to connect to)
  addr.rc_family = AF_BLUETOOTH;
  addr.rc_channel = (uint8_t) 1;
  str2ba(JACKET_BT_ADDR, &addr.rc_bdaddr);

  // connect to server
  status = connect(jacket_fd, (struct sockaddr *)&addr, sizeof(addr));
  if (status < 0) {
    close(jacket_fd);
    return status;
  }

  return 0;

}

int jacketTest(int sleepSec) {

  int status;
  status = jacketConnect();
  if (status < 0) {
    printf("jacketConnect() failed %d\n", status);
    return status;
  }

  int i;
  for (i = 0; i < 60 / sleepSec; i++) {

    jacketSet(~JKP_MASK_CONTROL);
    status = jacketUpdate();
    if (status < 0) {
      printf("jacketUpdate() failed %d\n", status);
      jacketDisconnect();
      return status;
    }

    sleep(sleepSec);

    jacketUnset(~JKP_MASK_CONTROL);
    status = jacketUpdate();
    if (status < 0) {
      printf("jacketUpdate() failed %d\n", status);
      jacketDisconnect();
      return status;
    }

    sleep(sleepSec);

  }

  jacketDisconnect();
  return 0;

}

int main() {
  return jacketTest(3);
}