#include <lpc214x.h>
#include "schedule.h"

#define LED (1 << 2)

u32 ON_TIME;
u32 OFF_TIME;


/* Initialize LED */
void LED_Init(void)
{
    IODIR0 |= LED;

    /* LED ON initially */
    IOCLR0 = LED;
}


/* Convert HH:MM:SS into seconds */
u32 TimeToSeconds(u8 hour, u8 min, u8 sec)
{
    return ((u32)hour * 3600) +
           ((u32)min  * 60) +
           sec;
}


/* Set ON and OFF time */
void Schedule_SetTime(u8 on_hour,
                      u8 on_min,
                      u8 off_hour,
                      u8 off_min)
{
    ON_TIME  = TimeToSeconds(on_hour, on_min, 0);
    OFF_TIME = TimeToSeconds(off_hour, off_min, 0);
}


/* Check schedule */
u8 Schedule_IsActive(u8 hour, u8 min, u8 sec)
{
    u32 current_time;

    current_time = TimeToSeconds(hour, min, sec);

    /* Normal schedule */
    if(ON_TIME < OFF_TIME)
    {
        if((current_time >= ON_TIME) &&
           (current_time < OFF_TIME))
        {
            return 1;
        }
        else
        {
            return 0;
        }
    }

    /* Midnight-crossing schedule */
    else if(ON_TIME > OFF_TIME)
    {
        if((current_time >= ON_TIME) ||
           (current_time < OFF_TIME))
        {
            return 1;
        }
        else
        {
            return 0;
        }
    }

    /* ON time = OFF time ? invalid */
    else
    {
        return 0;
    }
}


/* Control LED */
void LED_Control(u8 state)
{
    if(state == 1)
    {
        /* LED ON - active HIGH connection */
        IOSET0 = LED;
    }
    else
    {
        /* LED OFF */
        IOCLR0 = LED;
    }
}

