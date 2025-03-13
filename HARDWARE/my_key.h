#ifndef __MY_KEY_H
#define __MY_KEY_H	 
#include "my_include.h"

typedef enum
{
	KEY_1=0,
	KEY_2,
	KEY_3,
	KEY_4,
}My_KeyDef;

//按键定义
const static MyPinDef Pins_Key[] = {PB0,PB1,PB3,PB4};

#define KeyFlagType			u32					  
#define KEY_STATE_PRESS		0x00000000  //按键按下时IO口的值，32个位对应32个按键的状态
#define COUNT_LONG_PRESS	1000 			//这个数值要在实际调试中测试
#define LONG_PRESS			0  //按键长按使能定义，为0时长按功能无效

void My_KEY_Init(void);//IO初始化
void My_KeyScan(void);
#if	LONG_PRESS>0
bool KeyIsLongPress(My_KeyDef key);
bool KeyIsLongPressed(My_KeyDef key);
#endif
bool KeyIsPress(My_KeyDef key);
bool KeyIsPressed(My_KeyDef key);
bool KeyIsRelease(My_KeyDef key);
bool KeyIsReleased(My_KeyDef key);
bool HasKeyEvent(void);
#if	LONG_PRESS>0
void My_Key_PerformLongPress(My_KeyDef key);
void My_Key_PerformLongPressed(My_KeyDef key);
#endif
void My_Key_PerformPress(My_KeyDef key);
void My_Key_PerformPressed(My_KeyDef key);
void My_Key_PerformRelease(My_KeyDef key);
			    
#endif
