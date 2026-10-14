#include <lpc21xx.h>

#include "LCD.h"
#include "lcd_defines.h"
#include "rtclock.h"
#include "kpm.h"
#include "schedule.h"
#include "delay.h"


//EINT0 ISR
void EINT0_ISR(void) __irq;


//menu request flag
volatile u32 menu_request = 0;


//EINT0 initialization
void EINT0_Init(void)
{
    //P0.1 as EINT0
    PINSEL0 &= ~(3 << 2);
    PINSEL0 |= (3 << 2);

    //edge triggered
    EXTMODE |= (1 << 0);

    //falling edge
    EXTPOLAR &= ~(1 << 0);

    //IRQ
    VICIntSelect &= ~(1 << 14);

    //clear interrupt
    EXTINT = (1 << 0);

    //VIC slot 0
    VICVectAddr0 = (u32)EINT0_ISR;
    VICVectCntl0 = (1 << 5) | 14;

    //enable EINT0
    VICIntEnable |= (1 << 14);
}


//EINT0 ISR
void EINT0_ISR(void) __irq
{
    //set menu request
    menu_request = 1;

    //clear interrupt
    EXTINT = (1 << 0);

    //end of interrupt
    VICVectAddr = 0;
}


//global variables
s32 hour;
s32 min;
s32 sec;

s32 date;
s32 month;
s32 year;

s32 day;

u32 on_hour;
u32 on_min;

u32 off_hour;
u32 off_min;


//week table
u8 week[][4] =
{
    "SUN",
    "MON",
    "TUE",
    "WED",
    "THU",
    "FRI",
    "SAT"
};


//leap year
u32 IsLeapYear(u32 year)
{
    if((year % 400) == 0)
        return 1;

    if((year % 100) == 0)
        return 0;

    if((year % 4) == 0)
        return 1;

    return 0;
}


//get days in month
u32 GetDaysInMonth(u32 month, u32 year)
{
    switch(month)
    {
        case 1:
        case 3:
        case 5:
        case 7:
        case 8:
        case 10:
        case 12:
            return 31;

        case 4:
        case 6:
        case 9:
        case 11:
            return 30;

        case 2:

            if(IsLeapYear(year))
                return 29;

            return 28;

        default:
            return 0;
    }
}


//display RTC
void DisplayRTC(void)
{
    static u32 count = 0;
    static u32 mode = 0;
    static u32 first_time = 1;

    //initial schedule
    if(first_time == 1)
    {
        on_hour = 9;
        on_min = 1;

        off_hour = 9;
        off_min = 2;

        first_time = 0;
    }


    //get time
    GetRTCTimeInfo(&hour,
                   &min,
                   &sec);


    //display time
    WRITE_LCD_CMD(GOTO_LINE1_POS0);

    WRITE_LCD_DATA((hour / 10) + '0');
    WRITE_LCD_DATA((hour % 10) + '0');

    WRITE_LCD_DATA(':');

    WRITE_LCD_DATA((min / 10) + '0');
    WRITE_LCD_DATA((min % 10) + '0');

    WRITE_LCD_DATA(':');

    WRITE_LCD_DATA((sec / 10) + '0');
    WRITE_LCD_DATA((sec % 10) + '0');


    //get day
    GetRTCDay(&day);


    //display day
    WRITE_LCD_CMD(GOTO_LINE1_POS0 + 10);

    if(day <= 6)
    {
        StrLCD(week[day]);
    }


    //get date
    GetRTCDateInfo(&date,
                   &month,
                   &year);


    //display second line
    if(count == 0)
    {
        WRITE_LCD_CMD(GOTO_LINE2_POS0);

        //clear second line
        StrLCD("                ");

        WRITE_LCD_CMD(GOTO_LINE2_POS0);


        if(mode == 0)
        {
            //display date
            WRITE_LCD_DATA((date / 10) + '0');
            WRITE_LCD_DATA((date % 10) + '0');

            WRITE_LCD_DATA('/');

            WRITE_LCD_DATA((month / 10) + '0');
            WRITE_LCD_DATA((month % 10) + '0');

            WRITE_LCD_DATA('/');

            U32LCD(year);
        }
        else
        {
            //display on off time
            StrLCD("ON:");

            WRITE_LCD_DATA((on_hour / 10) + '0');
            WRITE_LCD_DATA((on_hour % 10) + '0');

					WRITE_LCD_DATA(':');

            WRITE_LCD_DATA((on_min / 10) + '0');
            WRITE_LCD_DATA((on_min % 10) + '0');

            StrLCD("OF:");

            WRITE_LCD_DATA((off_hour / 10) + '0');
            WRITE_LCD_DATA((off_hour % 10) + '0');

            WRITE_LCD_DATA(':');

            WRITE_LCD_DATA((off_min / 10) + '0');
            WRITE_LCD_DATA((off_min % 10) + '0');
        }
    }


    //delay is 200ms
    count++;

    //2 seconds
    if(count >= 10)
    {
        count = 0;

        if(mode == 0)
        {
            mode = 1;
        }
        else
        {
            mode = 0;
        }
    }
}

//main menu
u32 RTC_Menu(void)
{
    u32 key;

    WRITE_LCD_CMD(0x01);

    WRITE_LCD_CMD(GOTO_LINE1_POS0);
    StrLCD("1.RTC  2.SCH");

    WRITE_LCD_CMD(GOTO_LINE2_POS0);
    StrLCD("3.EXIT");


    //wait for key
    key = KeyScan();

    return key;
}


//RTC menu
u32 RTC_SubMenu(void)
{
    u32 key;

    WRITE_LCD_CMD(0x01);

    WRITE_LCD_CMD(GOTO_LINE1_POS0);
    StrLCD("1.TIME 2.DATE");

    WRITE_LCD_CMD(GOTO_LINE2_POS0);
    StrLCD("3.DAY  4.BACK");


    //wait for key
    key = KeyScan();

    return key;
}


//set time
void SetTimeMenu(void)
{
    //hour
    do
    {
        WRITE_LCD_CMD(0x01);

        StrLCD("ENTER HOUR:");

        hour = ReadNum();

        if(hour > 23)
        {
            WRITE_LCD_CMD(0x01);

            StrLCD("INVALID HOUR");

            delay_ms(1000);
        }

    }while(hour > 23);


    //minute
    do
    {
        WRITE_LCD_CMD(0x01);

        StrLCD("ENTER MIN:");

        min = ReadNum();

        if(min > 59)
        {
            WRITE_LCD_CMD(0x01);

            StrLCD("INVALID MIN");

            delay_ms(1000);
        }

    }while(min > 59);


    //second
    do
    {
        WRITE_LCD_CMD(0x01);

        StrLCD("ENTER SEC:");

        sec = ReadNum();

        if(sec > 59)
        {
            WRITE_LCD_CMD(0x01);

            StrLCD("INVALID SEC");

            delay_ms(1000);
        }

    }while(sec > 59);


    //set RTC time
    SetRTCTimeInfo(hour,
                   min,
                   sec);
}


//set date
void SetDateMenu(void)
{
    //month
    do
    {
        WRITE_LCD_CMD(0x01);

        StrLCD("ENTER MONTH:");

        month = ReadNum();

        if((month < 1) ||
           (month > 12))
        {
            WRITE_LCD_CMD(0x01);

            StrLCD("INVALID MONTH");

            delay_ms(1000);
        }

    }while((month < 1) ||
           (month > 12));


    //year
    do
    {
        WRITE_LCD_CMD(0x01);

        StrLCD("ENTER YEAR:");

        year = ReadNum();

        if((year < 2000) ||
           (year > 2099))
        {
            WRITE_LCD_CMD(0x01);

            StrLCD("INVALID YEAR");

            delay_ms(1000);
        }

    }while((year < 2000) ||
           (year > 2099));


    //date
    do
    {
        WRITE_LCD_CMD(0x01);

        StrLCD("ENTER DATE:");

        date = ReadNum();

        if((date < 1) ||
           (date > GetDaysInMonth(month, year)))
        {
            WRITE_LCD_CMD(0x01);

            StrLCD("INVALID DATE");

            delay_ms(1000);
        }

    }while((date < 1) ||
           (date > GetDaysInMonth(month, year)));


    //set RTC date
    SetRTCDateInfo(date,
                   month,
                   year);
}


//set day
void SetDayMenu(void)
{
    do
    {
        WRITE_LCD_CMD(0x01);

        StrLCD("0S 1M 2T 3W");

        WRITE_LCD_CMD(0xC0);

        StrLCD("4T 5F 6S");


        //get day
        day = KeyScan() - '0';

    }while(day > 6);


    //set RTC day
    SetRTCDay(day);
}


//set schedule
void SetScheduleMenu(void)
{
    //ON hour
    do
    {
        WRITE_LCD_CMD(0x01);

        StrLCD("ON HOUR:");

        on_hour = ReadNum();

        if(on_hour > 23)
        {
            WRITE_LCD_CMD(0x01);

            StrLCD("INVALID HOUR");

            delay_ms(1000);
        }

    }while(on_hour > 23);


    //ON minute
    do
    {
        WRITE_LCD_CMD(0x01);

        StrLCD("ON MIN:");

        on_min = ReadNum();

        if(on_min > 59)
        {
            WRITE_LCD_CMD(0x01);

            StrLCD("INVALID MIN");

            delay_ms(1000);
        }

    }while(on_min > 59);


    //OFF hour
    do
    {
        WRITE_LCD_CMD(0x01);

        StrLCD("OFF HOUR:");

        off_hour = ReadNum();

        if(off_hour > 23)
        {
            WRITE_LCD_CMD(0x01);

            StrLCD("INVALID HOUR");

            delay_ms(1000);
        }

    }while(off_hour > 23);


    //OFF minute
    do
    {
        WRITE_LCD_CMD(0x01);

        StrLCD("OFF MIN:");

        off_min = ReadNum();

        if(off_min > 59)
        {
            WRITE_LCD_CMD(0x01);

            StrLCD("INVALID MIN");

            delay_ms(1000);
        }

    }while(off_min > 59);


    //check same time
    if((on_hour == off_hour) &&
       (on_min == off_min))
    {
        WRITE_LCD_CMD(0x01);

        StrLCD("SAME TIME");

        delay_ms(1000);

        return;
    }


    //save schedule
    Schedule_SetTime(on_hour,
                     on_min,
                     off_hour,
                     off_min);
}


//main
int main()
{
    u32 choice;
    u32 rtc_choice;


    //initialize peripherals
    Init_LCD();

    RTC_Init();

    INIT_KPM();

    LED_Init();

    EINT0_Init();


    //initial time
    SetRTCTimeInfo(9,
                   00,
                   0);


    //initial date
    SetRTCDateInfo(22,
                   9,
                   2026);


    //initial day
    SetRTCDay(1);


    //initial schedule
    Schedule_SetTime(9,
                     1,
                     9,
                     2);


    while(1)
    {
        //display RTC
        DisplayRTC();


        //delay for LCD refresh
        delay_ms(200);


        //get current time
        GetRTCTimeInfo(&hour,
                       &min,
                       &sec);


        //control LED
        if(Schedule_IsActive(hour,
                             min,
                             sec))
        {
            LED_Control(1);
        }
        else
        {
            LED_Control(0);
        }


        //check menu request
        if(menu_request == 1)
        {
            menu_request = 0;


            //display main menu
            choice = RTC_Menu();


            //RTC
            if(choice == '1')
            {
                rtc_choice = RTC_SubMenu();


                //time
                if(rtc_choice == '1')
                {
                    SetTimeMenu();
									WRITE_LCD_CMD(0x01);
									delay_ms(2);
                }


                //date
                else if(rtc_choice == '2')
                {
                    SetDateMenu();
									WRITE_LCD_CMD(0x01);
									delay_ms(2);
                }


                //day
                else if(rtc_choice == '3')
                {
                    SetDayMenu();
									  WRITE_LCD_CMD(0x01);
									    delay_ms(2);
                }


                //back
                else if(rtc_choice == '4')
                {
                    WRITE_LCD_CMD(0x01);
									delay_ms(2);

                    continue;
                }
            }


            //schedule
            else if(choice == '2')
            {
                SetScheduleMenu();
							//clear screen 
	             WRITE_LCD_CMD(0x01);
	               delay_ms(2);
            }


            //exit
            else if(choice == '3')
            {
                WRITE_LCD_CMD(0x01);

                continue;
            }
        }
    }
}
