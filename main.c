#include "my_include.h"

//**All notes can be deleted and modified**//
char dis0[50];//液晶显示暂存数组
char dis1[128];//液晶显示暂存数组

#define F_SIZE      16
#define MyLCD_Show(m,n,p)     LCD_ShowString(LCD_GetPos_X(F_SIZE,m),LCD_GetPos_Y(F_SIZE,n),p,F_SIZE,false)

float nowTemp = 0;
u16 nowPwmVal = 95;//pwm占空比调整 
u16 nowDir = 1;//当前设置方向 
float nowZs = 0;//当前速度
u16 setZsYz = 30;//设置转速
u16 setZs = 20;//设置转速
u16 plu=0;//脉冲中断计数
u16 setTemp = 35;//设置温度阈值
u8 readTemp = 0;//读取温度标志

u8 setMode =0;//设置模式
u8 updataDis =1;//有数据更新
u8 buzzerFlag =0;//蜂鸣器报警状态

u16 i ;//中间变量 i

void OnGetUartMessage(const _uart_msg_obj *uartMsgRec)
{
    char *strPtr;
    if((strPtr=strstr(uartMsgRec->payload,"RTC"))!=NULL)//接收到rtc字符串 *RTC20200420173611#
    {
        My_RTC_SetStr(strPtr+3);//设置时间
    }
    if((strPtr=strstr(uartMsgRec->payload,"MD"))!=NULL)//接收到字符串
    {
        setMode = *(strPtr+2) - 0x30;//定时模式
    }
    if((strPtr=strstr(uartMsgRec->payload,"FX"))!=NULL)//接收到字符串
    {
        if(*(strPtr+2) == '1') nowDir= 1;
        else nowDir =2;
    }
    if((strPtr=strstr(uartMsgRec->payload,"WD"))!=NULL)//接收到rtc字符串 
    {
        setTemp=ParseInteger(strPtr+2,3);//提取光照
    }
    if((strPtr=strstr(uartMsgRec->payload,"SY"))!=NULL)//接收到rtc字符串 
    {
        setZsYz=ParseInteger(strPtr+2,3);//提取光照	
    }
    if((strPtr=strstr(uartMsgRec->payload,"KS"))!=NULL)//接收到rtc字符串 
    {
        setZs=ParseInteger(strPtr+2,3);//提取光照
    }
    if(setMode == 0) //调速模式下 pwm可设
    {
        if((strPtr=strstr(uartMsgRec->payload,"PW"))!=NULL)//接收到rtc字符串 
        {
            nowPwmVal=ParseInteger(strPtr+2,3);//提取光照
        }
    }
    updataDis=1;//需要更新显示参数
}

void ctrlPwmDjFan(u8 dir,u8 pre) //设置方向急占空比
{
    if(dir == 1) //方向1
    {
        My_PWM_SetDuty(TIM1,TIM_CH_1,pre);//调节定时器 通道 占空比
        My_PWM_SetDuty(TIM3,TIM_CH_2,0);//调节定时器 通道 占空比
    }
    else
    {
        My_PWM_SetDuty(TIM1,TIM_CH_1,0);//调节定时器 通道 占空比
        My_PWM_SetDuty(TIM3,TIM_CH_2,pre);//调节定时器 通道 占空比
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
    u8 timesCount=0;//计数统计
    u8 zSupdateFla=0;//转速数据有更新
    u8 rememberMode =0xff;//记录上一次设置状态
    u16 rememberTiemSe=0xffff;//记录当前秒数据 数据没有变化不更新显示 因为lcd更新显示很浪费时间
    u8 disYplace=0; //显示所在行递增变量
    
    USARTx_Init(USART1,9600);
//	USARTx_Init(USART2,9600);
    My_RTC_Init(false);
    
    My_PWM_Init(TIM3,TIM_CH_2,1000);//初始化定时器 通道 周期1ms pwm初始化请放在按键和led的前面
    My_PWM_Init(TIM1,TIM_CH_1,1000);//初始化定时器 通道 周期1ms
    delay_ms(2);
    My_LED_Init(); //初始化led
    My_KEY_Init();	//初始化按键
    
    init_FlashBuf();//初始化flash中数据
    
    EXTIx_Init(PA0,EXTI_Trigger_Rising,2);
    
    My_LEDBlink(PA1,BEEP_ON,1,50,500);//上电硬件动作下
    ctrlPwmDjFan(1,0); //启动下电机
    
    LCD_Init();   //tft初始化
    LCD_Clear(Color16_BLACK);//清全屏
    BACK_COLOR=Color16_BLACK;
    FRONT_COLOR=Color16_LIGHTGRAY;

    keySetTime();//进入按键设置时钟 如果是有无线通信的情况下 该按键设置可以取消
    ctrlPwmDjFan(1,80); //启动下电机
    
    MyLCD_Show(2,0,"电机控制器");//显示
    FRONT_COLOR=Color16_RED;disYplace=2; //显示所在行递增变量
    MyLCD_Show(0,disYplace++,"模式: ");//显示
    FRONT_COLOR=Color16_YELLOW;
    MyLCD_Show(0,disYplace++,"实速: ");//显示
    MyLCD_Show(0,disYplace++,"PWM : ");//显示
    MyLCD_Show(0,disYplace++,"阈值: ");//显示
    MyLCD_Show(0,disYplace++,"方向: ");//显示
    MyLCD_Show(0,disYplace++,"温度: ");//显示
    
    if(My_DS18B20_Init(PA5)){printf("18B20 exist");}
    
    while(1)
    {
        scanKeyAnddealKey(); //按键扫描及处理
        if(rememberTiemSe!=calendar.second) //时间发生了变化 更新显示
        {
            rememberTiemSe = calendar.second;//记录此刻时间
            FRONT_COLOR=Color16_LIGHTGRAY;
            sprintf((char*)dis0,"%02d-%02d %02d:%02d:%02d %d ",calendar.month,calendar.day,calendar.hour,calendar.minute,calendar.second,calendar.week);//年月日周
            MyLCD_Show(0,1,dis0);//显示	
        }  
        
        if(myReadFlag_tick)//更新显示
        {
            myReadFlag_tick =false;
            
            readTemp++; //计数
            if(readTemp>2)
            {
                readTemp =0;//清空表示
                nowTemp = My_DS18B20_GetTemp(PA1);//采集温度
            }

            timesCount++;
            if(timesCount>=6)//每500ms 记录一次
            {
                timesCount = 0; //清空 重新计数
                nowZs = (float)plu /3; //处理一次 一圈1个脉冲
                if(plu != 0){
                    nowZs = (nowZs + (float)plu /3) / 2; //如果检测到脉冲 前后两次取平均值
                }
                else{
                    nowZs = 0; //检测不到 表示停止
                }
                zSupdateFla =1;//速度数据有更新
                plu = 0;//清空计数
            }
            
            FRONT_COLOR=Color16_RED;disYplace=2; //显示所在行递增变量
            if(rememberMode != setMode)
            {
                rememberMode = setMode;//记录设置模式
                if(setMode == 0)
                {
                    MyLCD_Show(6,disYplace,"调速测速 ");//显示
                    MyLCD_Show(0,disYplace+3,"阈值: ");//显示
                }
                else if(setMode == 1)
                {
                    MyLCD_Show(6,disYplace,"闭环控速 ");//显示
                    MyLCD_Show(0,disYplace+3,"控速: ");//显示
                }
            }disYplace++;
            
            FRONT_COLOR=Color16_LIGHTGRAY;
            sprintf(dis0,"%4.2fr/s ",nowZs);//打印
            MyLCD_Show(6,disYplace++,dis0);//显示
            
            switch(setMode) //分模式显示
            {
                case 0: //调速检测
                            FRONT_COLOR=Color16_LIGHTBLUE;
                            sprintf(dis0,"%d  ",nowPwmVal);//打印
                            MyLCD_Show(6,disYplace++,dis0);//显示
                        
                            if(nowZs > (float)setZsYz/10)FRONT_COLOR=Color16_RED;
                            else FRONT_COLOR=Color16_LIGHTBLUE;
                            sprintf(dis0,"%3.1f ",(float)setZsYz/10);//打印
                            MyLCD_Show(6,disYplace++,dis0);//显示
                            break;
                case 1: //控速 反馈
                            sprintf(dis0," %d  ",nowPwmVal);//打印
                            MyLCD_Show(5,disYplace++,dis0);//显示
                            
                            FRONT_COLOR=Color16_LIGHTBLUE;
                            sprintf(dis0,"%3.1f ",(float)setZs/10);//打印
                            MyLCD_Show(6,disYplace++,dis0);//显示
                
                            break;
                case 2: //定时模式
                            break;
                default: break;
            }
            FRONT_COLOR=Color16_LIGHTBLUE;
            sprintf(dis0,"%d ",nowDir);//打印
            MyLCD_Show(6,disYplace++,dis0);//显示

            sprintf(dis0,"%02d ",setTemp);//打印
            MyLCD_Show(6,disYplace,dis0);//显示
            
            if(nowTemp > setTemp)FRONT_COLOR=Color16_RED;
                else FRONT_COLOR=Color16_LIGHTGRAY;
            sprintf(dis0,"%4.1f'C ",nowTemp);//打印
            MyLCD_Show(9,disYplace++,dis0);//显示

            switch(setMode) //分模式控制
            {
                case 0: //调速检测
                            if(nowTemp > setTemp || nowZs > (float)setZsYz/10){buzzerFlag = 1;}
                            else {buzzerFlag = 0;}
                            ctrlPwmDjFan(nowDir,nowPwmVal); //控制电机方向 及采用pwm控制速度
                            break;
                case 1: //控速 反馈
                            if(nowTemp > setTemp ){buzzerFlag = 1;}
                            else {buzzerFlag = 0;}
                            if(zSupdateFla == 1)//速度数据有更新 进行处理
                            {
                                zSupdateFla = 0;//清楚标志
                                
                                nowZs = myZsPIDCalc(nowZs);//转速经过 PID运算后 再次进行反馈对比 效果更好
                                                                
                                if(nowZs > ((float)setZs/10 + 0.2 )) //对比速度
                                {
                                    if(nowPwmVal >= 5)nowPwmVal -= 5;//自动调整速度
                                }
                                else if(nowZs < ((float)setZs/10 - 0.2 ))//对比速度
                                {
                                    if(nowPwmVal < 100)nowPwmVal += 5;//自动调整速度
                                }
                            }
                            ctrlPwmDjFan(nowDir,nowPwmVal); //控制电机方向 及采用pwm控制速度
                            break;
                default: break;
            }
            if(buzzerFlag)//蜂鸣器报警处理
            {My_LEDBlink(PA4,BEEP_ON,5,100,500);}//蜂鸣器报警
            else
            {My_LEDBlink_Stop(PA4);}//停止报警
        }

        if(mySendFlag_tick)//更新显示
        {
            mySendFlag_tick =false;//清空标志

            sprintf((char*)dis1,"*D%04d%02d%02d%02d%02d%02d",calendar.year,calendar.month,calendar.day,calendar.hour,calendar.minute,calendar.second);//打印
            sprintf((char*)dis1,"M%dZ%04dY%03dP%03dT%03dt%03d",(u16)setMode,(u16)(nowZs*100),setZsYz,nowPwmVal,(u16)(nowTemp*10),setTemp);
            sprintf((char*)dis1,"K%03dF%dB%d#",setZs,nowDir,(u16)buzzerFlag);
            USARTSendString(USART1 , dis1);//发送数据
        }
        My_UartMessage_Process();//处理串口数据
    }
}


void scanKeyAnddealKey(void)
{
        static u8 setParaCount_0=0; //设置变量位置
        static u8 setParaCount_1=0; //设置变量位置
        u8 disYplace=0; //显示所在行递增变量
        My_KeyScan();
        
        if(KeyIsPress(KEY_4))
        {
            setMode++;//模式切换
            if(setMode >= 2 )setMode =0;//模式切换
        }
        switch(setMode)
        {
            case 0: //自动模式
                        setParaCount_1 = 0 ;//模式1设置 位置清空
                        if(KeyIsPress(KEY_1))
                        {
                            setParaCount_0++;//切换设置当前参数
                            if(setParaCount_0>=5)
                            {
                                setParaCount_0=0;//设置当前参数
                                write_FlashBuf();//将设置量写入flash
                            }
                        }
                        if(KeyIsPress(KEY_2))//按键按下
                        {
                            if(setParaCount_0 == 1) //当前设置pwm
                            {
                                if(nowPwmVal>=5){nowPwmVal-=5;}//设置pwm
                            }
                            else if(setParaCount_0 == 2) //当前设置 转速阈值 
                            {
                                if(setZsYz>0){setZsYz-=1;}
                            }
                            else if(setParaCount_0 == 3) //设置方向
                            {
                                if(nowDir!=1){nowDir = 1;}
                            }
                            else if(setParaCount_0 == 4) //当前设置温度阈值
                            {
                                if(setTemp>0){setTemp-=1;}
                            }//减小 人数阈值
                        }
                        if(KeyIsPress(KEY_3))//按键按下
                        {
                            if(setParaCount_0 == 1) //当前设置pwm
                            {
                                if(nowPwmVal<100){nowPwmVal+=5;}//设置pwm
                            }
                            else if(setParaCount_0 == 2) //当前设置 转速阈值 
                            {
                                if(setZsYz<999){setZsYz+=1;}
                            }
                            else if(setParaCount_0 == 3) //设置方向
                            {
                                if(nowDir!=2){nowDir = 2;}
                            }
                            else if(setParaCount_0 == 4) //当前设置温度阈值
                            {
                                if(setTemp<99){setTemp+=1;}
                            }//减小 人数阈值
                        }
                        if(HasKeyEvent()||updataDis==1)//有任何按键按下 或者需要数据更新 否则更新过于频繁浪费时间
                        {
                            if(updataDis==1)write_FlashBuf();//将设置量写入flash 可能串口数据有更新
                            updataDis =0;//需要数据更新 处理
                            FRONT_COLOR=Color16_RED;disYplace=4;
                            for(i=1;i<5;i++) //显示设置位置及设置参数 设置数组第一个数据为空白非设置参数 所以从1开始
                            {
                                if(i == setParaCount_0)
                                    {MyLCD_Show(5,disYplace++,">");}//对所在设置位置 显示>
                                //**All notes can be deleted and modified**//
                            }
                        }
                        break;
            case 1: //手动模式
                        setParaCount_0 = 0 ;//模式1设置 位置清空
                        if(KeyIsPress(KEY_1))
                        {
                            setParaCount_1++;//切换设置当前参数
                            if(setParaCount_1>=4)
                            {
                                setParaCount_1=0;//设置当前参数
                                write_FlashBuf();//将设置量写入flash
                            }
                        }
                        if(KeyIsPress(KEY_2))//按键按下
                        {
                            if(setParaCount_1 == 1) //当前设置 转速阈值 
                            {
                                if(setZs>0){setZs-=1;}
                            }
                            else if(setParaCount_1 == 2) //设置方向
                            {
                                if(nowDir!=1){nowDir = 1;}
                            }
                            else if(setParaCount_1 == 3) //当前设置温度阈值
                            {
                                if(setTemp>0){setTemp-=1;}
                            }//减小 人数阈值
                        }
                        if(KeyIsPress(KEY_3))//按键按下
                        {
                            if(setParaCount_1 == 1) //当前设置 转速阈值 
                            {
                                if(setZs<999){setZs+=1;}
                            }
                            else if(setParaCount_1 == 2) //设置方向
                            {
                                if(nowDir!=2){nowDir = 2;}
                            }
                            else if(setParaCount_1 == 3) //当前设置温度阈值
                            {
                                if(setTemp<99){setTemp+=1;}
                            }//减小 人数阈值
                        }
                        if(HasKeyEvent()||updataDis==1)//有任何按键按下 或者需要数据更新 否则更新过于频繁浪费时间
                        {
                            if(updataDis==1)write_FlashBuf();//将设置量写入flash 可能串口数据有更新
                            updataDis =0;//需要数据更新 处理
                            FRONT_COLOR=Color16_RED;disYplace=5;
                            for(i=1;i<4;i++) //显示设置位置及设置参数 设置数组第一个数据为空白非设置参数 所以从1开始
                            {
                                if(i == setParaCount_1)
                                    {MyLCD_Show(5,disYplace++,">");}//对所在设置位置 显示>
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
    double  SetPoint;           //  设定目标 Desired Value
    double  Proportion;         //  比例常数 Proportional Const
    double  Integral;           //  积分常数 Integral Const
    double  Derivative;         //  微分常数 Derivative Const
    double  LastError;          //  Error[-1]
    double  PrevError;          //  Error[-2]
    double  SumError;           //  Sums of Errors
} PID;  

/*PID计算部分*/
double PIDCalc( PID *pp, double NextPoint )
    {
        double  dError, Error;
        Error = pp->SetPoint -  NextPoint;          // 偏差 
        pp->SumError += Error;                      // 积分
        dError = pp->LastError - pp->PrevError;     // 当前微分
        pp->PrevError = pp->LastError;         
        pp->LastError = Error;  
        return (pp->Proportion * Error              // 比例项 
            +   pp->Integral * pp->SumError         // 积分项 
        +   pp->Derivative * dError             // 微分项 
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

/******************把数据保存到单片机内部eepom中******************/
void write_FlashBuf(void)
{
    setParaTab[0] = initFlag;   //修改值
    setParaTab[1]=nowDir;
    setParaTab[2]=setTemp;
    setParaTab[3]=setZsYz;
    setParaTab[4]=setZs;
    setParaTab[5]=nowPwmVal;
    My_STMFlash_Write(FLASH_SAVE_ADDR,(u16*)setParaTab,MAX_SET_SIZE);//读取数据
}

/******************把数据从单片机内部eepom中读出来*****************/
void read_FlashBuf(void)
{
    My_STMFlash_Read(FLASH_SAVE_ADDR,(u16*)setParaTab,MAX_SET_SIZE);//读取数据
    initFlag = setParaTab[0] ;   //修改值
    nowDir  = setParaTab[1];
    setTemp = setParaTab[2];
    setZsYz = setParaTab[3];
    setZs   = setParaTab[4];
    nowPwmVal = setParaTab[5];
}


void init_FlashBuf(void)
{
    read_FlashBuf(); //读取flash值
    if(initFlag != 0xaa)// 如果不是 重新写入
    {
        initFlag    = 0xaa ;   //修改值
        nowDir      = 1;
        setTemp     = 35; 
        setZsYz     = 30;
        setZs       = 20;
        nowPwmVal = 80;
//      setParaTab[6]=20;
        delay_ms(5);
        write_FlashBuf();//读取数据
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
    u8 delay=0;//延时跳出
    u8 setTimeFlag =0;//不进行设置时间
    u8 disPlace =0;
    u16 setYear,setMonth,setDay,setHour,setMin,setSec;
    u8 setFlag = 0;
    u8 i= 0;
    
    MyLCD_ShowSetTime(1,2,"Set Rtc ?");
    MyLCD_ShowSetTime(1,3,"K1:Yes K2:No");
    delay = 100; //延时选择约2s
    while(delay)
    {
        My_KeyScan();//按键扫描
        if(KeyIsPressed(0)) 
        {
            setTimeFlag = 1;//进入时间设置
            LCD_Clear(Color16_BLACK);//清除液晶屏幕
            FRONT_COLOR = Color16_WHITE;//设置液晶前景色（画笔颜色）
            MyLCD_ShowSetTime(0,0,"set RTC :");
            setYear = calendar.year;setMonth = calendar.month;setDay = calendar.day;
            setHour = calendar.hour;setMin = calendar.minute;setSec = calendar.second;
            break;
        }
        else if(KeyIsPress(1))
        {
            break;
        }
        delay--; //延时--
        delay_ms(10);
    }
    
    while(setTimeFlag)//时间设置 设置到最后一项后再点击设置跳出
    {
        disPlace =1;//显示位置依次增加
        FRONT_COLOR = Color16_WHITE;//设置液晶前景色（画笔颜色）
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

        My_KeyScan();//按键扫描
        
            if(KeyIsPress(0)) //设置值键
            {
                setFlag++;//设置标志
                if(setFlag >= 7) 
                {
                    setFlag = 0;//设置结束
                    setTimeFlag =0;//设置结束
                    LCD_Clear(Color16_BLACK);//清除液晶屏幕
                    FRONT_COLOR = Color16_WHITE;//设置液晶前景色（画笔颜色）
                    BACK_COLOR = Color16_BLACK;//设置液晶背景色（画布颜色）
                    break;
                }
            }
            else if(KeyIsPress(1)) //设置值键+
            {
                if(setFlag == 1)if(setYear  < 2999){setYear  = setYear  +1;}//设置年   +1
                if(setFlag == 2)if(setMonth < 12  ){setMonth = setMonth +1;}//设置月
                if(setFlag == 3)if(setDay   < 31  ){setDay   = setDay   +1;}//设置日
                //  不设置 周 周是自动的
                if(setFlag == 4)if(setHour  < 23  ){setHour = setHour   +1;}//设置时
                if(setFlag == 5)if(setMin   < 59  ){setMin  = setMin    +1;}//设置分
                if(setFlag == 6)if(setSec   < 59  ){setSec  = setSec    +1;}//设置秒
            }
            else if(KeyIsPress(2)) //设置值键-
            {
                if(setFlag == 1)if(setYear  > 0  ){setYear  = setYear   -1;}//设置年  -1
                if(setFlag == 2)if(setMonth > 0  ){setMonth = setMonth  -1;}//设置月
                if(setFlag == 3)if(setDay   > 0  ){setDay   = setDay    -1;}//设置日
                if(setFlag == 4)if(setHour  > 0  ){setHour  = setHour   -1;}//设置时
                if(setFlag == 5)if(setMin   > 0  ){setMin   = setMin    -1;}//设置分
                if(setFlag == 6)if(setSec   > 0  ){setSec   = setSec    -1;}//设置秒
            }
            My_RTC_Set(setYear,setMonth,setDay,setHour,setMin,setSec);
            FRONT_COLOR = Color16_RED;//设置液晶前景色（画笔颜色）
            for(i=1;i<7;i++) //显示设置位置
            {
                if(i == setFlag){MyLCD_ShowSetTime(1,i,">");}
                else MyLCD_ShowSetTime(1,i," ");
            }
    }
    LCD_Clear(Color16_BLACK);//清全屏
    FRONT_COLOR=Color16_RED;
}







