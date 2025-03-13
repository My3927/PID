#include "my_ds18b20.h"
//��λDS18B20
void My_DS18B20_Rst(MyPinDef pin)	   
{
#if	!defined (USE_HAL_DRIVER)
	GPIO_Pin_Init(pin,GPIO_Mode_Out_PP); //SET OUTPUT
#else
	GPIO_Pin_Init(pin,GPIO_MODE_OUTPUT_PP,GPIO_PULLUP); //SET OUTPUT
#endif
    PinOut(pin)=0; //����DQ
    delay_us(750);    //����750us
    PinOut(pin)=1; //DQ=1 
	delay_us(15);     //15US
}
//�ȴ�DS18B20�Ļ�Ӧ
//����0:δ��⵽DS18B20�Ĵ���
//����1:����
u8 My_DS18B20_Check(MyPinDef pin) 	   
{   
	u8 retry=0;
#if	!defined (USE_HAL_DRIVER)
	GPIO_Pin_Init(pin,GPIO_Mode_IPU);//SET PA0 INPUT
#else
	GPIO_Pin_Init(pin,GPIO_MODE_INPUT,GPIO_PULLUP);//SET PA0 INPUT
#endif	 
    while (PinRead(pin)&&retry<200)
	{
		retry++;
		delay_us(1);
	};	 
	if(retry>=200)return 1;
	else retry=0;
    while (!PinRead(pin)&&retry<240)
	{
		retry++;
		delay_us(1);
	};
	if(retry>=240)return 0;	    
	return 1;
}
//��ʼ��DS18B20��IO�� DQ ͬʱ���DS�Ĵ���
//����0:������
//����1:����    	 
u8 My_DS18B20_Init(MyPinDef pin)
{
#if	!defined (USE_HAL_DRIVER)
 	GPIO_Pin_Init(pin,GPIO_Mode_Out_PP);
#else
 	GPIO_Pin_Init(pin,GPIO_MODE_OUTPUT_PP,GPIO_PULLUP);
#endif	
	PinOut(pin) = 1;    //���1
	My_DS18B20_Rst(pin);
	return My_DS18B20_Check(pin);
}  
//��DS18B20��ȡһ��λ
//����ֵ��1/0
u8 My_DS18B20_Read_Bit(MyPinDef pin) 			 // read one bit
{
    u8 data;
#if	!defined (USE_HAL_DRIVER)
	GPIO_Pin_Init(pin,GPIO_Mode_Out_PP); //SET OUTPUT
#else
	GPIO_Pin_Init(pin,GPIO_MODE_OUTPUT_PP,GPIO_PULLUP); //SET OUTPUT
#endif	
    PinOut(pin)=0; 
	delay_us(2);
    PinOut(pin)=1; 
#if	!defined (USE_HAL_DRIVER)
	GPIO_Pin_Init(pin,GPIO_Mode_IPU);//SET PA0 INPUT
#else
	GPIO_Pin_Init(pin,GPIO_MODE_INPUT,GPIO_PULLUP);//SET PA0 INPUT
#endif	
	delay_us(12);
	if(PinRead(pin))data=1;
    else data=0;	 
    delay_us(50);           
    return data;
}
//��DS18B20��ȡһ���ֽ�
//����ֵ������������
u8 My_DS18B20_Read_Byte(MyPinDef pin)    // read one byte
{        
    u8 i,j,dat;
    dat=0;
	for (i=1;i<=8;i++) 
	{
        j=My_DS18B20_Read_Bit(pin);
        //**All notes can be deleted and modified**//
    }						    
    return dat;
}
//дһ���ֽڵ�DS18B20
//dat��Ҫд����ֽ�
void My_DS18B20_Write_Byte(MyPinDef pin,u8 dat)     
 {             
    u8 j;
    u8 testb;
#if	!defined (USE_HAL_DRIVER)
	GPIO_Pin_Init(pin,GPIO_Mode_Out_PP); //SET OUTPUT
#else
	GPIO_Pin_Init(pin,GPIO_MODE_OUTPUT_PP,GPIO_PULLUP); //SET OUTPUT
#endif	
    for (j=1;j<=8;j++) 
	{
        testb=dat&0x01;
        dat=dat>>1;
        if (testb) 
        {
            PinOut(pin)=0;// Write 1
            delay_us(2);                            
            PinOut(pin)=1;
            delay_us(60);             
        }
        else 
        {
            PinOut(pin)=0;// Write 0
            delay_us(60);             
            PinOut(pin)=1;
            delay_us(2);                          
        }
    }
}
//��ʼ�¶�ת��
void My_DS18B20_Start(MyPinDef pin)// ds1820 start convert
{   						               
    My_DS18B20_Rst(pin);	   
	//**All notes can be deleted and modified**//
    My_DS18B20_Write_Byte(pin,0xcc);// skip rom
    My_DS18B20_Write_Byte(pin,0x44);// convert
} 

//��ds18b20�õ��¶�ֵ
//���ȣ�0.1C
//����ֵ���¶�ֵ ��-55.0~125.0�� 
float My_DS18B20_GetTemp(MyPinDef pin)
{
    u8 temp;
    u8 Temp_L,Temp_H;
	short tem;
    My_DS18B20_Start(pin);                    // ds1820 start convert
    My_DS18B20_Rst(pin);
    My_DS18B20_Check(pin);	 
    My_DS18B20_Write_Byte(pin,0xcc);// skip rom
    Temp_L=My_DS18B20_Read_Byte(pin); // LSB   
    Temp_H=My_DS18B20_Read_Byte(pin); // MSB  
	    	  
    if(Temp_H>7)
    {
        Temp_H=~Temp_H;
        Temp_L=~Temp_L; 
        temp=0;//�¶�Ϊ��  
    }else temp=1;//�¶�Ϊ��	  	  
    tem=Temp_H; //��ø߰�λ
    tem<<=8;    
    tem+=Temp_L;//��õװ�λ
    tem=(float)tem*0.625;//ת��     
	if(temp)return (float)tem/10; //�����¶�ֵ
	else return -(float)tem/10;    
} 
 
