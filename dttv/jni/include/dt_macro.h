#ifndef DT_MACRO_H
#define DT_MACRO_H

 /*COMMON*/
#define TRUE 1
#define FALSE 0
#define  MIN(x,y)       ((x)<(y)?(x):(y))
#define  MAX(x,y)       ((x)>(y)?(x):(y))

//#define DT_NOPTS_VALUE          ((int64_t)UINT64_C(0x8000000000000000)) // FROM FFMPEG
#define DT_NOPTS_VALUE          (-1)

/*PLAYER*/
#define FILE_NAME_MAX_LENGTH 1024
#define DT_PTS_FREQ    90000
#define DT_PTS_FREQ_MS 90
#define DT_SYNC_THRESHOLD  PTS_FREQ_MS*150
#define INT64_0     INT64_C(0x8000000000000000)
/*CODEC*/

/*HOST*/
#define AVSYNC_THRESHOLD 150    //ms
#define AVSYNC_THRESHOLD_MAX  3*1000 //ms
#define AVSYNC_DROP_THRESHOLD  30*1000 //ms

/*PORT*/ 
/*AUDIO*/ 
/*VIDEO*/
#endif
