
#include "lidar.h"

/** @brief  I2C bus number */
#define LIDAR_I2C_BUS 1
/** @brief  The LIDAR's default I2C address */
#define LIDAR_I2C_ADDR_DEFAULT 0x62

/* ==== LIDAR registers ==== */
/** @brief  Acquisition command (W) */
#define LIDAR_REG_ACQ_COMMAND 0x00
/** @brief  */
#define LIDAR_REG_STATUS 0x01
/** @brief  */
#define LIDAR_REG_SIG_COUNT_VAL 0x02
/** @brief  */
#define LIDAR_REG_ACQ_CONFIG_REG 0x04
/** @brief  */
#define LIDAR_REG_LEGACY_RESET_EN 0x06
/** @brief  */
#define LIDAR_REG_SIGNAL_STRENGTH 0x0E
/** @brief  */
#define LIDAR_REG_FULL_DELAY_HIGH 0x0F
/** @brief  */
#define LIDAR_REG_FULL_DELAY_LOW 0x10
/** @brief  */
#define LIDAR_REG_REF_COUNT_VAL 0x12
/** @brief  */
#define LIDAR_REG_UNIT_ID_HIGH 0x16
/** @brief  */
#define LIDAR_REG_UNIT_ID_LOW 0x17
/** @brief  */
#define LIDAR_REG_I2C_ID_HIGH 0x18
/** @brief  */
#define LIDAR_REG_I2C_ID_LOW 0x19
/** @brief  */
#define LIDAR_REG_I2C_SEC_ADDR 0x1A
/** @brief  */
#define LIDAR_REG_THRESHOLD_BYPASS 0x1C
/** @brief  */
#define LIDAR_REG_I2C_CONFIG 0x1E

#define LIDAR_QLEN 16
#define LIDAR_QLEN_MASK 0xF

typedef struct __lidar_dev_t {
  int i2cHandle;
  uint16_t unitId;

  uint16_t dist; // cm
  uint16_t qDist[LIDAR_QLEN];
  int32_t vel; // mm/s
  int32_t qVel[LIDAR_QLEN];
  uint32_t qTick[LIDAR_QLEN];
  uint8_t qIndex;
} lidar_dev_t;

int lidarIdGet(lidar_dev_t *dev) {

  int readVal;

  readVal = i2cReadWordData(dev->i2cHandle, LIDAR_REG_UNIT_ID_HIGH);
  if (readVal < 0) return readVal;

  return __bswap_16(readVal);

}

uint16_t lidarDistGet(lidar_dev_t *dev) {
  return dev->dist;
}

int32_t lidarVelGet(lidar_dev_t *dev) {
  return dev->vel;
}

uint32_t lidarTimeToImpactGet(lidar_dev_t *dev) {

  return dev->dist * MM_PER_CM * MSEC_PER_SEC / dev->vel;

}

int lidarAddressSet(lidar_dev_t *dev, uint8_t newAddr) {

  if (newAddr & 0x80) return -1;

  int status = 0;
  status = i2cWriteWordData(dev->i2cHandle, LIDAR_REG_I2C_ID_HIGH, __bswap_16(dev->unitId));
  if (status < 0) return status;

  status = i2cWriteByteData(dev->i2cHandle, LIDAR_REG_I2C_SEC_ADDR, newAddr << 1);
  if (status < 0) return status;

  i2cClose(dev->i2cHandle);

  int device;
  device = i2cOpen(LIDAR_I2C_BUS, newAddr, 0);
  if (device < 0) {
    return device;
  }
  dev->i2cHandle = device;

  return 0;
}

int lidarAcquireStart(lidar_dev_t *dev) {
  return i2cWriteByteData(dev->i2cHandle, LIDAR_REG_ACQ_COMMAND, 0x1);
}

int lidarDistRead(lidar_dev_t *dev) {

  int status;

  do {
    status = i2cReadByteData(dev->i2cHandle, LIDAR_REG_STATUS);
    if (status < 0) return status;
  } while (status & 1);

  int readVal;
  readVal = i2cReadWordData(dev->i2cHandle, LIDAR_REG_FULL_DELAY_HIGH);
  if (readVal < 0) return readVal;

  return __bswap_16(readVal);

}

void lidarDistEnq(lidar_dev_t *dev, uint16_t dist, uint16_t tick, int32_t vel) {

  int i = dev->qIndex;
  dev->qDist[i] = dist;
  dev->qTick[i] = tick;
  dev->qVel[i] = vel;

  i++;

  if (dev->qDist[0] == 0) {
    // First reading
    dev->dist = dist;
  }
  else {
    dev->dist = (7 * dev->dist + dist) / 8;

    if (dev->qDist[1] == 0) {
      // Second reading
      dev->vel = vel;
    }
    else {
      dev->vel = (3 * dev->vel + vel) / 4;
    }
  }

}

int lidarUpdate(lidar_dev_t *dev) {

  int status;
  int readVal;

  uint32_t tick, prevTick; // ppTick;
  uint16_t dist, prevDist; // ppDist;
  int32_t vel;
  int i = dev->qIndex;

  status = i2cWriteByteData(dev->i2cHandle, LIDAR_REG_ACQ_COMMAND, 0x1);
  if (status < 0) {
    return status;
  }

  do {
    status = i2cReadByteData(dev->i2cHandle, LIDAR_REG_STATUS);
    if (status < 0) return status;
  } while (status & 1);

  tick = gpioTick();

  readVal = i2cReadWordData(dev->i2cHandle, LIDAR_REG_FULL_DELAY_HIGH);
  if (readVal < 0) return readVal;

  dist = __bswap_16(readVal);

  // Try to filter out bad readings.
  // Throw out anything too close or too far (20cm - 25m)
  if (dist < 20 || dist > 2500) return 1;

  prevDist = dev->qDist[(i - 1) & LIDAR_QLEN_MASK];
  prevTick = dev->qTick[(i - 1) & LIDAR_QLEN_MASK];

  // This is in um/ms, or mm/s
  vel = (((int32_t) dist) - ((int32_t) prevDist)) * UM_PER_CM * USEC_PER_MSEC / (int32_t) (tick - prevTick);

  lidarDistEnq(dev, dist, tick, vel);

  /*
  // Throw out the previous reading if it seems like an anomaly.
  ppDist = dev->qDist[(i - 2) & LIDAR_QLEN_MASK];
  ppTick = dev->qTick[(i - 2) & LIDAR_QLEN_MASK];
  */
  return 0;


}

void lidarClose(lidar_dev_t *dev) {

  i2cClose(dev->i2cHandle);
  free(dev);

}

lidar_dev_t *lidarInit(uint16_t id) {

  lidar_dev_t *dev = malloc(sizeof(lidar_dev_t));
  if (dev == NULL) {
    ERRP("malloc() failed.\n");
    return NULL;
  }

  dev->i2cHandle = i2cOpen(LIDAR_I2C_BUS, LIDAR_I2C_ADDR_DEFAULT, 0);
  if (dev->i2cHandle < 0) {
    ERRP("i2cOpen() failed.\n");
    free(dev);
    return NULL;
  }
  dev->unitId = id;

  int i = 0;
  while (i < LIDAR_QLEN) dev->qDist[i++] = 0;
  dev->qIndex = 0;

  return dev;

}

int lidarTest(int reps, int delayUs) {

  int status;
  lidar_dev_t *dev;

  int readVal;

  dev = lidarInit(0);
  if (dev == NULL) {
    printf("lidarInit() error\n");
    return -1;
  }

  readVal = lidarIdGet(dev);
  if (readVal >= 0) printf("LIDAR ID: %d\n", readVal);

  int i;
  for (i = 0; i < reps; i++) {
    status = lidarAcquireStart(dev);
    if (status < 0) {
      printf("lidarAcquireStart() error %d\n", status);
      break;
    }

    int dist;
    dist = lidarDistRead(dev);
    if (dist < 0) {
      printf("lidarDistRead() error %d\n", dist);
      break;
    }
    printf("%d cm\n", dist);

    gpioSleep(PI_TIME_RELATIVE, 0, delayUs);

  }

  lidarClose(dev);

  return 0;

}

#ifndef FULL_BUILD
int main() {

  int status = gpioInitialise();
  if (status < 0) {
    printf("gpioInitialise() error %d\n", status);
    return status;
  }

  lidarTest(50, 500000);

  gpioTerminate();

  return 0;

}
#endif
