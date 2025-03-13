#include "my_ledblink.h"
_LED_Blink_obj ledBlink[NUM_LED_Blink];//控制参数结构体数组

void My_LEDBlink(MyPinDef _pin,u8 volt_ledON,u16 times,u16 time_on,u16 time_off)
{
    u8 i;
    for(i=0;i<ArrayCount(ledBlink);i++)
    {
        if(ledBlink[i].pin==_pin)
        {
            break;
        }
    }
    if(i==ArrayCount(ledBlink))
    {
        for(i=0;i<ArrayCount(ledBlink);i++)
        {
            if(ledBlink[i].enable == false)
            {
                break;
            }
        }
    }
    ledBlink[i].pin = _pin;//控制管脚
    ledBlink[i].volt_led_on = volt_ledON;//器件工作时的电平
    ledBlink[i].times = times;//器件控制的次数
    ledBlink[i].time_on = time_on;//器件工作的时间
    ledBlink[i].time_off = time_off;//期间不工作的时间
    ledBlink[i].enable = true;
}
//处理蜂鸣器鸣叫或LED闪烁事件处理（状态机）
void My_LEDBlinkProcess(void)
{
    u8 i = 0;//
    for(i=0;i<ArrayCount(ledBlink);i++)
    {
        if(ledBlink[i].enable == true)
        {
            if(ledBlink[i].times>0)//如果该器件控制次数大于0
            {
                //**All notes can be deleted and modified**//

                switch(ledBlink[i].blinkState)
                {
                    case 1:
                        //根据设置的器件工作电平控制器件
                        PinOut(ledBlink[i].pin) = ledBlink[i].volt_led_on?1:0;
                        ledBlink[i].ticks = My_SysTick_GetTicks();//记录系统时间
                        //**All notes can be deleted and modified**//
                        break;
                    case 2:
                        if(ledBlink[i].time_on<=((My_SysTick_GetTicks()-ledBlink[i].ticks)*My_SysTick_GetPeriod()))//开启时间到达
                        {
                            PinOut(ledBlink[i].pin) = ledBlink[i].volt_led_on?0:1;
                            ledBlink[i].ticks = My_SysTick_GetTicks();//记录系统时间
                            ledBlink[i].blinkState++;//指向下一个状态
                        }
                        break;
                    case 3:
                        if(ledBlink[i].time_off<=((My_SysTick_GetTicks()-ledBlink[i].ticks)*My_SysTick_GetPeriod()))//关闭时间到达
                        {
                            ledBlink[i].times--;//该器件控制次数-1
                            ledBlink[i].blinkState=1;//回到第一个处理状态
                        }
                        break;
                        default:break;
                }
            }
            else//本次器件控制事件结束
            {
                PinOut(ledBlink[i].pin) = ledBlink[i].volt_led_on?0:1;
                ledBlink[i].blinkState = 0;//回到初始状态
                //**All notes can be deleted and modified**//

            }
        }
    }
}
void My_LEDBlink_SetTimes(MyPinDef _pin,u16 times)
{
    u8 i;
    for(i=0;i<ArrayCount(ledBlink);i++)
    {
        if(ledBlink[i].pin==_pin)
        {
            ledBlink[i].times = times;//器件控制的次数
            break;
        }
    }
}
