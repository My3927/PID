#ifndef __MY_DS18B20_H
#define __MY_DS18B20_H 
#include "my_include.h"   

u8 My_DS18B20_Init(MyPinDef pin);			//初始化DS18B20
float My_DS18B20_GetTemp(MyPinDef pin);	//获取温度  
#endif















