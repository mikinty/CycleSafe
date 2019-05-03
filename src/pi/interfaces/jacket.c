
#include "jacket.h"

uint32_t prev_jacket_state = 0;;
uint32_t jacket_state = 0;
double last_update_time = 0.0;
int jacket_fd = -1;

blinker_t *blink_list = NULL;

int blinkerStart(uint32_t mask, int slow) {

  // Check if signal already in the blink list
  blinker_t *bl = blink_list;
  while (bl != NULL) {
    if (bl->mask & mask) return -1;
    bl = bl->next;
  }

  blinker_t *newB = malloc(sizeof(blinker_t));
  newB->mask = mask;
  newB->next = blink_list;
  newB->slow = slow;
  blink_list = newB;

  double time_curr = time_time();
  if (slow) {
    newB->t_on = time_curr + BLINKER_SLOW_T_MSEC;
    newB->t_off = time_curr + BLINKER_SLOW_ON_MSEC;
  }
  else {
    newB->t_on = time_curr + BLINKER_FAST_T_MSEC;
    newB->t_off = time_curr + BLINKER_FAST_ON_MSEC;
  }
  return 0;
}

int blinkerStop(uint32_t mask) {

  blinker_t *currB = blink_list;
  blinker_t *prevB = NULL;
  while (currB != NULL) {
    if (currB->mask & mask) break;
    prevB = currB;
    currB = currB->next;
  }
  if (currB == NULL) return -1;

  jacketUnset(currB->mask);
  if (prevB == NULL) blink_list = currB->next;
  else prevB->next = currB->next;
  free(currB);

}

void blinkerUpdate() {

  blinker_t *bl = blink_list;
  int time_curr = time_time();
  while (bl != NULL) {
    if (time_curr > bl->t_on) { // could do a while too
      if (bl->slow) bl->t_on += BLINKER_SLOW_T_MSEC;
      else bl->t_on += BLINKER_FAST_T_MSEC;
    }

    if (time_curr > bl->t_off) {  // could do a while too
      if (bl->slow) bl->t_off += BLINKER_SLOW_T_MSEC;
      else bl->t_off += BLINKER_FAST_T_MSEC;
    }

    if (bl->t_on < bl->t_off) {
      jacketUnset(bl->mask);
    } else {
      jacketSet(bl->mask);
    }

    bl = bl->next;
  }

}

void blinkerStopAll() {

  while (blink_list != NULL) {
    blinkerStop(blink_list->mask);
  }

}


void jacketSet(uint32_t mask) {
  jacket_state |= mask;
}

void jacketUnset(uint32_t mask) {
  jacket_state &= ~mask;
}

int jacketUpdate() {

  blinkerUpdate();
  int status = 0;
  uint32_t magic = JKP_MAGIC;

  if (prev_jacket_state != jacket_state || last_update_time + JACKET_UPDATE_PERIOD_MIN_SEC < time_time()) {
    prev_jacket_state = jacket_state;
    DBGP("Send: 0x%x\n", jacket_state);
    last_update_time = time_time();
#ifdef NOJACKET
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
#ifdef NOJACKET
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
      ERRP("jacketUpdate() failed %d\n", status);
    }
    nanosleep(&ts, NULL);

    jacketUnset(i);
    status = jacketUpdate();
    if (status < 0) {
      ERRP("jacketUpdate() failed %d\n", status);
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
