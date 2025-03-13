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

//��������
const static MyPinDef Pins_Key[] = {PB0,PB1,PB3,PB4};

#define KeyFlagType			u32					  
#define KEY_STATE_PRESS		0x00000000  //��������ʱIO�ڵ�ֵ��32��λ��Ӧ32��������״̬
#define COUNT_LONG_PRESS	1000 			//�����ֵҪ��ʵ�ʵ����в���
#define LONG_PRESS			0  //��������ʹ�ܶ��壬Ϊ0ʱ����������Ч

void My_KEY_Init(void);//IO��ʼ��
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
