
#ifndef _CONSTANTS_H_
#define _CONSTANTS_H_

// Debug

// Parameters

#define THRESH_FRONT_TTI_MSEC 3000
#define THRESH_PROX_UM 300000
#define THRESH_DECCELERATION_BRAKE_MM_PER_SEC_2 10
#define ULTSND_MIN_DIST_UM 50000
#define LIDAR_MIN_DIST_CM 20
#define LIDAR_MAX_DIST_CM 2500

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

#define LIDAR_HP_ID 13113

/** @brief  Top-middle USB port */
#define SPEED_USB_PORT "/dev/serial/by-path/platform-3f980000.usb-usb-0:1.2:1.0"

#define ERRP(...) fprintf(stderr, "%s:%d in %s: ", __FILE__, __LINE__, __FUNCTION__);\
  fprintf(stderr, __VA_ARGS__);
#define INFOP(...) fprintf(stdout, "%s", __VA_ARGS__);
  

#endif //_CONSTANTS_H_
