#ifndef SCHEDULE_H
#define SCHEDULE_H

typedef unsigned char  u8;
typedef unsigned int   u32;
typedef signed int     s32;


/* Fixed ON/OFF time */
void Schedule_SetTime(u8 on_hour,
                      u8 on_min,
                      u8 off_hour,
                      u8 off_min);


/* Check whether current time is inside schedule */
u8 Schedule_IsActive(u8 hour, u8 min, u8 sec);


/* Initialize LED */
void LED_Init(void);


/* Control LED according to schedule */
void LED_Control(u8 state);

#endif

