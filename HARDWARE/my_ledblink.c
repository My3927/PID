#include "my_ledblink.h"
_LED_Blink_obj ledBlink[NUM_LED_Blink];//���Ʋ����ṹ������

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
    ledBlink[i].pin = _pin;//���ƹܽ�
    ledBlink[i].volt_led_on = volt_ledON;//��������ʱ�ĵ�ƽ
    ledBlink[i].times = times;//�������ƵĴ���
    ledBlink[i].time_on = time_on;//����������ʱ��
    ledBlink[i].time_off = time_off;//�ڼ䲻������ʱ��
    ledBlink[i].enable = true;
}
//������������л�LED��˸�¼�����״̬����
void My_LEDBlinkProcess(void)
{
    u8 i = 0;//
    for(i=0;i<ArrayCount(ledBlink);i++)
    {
        if(ledBlink[i].enable == true)
        {
            if(ledBlink[i].times>0)//������������ƴ�������0
            {
                //**All notes can be deleted and modified**//

                switch(ledBlink[i].blinkState)
                {
                    case 1:
                        //�������õ�����������ƽ��������
                        PinOut(ledBlink[i].pin) = ledBlink[i].volt_led_on?1:0;
                        ledBlink[i].ticks = My_SysTick_GetTicks();//��¼ϵͳʱ��
                        //**All notes can be deleted and modified**//
                        break;
                    case 2:
                        if(ledBlink[i].time_on<=((My_SysTick_GetTicks()-ledBlink[i].ticks)*My_SysTick_GetPeriod()))//����ʱ�䵽��
                        {
                            PinOut(ledBlink[i].pin) = ledBlink[i].volt_led_on?0:1;
                            ledBlink[i].ticks = My_SysTick_GetTicks();//��¼ϵͳʱ��
                            ledBlink[i].blinkState++;//ָ����һ��״̬
                        }
                        break;
                    case 3:
                        if(ledBlink[i].time_off<=((My_SysTick_GetTicks()-ledBlink[i].ticks)*My_SysTick_GetPeriod()))//�ر�ʱ�䵽��
                        {
                            ledBlink[i].times--;//���������ƴ���-1
                            ledBlink[i].blinkState=1;//�ص���һ������״̬
                        }
                        break;
                        default:break;
                }
            }
            else//�������������¼�����
            {
                PinOut(ledBlink[i].pin) = ledBlink[i].volt_led_on?0:1;
                ledBlink[i].blinkState = 0;//�ص���ʼ״̬
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
            ledBlink[i].times = times;//�������ƵĴ���
            break;
        }
    }
}
