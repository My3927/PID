#ifndef __MY_RTC_H
#define __MY_RTC_H
#include "my_include.h"

//ʱ��ṹ��
typedef struct 
{
    //������������
    u16 year;
    u8  month;
    u8  day;
    u8  week;
    u8 hour;
    u8 minute;
    u8 second;
}_calendar_obj;
extern _calendar_obj calendar;//�����ṹ��
#define My_RTC_SetCalendar(cal)     My_RTC_SetCounter(My_RTC_CalendarToInteger(cal))

bool My_RTC_Init(bool alarmEnable);//��ʼ��RTC,����0,ʧ��;1,�ɹ�;
u32 My_RTC_CalendarToInteger(const _calendar_obj *cal);
void My_RTC_StringToCalendar(const char *str,_calendar_obj *cal);
u32 My_RTC_StringToInteger(const char *str);
void My_RTC_IntegerToCalendar(u32 secondCount,_calendar_obj *cal);
bool Is_Leap_Year(u16 year);//ƽ��,�����ж�
void My_RTC_Update(void);//����ʱ��
u8 My_RTC_GetWeekday(u16 year,u8 month,u8 day);
bool My_RTC_Set(u16 syear,u8 smon,u8 sday,u8 hour,u8 min,u8 sec);//����ʱ��
void My_RTC_AddYear(int8 yearCount);
void My_RTC_AddMonth(int16 monthCount);
void My_RTC_AddDay(int16 dayCount);
void My_RTC_AddHour(int16 hourCount);
void My_RTC_AddMinute(int16 minuteCount);
void My_RTC_AddSecond(int16 secondCount);
bool My_RTC_SetStr(const char *str);
void My_RTC_SetCounter(u32 CounterValue);//����ʱ��
u16 My_RTC_GetMilliSecond(void);//��ȡ����ʱ��
#endif


