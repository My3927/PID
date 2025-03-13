#ifndef __MY_LED_BLINK_H
#define __MY_LED_BLINK_H
#include "my_include.h"

typedef struct _packed
{
    MyPinDef pin;
    u8 blinkState:6;
    bool enable:1;
    u8 volt_led_on:1;
    u16 times;
    u16 time_on;
    u16 time_off;
    u32 ticks;//系统定时器计数值
}_LED_Blink_obj;

#define NUM_LED_Blink               5 //
#define My_LEDBlink_Stop(n)         My_LEDBlink_SetTimes(n,0)

void My_LEDBlink(MyPinDef _pin,u8 volt_ledON,u16 times,u16 time_on,u16 time_off);
void My_LEDBlinkProcess(void);
void My_LEDBlink_SetTimes(MyPinDef _pin,u16 times);
#endif
