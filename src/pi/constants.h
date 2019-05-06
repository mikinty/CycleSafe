
#ifndef _CONSTANTS_H_
#define _CONSTANTS_H_

// Pins

// 2-3 taken by I2C
#define PIN_LIDAR_INTR_FRONT 7
#define PIN_LIDAR_INTR_FAR 5
#define PIN_LIDAR_INTR_NEAR 6
#define PIN_RESET 8
#define PIN_TURNSIG_L 12
#define PIN_TURNSIG_R 13
// 16-27 taken by ultrasound sensors

// Parameters

#define THRESH_FRONT_TTI_MSEC 3000
#define THRESH_BACK_TTI_NOTIF_MSEC 2500
#define THRESH_BACK_TTI_ALERT_MSEC 1500
#define THRESH_BACK_FAR_MAX_DIST_CM 2500
#define THRESH_BACK_NEAR_MAX_DIST_CM 1300
#ifdef TRAINER
  #define THRESH_PROX_UM 500000
#else
  #define THRESH_PROX_UM 2000000
#endif
#define THRESH_BRAKE_SPEED_MM_PER_SEC 500
#define THRESH_BRAKE_DECCELERATION_STEPS 3
#define ULTSND_MIN_DIST_UM 50000
#define LIDAR_MIN_DIST_CM 30
#define LIDAR_MAX_DIST_CM 3000
#define LIDAR_MIN_VEL_DT_US 20000

#define BS_FIRST_STEP_FAR_SEC 0.08
#define BS_FIRST_STEP_NEAR_SEC 0.06

#define BR_PERSIST_SEC 0.5

#define BLINKER_SLOW_ON_SEC 0.7
#define BLINKER_SLOW_T_SEC 1.0

#define BLINKER_FAST_ON_SEC 0.2
#define BLINKER_FAST_T_SEC 0.4

#define JACKET_UPDATE_PERIOD_MIN_SEC 1.0
#define SYSTEM_UPDATE_PERIOD_MIN_USEC 8000

// Fixed values

#define MSEC_PER_SEC 1000
#define USEC_PER_SEC 1000000
#define NSEC_PER_SEC 1000000000
#define USEC_PER_MSEC 1000
#define NSEC_PER_MSEC 1000000
#define NSEC_PER_USEC 1000

#define CM_PER_M 100
#define MM_PER_M 1000
#define UM_PER_M 1000000
#define MM_PER_CM 10
#define UM_PER_CM 10000

#define V_SOUND_M_PER_S 343

#define LIDAR_ID_HP 13113
#define LIDAR_ID_SPMARK 25736
#define LIDAR_ID_SP 10384

#define LIDAR_I2C_ADDR_HP 0x6A
#define LIDAR_I2C_ADDR_SP 0x6C
#define LIDAR_I2C_ADDR_SPMARK 0x6E

/** @brief  Top-middle USB port */
#define SPEED_USB_PORT "/dev/serial/by-path/platform-3f980000.usb-usb-0:1.2:1.0"


// Printers
//#define UI
#ifdef UI
#define UIP(...) printf(__VA_ARGS__); fflush(stdout);
#else
#define UIP(...)
#endif

#define ERRP(...) fprintf(stderr, "%s:%d in %s: ", __FILE__, __LINE__, __FUNCTION__);\
  fprintf(stderr, __VA_ARGS__);
#define INFOP(...) fprintf(stdout, __VA_ARGS__);
#define DBGP(...) fprintf(stderr, "%s:%d: ", __FILE__, __LINE__);\
  fprintf(stderr, __VA_ARGS__);

#endif //_CONSTANTS_H_
