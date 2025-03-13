#include "my_led.h"
//LED IO≥ı ºªØ
void My_LED_Init(void)
{
	u8 i;
	for(i=0;i<ArrayCount(Pins_LED);i++)
	{
#if	!defined (USE_HAL_DRIVER)
		GPIO_Pin_Init(Pins_LED[i],GPIO_Mode_Out_PP);
#else
		GPIO_Pin_Init(Pins_LED[i],GPIO_MODE_OUTPUT_PP,GPIO_PULLUP);
#endif
	}
}
 
