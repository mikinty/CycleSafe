
#include "jacket.h"


uint32_t prev_jacket_state = 0;;
uint32_t jacket_state = 0;
uint32_t last_update_time = 0;
int jacket_fd = -1;

void jacketSet(uint32_t mask) {
  jacket_state |= mask;
}

void jacketUnset(uint32_t mask) {
  jacket_state &= ~mask;
}

int jacketUpdate() {
  
  int status = 0;
  uint32_t magic = JKP_MAGIC;
  
  if (prev_jacket_state != jacket_state) {
    prev_jacket_state = jacket_state;
    printf("Send: 0x%x\n", jacket_state);
    // last_update_time = currTick;
#ifdef DEBUG_NO_CONNECT
    return 0;
#else
    status += write(jacket_fd, &magic, sizeof(uint32_t));
    status += write(jacket_fd, &jacket_state, sizeof(uint32_t));
    return status;
#endif
  }
  return status;
}

void jacketDisconnect() {
  if (jacket_fd >= 0) close(jacket_fd);
  jacket_fd = -1;
}

int jacketConnect() {
#ifdef DEBUG_NO_CONNECT
  return 0;
#else
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
#endif
}

int jacketCycle(int sleepMs) {
  int status;
  struct timespec ts;
  ts.tv_sec = 0;
  ts.tv_nsec = NSEC_PER_MSEC * sleepMs;

  int i;
  for (i = 0x2; i != JKP_MASK_MAX; i <<= 1) {

    jacketSet(i);
    status = jacketUpdate();
    if (status < 0) {
      printf("jacketUpdate() failed %d\n", status);
    }
    nanosleep(&ts, NULL);

    jacketUnset(i);
    status = jacketUpdate();
    if (status < 0) {
      printf("jacketUpdate() failed %d\n", status);
    }

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
  printf("Connected...\n");

  int i;
  for (i = 1; i < 32 / sleepSec; i++) {

    jacketSet(1 << i);
    status = jacketUpdate();
    if (status < 0) {
      printf("jacketUpdate() failed %d\n", status);
      jacketDisconnect();
      return status;
    }
    printf("ON\n");

    sleep(sleepSec);

    jacketUnset(1 << i);
    status = jacketUpdate();
    if (status < 0) {
      printf("jacketUpdate() failed %d\n", status);
      jacketDisconnect();
      return status;
    }
    printf("OFF\n");

    sleep(sleepSec);

  }

  jacketDisconnect();
  return 0;

}

#ifndef FULL_BUILD
int main() {
  return jacketTest(1);
}
#endif
