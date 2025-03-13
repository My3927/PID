#include "my_pwm.h"
u16 Pwm_Per[4];
/************************************************************
�������ܣ�PWM�����ʼ��
TIMx��PWMʹ�õĶ�ʱ������TIM2
ch����ʱ�����ͨ�����ο�TIM_Channel����
period��PWM������ڣ���λ��΢�룩
************************************************************/
void My_PWM_Init(TIM_TypeDef* TIMx,TIM_Channel ch, u16 period)
{
/* Set TIM3 and TIM4 for PWM mode */
    TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
    TIM_OCInitTypeDef  TIM_OCInitStructure;

    switch((u32)TIMx)
    {
        case (u32)TIM1:
            RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1, ENABLE);
            if(ch&TIM_CH_1)GPIO_Pin_Init(PA8,GPIO_Mode_AF_PP);
            if(ch&TIM_CH_2)GPIO_Pin_Init(PA9,GPIO_Mode_AF_PP);
            if(ch&TIM_CH_3)GPIO_Pin_Init(PA10,GPIO_Mode_AF_PP);
            if(ch&TIM_CH_4)GPIO_Pin_Init(PA11,GPIO_Mode_AF_PP);
            Pwm_Per[0] = period;
            break;
        case (u32)TIM2:
            RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);
            if(ch&TIM_CH_1)GPIO_Pin_Init(PA0,GPIO_Mode_AF_PP);
            if(ch&TIM_CH_2)GPIO_Pin_Init(PA1,GPIO_Mode_AF_PP);
            if(ch&TIM_CH_3)GPIO_Pin_Init(PA2,GPIO_Mode_AF_PP);
            if(ch&TIM_CH_4)GPIO_Pin_Init(PA3,GPIO_Mode_AF_PP);
            Pwm_Per[1] = period;
            break;
        case (u32)TIM3:
            RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);
            if(ch&TIM_CH_1)GPIO_Pin_Init(PA6,GPIO_Mode_AF_PP);
            if(ch&TIM_CH_2)GPIO_Pin_Init(PA7,GPIO_Mode_AF_PP);
            if(ch&TIM_CH_3)GPIO_Pin_Init(PB0,GPIO_Mode_AF_PP);
            if(ch&TIM_CH_4)GPIO_Pin_Init(PB1,GPIO_Mode_AF_PP);
            Pwm_Per[2] = period;
            break;
        case (u32)TIM4:
            RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE);
            if(ch&TIM_CH_1)GPIO_Pin_Init(PB6,GPIO_Mode_AF_PP);
            if(ch&TIM_CH_2)GPIO_Pin_Init(PB7,GPIO_Mode_AF_PP);
            if(ch&TIM_CH_3)GPIO_Pin_Init(PB8,GPIO_Mode_AF_PP);
            if(ch&TIM_CH_4)GPIO_Pin_Init(PB9,GPIO_Mode_AF_PP);
            Pwm_Per[3] = period;
            break;
        default:break;
    }

    TIM_TimeBaseStructure.TIM_Period = period-1;//��������һ�������¼�װ�����Զ���װ�ؼĴ������ڵ�ֵ
    TIM_TimeBaseStructure.TIM_Prescaler =7199;//����������ΪTIMxʱ��Ƶ�ʳ�����Ԥ��Ƶֵ
    TIM_TimeBaseStructure.TIM_ClockDivision = 0;//����ʱ�ӷָ�:TDTS = Tck_tim
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;//TIM���ϼ���ģʽ
    TIM_TimeBaseInit(TIMx, &TIM_TimeBaseStructure); //����TIM_TimeBaseInitStruct��ָ���Ĳ�����ʼ��TIMx��ʱ�������λ

    TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1; //ѡ��ʱ��ģʽ:TIM�����ȵ���ģʽ1
    TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable; //�Ƚ����ʹ��
    TIM_OCInitStructure.TIM_Pulse = period; //���ô�װ�벶��ȽϼĴ���������ֵ
    TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High; //�������:TIM����Ƚϼ��Ը�
    TIM_OCInitStructure.TIM_OCIdleState=TIM_OCIdleState_Set;
    //**All notes can be deleted and modified**//

    if(ch&TIM_CH_1)TIM_SetCompare1(TIMx, 0);
    if(ch&TIM_CH_2)TIM_SetCompare2(TIMx, 0);
    if(ch&TIM_CH_3)TIM_SetCompare3(TIMx, 0);
    if(ch&TIM_CH_4)TIM_SetCompare4(TIMx, 0);

    TIM_CtrlPWMOutputs(TIMx,ENABLE);//MOE �����ʹ��

    if(ch&TIM_CH_1)TIM_OC1PreloadConfig(TIMx, TIM_OCPreload_Enable);  //CH1Ԥװ��ʹ��
    if(ch&TIM_CH_2)TIM_OC2PreloadConfig(TIMx, TIM_OCPreload_Enable);  //CH2Ԥװ��ʹ��
    if(ch&TIM_CH_3)TIM_OC3PreloadConfig(TIMx, TIM_OCPreload_Enable);  //CH3Ԥװ��ʹ��
    if(ch&TIM_CH_4)TIM_OC4PreloadConfig(TIMx, TIM_OCPreload_Enable);  //CH4Ԥװ��ʹ��

    TIM_ARRPreloadConfig(TIMx, ENABLE); //ʹ��TIMx��ARR�ϵ�Ԥװ�ؼĴ���

    TIM_Cmd(TIMx, ENABLE);  //ʹ��TIMx
}
/************************************************************
�������ܣ�PWM���ռ�ձȵ�����
TIMx��PWMʹ�õĶ�ʱ������TIM2
ch����ʱ�����ͨ�����ο�TIM_Channel����
percent��PWM���ռ�ձ�
************************************************************/
void My_PWM_SetDuty(TIM_TypeDef* TIMx,TIM_Channel ch,float percent)
{
    u16 per;
    if(percent>100){
        percent = 100;
    }
    switch((u32)TIMx)
    {
        case (u32)TIM1:per = Pwm_Per[0];break;
        case (u32)TIM2:per = Pwm_Per[1];break;
        case (u32)TIM3:per = Pwm_Per[2];break;
        case (u32)TIM4:per = Pwm_Per[3];break;
        default:break;
    }
    if(ch&TIM_CH_1)TIM_SetCompare1(TIMx, per*percent/100);
    if(ch&TIM_CH_2)TIM_SetCompare2(TIMx, per*percent/100);
    if(ch&TIM_CH_3)TIM_SetCompare3(TIMx, per*percent/100);
    if(ch&TIM_CH_4)TIM_SetCompare4(TIMx, per*percent/100);
}
