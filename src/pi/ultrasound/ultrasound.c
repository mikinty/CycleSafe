
#include "ultrasound.h"

// PIN NUMBERS
/** @brief  trigger pin numbers */
static const int GPIO_TRIG_PIN[ULTSND_SENSOR_COUNT] = {16, 18, 20, 22, 24, 26};
/** @brief  echo pin numbers */
static const int GPIO_ECHO_PIN[ULTSND_SENSOR_COUNT] = {17, 19, 21, 23, 25, 27};

/* forward prototypes */

void sonarTriggerCallback(void *data);
void sonarEcho(int gpio, int level, uint32_t tick, void *data);

/* globals */

/** @brief  the last measured tick difference */
static volatile uint32_t tickData[ULTSND_SENSOR_COUNT];
/** @brief  the starting tick value */
static volatile uint32_t tickStart[ULTSND_SENSOR_COUNT];

uint32_t tickToUm(uint32_t ticks) {
  return ticks * (V_SOUND_M_PER_S / 2);
}

uint32_t cmToTick(uint32_t cm) {
  return cm * ((2 * UM_PER_CM) / V_SOUND_M_PER_S);
}

/** @brief  read the last sonar reading */
uint32_t sonarReadUm(uint32_t sensor) {
  return tickToUm(tickData[sensor]);
}

/** @brief  trigger a sonar reading */
int sonarTrigger(uint32_t sensor) {
  return gpioTrigger(GPIO_TRIG_PIN[sensor], 10, 1);
}

void sonarTriggerCallback(void *data) {
  sonarTrigger((uint32_t) data);
}

void sonarEcho(int gpio, int level, uint32_t tick, void *data) {
  uint32_t i = (uint32_t) data;

  if (level == PI_ON) tickStart[i] = tick;
  else if (level == PI_OFF) {
    tickData[i] = tick - tickStart[i];
  }
}

int sonarPollStart() {
  int status = 0;
  int i;
  for (i = 0; i < ULTSND_SENSOR_COUNT; i++) {
    status |= gpioSetTimerFuncEx(i, ULTSND_SAMPLE_PERIOD_MS, sonarTriggerCallback, (void *)i);
    gpioSleep(PI_TIME_RELATIVE, 0, ULTSND_SAMPLE_PERIOD_MS * USEC_PER_MSEC / ULTSND_SENSOR_COUNT);
  }

  if (status != 0) {
    sonarPollStop();
  }

  return status;
}

int sonarPollStop() {
  int status = 0;
  int i;
  for (i = 0; i < ULTSND_SENSOR_COUNT; i++) {
    status |= gpioSetTimerFuncEx(i, ULTSND_SAMPLE_PERIOD_MS, NULL, NULL);
  }
  return status;
}

void sonarStart() {

  int i;
  for (i = 0; i < ULTSND_SENSOR_COUNT; i++) {
    gpioSetMode(GPIO_TRIG_PIN[i], PI_OUTPUT);
    gpioWrite(GPIO_TRIG_PIN[i], PI_OFF);
    gpioSetMode(GPIO_ECHO_PIN[i], PI_INPUT);
    gpioSetAlertFuncEx(GPIO_ECHO_PIN[i], sonarEcho, (void *)i);
  }

}

void sonarTest() {

  printf("Enter 0-5 to sample, c to poll, x to exit\n");
  sonarStart();

  while (1) {
    int status;
    char c = getchar();
    switch (c) {
    case '0':
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
      sonarTrigger(c - '0');
      gpioSleep(PI_TIME_RELATIVE, 0, 500000);
      printf("Sensor %c: %d cm\n", c, sonarReadUm(c - '0') / 10000);
      break;
    case 'x':
      return;
    case 'c':
      status = sonarPollStart();
      if (status != 0) {
        printf("sonarPollStart() error\n");
      }
      else {
        int i;
        for (i = 0; i < 30; i++) {
          gpioSleep(PI_TIME_RELATIVE, 0, 500000);
          printf("== Reading %d ==\n", i);
          int j;
          for (j = 0; j < ULTSND_SENSOR_COUNT; j++) {
            printf("Sensor %d: %d cm\n", j, sonarReadUm(j) / 10000);
          }
        }
        status = sonarPollStop();
        if (status != 0) printf("sonarPollStop() error\n");
      }
      break;
    }
  }

}

int main() {
  int status = gpioInitialise();
  if (status < 0) {
    printf("gpioInitialise() error %d\n", status);
    return status;
  }
  sonarTest();
  gpioTerminate();
  return 0;
}
