#include "my_include.h"

//**All notes can be deleted and modified**//
char dis0[50];//Һ����ʾ�ݴ�����
char dis1[128];//Һ����ʾ�ݴ�����

#define F_SIZE      16
#define MyLCD_Show(m,n,p)     LCD_ShowString(LCD_GetPos_X(F_SIZE,m),LCD_GetPos_Y(F_SIZE,n),p,F_SIZE,false)

float nowTemp = 0;
u16 nowPwmVal = 95;//pwmռ�ձȵ��� 
u16 nowDir = 1;//��ǰ���÷��� 
float nowZs = 0;//��ǰ�ٶ�
u16 setZsYz = 30;//����ת��
u16 setZs = 20;//����ת��
u16 plu=0;//�����жϼ���
u16 setTemp = 35;//�����¶���ֵ
u8 readTemp = 0;//��ȡ�¶ȱ�־

u8 setMode =0;//����ģʽ
u8 updataDis =1;//�����ݸ���
u8 buzzerFlag =0;//����������״̬

u16 i ;//�м���� i

void OnGetUartMessage(const _uart_msg_obj *uartMsgRec)
{
    char *strPtr;
    if((strPtr=strstr(uartMsgRec->payload,"RTC"))!=NULL)//���յ�rtc�ַ��� *RTC20200420173611#
    {
        My_RTC_SetStr(strPtr+3);//����ʱ��
    }
    if((strPtr=strstr(uartMsgRec->payload,"MD"))!=NULL)//���յ��ַ���
    {
        setMode = *(strPtr+2) - 0x30;//��ʱģʽ
    }
    if((strPtr=strstr(uartMsgRec->payload,"FX"))!=NULL)//���յ��ַ���
    {
        if(*(strPtr+2) == '1') nowDir= 1;
        else nowDir =2;
    }
    if((strPtr=strstr(uartMsgRec->payload,"WD"))!=NULL)//���յ�rtc�ַ��� 
    {
        setTemp=ParseInteger(strPtr+2,3);//��ȡ����
    }
    if((strPtr=strstr(uartMsgRec->payload,"SY"))!=NULL)//���յ�rtc�ַ��� 
    {
        setZsYz=ParseInteger(strPtr+2,3);//��ȡ����	
    }
    if((strPtr=strstr(uartMsgRec->payload,"KS"))!=NULL)//���յ�rtc�ַ��� 
    {
        setZs=ParseInteger(strPtr+2,3);//��ȡ����
    }
    if(setMode == 0) //����ģʽ�� pwm����
    {
        if((strPtr=strstr(uartMsgRec->payload,"PW"))!=NULL)//���յ�rtc�ַ��� 
        {
            nowPwmVal=ParseInteger(strPtr+2,3);//��ȡ����
        }
    }
    updataDis=1;//��Ҫ������ʾ����
}

void ctrlPwmDjFan(u8 dir,u8 pre) //���÷���ռ�ձ�
{
    if(dir == 1) //����1
    {
        My_PWM_SetDuty(TIM1,TIM_CH_1,pre);//���ڶ�ʱ�� ͨ�� ռ�ձ�
        My_PWM_SetDuty(TIM3,TIM_CH_2,0);//���ڶ�ʱ�� ͨ�� ռ�ձ�
    }
    else
    {
        My_PWM_SetDuty(TIM1,TIM_CH_1,0);//���ڶ�ʱ�� ͨ�� ռ�ձ�
        My_PWM_SetDuty(TIM3,TIM_CH_2,pre);//���ڶ�ʱ�� ͨ�� ռ�ձ�
    }
}
void scanKeyAnddealKey(void);
void write_FlashBuf(void);
void read_FlashBuf(void);
void init_FlashBuf(void);
void keySetTime( void );
double myZsPIDCalc(double rIn) ;
int main(void)
{ 
    u8 timesCount=0;//����ͳ��
    u8 zSupdateFla=0;//ת�������и���
    u8 rememberMode =0xff;//��¼��һ������״̬
    u16 rememberTiemSe=0xffff;//��¼��ǰ������ ����û�б仯��������ʾ ��Ϊlcd������ʾ���˷�ʱ��
    u8 disYplace=0; //��ʾ�����е�������
    
    USARTx_Init(USART1,9600);
//	USARTx_Init(USART2,9600);
    My_RTC_Init(false);
    
    My_PWM_Init(TIM3,TIM_CH_2,1000);//��ʼ����ʱ�� ͨ�� ����1ms pwm��ʼ������ڰ�����led��ǰ��
    My_PWM_Init(TIM1,TIM_CH_1,1000);//��ʼ����ʱ�� ͨ�� ����1ms
    delay_ms(2);
    My_LED_Init(); //��ʼ��led
    My_KEY_Init();	//��ʼ������
    
    init_FlashBuf();//��ʼ��flash������
    
    EXTIx_Init(PA0,EXTI_Trigger_Rising,2);
    
    My_LEDBlink(PA1,BEEP_ON,1,50,500);//�ϵ�Ӳ��������
    ctrlPwmDjFan(1,0); //�����µ��
    
    LCD_Init();   //tft��ʼ��
    LCD_Clear(Color16_BLACK);//��ȫ��
    BACK_COLOR=Color16_BLACK;
    FRONT_COLOR=Color16_LIGHTGRAY;

    keySetTime();//���밴������ʱ�� �����������ͨ�ŵ������ �ð������ÿ���ȡ��
    ctrlPwmDjFan(1,80); //�����µ��
    
    MyLCD_Show(2,0,"���������");//��ʾ
    FRONT_COLOR=Color16_RED;disYplace=2; //��ʾ�����е�������
    MyLCD_Show(0,disYplace++,"ģʽ: ");//��ʾ
    FRONT_COLOR=Color16_YELLOW;
    MyLCD_Show(0,disYplace++,"ʵ��: ");//��ʾ
    MyLCD_Show(0,disYplace++,"PWM : ");//��ʾ
    MyLCD_Show(0,disYplace++,"��ֵ: ");//��ʾ
    MyLCD_Show(0,disYplace++,"����: ");//��ʾ
    MyLCD_Show(0,disYplace++,"�¶�: ");//��ʾ
    
    if(My_DS18B20_Init(PA5)){printf("18B20 exist");}
    
    while(1)
    {
        scanKeyAnddealKey(); //����ɨ�輰����
        if(rememberTiemSe!=calendar.second) //ʱ�䷢���˱仯 ������ʾ
        {
            rememberTiemSe = calendar.second;//��¼�˿�ʱ��
            FRONT_COLOR=Color16_LIGHTGRAY;
            sprintf((char*)dis0,"%02d-%02d %02d:%02d:%02d %d ",calendar.month,calendar.day,calendar.hour,calendar.minute,calendar.second,calendar.week);//��������
            MyLCD_Show(0,1,dis0);//��ʾ	
        }  
        
        if(myReadFlag_tick)//������ʾ
        {
            myReadFlag_tick =false;
            
            readTemp++; //����
            if(readTemp>2)
            {
                readTemp =0;//��ձ�ʾ
                nowTemp = My_DS18B20_GetTemp(PA1);//�ɼ��¶�
            }

            timesCount++;
            if(timesCount>=6)//ÿ500ms ��¼һ��
            {
                timesCount = 0; //��� ���¼���
                nowZs = (float)plu /3; //����һ�� һȦ1������
                if(plu != 0){
                    nowZs = (nowZs + (float)plu /3) / 2; //�����⵽���� ǰ������ȡƽ��ֵ
                }
                else{
                    nowZs = 0; //��ⲻ�� ��ʾֹͣ
                }
                zSupdateFla =1;//�ٶ������и���
                plu = 0;//��ռ���
            }
            
            FRONT_COLOR=Color16_RED;disYplace=2; //��ʾ�����е�������
            if(rememberMode != setMode)
            {
                rememberMode = setMode;//��¼����ģʽ
                if(setMode == 0)
                {
                    MyLCD_Show(6,disYplace,"���ٲ��� ");//��ʾ
                    MyLCD_Show(0,disYplace+3,"��ֵ: ");//��ʾ
                }
                else if(setMode == 1)
                {
                    MyLCD_Show(6,disYplace,"�ջ����� ");//��ʾ
                    MyLCD_Show(0,disYplace+3,"����: ");//��ʾ
                }
            }disYplace++;
            
            FRONT_COLOR=Color16_LIGHTGRAY;
            sprintf(dis0,"%4.2fr/s ",nowZs);//��ӡ
            MyLCD_Show(6,disYplace++,dis0);//��ʾ
            
            switch(setMode) //��ģʽ��ʾ
            {
                case 0: //���ټ��
                            FRONT_COLOR=Color16_LIGHTBLUE;
                            sprintf(dis0,"%d  ",nowPwmVal);//��ӡ
                            MyLCD_Show(6,disYplace++,dis0);//��ʾ
                        
                            if(nowZs > (float)setZsYz/10)FRONT_COLOR=Color16_RED;
                            else FRONT_COLOR=Color16_LIGHTBLUE;
                            sprintf(dis0,"%3.1f ",(float)setZsYz/10);//��ӡ
                            MyLCD_Show(6,disYplace++,dis0);//��ʾ
                            break;
                case 1: //���� ����
                            sprintf(dis0," %d  ",nowPwmVal);//��ӡ
                            MyLCD_Show(5,disYplace++,dis0);//��ʾ
                            
                            FRONT_COLOR=Color16_LIGHTBLUE;
                            sprintf(dis0,"%3.1f ",(float)setZs/10);//��ӡ
                            MyLCD_Show(6,disYplace++,dis0);//��ʾ
                
                            break;
                case 2: //��ʱģʽ
                            break;
                default: break;
            }
            FRONT_COLOR=Color16_LIGHTBLUE;
            sprintf(dis0,"%d ",nowDir);//��ӡ
            MyLCD_Show(6,disYplace++,dis0);//��ʾ

            sprintf(dis0,"%02d ",setTemp);//��ӡ
            MyLCD_Show(6,disYplace,dis0);//��ʾ
            
            if(nowTemp > setTemp)FRONT_COLOR=Color16_RED;
                else FRONT_COLOR=Color16_LIGHTGRAY;
            sprintf(dis0,"%4.1f'C ",nowTemp);//��ӡ
            MyLCD_Show(9,disYplace++,dis0);//��ʾ

            switch(setMode) //��ģʽ����
            {
                case 0: //���ټ��
                            if(nowTemp > setTemp || nowZs > (float)setZsYz/10){buzzerFlag = 1;}
                            else {buzzerFlag = 0;}
                            ctrlPwmDjFan(nowDir,nowPwmVal); //���Ƶ������ ������pwm�����ٶ�
                            break;
                case 1: //���� ����
                            if(nowTemp > setTemp ){buzzerFlag = 1;}
                            else {buzzerFlag = 0;}
                            if(zSupdateFla == 1)//�ٶ������и��� ���д���
                            {
                                zSupdateFla = 0;//�����־
                                
                                nowZs = myZsPIDCalc(nowZs);//ת�پ��� PID����� �ٴν��з����Ա� Ч������
                                                                
                                if(nowZs > ((float)setZs/10 + 0.2 )) //�Ա��ٶ�
                                {
                                    if(nowPwmVal >= 5)nowPwmVal -= 5;//�Զ������ٶ�
                                }
                                else if(nowZs < ((float)setZs/10 - 0.2 ))//�Ա��ٶ�
                                {
                                    if(nowPwmVal < 100)nowPwmVal += 5;//�Զ������ٶ�
                                }
                            }
                            ctrlPwmDjFan(nowDir,nowPwmVal); //���Ƶ������ ������pwm�����ٶ�
                            break;
                default: break;
            }
            if(buzzerFlag)//��������������
            {My_LEDBlink(PA4,BEEP_ON,5,100,500);}//����������
            else
            {My_LEDBlink_Stop(PA4);}//ֹͣ����
        }

        if(mySendFlag_tick)//������ʾ
        {
            mySendFlag_tick =false;//��ձ�־

            sprintf((char*)dis1,"*D%04d%02d%02d%02d%02d%02d",calendar.year,calendar.month,calendar.day,calendar.hour,calendar.minute,calendar.second);//��ӡ
            sprintf((char*)dis1,"M%dZ%04dY%03dP%03dT%03dt%03d",(u16)setMode,(u16)(nowZs*100),setZsYz,nowPwmVal,(u16)(nowTemp*10),setTemp);
            sprintf((char*)dis1,"K%03dF%dB%d#",setZs,nowDir,(u16)buzzerFlag);
            USARTSendString(USART1 , dis1);//��������
        }
        My_UartMessage_Process();//����������
    }
}


void scanKeyAnddealKey(void)
{
        static u8 setParaCount_0=0; //���ñ���λ��
        static u8 setParaCount_1=0; //���ñ���λ��
        u8 disYplace=0; //��ʾ�����е�������
        My_KeyScan();
        
        if(KeyIsPress(KEY_4))
        {
            setMode++;//ģʽ�л�
            if(setMode >= 2 )setMode =0;//ģʽ�л�
        }
        switch(setMode)
        {
            case 0: //�Զ�ģʽ
                        setParaCount_1 = 0 ;//ģʽ1���� λ�����
                        if(KeyIsPress(KEY_1))
                        {
                            setParaCount_0++;//�л����õ�ǰ����
                            if(setParaCount_0>=5)
                            {
                                setParaCount_0=0;//���õ�ǰ����
                                write_FlashBuf();//��������д��flash
                            }
                        }
                        if(KeyIsPress(KEY_2))//��������
                        {
                            if(setParaCount_0 == 1) //��ǰ����pwm
                            {
                                if(nowPwmVal>=5){nowPwmVal-=5;}//����pwm
                            }
                            else if(setParaCount_0 == 2) //��ǰ���� ת����ֵ 
                            {
                                if(setZsYz>0){setZsYz-=1;}
                            }
                            else if(setParaCount_0 == 3) //���÷���
                            {
                                if(nowDir!=1){nowDir = 1;}
                            }
                            else if(setParaCount_0 == 4) //��ǰ�����¶���ֵ
                            {
                                if(setTemp>0){setTemp-=1;}
                            }//��С ������ֵ
                        }
                        if(KeyIsPress(KEY_3))//��������
                        {
                            if(setParaCount_0 == 1) //��ǰ����pwm
                            {
                                if(nowPwmVal<100){nowPwmVal+=5;}//����pwm
                            }
                            else if(setParaCount_0 == 2) //��ǰ���� ת����ֵ 
                            {
                                if(setZsYz<999){setZsYz+=1;}
                            }
                            else if(setParaCount_0 == 3) //���÷���
                            {
                                if(nowDir!=2){nowDir = 2;}
                            }
                            else if(setParaCount_0 == 4) //��ǰ�����¶���ֵ
                            {
                                if(setTemp<99){setTemp+=1;}
                            }//��С ������ֵ
                        }
                        if(HasKeyEvent()||updataDis==1)//���κΰ������� ������Ҫ���ݸ��� ������¹���Ƶ���˷�ʱ��
                        {
                            if(updataDis==1)write_FlashBuf();//��������д��flash ���ܴ��������и���
                            updataDis =0;//��Ҫ���ݸ��� ����
                            FRONT_COLOR=Color16_RED;disYplace=4;
                            for(i=1;i<5;i++) //��ʾ����λ�ü����ò��� ���������һ������Ϊ�հ׷����ò��� ���Դ�1��ʼ
                            {
                                if(i == setParaCount_0)
                                    {MyLCD_Show(5,disYplace++,">");}//����������λ�� ��ʾ>
                                //**All notes can be deleted and modified**//
                            }
                        }
                        break;
            case 1: //�ֶ�ģʽ
                        setParaCount_0 = 0 ;//ģʽ1���� λ�����
                        if(KeyIsPress(KEY_1))
                        {
                            setParaCount_1++;//�л����õ�ǰ����
                            if(setParaCount_1>=4)
                            {
                                setParaCount_1=0;//���õ�ǰ����
                                write_FlashBuf();//��������д��flash
                            }
                        }
                        if(KeyIsPress(KEY_2))//��������
                        {
                            if(setParaCount_1 == 1) //��ǰ���� ת����ֵ 
                            {
                                if(setZs>0){setZs-=1;}
                            }
                            else if(setParaCount_1 == 2) //���÷���
                            {
                                if(nowDir!=1){nowDir = 1;}
                            }
                            else if(setParaCount_1 == 3) //��ǰ�����¶���ֵ
                            {
                                if(setTemp>0){setTemp-=1;}
                            }//��С ������ֵ
                        }
                        if(KeyIsPress(KEY_3))//��������
                        {
                            if(setParaCount_1 == 1) //��ǰ���� ת����ֵ 
                            {
                                if(setZs<999){setZs+=1;}
                            }
                            else if(setParaCount_1 == 2) //���÷���
                            {
                                if(nowDir!=2){nowDir = 2;}
                            }
                            else if(setParaCount_1 == 3) //��ǰ�����¶���ֵ
                            {
                                if(setTemp<99){setTemp+=1;}
                            }//��С ������ֵ
                        }
                        if(HasKeyEvent()||updataDis==1)//���κΰ������� ������Ҫ���ݸ��� ������¹���Ƶ���˷�ʱ��
                        {
                            if(updataDis==1)write_FlashBuf();//��������д��flash ���ܴ��������и���
                            updataDis =0;//��Ҫ���ݸ��� ����
                            FRONT_COLOR=Color16_RED;disYplace=5;
                            for(i=1;i<4;i++) //��ʾ����λ�ü����ò��� ���������һ������Ϊ�հ׷����ò��� ���Դ�1��ʼ
                            {
                                if(i == setParaCount_1)
                                    {MyLCD_Show(5,disYplace++,">");}//����������λ�� ��ʾ>
                                //**All notes can be deleted and modified**//
                            }
                        }
                        break;
                case 2: 
                        break;
            default: break;
        }
}


/*
PID Function

*/
#define PID_PROPORTION   1
#define PID_INTEGRAL     1
#define PID_DERIVATIVE   0.1
#define PID_SETPOINT     0.001
#define PID_SENSOR_MAX   0.00001
typedef struct PID {
    double  SetPoint;           //  �趨Ŀ�� Desired Value
    double  Proportion;         //  �������� Proportional Const
    double  Integral;           //  ���ֳ��� Integral Const
    double  Derivative;         //  ΢�ֳ��� Derivative Const
    double  LastError;          //  Error[-1]
    double  PrevError;          //  Error[-2]
    double  SumError;           //  Sums of Errors
} PID;  

/*PID���㲿��*/
double PIDCalc( PID *pp, double NextPoint )
    {
        double  dError, Error;
        Error = pp->SetPoint -  NextPoint;          // ƫ�� 
        pp->SumError += Error;                      // ����
        dError = pp->LastError - pp->PrevError;     // ��ǰ΢��
        pp->PrevError = pp->LastError;         
        pp->LastError = Error;  
        return (pp->Proportion * Error              // ������ 
            +   pp->Integral * pp->SumError         // ������ 
        +   pp->Derivative * dError             // ΢���� 
        );
}   

double sensor (void)    //  Dummy Sensor Function 
{
    return PID_SENSOR_MAX; 
}  

double myZsPIDCalc(double rIn) 
{   
    PID         sPID;                   //  PID Control Structure  
    double      rOut;                   //  PID Response (Output) 

    memset ( &sPID,0,sizeof(sPID));                   //  Initialize Structure   
    sPID.Proportion = PID_PROPORTION;              //  Set PID Coefficients   
    sPID.Integral   = PID_INTEGRAL;     
    sPID.Derivative = PID_DERIVATIVE;   
    sPID.SetPoint   = PID_SETPOINT;            //  Set PID Setpoint    

    rIn = sensor ();                //  Read Input       
    rOut = PIDCalc ( &sPID,rIn );   //  Perform PID Interation   
    return  rOut ;              //  Effect Needed Changes    

}   


#define MAX_SET_SIZE 10
u16 setParaTab[MAX_SET_SIZE];
u16 initFlag = 0xaa;

 
#define FLASH_SAVE_ADDR         FLASH_BASE_ADDR+(STM32_FLASH_SIZE-1)*STM_SECTOR_SIZE

/******************�����ݱ��浽��Ƭ���ڲ�eepom��******************/
void write_FlashBuf(void)
{
    setParaTab[0] = initFlag;   //�޸�ֵ
    setParaTab[1]=nowDir;
    setParaTab[2]=setTemp;
    setParaTab[3]=setZsYz;
    setParaTab[4]=setZs;
    setParaTab[5]=nowPwmVal;
    My_STMFlash_Write(FLASH_SAVE_ADDR,(u16*)setParaTab,MAX_SET_SIZE);//��ȡ����
}

/******************�����ݴӵ�Ƭ���ڲ�eepom�ж�����*****************/
void read_FlashBuf(void)
{
    My_STMFlash_Read(FLASH_SAVE_ADDR,(u16*)setParaTab,MAX_SET_SIZE);//��ȡ����
    initFlag = setParaTab[0] ;   //�޸�ֵ
    nowDir  = setParaTab[1];
    setTemp = setParaTab[2];
    setZsYz = setParaTab[3];
    setZs   = setParaTab[4];
    nowPwmVal = setParaTab[5];
}


void init_FlashBuf(void)
{
    read_FlashBuf(); //��ȡflashֵ
    if(initFlag != 0xaa)// ������� ����д��
    {
        initFlag    = 0xaa ;   //�޸�ֵ
        nowDir      = 1;
        setTemp     = 35; 
        setZsYz     = 30;
        setZs       = 20;
        nowPwmVal = 80;
//      setParaTab[6]=20;
        delay_ms(5);
        write_FlashBuf();//��ȡ����
    }
}


#pragma diag_suppress 188
#ifdef LCD_2_4 
    #define MyLCD_ShowSetTime(m,n,p) LCD_ShowString(LCD_GetPos_X(24,m),LCD_GetPos_Y(24,n),p,24,false)	
#else 
    #define MyLCD_ShowSetTime(m,n,p) LCD_ShowString(LCD_GetPos_X(12,m),LCD_GetPos_Y(12,n),p,12,false)	
#endif
void keySetTime( void )
{
    u8 delay=0;//��ʱ����
    u8 setTimeFlag =0;//����������ʱ��
    u8 disPlace =0;
    u16 setYear,setMonth,setDay,setHour,setMin,setSec;
    u8 setFlag = 0;
    u8 i= 0;
    
    MyLCD_ShowSetTime(1,2,"Set Rtc ?");
    MyLCD_ShowSetTime(1,3,"K1:Yes K2:No");
    delay = 100; //��ʱѡ��Լ2s
    while(delay)
    {
        My_KeyScan();//����ɨ��
        if(KeyIsPressed(0)) 
        {
            setTimeFlag = 1;//����ʱ������
            LCD_Clear(Color16_BLACK);//���Һ����Ļ
            FRONT_COLOR = Color16_WHITE;//����Һ��ǰ��ɫ��������ɫ��
            MyLCD_ShowSetTime(0,0,"set RTC :");
            setYear = calendar.year;setMonth = calendar.month;setDay = calendar.day;
            setHour = calendar.hour;setMin = calendar.minute;setSec = calendar.second;
            break;
        }
        else if(KeyIsPress(1))
        {
            break;
        }
        delay--; //��ʱ--
        delay_ms(10);
    }
    
    while(setTimeFlag)//ʱ������ ���õ����һ����ٵ����������
    {
        disPlace =1;//��ʾλ����������
        FRONT_COLOR = Color16_WHITE;//����Һ��ǰ��ɫ��������ɫ��
        sprintf(dis0,"Year:%04d ",setYear);
        MyLCD_ShowSetTime(2,disPlace++,dis0);
        
        sprintf(dis0,"Mon :%02d ",setMonth);
        MyLCD_ShowSetTime(2,disPlace++,dis0);
        
        sprintf(dis0,"Day :%02d ",setDay);
        MyLCD_ShowSetTime(2,disPlace++,dis0);
        
        sprintf(dis0,"Hour:%02d ",setHour);
        MyLCD_ShowSetTime(2,disPlace++,dis0);
        
        sprintf(dis0,"Min :%02d ",setMin);
        MyLCD_ShowSetTime(2,disPlace++,dis0);
        
        sprintf(dis0,"Sec :%02d ",setSec);
        MyLCD_ShowSetTime(2,disPlace++,dis0);

        My_KeyScan();//����ɨ��
        
            if(KeyIsPress(0)) //����ֵ��
            {
                setFlag++;//���ñ�־
                if(setFlag >= 7) 
                {
                    setFlag = 0;//���ý���
                    setTimeFlag =0;//���ý���
                    LCD_Clear(Color16_BLACK);//���Һ����Ļ
                    FRONT_COLOR = Color16_WHITE;//����Һ��ǰ��ɫ��������ɫ��
                    BACK_COLOR = Color16_BLACK;//����Һ������ɫ��������ɫ��
                    break;
                }
            }
            else if(KeyIsPress(1)) //����ֵ��+
            {
                if(setFlag == 1)if(setYear  < 2999){setYear  = setYear  +1;}//������   +1
                if(setFlag == 2)if(setMonth < 12  ){setMonth = setMonth +1;}//������
                if(setFlag == 3)if(setDay   < 31  ){setDay   = setDay   +1;}//������
                //  ������ �� �����Զ���
                if(setFlag == 4)if(setHour  < 23  ){setHour = setHour   +1;}//����ʱ
                if(setFlag == 5)if(setMin   < 59  ){setMin  = setMin    +1;}//���÷�
                if(setFlag == 6)if(setSec   < 59  ){setSec  = setSec    +1;}//������
            }
            else if(KeyIsPress(2)) //����ֵ��-
            {
                if(setFlag == 1)if(setYear  > 0  ){setYear  = setYear   -1;}//������  -1
                if(setFlag == 2)if(setMonth > 0  ){setMonth = setMonth  -1;}//������
                if(setFlag == 3)if(setDay   > 0  ){setDay   = setDay    -1;}//������
                if(setFlag == 4)if(setHour  > 0  ){setHour  = setHour   -1;}//����ʱ
                if(setFlag == 5)if(setMin   > 0  ){setMin   = setMin    -1;}//���÷�
                if(setFlag == 6)if(setSec   > 0  ){setSec   = setSec    -1;}//������
            }
            My_RTC_Set(setYear,setMonth,setDay,setHour,setMin,setSec);
            FRONT_COLOR = Color16_RED;//����Һ��ǰ��ɫ��������ɫ��
            for(i=1;i<7;i++) //��ʾ����λ��
            {
                if(i == setFlag){MyLCD_ShowSetTime(1,i,">");}
                else MyLCD_ShowSetTime(1,i," ");
            }
    }
    LCD_Clear(Color16_BLACK);//��ȫ��
    FRONT_COLOR=Color16_RED;
}







