#ifndef __MY_LED_H
#define __MY_LED_H	 
#include "my_include.h"

//#define dj_in01	 		PinOut(Pins_LED[0])
//#define dj_in02	 		PinOut(Pins_LED[1])
#define buzzer	 		PinOut(Pins_LED[0])

const static MyPinDef Pins_LED[] = {PA4};

void My_LED_Init(void);//≥ı ºªØ

		 				    
#endif
