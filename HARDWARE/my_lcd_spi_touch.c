#include "my_lcd_spi_touch.h"
_m_tp_dev tp_dev=
{
    TP_Init,
    TP_Scan,
    TP_Adjust,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0
};
//Ĭ��Ϊtouchtype=0������.
u8 CMD_RDX=0XD0;
u8 CMD_RDY=0X90;

/*****************************************************************************
 * @name       :void TP_Write_Byte(u8 num)   
 * @function   :Write a byte data to the touch screen IC with SPI bus
 * @parameters :num:Data to be written
 * @retvalue   :None
******************************************************************************/
void TP_Write_Byte(u8 num)
{
    u8 count=0;   
    for(count=0;count<8;count++)
    {
        if(num&0x80)
        {
            LCD_MOSI = 1;//TDIN=1;
        }
        else
        {
            LCD_MOSI = 0;//TDIN=0;
        }
        num<<=1;
        TOUCH_CLK = 0;//TCLK=0;LCD_SCLK_SET
        TOUCH_CLK = 1;//TCLK=1;//��������Ч
    }
}

/*****************************************************************************
 * @name       :u16 TP_Read_AD(u8 CMD)	  
 * @function   :Reading adc values from touch screen IC with SPI bus
 * @parameters :CMD:Read command,0xD0 for x,0x90 for y
 * @retvalue   :Read data
******************************************************************************/    
u16 TP_Read_AD(u8 cmd)
{
    u8 count=0;
    u16 Num=0; 
    TOUCH_CLK = 0;//TCLK=0;//������ʱ��
    LCD_MOSI = 0;//TDIN=0;//����������

    LCD_CS1 = 1;

    TOUCH_CS2 = 0;//TCS=0;//ѡ�д�����IC
    TP_Write_Byte(cmd);//����������
    delay_us(6);//ADS7846��ת��ʱ���Ϊ6us
    TOUCH_CLK = 0;//TCLK=0;
    delay_us(1);
    TOUCH_CLK = 1;//TCLK=1;//��1��ʱ�ӣ����BUSY
    TOUCH_CLK = 0;//TCLK=0;
    for(count=0;count<16;count++)//����16λ����,ֻ�и�12λ��Ч 
    {
        Num<<=1;
        TOUCH_CLK = 0;//TCLK=0;//�½�����Ч
        TOUCH_CLK = 1;//TCLK=1;
        if(LCD_DOUT_READ!=0)Num++;
    }
    Num>>=4;//ֻ�и�12λ��Ч.
    TOUCH_CS2 = 1;//TCS=1;//�ͷ�Ƭѡ
    return(Num);  
//#endif
}

#define READ_TIMES 5 	//��ȡ����
#define LOST_VAL 1	  	//����ֵ
/*****************************************************************************
 * @name       :u16 TP_Read_XOY(u8 xy)  
 * @function   :Read the touch screen coordinates (x or y),
                Read the READ_TIMES secondary data in succession 
                and sort the data in ascending order,
                Then remove the lowest and highest number of LOST_VAL 
                and take the average
 * @parameters :xy:Read command(CMD_RDX/CMD_RDY)
 * @retvalue   :Read data
******************************************************************************/  
u16 TP_Read_XOY(u8 xy)
{
    u16 i, j;
    u16 buf[READ_TIMES];
    u16 sum=0;
    u16 temp;
    for(i=0;i<READ_TIMES;i++)buf[i]=TP_Read_AD(xy);	
    for(i=0;i<READ_TIMES-1; i++)//����
    {
        for(j=i+1;j<READ_TIMES;j++)
        {
            if(buf[i]>buf[j])//��������
            {
                temp=buf[i];
                buf[i]=buf[j];
                buf[j]=temp;
            }
        }
    }
    sum=0;
    for(i=LOST_VAL;i<READ_TIMES-LOST_VAL;i++)sum+=buf[i];
    temp=sum/(READ_TIMES-2*LOST_VAL);
    return temp;
} 

/*****************************************************************************
 * @name       :u8 TP_Read_XY(u16 *x,u16 *y)
 * @function   :Read touch screen x and y coordinates,
                The minimum value can not be less than 100
 * @parameters :x:Read x coordinate of the touch screen
                y:Read y coordinate of the touch screen
 * @retvalue   :0-fail,1-success
******************************************************************************/ 
u8 TP_Read_XY(u16 *x,u16 *y)
{
    u16 xtemp,ytemp;
    xtemp=TP_Read_XOY(CMD_RDX);
    ytemp=TP_Read_XOY(CMD_RDY);
    //if(xtemp<100||ytemp<100)return 0;//����ʧ��
    *x=xtemp;
    *y=ytemp;
    return 1;//�����ɹ�
}

#define ERR_RANGE 50 //��Χ 
/*****************************************************************************
 * @name       :u8 TP_Read_XY2(u16 *x,u16 *y) 
 * @function   :Read the touch screen coordinates twice in a row, 
                and the deviation of these two times can not exceed ERR_RANGE, 
                satisfy the condition, then think the reading is correct, 
                otherwise the reading is wrong.
                This function can greatly improve the accuracy.
 * @parameters :x:Read x coordinate of the touch screen
                y:Read y coordinate of the touch screen
 * @retvalue   :0-fail,1-success
******************************************************************************/ 
u8 TP_Read_XY2(u16 *x,u16 *y) 
{
    u16 x1,y1;
    u16 x2,y2;
    u8 flag;    
    flag=TP_Read_XY(&x1,&y1);
    if(flag==0)return(0);
    flag=TP_Read_XY(&x2,&y2);
    if(flag==0)return(0);
    if(((x2<=x1&&x1<x2+ERR_RANGE)||(x1<=x2&&x2<x1+ERR_RANGE))//ǰ�����β�����+-50��
    &&((y2<=y1&&y1<y2+ERR_RANGE)||(y1<=y2&&y2<y1+ERR_RANGE)))
    {
        *x=(x1+x2)/2;
        *y=(y1+y2)/2;
        return 1;
    }
    return 0;
} 

/*****************************************************************************
 * @name       :void TP_Drow_Touch_Point(u16 x,u16 y,u16 color)
 * @function   :Draw a touch point,Used to calibrate
 * @parameters :x:Read x coordinate of the touch screen
                y:Read y coordinate of the touch screen
                color:the color value of the touch point
 * @retvalue   :None
******************************************************************************/  
void TP_Drow_Touch_Point(u16 x,u16 y,Color16 color)
{
    FRONT_COLOR=color;
    LCD_DrawLine(x-12,y,x+13,y);//����
    LCD_DrawLine(x,y-12,x,y+13);//����
    LCD_DrawPoint(x+1,y+1);
    LCD_DrawPoint(x-1,y+1);
    LCD_DrawPoint(x+1,y-1);
    LCD_DrawPoint(x-1,y-1);
    LCD_DrawCircle(x,y,6,0);//������Ȧ
}

/*****************************************************************************
 * @name       :void TP_Draw_Big_Point(u16 x,u16 y,u16 color)
 * @function   :Draw a big point(2*2)
 * @parameters :x:Read x coordinate of the point
                y:Read y coordinate of the point
                color:the color value of the point
 * @retvalue   :None
******************************************************************************/   
void TP_Draw_Big_Point(u16 x,u16 y,Color16 color)
{
    FRONT_COLOR=color;
    LCD_DrawPoint(x,y);//���ĵ� 
    LCD_DrawPoint(x+1,y);
    LCD_DrawPoint(x,y+1);
    LCD_DrawPoint(x+1,y+1);
}

/*****************************************************************************
 * @name       :u8 TP_Scan(u8 tp)
 * @function   :Scanning touch event
 * @parameters :tp:0-screen coordinate 
                    1-Physical coordinates(For special occasions such as calibration)
 * @retvalue   :Current touch screen status,
                0-no touch
                1-touch
******************************************************************************/
u8 TP_Scan(u8 tp)
{
    if(LCD_PEN_READ==0)//�а�������
    {
        if(tp)TP_Read_XY2(&tp_dev.x,&tp_dev.y);//��ȡ��������
        else if(TP_Read_XY2(&tp_dev.x,&tp_dev.y))//��ȡ��Ļ����
        {
            tp_dev.x=tp_dev.xfac*tp_dev.x+tp_dev.xoff;//�����ת��Ϊ��Ļ����
            tp_dev.y=tp_dev.yfac*tp_dev.y+tp_dev.yoff;
        } 
        if((tp_dev.sta&TP_PRES_DOWN)==0)//֮ǰû�б�����
        {
            tp_dev.sta=TP_PRES_DOWN|TP_CATH_PRES;//��������  
            tp_dev.x0=tp_dev.x;//��¼��һ�ΰ���ʱ������
            tp_dev.y0=tp_dev.y;
        }
    }else
    {
        if(tp_dev.sta&TP_PRES_DOWN)//֮ǰ�Ǳ����µ�
        {
            tp_dev.sta&=~(1<<7);//��ǰ����ɿ�
        }else//֮ǰ��û�б�����
        {
            tp_dev.x0=0;
            tp_dev.y0=0;
            tp_dev.x=0xffff;
            tp_dev.y=0xffff;
        }
    }
    return tp_dev.sta&TP_PRES_DOWN;//���ص�ǰ�Ĵ���״̬
}

//////////////////////////////////////////////////////////////////////////	 
//������EEPROM����ĵ�ַ�����ַ,ռ��13���ֽ�(RANGE:SAVE_ADDR_BASE~SAVE_ADDR_BASE+12)
#define SAVE_ADDR_BASE 40
/*****************************************************************************
 * @name       :void TP_Save_Adjdata(void)
 * @function   :Save calibration parameters
 * @parameters :None
 * @retvalue   :None
******************************************************************************/
void TP_Save_Adjdata(void)
{
//	s32 temp;
//	//����У�����!
//	temp=tp_dev.xfac*100000000;//����xУ������      
////    AT24CXX_WriteLenByte(SAVE_ADDR_BASE,temp,4);   
//	temp=tp_dev.yfac*100000000;//����yУ������    
//    AT24CXX_WriteLenByte(SAVE_ADDR_BASE+4,temp,4);
//	//����xƫ����
//    AT24CXX_WriteLenByte(SAVE_ADDR_BASE+8,tp_dev.xoff,2);
//	//����yƫ����
//	AT24CXX_WriteLenByte(SAVE_ADDR_BASE+10,tp_dev.yoff,2);
//	//���津������
//	AT24CXX_WriteOneByte(SAVE_ADDR_BASE+12,tp_dev.touchtype);
//	temp=0X0A;//���У׼����
//	AT24CXX_WriteOneByte(SAVE_ADDR_BASE+13,temp); 
}

/*****************************************************************************
 * @name       :u8 TP_Get_Adjdata(void)
 * @function   :Gets the calibration values stored in the EEPROM
 * @parameters :None
 * @retvalue   :1-get the calibration values successfully
                0-get the calibration values unsuccessfully and Need to recalibrate
******************************************************************************/
//u8 TP_Get_Adjdata(void)
//{
//	s32 tempfac;
//	tempfac=AT24CXX_ReadOneByte(SAVE_ADDR_BASE+13);//��ȡ�����,���Ƿ�У׼����
//	if(tempfac==0X0A)//�������Ѿ�У׼����
//	{
//		tempfac=AT24CXX_ReadLenByte(SAVE_ADDR_BASE,4);
//		tp_dev.xfac=(float)tempfac/100000000;//�õ�xУ׼����
//		tempfac=AT24CXX_ReadLenByte(SAVE_ADDR_BASE+4,4);
//		tp_dev.yfac=(float)tempfac/100000000;//�õ�yУ׼����
//	    //�õ�xƫ����
//		tp_dev.xoff=AT24CXX_ReadLenByte(SAVE_ADDR_BASE+8,2);
// 	    //�õ�yƫ����
//		tp_dev.yoff=AT24CXX_ReadLenByte(SAVE_ADDR_BASE+10,2);
// 		tp_dev.touchtype=AT24CXX_ReadOneByte(SAVE_ADDR_BASE+12);//��ȡ�������ͱ��
//		if(tp_dev.touchtype)//X,Y��������Ļ�෴
//		{
//			CMD_RDX=0X90;
//			CMD_RDY=0XD0;
//		}else//X,Y��������Ļ��ͬ
//		{
//			CMD_RDX=0XD0;
//			CMD_RDY=0X90;
//		}
//		return 1;
//	}
//	return 0;
//}	
 
//��ʾ�ַ���

/*****************************************************************************
 * @name       :void TP_Adj_Info_Show(u16 x0,u16 y0,u16 x1,u16 y1,u16 x2,u16 y2,u16 x3,u16 y3,u16 fac)
 * @function   :Display calibration results	
 * @parameters :x0:the x coordinates of first calibration point
                y0:the y coordinates of first calibration point
                x1:the x coordinates of second calibration point
                y1:the y coordinates of second calibration point
                x2:the x coordinates of third calibration point
                y2:the y coordinates of third calibration point
                x3:the x coordinates of fourth calibration point
                y3:the y coordinates of fourth calibration point
                fac:calibration factor 
 * @retvalue   :None
******************************************************************************/ 	 					  
void TP_Adj_Info_Show(u16 x0,u16 y0,u16 x1,u16 y1,u16 x2,u16 y2,u16 x3,u16 y3,u16 fac)
{
    FRONT_COLOR=Color16_RED;
    LCD_JustString(40,140,"x1:",16);
    LCD_JustString(40+80,140,"y1:",16);
    LCD_JustString(40,160,"x2:",16);
    LCD_JustString(40+80, 16,"y2:",160);
    LCD_JustString(40,180, "x3:",16);
    LCD_JustString(40+80,180, "y3:",16);
    LCD_JustString(40,200, "x4:",16);
    LCD_JustString(40+80,200, "y4:",16);  
    LCD_JustString(40,220, "fac is:",16);     
    LCD_ShowNum(40+24,140,x0,4,16);//��ʾ��ֵ
    LCD_ShowNum(40+24+80,140,y0,4,16);//��ʾ��ֵ
    LCD_ShowNum(40+24,160,x1,4,16);//��ʾ��ֵ
    LCD_ShowNum(40+24+80,160,y1,4,16);//��ʾ��ֵ
    LCD_ShowNum(40+24,180,x2,4,16);//��ʾ��ֵ
    LCD_ShowNum(40+24+80,180,y2,4,16);//��ʾ��ֵ
    LCD_ShowNum(40+24,200,x3,4,16);//��ʾ��ֵ
    LCD_ShowNum(40+24+80,200,y3,4,16);//��ʾ��ֵ
    LCD_ShowNum(40+56,220,fac,3,16);//��ʾ��ֵ,����ֵ������95~105��Χ֮��.
}

/*****************************************************************************
 * @name       :u8 TP_Get_Adjdata(void)
 * @function   :Calibration touch screen and Get 4 calibration parameters
 * @parameters :None
 * @retvalue   :None

******************************************************************************/
void TP_Adjust(void)
{
    u16 pos_temp[4][2];//���껺��ֵ
    u8  cnt=0;	
    u16 d1,d2;
    u32 tem1,tem2;
    float fac; 	
    u16 outtime=0;
    cnt=0;
    FRONT_COLOR=Color16_BLUE;
    BACK_COLOR =Color16_WHITE;
    LCD_Clear(Color16_WHITE);//����   
    FRONT_COLOR=Color16_RED;//��ɫ 
    LCD_Clear(Color16_WHITE);//����
    FRONT_COLOR=Color16_BLACK;
    LCD_JustString(10,40,"Please use the stylus click",16);//��ʾ��ʾ��Ϣ
    LCD_JustString(10,56,"the cross on the screen.",16);//��ʾ��ʾ��Ϣ
    LCD_JustString(10,72,"The cross will always move",16);//��ʾ��ʾ��Ϣ
    LCD_JustString(10,88,"until the screen adjustment",16);//��ʾ��ʾ��Ϣ
    LCD_JustString(10,104,"is completed.",16);//��ʾ��ʾ��Ϣ
     
    TP_Drow_Touch_Point(20,20,Color16_RED);//����1 
    tp_dev.sta=0;//���������ź� 
    tp_dev.xfac=0;//xfac��������Ƿ�У׼��,����У׼֮ǰ�������!�������
    while(1)//�������10����û�а���,���Զ��˳�
    {
        tp_dev.scan(1);//ɨ����������
        if((tp_dev.sta&0xc0)==TP_CATH_PRES)//����������һ��(��ʱ�����ɿ���.)
        {
            outtime=0;
            tp_dev.sta&=~(1<<6);//��ǰ����Ѿ����������.

            pos_temp[cnt][0]=tp_dev.x;
            pos_temp[cnt][1]=tp_dev.y;
            cnt++;
            switch(cnt)
            {
                case 1:
                    TP_Drow_Touch_Point(20,20,Color16_WHITE);//�����1 
                    TP_Drow_Touch_Point(lcddev.width-20,20,Color16_RED);//����2
                    break;
                case 2:
                    TP_Drow_Touch_Point(lcddev.width-20,20,Color16_WHITE);//�����2
                    TP_Drow_Touch_Point(20,lcddev.height-20,Color16_RED);//����3
                    break;
                case 3:
                    TP_Drow_Touch_Point(20,lcddev.height-20,Color16_WHITE);//�����3
                    TP_Drow_Touch_Point(lcddev.width-20,lcddev.height-20,Color16_RED);//����4
                    break;
                case 4:	 //ȫ���ĸ����Ѿ��õ�
                    //�Ա����
                    tem1=abs(pos_temp[0][0]-pos_temp[1][0]);//x1-x2
                    tem2=abs(pos_temp[0][1]-pos_temp[1][1]);//y1-y2
                    tem1*=tem1;
                    tem2*=tem2;
                    d1=sqrt(tem1+tem2);//�õ�1,2�ľ���
                    
                    tem1=abs(pos_temp[2][0]-pos_temp[3][0]);//x3-x4
                    tem2=abs(pos_temp[2][1]-pos_temp[3][1]);//y3-y4
                    tem1*=tem1;
                    tem2*=tem2;
                    d2=sqrt(tem1+tem2);//�õ�3,4�ľ���
                    fac=(float)d1/d2;
                    if(fac<0.95||fac>1.05||d1==0||d2==0)//���ϸ�
                    {
                        cnt=0;
                        TP_Drow_Touch_Point(lcddev.width-20,lcddev.height-20,Color16_WHITE);//�����4
                        TP_Drow_Touch_Point(20,20,Color16_RED);//����1
                        TP_Adj_Info_Show(pos_temp[0][0],pos_temp[0][1],pos_temp[1][0],pos_temp[1][1],pos_temp[2][0],pos_temp[2][1],pos_temp[3][0],pos_temp[3][1],fac*100);//��ʾ����   
                        continue;
                    }
                    tem1=abs(pos_temp[0][0]-pos_temp[2][0]);//x1-x3
                    tem2=abs(pos_temp[0][1]-pos_temp[2][1]);//y1-y3
                    tem1*=tem1;
                    tem2*=tem2;
                    d1=sqrt(tem1+tem2);//�õ�1,3�ľ���
                    
                    tem1=abs(pos_temp[1][0]-pos_temp[3][0]);//x2-x4
                    tem2=abs(pos_temp[1][1]-pos_temp[3][1]);//y2-y4
                    tem1*=tem1;
                    tem2*=tem2;
                    d2=sqrt(tem1+tem2);//�õ�2,4�ľ���
                    fac=(float)d1/d2;
                    if(fac<0.95||fac>1.05)//���ϸ�
                    {
                        cnt=0;
                        TP_Drow_Touch_Point(lcddev.width-20,lcddev.height-20,Color16_WHITE);//�����4
                        TP_Drow_Touch_Point(20,20,Color16_RED);//����1
                        TP_Adj_Info_Show(pos_temp[0][0],pos_temp[0][1],pos_temp[1][0],pos_temp[1][1],pos_temp[2][0],pos_temp[2][1],pos_temp[3][0],pos_temp[3][1],fac*100);//��ʾ����   
                        continue;
                    }//��ȷ��

                    //�Խ������
                    tem1=abs(pos_temp[1][0]-pos_temp[2][0]);//x1-x3
                    tem2=abs(pos_temp[1][1]-pos_temp[2][1]);//y1-y3
                    tem1*=tem1;
                    tem2*=tem2;
                    d1=sqrt(tem1+tem2);//�õ�1,4�ľ���

                    tem1=abs(pos_temp[0][0]-pos_temp[3][0]);//x2-x4
                    tem2=abs(pos_temp[0][1]-pos_temp[3][1]);//y2-y4
                    tem1*=tem1;
                    tem2*=tem2;
                    d2=sqrt(tem1+tem2);//�õ�2,3�ľ���
                    fac=(float)d1/d2;
                    if(fac<0.95||fac>1.05)//���ϸ�
                    {
                        cnt=0;
                        TP_Drow_Touch_Point(lcddev.width-20,lcddev.height-20,Color16_WHITE);//�����4
                        TP_Drow_Touch_Point(20,20,Color16_RED);//����1
                        TP_Adj_Info_Show(pos_temp[0][0],pos_temp[0][1],pos_temp[1][0],pos_temp[1][1],pos_temp[2][0],pos_temp[2][1],pos_temp[3][0],pos_temp[3][1],fac*100);//��ʾ����   
                        continue;
                    }//��ȷ��
                    //������
                    tp_dev.xfac=(float)(lcddev.width-40)/(pos_temp[1][0]-pos_temp[0][0]);//�õ�xfac
                    tp_dev.xoff=(lcddev.width-tp_dev.xfac*(pos_temp[1][0]+pos_temp[0][0]))/2;//�õ�xoff
                          
                    tp_dev.yfac=(float)(lcddev.height-40)/(pos_temp[2][1]-pos_temp[0][1]);//�õ�yfac
                    tp_dev.yoff=(lcddev.height-tp_dev.yfac*(pos_temp[2][1]+pos_temp[0][1]))/2;//�õ�yoff  
                    if(fabs(tp_dev.xfac)>2||fabs(tp_dev.yfac)>2)//������Ԥ����෴��.
                    {
                        cnt=0;
                        TP_Drow_Touch_Point(lcddev.width-20,lcddev.height-20,Color16_WHITE);//�����4
                        TP_Drow_Touch_Point(20,20,Color16_RED);//����1
                        LCD_JustString(40,26, "TP Need readjust!",16);
                        tp_dev.touchtype=!tp_dev.touchtype;//�޸Ĵ�������.
                        if(tp_dev.touchtype)//X,Y��������Ļ�෴
                        {
                            CMD_RDX=0X90;
                            CMD_RDY=0XD0;
                        }else//X,Y��������Ļ��ͬ
                        {
                            CMD_RDX=0XD0;
                            CMD_RDY=0X90;
                        }
                        continue;
                    }
                    FRONT_COLOR=Color16_BLUE;
                    LCD_Clear(Color16_WHITE);//����
                    LCD_JustString(35,110, "Touch Screen Adjust OK!",16);//У�����
                    delay_ms(1000);
                    TP_Save_Adjdata();  
                    LCD_Clear(Color16_WHITE);//����   
                    return;//У�����
            }
        }
        delay_ms(10);
        outtime++;
        if(outtime>1000)
        {
//            TP_Get_Adjdata();
            break;
        }
    }
}

void Touch_GPIOInit(void)
{
#if !defined (USE_HAL_DRIVER)
    GPIO_Pin_Init(Pins_LCD_Touch[0],GPIO_Mode_IPU);
    GPIO_Pin_Init(Pins_LCD_Touch[1],GPIO_Mode_Out_PP);
    GPIO_Pin_Init(Pins_LCD_Touch[2],GPIO_Mode_IPD);
#else
    GPIO_Pin_Init(Pins_LCD_Touch[0],GPIO_MODE_INPUT,GPIO_PULLUP);
    GPIO_Pin_Init(Pins_LCD_Touch[1],GPIO_MODE_OUTPUT_PP,GPIO_PULLUP);
    GPIO_Pin_Init(Pins_LCD_Touch[2],GPIO_MODE_INPUT,GPIO_PULLDOWN);
#endif
}
/*****************************************************************************
 * @name       :u8 TP_Init(void)
 * @function   :Initialization touch screen
 * @parameters :None
 * @retvalue   :0-no calibration
                1-Has been calibrated
******************************************************************************/  
u8 TP_Init(void)
{
    /************
    //����У׼��� �������ֶ�У׼ ֱ�ӽ�֮ǰУ׼�����ݽ�����и�ֵ
    LCD_direction(0);//���������� ��Ӧ������ ����һ��У׼�����ͬ ���������������������л�ȡ
    ******************/
    tp_dev.xfac=0.0657246113;//�õ�xfac
    tp_dev.xoff=0xFFF4;//�õ�xoff
    tp_dev.yfac=0.0891719759;//�õ�yfac
    tp_dev.yoff=0xFFF1;//�õ�yoff  

//    TP_Adjust();  //��ĻУ׼ �ֶ�����ƽ��У׼

    return 1;
}

/*****************************************************************************
 * @name       :void Touch_Test(void)
 * @function   :touch test
 * @parameters :None
 * @retvalue   :None
******************************************************************************/
void Touch_Test(void)
{
//	TP_Init();
    FRONT_COLOR=Color16_RED;
    LCD_JustString(lcddev.width-24,0,"RST",16);//��ʾ��������
    LCD_Fill(lcddev.width-52,2,lcddev.width-50+20,18,Color16_RED); 
    while(1)
    {
        tp_dev.scan(0);
        if(tp_dev.sta&TP_PRES_DOWN)//������������
        {
            if(tp_dev.x<lcddev.width&&tp_dev.y<lcddev.height)
            {
                TP_Draw_Big_Point(tp_dev.x,tp_dev.y,FRONT_COLOR);//��ͼ
            }
        }else delay_ms(10);	//û�а������µ�ʱ��
    }
}

