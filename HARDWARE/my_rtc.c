#include "my_rtc.h"
_calendar_obj calendar;//ʱ�ӽṹ��
static u8 count_addOneSecond = 0;

static void RTC_NVIC_Config(void)
{
    NVIC_InitTypeDef NVIC_InitStructure;
    NVIC_InitStructure.NVIC_IRQChannel = RTC_IRQn;//RTCȫ���ж�
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;//��ռ���ȼ�1λ,�����ȼ�3λ
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;//��ռ���ȼ�0λ,�����ȼ�4λ
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;//ʹ�ܸ�ͨ���ж�
    NVIC_Init(&NVIC_InitStructure);//����NVIC_InitStruct��ָ���Ĳ�����ʼ������NVIC�Ĵ���
}

//ʵʱʱ������
//��ʼ��RTCʱ��,ͬʱ���ʱ���Ƿ�������
//BKP->DR1���ڱ����Ƿ��һ�����õ�����
//����0:����
//����:�������

bool My_RTC_Init(bool alarmEnable)
{
    //����ǲ��ǵ�һ������ʱ��
    u8 temp=0;
    if (BKP_ReadBackupRegister(BKP_DR1) != 0x5050)//��ָ���ĺ󱸼Ĵ����ж�������:��������д���ָ�����ݲ����
    {
        RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR | RCC_APB1Periph_BKP, ENABLE);//ʹ��PWR��BKP����ʱ��
        PWR_BackupAccessCmd(ENABLE);//ʹ�ܺ󱸼Ĵ�������
        BKP_DeInit();//��λ��������
        RCC_LSEConfig(RCC_LSE_ON);//�����ⲿ���پ���(LSE),ʹ��������پ���
        while ((RCC_GetFlagStatus(RCC_FLAG_LSERDY) == RESET) && ++temp)
        {
            delay_ms(10);
        }//���ָ����RCC��־λ�������,�ȴ����پ������
        if(temp==0)
        {
            return false;//��ʼ��ʱ��ʧ��,����������
        }
        RCC_RTCCLKConfig(RCC_RTCCLKSource_LSE);//����RTCʱ��(RTCCLK),ѡ��LSE��ΪRTCʱ��    
        RCC_RTCCLKCmd(ENABLE);//ʹ��RTCʱ��
        RTC_WaitForLastTask();//�ȴ����һ�ζ�RTC�Ĵ�����д�������
        RTC_WaitForSynchro();//�ȴ�RTC�Ĵ���ͬ��
        RTC_ITConfig(RTC_IT_SEC,ENABLE);//ʹ��RTC���ж�
        //**All notes can be deleted and modified**//
        RTC_SetPrescaler(32767);//����RTCԤ��Ƶ��ֵ
        RTC_WaitForLastTask();//�ȴ����һ�ζ�RTC�Ĵ�����д�������
        RTC_ExitConfigMode();//�˳�����ģʽ
        BKP_WriteBackupRegister(BKP_DR1, 0X5050);//��ָ���ĺ󱸼Ĵ�����д���û���������
    }
    else//ϵͳ������ʱ
    {
        RTC_WaitForSynchro();//�ȴ����һ�ζ�RTC�Ĵ�����д�������
        RTC_ITConfig(RTC_IT_SEC, ENABLE);//ʹ��RTC���ж�
        RTC_ITConfig(RTC_IT_ALR, alarmEnable==true?ENABLE:DISABLE);//ʹ��RTC�����ж�
        RTC_WaitForLastTask();//�ȴ����һ�ζ�RTC�Ĵ�����д�������
    }
    RTC_NVIC_Config();//RCT�жϷ�������
    My_RTC_Update();//����ʱ��
    return true; //ok
}
//RTCʱ���ж�
//ÿ�봥��һ��  
//extern u16 tcnt; 
void RTC_IRQHandler(void)
{
    if (RTC_GetITStatus(RTC_IT_SEC) != RESET)//�����ж�
    {
        My_RTC_Update();//����ʱ��
        //**All notes can be deleted and modified**//
    }
    if(RTC_GetITStatus(RTC_IT_ALR)!= RESET)//�����ж�
    {
        RTC_ClearITPendingBit(RTC_IT_ALR);//�������ж�
    }
    RTC_ClearITPendingBit(RTC_IT_SEC|RTC_IT_OW);//�������ж�
    RTC_WaitForLastTask();
}
//�ж��Ƿ������꺯��
//�·�   1  2  3  4  5  6  7  8  9  10 11 12
//����   31 29 31 30 31 30 31 31 30 31 30 31
//������ 31 28 31 30 31 30 31 31 30 31 30 31
//����:���
//���:������ǲ�������.1,��.0,����
bool Is_Leap_Year(u16 year)
{
    if(year%4==0) //�����ܱ�4����
    { 
        if(year%100==0) 
        { 
            if(year%400==0)
            {
                return true;//�����00��β,��Ҫ�ܱ�400����
            }
            return false; 
        }
        return true;
    }
    return false;
}
//����ʱ��
//�������ʱ��ת��Ϊ����
//��1970��1��1��Ϊ��׼
//1970~2099��Ϊ�Ϸ����
//����ֵ:0,�ɹ�;����:�������.
//�·����ݱ�
u8 const table_week[12]={0,3,3,6,1,4,6,2,5,0,3,5}; //���������ݱ�
//ƽ����·����ڱ�
const u8 mon_table[12]={31,28,31,30,31,30,31,31,30,31,30,31};
/*************************************************************************
�������ṹ������ת��Ϊ32λ���������㱣��
**************************************************************************/
u32 My_RTC_CalendarToInteger(const _calendar_obj *cal)
{
    u16 t;
    u32 secCount=0;
    if(cal->year<1970||cal->year>2099)
    {
        return false;
    }
    for(t=1970;t<cal->year;t++)//��������ݵ��������
    {
        if(Is_Leap_Year(t))
        {
            secCount+=31622400;//�����������
        }
        else 
        {
            secCount+=31536000;//ƽ���������
        }
    }
    for(t=0;t<cal->month-1;t++)//��ǰ���·ݵ����������
    {
        secCount+=(u32)mon_table[t]*86400;//�·����������
        //**All notes can be deleted and modified**//
    }
    secCount+=(u32)(cal->day-1)*86400;//��ǰ�����ڵ����������
    secCount+=(u32)cal->hour*3600;//Сʱ������
    secCount+=(u32)cal->minute*60;//����������
    secCount+=cal->second;//�������Ӽ���ȥ
    return secCount;
}
/***************************************************************
���̶���ʽ���ַ���ת��Ϊ�����ṹ�壬��20200420173611Ϊ2020��4��20��17ʱ36��11��
***************************************************************/
void My_RTC_StringToCalendar(const char *str,_calendar_obj *cal)
{
    if(strlen(str)<14)
    {
        return;
    }
    cal->year = (str[0]-0x30)*1000 + (str[1]-0x30)*100 + (str[2]-0x30)*10 + (str[3]-0x30);//�������
    cal->month = (str[4]-0x30)*10 + (str[5]-0x30);//�����·�
    cal->day = (str[6]-0x30)*10 + (str[7]-0x30);//������
    cal->hour = (str[8]-0x30)*10 + (str[9]-0x30);//����Сʱ
    cal->minute = (str[10]-0x30)*10 + (str[11]-0x30);//�������
    cal->second = (str[12]-0x30)*10 + (str[13]-0x30);//������
}
/***************************************************************
���̶���ʽ���ַ���ת��Ϊ�������㱣�棬��20200420173611Ϊ2020��4��20��17ʱ36��11��
***************************************************************/
u32 My_RTC_StringToInteger(const char *str)
{
    _calendar_obj cal;
    My_RTC_StringToCalendar(str,&cal);
    return My_RTC_CalendarToInteger(&cal);//
}
/***************************************************************
��RTC�����Ӽ���ֵת��Ϊ�����ṹ��
***************************************************************/
void My_RTC_IntegerToCalendar(u32 secondCount,_calendar_obj *cal)
{
    u32 temp=0;
    u16 temp1=1970;
    temp=secondCount/86400;//�õ�����(��������Ӧ��)

    while(temp>=365)
    {
        if(Is_Leap_Year(temp1))//������
        {
            if(temp>=366)
            {
                temp-=366;//�����������
            }
            else 
            {
                break;
            }
        }
        else 
        {
            temp-=365;//ƽ�� 
        }
        temp1++;  
    } 
    cal->year=temp1;//�õ����
    temp1=0;
    while(temp>=28)//������һ����
    {
        if(Is_Leap_Year(cal->year)&&temp1==1)//�����ǲ�������/2�·�
        {
            if(temp>=29)
            {
                temp-=29;//�����������
            }
            else 
            {
                break; 
            }
        }
        else 
        {
            if(temp>=mon_table[temp1])
            {
                temp-=mon_table[temp1];//ƽ��
            }
            else 
            {
                break;
            }
        }
        temp1++;
    }
    cal->month=temp1+1;//�õ��·�
    cal->day=temp+1;//�õ����� 

    temp=secondCount%86400;//�õ�������
    cal->hour=temp/3600;//Сʱ
    //**All notes can be deleted and modified**//
    cal->week=My_RTC_GetWeekday(cal->year,cal->month,cal->day);//��ȡ����
}
/***************************************************************
����RTCʱ��
***************************************************************/
bool My_RTC_Set(u16 syear,u8 smon,u8 sday,u8 hour,u8 min,u8 sec)
{
    u32 secCount=0;
    _calendar_obj cal;
    cal.year = syear;
    cal.month = smon;
    cal.day = sday;
    cal.hour = hour;
    cal.minute = min;
    cal.second = sec;
    secCount = My_RTC_CalendarToInteger(&cal);
    My_RTC_SetCounter(secCount);
    return true;
}
void My_RTC_AddYear(int8 yearCount)
{
    u32 secCount=0;
    if(yearCount==0)
    {
        return;
    }
    calendar.year+=yearCount;
    secCount = My_RTC_CalendarToInteger(&calendar);
    My_RTC_SetCounter(secCount);
}
void My_RTC_AddMonth(int16 monthCount)
{
    u32 secCount=0;
    u32 monCount = (calendar.year<<3) + (calendar.year<<2) + calendar.month - 1;
    if(monthCount==0)
    {
        return;
    }
    monCount += monthCount;
    //**All notes can be deleted and modified**//
    secCount = My_RTC_CalendarToInteger(&calendar);
    My_RTC_SetCounter(secCount);
}
void My_RTC_AddDay(int16 dayCount)
{
    My_RTC_SetCounter(RTC_GetCounter()+dayCount*86400);//24*60*60=86400
}
void My_RTC_AddHour(int16 hourCount)
{
    My_RTC_SetCounter(RTC_GetCounter()+hourCount*3600);//60*60=3600
}
void My_RTC_AddMinute(int16 minuteCount)
{
    My_RTC_SetCounter(RTC_GetCounter()+minuteCount*60);//
}
void My_RTC_AddSecond(int16 secondCount)
{
    My_RTC_SetCounter(RTC_GetCounter()+secondCount);//
}
/***************************************************************
//ʹ��������ʱ������ַ�������ʱ�䣬��20200420173611Ϊ2020��4��20��17ʱ36��11��
***************************************************************/
bool My_RTC_SetStr(const char *str)
{
    _calendar_obj cal;
    My_RTC_StringToCalendar(str,&cal);
    return My_RTC_Set(cal.year,cal.month,cal.day,cal.hour,cal.minute,cal.second);//����ʵʱʱ��ʱ��
}
void My_RTC_SetCounter(u32 value)
{
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR | RCC_APB1Periph_BKP, ENABLE);//ʹ��PWR��BKP����ʱ��  
    PWR_BackupAccessCmd(ENABLE);//ʹ��RTC�ͺ󱸼Ĵ������� 
    if(count_addOneSecond==0)
    {
        count_addOneSecond=10;
        value += 1;
    }
    //**All notes can be deleted and modified**//
    RTC_SetCounter(value);//����RTC��������ֵ
    
    RTC_WaitForLastTask();//�ȴ����һ�ζ�RTC�Ĵ�����д�������
    My_RTC_Update();
}
//�õ���ǰ��ʱ��
//����ֵ:0,�ɹ�;����:�������.
void My_RTC_Update(void)
{
    My_RTC_IntegerToCalendar(RTC_GetCounter(),&calendar);//��ȡ����
}
//������������ڼ�
//��������:���빫�����ڵõ�����(ֻ����1901-2099��)
//������������������� 
//����ֵ�����ں�
u8 My_RTC_GetWeekday(u16 year,u8 month,u8 day)
{
    u16 temp2;
    u8 yearH,yearL;

    yearH=year/100;
    yearL=year%100;
    // ���Ϊ21����,�������100
    if (yearH>19)
    {
        yearL+=100;
    }
    // ����������ֻ��1900��֮���
    temp2=yearL+yearL/4;
    temp2=temp2%7; 
    temp2=temp2+day+table_week[month-1];
    if (yearL%4==0&&month<3)
    {
        temp2--;
    }
    return(temp2%7);
}
//��ʼ������
//��1970��1��1��Ϊ��׼
//1970~2099��Ϊ�Ϸ����
//syear,smon,sday,hour,min,sec�����ӵ�������ʱ����   
//����ֵ:0,�ɹ�;����:�������.
u8 RTC_Alarm_Set(u16 syear,u8 smon,u8 sday,u8 hour,u8 min,u8 sec)
{
    u16 t;
    u32 seccount=0;
    if(syear<1970||syear>2099)return 1;
    for(t=1970;t<syear;t++)//��������ݵ��������
    {
        if(Is_Leap_Year(t))seccount+=31622400;//�����������
        else seccount+=31536000;//ƽ���������
    }
    smon-=1;
    for(t=0;t<smon;t++)//��ǰ���·ݵ����������
    {
        seccount+=(u32)mon_table[t]*86400;//�·����������
        if(Is_Leap_Year(syear)&&t==1)seccount+=86400;//����2�·�����һ���������
    }
    seccount+=(u32)(sday-1)*86400;//��ǰ�����ڵ����������
    seccount+=(u32)hour*3600;//Сʱ������
    seccount+=(u32)min*60;//����������
    seccount+=sec;//�������Ӽ���ȥ
    //**All notes can be deleted and modified**//
    PWR->CR|=1<<8;    //ȡ��������д����
    //���������Ǳ����!
    RTC->CRL|=1<<4;   //�������� 
    RTC->ALRL=seccount&0xffff;
    RTC->ALRH=seccount>>16;
    RTC->CRL&=~(1<<4);//���ø���
    while(!(RTC->CRL&(1<<5)));//�ȴ�RTC�Ĵ����������  
    return 0;
}
u16 My_RTC_GetMilliSecond(void)
{
    return (32767-RTC_GetDivider())*1000/32767;
}
