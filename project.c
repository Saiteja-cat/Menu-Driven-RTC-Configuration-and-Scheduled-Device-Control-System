#include "KPM.h"
#include "types.h"
#include "LCD.h"
#include <lpc21xx.h>
#include "lcd_defines.h"
#include "delay.h"

#define PCLK 15000000
#define PREINT_VAL ((PCLK/32768)-1)
#define PREFRAC_VAL (PCLK-((PREINT_VAL+1)*32768))

#define RTC_ENABLE  (1<<0)
#define RTC_RESET   (1<<1)
#define RTC_CLKSRC  (1<<4)

u8 week[][4]={"SUN","MON","TUE","WED","THU","FRI","SAT"};

#define SUN 0
#define MON 1
#define TUE 2
#define WED 3
#define THU 4
#define FRI 5
#define SAT 6

#define DEVICE_PIN 7

#define EINT0_PIN 1
#define EINT0_CHNO 14

s32 hour,min,sec;
s32 date,month,year;

u32 h,m,s;
u32 d,mm,y;
u32 day;

u32 on_h,on_m,on_s;
u32 off_h,off_m,off_s;

volatile u32 flag=0;
u32 schedule_valid=0;

void RTC_Init(void)
{
    CCR=RTC_RESET;

#ifndef CPU_LPC2148
    PREINT=PREINT_VAL;
    PREFRAC=PREFRAC_VAL;
    CCR=RTC_ENABLE;
#else
    CCR=RTC_ENABLE|RTC_CLKSRC;
#endif
}

void SetRTCTimeInfo(u32 hour,u32 minute,u32 second)
{
    HOUR=hour;
    MIN=minute;
    SEC=second;
}

void GetRTCTimeInfo(s32 *hour,s32 *minute,s32 *second)
{
    *hour=HOUR;
    *minute=MIN;
    *second=SEC;
}

void DisplayRTCTime(u32 hour,u32 minute,u32 second)
{
    WRITE_LCD_CMD(0x80);

    WRITE_LCD_DATA((hour/10)+'0');
    WRITE_LCD_DATA((hour%10)+'0');
    WRITE_LCD_DATA(':');

    WRITE_LCD_DATA((minute/10)+'0');
    WRITE_LCD_DATA((minute%10)+'0');
    WRITE_LCD_DATA(':');

    WRITE_LCD_DATA((second/10)+'0');
    WRITE_LCD_DATA((second%10)+'0');

    WRITE_LCD_DATA(' ');

    StrLCD(week[DOW]);

    WRITE_LCD_DATA(' ');
    WRITE_LCD_DATA(' ');
    WRITE_LCD_DATA(' ');
    WRITE_LCD_DATA(' ');
}

void SetRTCDateInfo(u32 date,u32 month,u32 year)
{
    DOM=date;
    MONTH=month;
    YEAR=year;
}

void GetRTCDateInfo(s32 *date,s32 *month,s32 *year)
{
    *date=DOM;
    *month=MONTH;
    *year=YEAR;
}

void DisplayRTCDate(u32 date,u32 month,u32 year)
{
    WRITE_LCD_CMD(0xC0);

    WRITE_LCD_DATA((date/10)+'0');
    WRITE_LCD_DATA((date%10)+'0');
    WRITE_LCD_DATA('/');

    WRITE_LCD_DATA((month/10)+'0');
    WRITE_LCD_DATA((month%10)+'0');
    WRITE_LCD_DATA('/');

    U32LCD(year);

    WRITE_LCD_DATA(' ');
    WRITE_LCD_DATA(' ');
    WRITE_LCD_DATA(' ');
    WRITE_LCD_DATA(' ');
    WRITE_LCD_DATA(' ');
    WRITE_LCD_DATA(' ');
}

u32 IsLeapYear(u32 year)
{
    if((year%400)==0)
        return 1;

    if((year%100)==0)
        return 0;

    if((year%4)==0)
        return 1;

    return 0;
}

u32 DaysInMonth(u32 month,u32 year)
{
    switch(month)
    {
        case 1:return 31;

        case 2:
            if(IsLeapYear(year))
                return 29;
            else
                return 28;

        case 3:return 31;
        case 4:return 30;
        case 5:return 31;
        case 6:return 30;
        case 7:return 31;
        case 8:return 31;
        case 9:return 30;
        case 10:return 31;
        case 11:return 30;
        case 12:return 31;

        default:return 0;
    }
}

u32 IsValidTime(u32 hour,u32 minute,u32 second)
{
    if(hour>23)
        return 0;

    if(minute>59)
        return 0;

    if(second>59)
        return 0;

    return 1;
}

u32 IsValidDate(u32 date,u32 month,u32 year)
{
    u32 max_days;

    if(month<1||month>12)
        return 0;

    max_days=DaysInMonth(month,year);

    if(date<1||date>max_days)
        return 0;

    return 1;
}

u32 IsValidDay(u32 day)
{
    if(day>6)
        return 0;

    return 1;
}

u32 TimeToSeconds(u32 hour,u32 minute,u32 second)
{
    return (hour*3600)+(minute*60)+second;
}

void time(void)
{
    while(1)
    {
        WRITE_LCD_CMD(0x01);
        delay_ms(2);

        WRITE_LCD_CMD(0x80);
        StrLCD("TIME:");

        WRITE_LCD_CMD(0xC0);

        h=ReadNum();

        WRITE_LCD_DATA(':');

        m=ReadNum();

        WRITE_LCD_DATA(':');

        s=ReadNum();

        if(IsValidTime(h,m,s))
            break;

        WRITE_LCD_CMD(0x01);
        delay_ms(2);

        WRITE_LCD_CMD(0x80);
        StrLCD("INVALID TIME");

        delay_ms(1500);
    }
}

void setday(void)
{
    while(1)
    {
        WRITE_LCD_CMD(0x01);
        delay_ms(2);

        WRITE_LCD_CMD(0x80);
        StrLCD("DAY 0-6:");

        WRITE_LCD_CMD(0xC0);

        day=ReadNum();

        if(IsValidDay(day))
            break;

        WRITE_LCD_CMD(0x01);
        delay_ms(2);

        WRITE_LCD_CMD(0x80);
        StrLCD("INVALID DAY");

        delay_ms(1500);
    }
}

void setdate(void)
{
    while(1)
    {
        WRITE_LCD_CMD(0x01);
        delay_ms(2);

        WRITE_LCD_CMD(0x80);
        StrLCD("DATE:");

        WRITE_LCD_CMD(0xC0);

        d=ReadNum();

        WRITE_LCD_DATA('/');

        mm=ReadNum();

        WRITE_LCD_DATA('/');

        y=ReadNum();

        if(IsValidDate(d,mm,y))
            break;

        WRITE_LCD_CMD(0x01);
        delay_ms(2);

        WRITE_LCD_CMD(0x80);
        StrLCD("INVALID DATE");

        delay_ms(1500);
    }

    delay_ms(1000);
}

void onf(void)
{
    while(1)
    {
        WRITE_LCD_CMD(0x01);
        delay_ms(2);

        WRITE_LCD_CMD(0x80);
        StrLCD("ON TIME:");

        WRITE_LCD_CMD(0xC0);

        on_h=ReadNum();

        WRITE_LCD_DATA(':');

        on_m=ReadNum();

        WRITE_LCD_DATA(':');

        on_s=ReadNum();

        if(IsValidTime(on_h,on_m,on_s))
            break;

        WRITE_LCD_CMD(0x01);
        delay_ms(2);

        WRITE_LCD_CMD(0x80);
        StrLCD("INVALID ON TIME");

        delay_ms(1500);
    }

    while(1)
    {
        WRITE_LCD_CMD(0x01);
        delay_ms(2);

        WRITE_LCD_CMD(0x80);
        StrLCD("OFF TIME:");

        WRITE_LCD_CMD(0xC0);

        off_h=ReadNum();

        WRITE_LCD_DATA(':');

        off_m=ReadNum();

        WRITE_LCD_DATA(':');

        off_s=ReadNum();

        if(IsValidTime(off_h,off_m,off_s))
            break;

        WRITE_LCD_CMD(0x01);
        delay_ms(2);

        WRITE_LCD_CMD(0x80);
        StrLCD("INVALID OFF TIME");

        delay_ms(1500);
    }

    if(TimeToSeconds(on_h,on_m,on_s)==TimeToSeconds(off_h,off_m,off_s))
    {
        WRITE_LCD_CMD(0x01);
        delay_ms(2);

        WRITE_LCD_CMD(0x80);
        StrLCD("SAME TIME");

        WRITE_LCD_CMD(0xC0);
        StrLCD("NOT ALLOWED");

        delay_ms(2000);

        schedule_valid=0;

        IOCLR0=(1<<DEVICE_PIN);
    }
    else
    {
        schedule_valid=1;

        WRITE_LCD_CMD(0x01);
        delay_ms(2);

        WRITE_LCD_CMD(0x80);
        StrLCD("SCHEDULE SAVED");

        delay_ms(1500);
    }
}

void edit(void)
{
    u32 num;

    WRITE_LCD_CMD(0x01);
    delay_ms(2);

    WRITE_LCD_CMD(0x80);
    StrLCD("1 TIME 2 ON/OFF");

    WRITE_LCD_CMD(0xC0);
    StrLCD("3 EXIT");

    num=ReadNum();

    switch(num)
    {
        case 1:

            WRITE_LCD_CMD(0x01);
            delay_ms(2);

            WRITE_LCD_CMD(0x80);
            StrLCD("1 TIME 2 DATE");

            WRITE_LCD_CMD(0xC0);
            StrLCD("3 DAY");

            num=ReadNum();

            switch(num)
            {
                case 1:

                    time();

                    SetRTCTimeInfo(h,m,s);

                    break;

                case 2:

                    setdate();

                    SetRTCDateInfo(d,mm,y);

                    break;

                case 3:

                    setday();

                    DOW=day;

                    break;

                default:

                    WRITE_LCD_CMD(0x01);
                    delay_ms(2);

                    WRITE_LCD_CMD(0x80);
                    StrLCD("INVALID OPTION");

                    delay_ms(1500);

                    break;
            }

            break;

        case 2:

            onf();

            break;

        case 3:

            break;

        default:

            WRITE_LCD_CMD(0x01);
            delay_ms(2);

            WRITE_LCD_CMD(0x80);
            StrLCD("INVALID OPTION");

            delay_ms(1500);

            break;
    }

    WRITE_LCD_CMD(0x01);
    delay_ms(2);
}

u32 IsDeviceON(u32 hour,u32 minute,u32 second)
{
    u32 current_time;
    u32 on_time;
    u32 off_time;

    if(schedule_valid==0)
        return 0;

    current_time=TimeToSeconds(hour,minute,second);

    on_time=TimeToSeconds(on_h,on_m,on_s);

    off_time=TimeToSeconds(off_h,off_m,off_s);

    if(on_time<off_time)
    {
        if((current_time>=on_time)&&(current_time<off_time))
            return 1;

        return 0;
    }

    if((current_time>=on_time)||(current_time<off_time))
        return 1;

    return 0;
}

void eint0_isr(void) __irq
{
    flag=1;

    EXTINT=(1<<0);

    VICVectAddr=0;
}

void eint0_enable(void)
{
    PINSEL0&=~(3<<(EINT0_PIN*2));

    PINSEL0|=(3<<(EINT0_PIN*2));

    VICIntSelect&=~(1<<EINT0_CHNO);

    VICIntEnable|=(1<<EINT0_CHNO);

    VICVectAddr0=(u32)eint0_isr;

    VICVectCntl0=(1<<5)|EINT0_CHNO;

    EXTMODE|=(1<<0);

    EXTPOLAR&=~(1<<0);

    EXTINT=(1<<0);
}

int main(void)
{
    Init_LCD();

    InitKPM();

    PINSEL0&=~(3<<(DEVICE_PIN*2));

    IODIR0|=(1<<DEVICE_PIN);

    IOCLR0=(1<<DEVICE_PIN);

    RTC_Init();

    time();

    setday();

    setdate();

    SetRTCTimeInfo(h,m,s);

    SetRTCDateInfo(d,mm,y);

    DOW=day;

    onf();

    eint0_enable();

    while(1)
    {
        if(flag==1)
        {
            flag=0;

            edit();
        }

        GetRTCTimeInfo(&hour,&min,&sec);

        if(IsDeviceON(hour,min,sec))
        {
            IOSET0=(1<<DEVICE_PIN);
        }
        else
        {
            IOCLR0=(1<<DEVICE_PIN);
        }

        DisplayRTCTime(hour,min,sec);

        GetRTCDateInfo(&date,&month,&year);

        DisplayRTCDate(date,month,year);

        delay_ms(100);
    }
}
