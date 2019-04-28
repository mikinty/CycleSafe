
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
#define LIDARHP_REG_LEGACY_RESET_EN 0x06
/** @brief  */
#define LIDARSP_REG_VELOCITY 0x09
/** @brief  */
#define LIDARSP_REG_PEAK_CORR 0x0C
/** @brief  */
#define LIDARSP_REG_NOISE_PEAK 0x0D
/** @brief  */
#define LIDAR_REG_SIGNAL_STRENGTH 0x0E
/** @brief  */
#define LIDAR_REG_FULL_DELAY_HIGH 0x0F
/** @brief  */
#define LIDAR_REG_FULL_DELAY_LOW 0x10
/** @brief  */
#define LIDAR_REG_REF_COUNT_VAL 0x12
/** @brief  */
#define LIDARSP_REG_LAST_DELAY_HIGH 0x14
/** @brief  */
#define LIDARSP_REG_LAST_DELAY_LOW 0x15
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

#define LIDARSP_I2C_REG_BLOCK_MASK 0x80

#define LIDARSP_MASK_ACQ_CONFIG_REG_ACQ 0x4
#define LIDARSP_MASK_ACQ_CONFIG_REG_ACQ_NOBIAS 0x3

#define LIDARHP_MASK_I2C_CONFIG_CHANGE_ADDR (0x1 << 4)
#define LIDAR_MASK_I2C_CONFIG_DISABLE_DEFAULT (0x1 << 3)

#define LIDAR_QLEN 16
#define LIDAR_QLEN_MASK 0xF

#define LIDAR_TIMEOUT_USEC 100000

typedef struct __lidar_dev_t {
  int i2cHandle;
  uint16_t unitId;
  uint8_t is_hp;

  uint16_t dist; // cm
  uint16_t qDist[LIDAR_QLEN];
  int32_t vel; // mm/s
  int32_t qVel[LIDAR_QLEN];
  uint32_t qTick[LIDAR_QLEN];
  uint8_t qIndex;
  uint8_t qVIndex;  
} lidar_dev_t;

int lidarIdGet(lidar_dev_t *dev) {

  int readVal;
  
  unsigned reg = (dev->is_hp ? 0 : LIDARSP_I2C_REG_BLOCK_MASK) | LIDAR_REG_UNIT_ID_HIGH;

  readVal = i2cReadWordData(dev->i2cHandle, reg);
  if (readVal < 0) return readVal;

  return __bswap_16(readVal);

}

uint16_t lidarDistGet(lidar_dev_t *dev) {
  int currTick = gpioTick();
  int i = (dev->qIndex - 1) & LIDAR_QLEN_MASK;
  if (currTick - dev->qTick[i] > LIDAR_TIMEOUT_USEC) {
    return LIDAR_MAX_DIST_CM;
  }
  return dev->dist;
}

int32_t lidarVelGet(lidar_dev_t *dev) {
  int currTick = gpioTick();
  int i = (dev->qIndex - 1) & LIDAR_QLEN_MASK;
  if (currTick - dev->qTick[i] > LIDAR_TIMEOUT_USEC) {
    return 0;
  }
  return dev->vel;
}

uint32_t lidarTimeToImpactGetMs(lidar_dev_t *dev) {

  if (dev->vel >= 0) return (uint32_t) -1;
  return (dev->dist * MM_PER_CM * MSEC_PER_SEC) / -(dev->vel);

}

int lidarAddressSet(lidar_dev_t *dev, uint8_t newAddr) {

  if (newAddr & 0x81) return -1;

  int status = 0;
  
  unsigned reg = (dev->is_hp ? 0 : LIDARSP_I2C_REG_BLOCK_MASK) | LIDAR_REG_I2C_ID_HIGH;
  status = i2cWriteWordData(dev->i2cHandle, reg, __bswap_16(dev->unitId));
  if (status < 0) {
    ERRP("i2cWriteWordData() failed 0x%x.\n", status);
    return status;
  }

  // printf("Addr set: 0x%x\n", newAddr);

  status = i2cWriteByteData(dev->i2cHandle, LIDAR_REG_I2C_SEC_ADDR, dev->is_hp ? (newAddr << 1) : newAddr);
  if (status < 0) {
    ERRP("i2cWriteByteData() failed 0x%x.\n", status);
    return status;
  }
  
  status = i2cWriteByteData(dev->i2cHandle, LIDAR_REG_I2C_CONFIG, dev->is_hp ? LIDARHP_MASK_I2C_CONFIG_CHANGE_ADDR : 0x0);
  if (status < 0) {
    ERRP("i2cWriteByteData() failed 0x%x.\n", status);
    return status;
  }

  i2cClose(dev->i2cHandle);

  int device;
  device = i2cOpen(LIDAR_I2C_BUS, newAddr, 0);
  if (device < 0) {
    ERRP("i2cOpen() failed 0x%x.\n", status);
    return device;
  }
  
  dev->i2cHandle = device;
  
  status = i2cWriteByteData(
    dev->i2cHandle,
    LIDAR_REG_I2C_CONFIG, 
    (dev->is_hp ? LIDARHP_MASK_I2C_CONFIG_CHANGE_ADDR : 0x0) | LIDAR_MASK_I2C_CONFIG_DISABLE_DEFAULT
  );
  if (status < 0) {
    ERRP("i2cWriteByteData() failed 0x%x.\n", status);
    return status;
  }

  return 0;
}

int lidarAcquireStart(lidar_dev_t *dev, int noBiasFlag) {
  if (noBiasFlag) {
    return i2cWriteByteData(dev->i2cHandle, LIDAR_REG_ACQ_COMMAND, LIDARSP_MASK_ACQ_CONFIG_REG_ACQ_NOBIAS);
  }
  else {
    return i2cWriteByteData(dev->i2cHandle, LIDAR_REG_ACQ_COMMAND, LIDARSP_MASK_ACQ_CONFIG_REG_ACQ);
  }
}

int lidarDistRead(lidar_dev_t *dev) {

  int status;

  do {
    status = i2cReadByteData(dev->i2cHandle, LIDAR_REG_STATUS);
    if (status < 0) return status;
  } while (status & 1);

  int readVal;
  
  unsigned reg = (dev->is_hp ? 0 : LIDARSP_I2C_REG_BLOCK_MASK) | LIDAR_REG_FULL_DELAY_HIGH;
  readVal = i2cReadWordData(dev->i2cHandle, reg);
  if (readVal < 0) return readVal;

  return __bswap_16(readVal);

}

void lidarDistEnq(lidar_dev_t *dev, uint16_t dist, uint32_t tick) {

  int i = dev->qIndex;
  int32_t vel;
  uint16_t prevDist;
  uint32_t prevTick;
  
  if (dev->qVIndex == i) {
    // This is the first reading
    vel = 0;
  }
  else {
  
    prevDist = dev->qDist[dev->qVIndex];
    prevTick = dev->qTick[dev->qVIndex];

    // This is in um/ms, or mm/s

    int64_t dx = ((((int64_t) dist) - ((int64_t) prevDist)) * UM_PER_CM * USEC_PER_MSEC);
    int64_t dt = tick - prevTick;
    vel = dx / dt;
    
  }
  
  // printf("i:%d, vi:%d, d:%d, v:%d\n", i, dev->qVIndex, dist, vel);
  
  dev->qDist[i] = dist;
  dev->qTick[i] = tick;
  dev->qVel[i] = vel;

  dev->qIndex = (i + 1) & LIDAR_QLEN_MASK;

  if (dev->qDist[1] == 0) {
    // First reading
    dev->dist = dist;
  }
  else {
    dev->dist = (3 * dev->dist + dist) / 4;

    if (dev->qDist[2] == 0) {
      // Second reading
      dev->vel = vel;
    }
    else {
      dev->vel = (3 * dev->vel + vel) / 4;
    }
    
    if (dev->qIndex == dev->qVIndex || tick - prevTick > LIDAR_MIN_VEL_DT_US) {
      // Increment the dx index
      dev->qVIndex = (dev->qVIndex + 1) & LIDAR_QLEN_MASK;
    }
    
  }

}

int lidarUpdate(lidar_dev_t *dev) {

  int status;
  int readVal;

  uint32_t tick; // ppTick;
  uint16_t dist; // ppDist;

  status = lidarAcquireStart(dev, 0);
  if (status < 0) {
    ERRP("lidarAcquireStart() failed 0x%x.\n", status);
    return status;
  }

  do {
    status = i2cReadByteData(dev->i2cHandle, LIDAR_REG_STATUS);
    if (status < 0) {
      ERRP("i2cReadByteData() failed 0x%x.\n", status);
      return status;
    }
  } while (status & 1);

  tick = gpioTick();

  unsigned reg = (dev->is_hp ? 0 : LIDARSP_I2C_REG_BLOCK_MASK) | LIDAR_REG_FULL_DELAY_HIGH;
  readVal = i2cReadWordData(dev->i2cHandle, reg);
    
  if (readVal < 0) {
    ERRP("i2cReadWordData() failed 0x%x.\n", status);
    return readVal;
  }
  
  dist = __bswap_16(readVal);
  //printf("%d\n", dist);

  // Try to filter out bad readings.
  // Throw out anything too close or too far (20cm - 25m)
  if (dist < LIDAR_MIN_DIST_CM || dist > LIDAR_MAX_DIST_CM) return 1;

  lidarDistEnq(dev, dist, tick);

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
  
  if (id == LIDAR_ID_HP) {
    dev->is_hp = 1;
  }
  else dev->is_hp = 0;

  int i = 0;
  while (i < LIDAR_QLEN) dev->qDist[i++] = 0;
  dev->qIndex = 0;
  dev->qVIndex = 0;

  return dev;

}

int lidarTest(int reps, int delayUs, uint16_t id) {

  int status;
  lidar_dev_t *dev;

  int readVal;

  dev = lidarInit(id);
  if (dev == NULL) {
    printf("lidarInit() error\n");
    return -1;
  }

  readVal = lidarIdGet(dev);
  if (readVal >= 0) printf("LIDAR ID: %d\n", readVal);
  

  status = lidarAddressSet(dev, 0x42);
  if (status < 0) {
    printf("lidarAddrSet() error\n");
    return -1;
  }

  int i;
  for (i = 0; i < reps; i++) {
    status = lidarAcquireStart(dev, 0);
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

int lidarTestAddrSet(int devID) {

  int status;
  lidar_dev_t *dev;

  int readVal;

  dev = lidarInit(devID);
  if (dev == NULL) {
    printf("lidarInit() error\n");
    return -1;
  }

  readVal = lidarIdGet(dev);
  if (readVal != devID) {
    printf("Incorrect LIDAR ID: Got %d\n", readVal);
    return -1;
  }

  status = lidarAddressSet(dev, 0x42);
  if (status < 0) {
    printf("lidarAddrSet() error\n");
    return -1;
  }
  
  printf("Address changed.\n");
  
  readVal = lidarIdGet(dev);
  if (readVal != devID) {
    printf("Incorrect LIDAR ID: Got %d\n", readVal);
    return -1;
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

  printf("Resetting LIDAR...");
  gpioSetMode(PIN_RESET, PI_OUTPUT);
  gpioWrite(PIN_RESET, PI_OFF);
  gpioSleep(0, 0, 200000);
  gpioWrite(PIN_RESET, PI_ON);

  uint16_t id;

#ifdef TEST_HP
  id = LIDAR_ID_HP;
#else
  id = LIDAR_ID_SP;
#endif

#ifdef TEST_ADDR
  status = lidarTestAddrSet(id);
  if (status == 0) printf("Succes   s!\n");
#else
  status = lidarTest(50, 500000, id);
#endif

  gpioTerminate();

  return status;

}
#endif
