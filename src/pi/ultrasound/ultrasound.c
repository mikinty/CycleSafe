
#include <stdio.h>
#include <unistd.h>

#include <pigpio.h>

/** @brief trigger pin numbers */
#define SENSOR_COUNT 6
/** @brief trigger pin numbers */
#define SENSOR_POLLING_PERIOD_MS 100

// PIN NUMBERS
/** @brief trigger pin numbers */
#define GPIO_TRIG_PIN {16, 18, 20, 22, 24, 26}
/** @brief echo pin numbers */
#define GPIO_ECHO_PIN {17, 19, 21, 23, 25, 27}

/** @brief echo callback functions */
#define GPIO_ECHO_FUNC {sonarEcho0, sonarEcho1, sonarEcho2, sonarEcho3, sonarEcho4, sonarEcho5}



/* forward prototypes */

void sonarTrigger(void);

void sonarEcho0(int gpio, int level, uint32_t tick);
void sonarEcho1(int gpio, int level, uint32_t tick);
void sonarEcho2(int gpio, int level, uint32_t tick);
void sonarEcho3(int gpio, int level, uint32_t tick);
void sonarEcho4(int gpio, int level, uint32_t tick);
void sonarEcho5(int gpio, int level, uint32_t tick);

/* globals */

/** @brief  the last measured tick difference */
uint32_t tickData[SENSOR_COUNT];


uint32_t tickToUm(uint32_t ticks) {
  return ticks * 343;
}

uint32_t cmToTick(uint32_t cm) {
  return cm * 29;
}

/** @brief  read the last sonar reading */
uint32_t sonarReadUm(uint32_t sensor) {
  return tickToUm(tickData[SENSOR_COUNT]);
}

/** @brief  trigger a sonar reading */
uint32_t sonarTrigger(uint32_t sensor) {
  gpioTrigger(GPIO_TRIG_PIN[sensor], 10, 1);
}

void sonarEcho0(int gpio, int level, uint32_t tick) {
  const uint32_t i = 0;
  static uint32_t startTick;

  if (level == PI_ON) startTick = tick;
  else if (level == PI_OFF) tickData[i] = tick - startTick;
}

void sonarEcho1(int gpio, int level, uint32_t tick) {
  const uint32_t i = 1;
  static uint32_t startTick;

  if (level == PI_ON) startTick = tick;
  else if (level == PI_OFF) tickData[i] = tick - startTick;
}

void sonarEcho2(int gpio, int level, uint32_t tick) {
  const uint32_t i = 2;
  static uint32_t startTick;

  if (level == PI_ON) startTick = tick;
  else if (level == PI_OFF) tickData[i] = tick - startTick;
}

void sonarEcho3(int gpio, int level, uint32_t tick) {
  const uint32_t i = 3;
  static uint32_t startTick;

  if (level == PI_ON) startTick = tick;
  else if (level == PI_OFF) tickData[i] = tick - startTick;
}

void sonarEcho4(int gpio, int level, uint32_t tick) {
  const uint32_t i = 4;
  static uint32_t startTick;

  if (level == PI_ON) startTick = tick;
  else if (level == PI_OFF) tickData[i] = tick - startTick;
}

void sonarEcho5(int gpio, int level, uint32_t tick) {
  const uint32_t i = 5;
  static uint32_t startTick;

  if (level == PI_ON) startTick = tick;
  else if (level == PI_OFF) tickData[i] = tick - startTick;
}

int sonarStart() {

  int status = 0;

  status = gpioInitialise();
  if (status < 0) return status;

  int i;
  for (i = 0; i < SENSOR_COUNT; i++) {
    gpioSetMode(GPIO_TRIG_PIN[i], PI_OUTPUT);
    gpioWrite(GPIO_TRIG_PIN[i], PI_OFF);
    // gpioSetTimerFunc(0, SENSOR_POLLING_PERIOD_MS, sonarTrigger);
    gpioSetMode(GPIO_ECHO_PIN[i], PI_INPUT);
    gpioSetAlertFunc(GPIO_ECHO_PIN[i], GPIO_ECHO_FUNC[i]);
  }

  return status;

}

void sonarStop() {
  gpioTerminate();
}
