#ifndef __MY_PWM_H
#define __MY_PWM_H

#include "my_include.h"

void My_PWM_Init(TIM_TypeDef* TIMx,TIM_Channel ch, u16 period);
void My_PWM_SetDuty(TIM_TypeDef* TIMx,TIM_Channel channel,float percent);

#endif
