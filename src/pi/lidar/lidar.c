
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

int lidarIdGet(int device) {
  
  int readVal;
  
  readVal = i2cReadWordData(device, LIDAR_REG_UNIT_ID_HIGH);
  if (readVal < 0) return readVal;
  
  return __bswap_16(readVal);
  
}

int lidarSetAddress(int device, uint8_t newAddr, uint16_t unitId) {

  if (newAddr & 0x80) return -1;

  int status = 0;
  status = i2cWriteWordData(device, LIDAR_REG_I2C_ID_HIGH, __bswap_16(unitId);
  if (status < 0) return status;
  
  status = i2cWriteByteData(device, LIDAR_REG_I2C_SEC_ADDR, newAddr << 1);
  if (status < 0) return status;
  
  i2cClose(device);
  device = i2cOpen(LIDAR_I2C_BUS, newAddr, 0);
  if (device < 0) {
    return device;
  }
  
  return status;
}

int lidarTest(int reps, int delayUs) {

  int gpioInitStatus = -1, status = -1;
  int device = -1;

  do {
    gpioInitStatus = gpioInitialise();
    if (gpioInitStatus < 0) {
      printf("gpioInitialise() error %d\n", gpioInitStatus);
      status = gpioInitStatus;
      break;
    }

    device = i2cOpen(LIDAR_I2C_BUS, LIDAR_I2C_ADDR_DEFAULT, 0);
    if (device < 0) {
      printf("i2cOpen() error %d\n", device);
      status = device;
      break;
    }

    readVal = lidarGetId(LIDAR_I2C_ADDR_DEFAULT);
    if (readVal >= 0) printf("LIDAR ID: %d", readVal);

    int i;
    for (i = 0; i < reps; i++) {
      status = i2cWriteByteData(device, LIDAR_REG_ACQ_COMMAND, 0x1);
      if (status < 0) {
        printf("i2cWriteByteData() error %d\n", status);
        break;
      }

      do {
        status = i2cReadByteData(device, LIDAR_REG_STATUS);
        if (status < 0) {
          printf("i2cReadByteData() error %d\n", status);
          break;
        }
      } while (status & 1);
      if (status < 0) break;

      int readVal, dist;
      readVal = i2cReadWordData(device, LIDAR_REG_FULL_DELAY_HIGH);
      dist = __bswap_16(readVal);
      if (readVal < 0) {
        printf("i2cReadByteData() error %d\n", status);
        break;
      }
      printf("%d cm\n", dist);
      
      gpioSleep(0, 0, delayUs);

    }

  } while (0);

  if (device >= 0) {
    i2cClose(device);
  }
  if (gpioInitStatus >= 0) {
    gpioTerminate();
  }

  return status;

}

int main() {
  lidarTest(50, 500000);
  return 0;
}
