
#include "phone.h"

int phone_fd = -1;
int listen_fd = -1;


int phoneRead() {

  int bytes_read;
  char result = 0;

  bytes_read = read(phone_fd, &result, sizeof(char));
  if (bytes_read == 1) {
    return result;
  }
  else return -1;

}

int phoneInit() {

  struct sockaddr_rc loc_addr = { 0 }, rem_addr = { 0 };
  char buf[1024] = { 0 };
  int status, bytes_read;
  socklen_t opt = sizeof(rem_addr);

  // allocate socket
  listen_fd = socket(AF_BLUETOOTH, SOCK_STREAM, BTPROTO_RFCOMM);
  if (listen_fd < 0) {
    ERRP("socket() failed.\n");
    return -1;
  }

  // bind socket to port 1 of the first available
  // local bluetooth adapter
  loc_addr.rc_family = AF_BLUETOOTH;
  loc_addr.rc_bdaddr = *BDADDR_ANY;
  loc_addr.rc_channel = (uint8_t) 1;
  status = bind(listen_fd, (struct sockaddr *)&loc_addr, sizeof(loc_addr));
  if (status < 0) {
    ERRP("bind() failed.\n");
    close(listen_fd);
    return -1;
  }

  // put socket into listening mode
  status = listen(listen_fd, 1);
  if (status < 0) {
    ERRP("listen() failed.\n");
    close(listen_fd);
    return -1;
  }

  INFOP("Waiting for phone to connect...\n");

  // accept one connection
  phone_fd = accept(listen_fd, (struct sockaddr *)&rem_addr, &opt);

  ba2str(&rem_addr.rc_bdaddr, buf);
  INFOP(stderr, "Connected to %s.\n", buf);

  status = close(listen_fd);
  if (status < 0) {
    ERRP("close(listen_fd) failed.\n");
  }
  else {
    listen_fd = -1;
  }

  return 0;
}

int phoneClose() {
  int status;

  status = close(phone_fd);
  if (status < 0) {
    ERRP("close(phone_fd) failed.\n");
  }
  else {
    phone_fd = -1;
  }

  return status;
}
