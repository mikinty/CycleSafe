
#ifndef _CONSTANTS_H_
#define _CONSTANTS_H_

// Parameters

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

#define ERRP(...) fprintf(stderr, "%s:%d in %s", __FILE__, __LINE__, __FUNCTION__);\
  fprintf(stderr, __VA_ARGS__);

#endif //_CONSTANTS_H_
