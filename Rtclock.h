#ifndef RTCLOCK_H
#define RTCLOCK_H

typedef unsigned char  u8;
typedef unsigned int   u32;
typedef signed int     s32;


/* RTC Initialization */
void RTC_Init(void);


/* Set RTC Time */
void SetRTCTimeInfo(u32 hour, u32 minute, u32 second);


/* Get RTC Time */
void GetRTCTimeInfo(s32 *hour, s32 *minute, s32 *second);


/* Set RTC Date */
void SetRTCDateInfo(u32 date, u32 month, u32 year);


/* Get RTC Date */
void GetRTCDateInfo(s32 *date, s32 *month, s32 *year);


/* Set Day of Week */
void SetRTCDay(u32 dow);


/* Get Day of Week */
void GetRTCDay(s32 *dow);

#endif

