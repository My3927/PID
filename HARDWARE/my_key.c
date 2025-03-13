#include "my_key.h"
static KeyFlagType keyFlag_state = 0;
static KeyFlagType keyFlag_event = 0;
#if	LONG_PRESS>0
static u16 key_count[ArrayCount(Pins_Key)];
#endif
/***********************************						

***********************************/
void My_KEY_Init(void) //IO��ʼ��
{
	uint8_t index;	
	for(index=0;index<ArrayCount(Pins_Key);index++){
		if(KEY_STATE_PRESS&(0x01<<index))
		{
#if	!defined (USE_HAL_DRIVER)
			GPIO_Pin_Init(Pins_Key[index],GPIO_Mode_IPD);
#else
			GPIO_Pin_Init(Pins_Key[index],GPIO_MODE_INPUT,GPIO_PULLDOWN);
#endif
		}
		else
		{
#if	!defined (USE_HAL_DRIVER)
			GPIO_Pin_Init(Pins_Key[index],GPIO_Mode_IPU);
#else
			GPIO_Pin_Init(Pins_Key[index],GPIO_MODE_INPUT,GPIO_PULLUP);
#endif
		}
	}	
}
KeyFlagType My_KeyReadState(void)
{
	KeyFlagType key_state=0;
	u8 i;
	//��ȡ���ж���İ���״̬
	for(i=0;i<ArrayCount(Pins_Key);i++){
		if(PinRead(Pins_Key[i])==(KEY_STATE_PRESS&(0x01<<i)))
		{
			key_state |= (0x01<<i);
		}
	}
	return key_state;
}
void My_KeyScan(void)
{
	KeyFlagType key_state=0;
#if	LONG_PRESS>0
	u8 i;
#endif
	key_state = My_KeyReadState();
	if(key_state!=keyFlag_state){//����а���״̬�����仯����ʱ
		delay_ms(8);   //��������
		//**All notes can be deleted and modified**//
	}
	
	keyFlag_event = keyFlag_state^key_state;
#if	LONG_PRESS>0
	for(i=0;i<ArrayCount(Pins_Key);i++)
	{
		if(key_state&(0x0001<<i))//�������������
		{
			if(keyFlag_state&(0x01<<i))//���������һ�ε�״̬�ǰ��µ�
			{
				//ֻҪ�����������ΰ��¾Ϳ�ʼ����	
				if(key_count[i]<0xffff)
				{
					key_count[i]++;
				}
				if(key_count[i]==COUNT_LONG_PRESS)
				{
					keyFlag_event |= 0x01<<i;
				}
				else
				{
					keyFlag_event &= ~(0x01<<i);
				}
			}
		}
		else //�������û�а���
		{
			if(keyFlag_state&(0x01<<i))//���������һ�ε�״̬�ǰ��µ�
			{
				key_count[i]=0;
			}
		}
	}
#endif
	keyFlag_state = key_state;
}

#if	LONG_PRESS>0
bool KeyIsLongPress(My_KeyDef key)
{
	return (bool)((keyFlag_event&(0x01<<key))&&(keyFlag_state&(0x01<<key))&&(key_count[key]>=COUNT_LONG_PRESS));
}
bool KeyIsLongPressed(My_KeyDef key)
{
	return (bool)(!(keyFlag_event&(0x01<<key))&&(keyFlag_state&(0x01<<key))&&(key_count[key]>=COUNT_LONG_PRESS));
}
bool KeyIsPress(My_KeyDef key)
{
	return (bool)((keyFlag_event&(0x01<<key))&&(keyFlag_state&(0x01<<key))&&(key_count[key]<COUNT_LONG_PRESS));
}
#else
bool KeyIsPress(My_KeyDef key)
{
	return (bool)((keyFlag_event&(0x01<<key))&&(keyFlag_state&(0x01<<key)));
}
#endif
bool KeyIsPressed(My_KeyDef key)
{
	return (bool)((!(keyFlag_event&(0x01<<key)))&&(keyFlag_state&(0x01<<key)));
}
bool KeyIsRelease(My_KeyDef key)
{
	return (bool)((keyFlag_event&(0x01<<key))&&!(keyFlag_state&(0x01<<key)));
}
bool KeyIsReleased(My_KeyDef key)
{
	return (bool)((!(keyFlag_event&(0x01<<key)))&&(!(keyFlag_state&(0x01<<key))));
}

bool HasKeyEvent(void)
{
	if(keyFlag_event)
	{
		return  true;
	}
	return false;
}

#if	LONG_PRESS>0
void My_Key_PerformLongPress(My_KeyDef key)
{
	keyFlag_event|=(0x01<<key);
	keyFlag_state|=(0x01<<key);
	key_count[key]=COUNT_LONG_PRESS;
}
void My_Key_PerformLongPressed(My_KeyDef key)
{
	keyFlag_event&=~(0x01<<key);
	keyFlag_state|=(0x01<<key);
	key_count[key]=COUNT_LONG_PRESS;
}
void My_Key_PerformPress(My_KeyDef key)
{
	keyFlag_event|=(0x01<<key);
	keyFlag_state|=(0x01<<key);
	key_count[key]=0;
}
void My_Key_PerformPressed(My_KeyDef key)
{
	keyFlag_event&=~(0x01<<key);
	keyFlag_state|=(0x01<<key);
	key_count[key]=0;
}
#else
void My_Key_PerformPress(My_KeyDef key)
{
	keyFlag_event|=(0x01<<key);
	keyFlag_state|=(0x01<<key);
}
void My_Key_PerformPressed(My_KeyDef key)
{
	keyFlag_event&=~(0x01<<key);
	keyFlag_state|=(0x01<<key);
}
#endif
void My_Key_PerformRelease(My_KeyDef key)
{
	keyFlag_event|=(0x01<<key);
	keyFlag_state&=~(0x01<<key);
}

