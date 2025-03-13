#ifndef __MY_FLASH_H__
#define __MY_FLASH_H__
#include "my_include.h"

//////////////////////////////////////////////////////////////////////////////////////////////////////
#define FLASH_BASE_ADDR			FLASH_BASE 	 		//在stm32f10x.h中有定义
//用户根据自己的需要设置
#define STM32_FLASH_SIZE 		64 	 		//所选STM32的FLASH容量大小(单位为K)
#define STM32_FLASH_WREN 		1              //使能FLASH写入(0，不是能;1，使能)
#define FLASH_WAITETIME  		50000          	//FLASH等待超时时间
//////////////////////////////////////////////////////////////////////////////////////////////////////
#define SVAE_BYTES_ORDE			0//使用读写字节函数时，字节的存储顺序，0为16位数据的高8位在flash低地址
									//1为16位数据的高8位在flash高地址，为0时程序执行效率较高
									
#if STM32_FLASH_SIZE<256
#define STM_SECTOR_SIZE 		1024 //字节
#else 
#define STM_SECTOR_SIZE			2048
#endif	
#ifndef	FLASH_FLAG_WRPRTERR
#define FLASH_FLAG_WRPRTERR		FLASH_FLAG_WRPERR
#endif
 
#if	defined (USE_HAL_DRIVER) 
extern void    FLASH_PageErase(uint32_t PageAddress);
#endif
//读取指定地址的半字(16位数据)
//faddr:读地址(此地址必须为2的倍数!!)
//返回值:对应数据.
uint16_t My_STMFlash_ReadHalfWord(u32 addr);
void My_STMFlash_ReadBytes(u32 ReadAddr,u8 *pBuffer,u16 length);
//从指定地址开始读出指定长度的数据
//ReadAddr:起始地址
//pBuffer:数据指针
//NumToWrite:半字(16位)数
void My_STMFlash_Read(u32 ReadAddr,u16 *pBuffer,u16 length);
#ifdef STM32_FLASH_WREN	//如果使能了写  
void My_STMFlash_WriteBytes(u32 WriteAddr,u8 *pBuffer,u16 length);
//不检查的写入
//WriteAddr:起始地址
//pBuffer:数据指针
//NumToWrite:半字(16位)数   
void My_STMFlash_Write_NoCheck(u32 WriteAddr,u16 *pBuffer,u16 length);
//从指定地址开始写入指定长度的数据
//WriteAddr:起始地址(此地址必须为2的倍数!!)
//pBuffer:数据指针
//NumToWrite:半字(16位)数(就是要写入的16位数据的个数.)	
void My_STMFlash_Write(u32 WriteAddr,u16 *pBuffer,u16 length);
#endif	
/***************************************************************************************
以下程序功能为将若干个数据保存到一个Flash扇区中，一般用于保存变化频率较高的数据
如电动车仪表保存里程，,若0.1公里保存一次，按照一般方法，保存一次就擦除一次，则只能保存最大10000公里
而使用此方法，以1扇区1024字节为例，保存0.1公里的数据是6个字节，则最大可保存到1700000公里
该方法的基本原理是将数据在同一个扇区按照地址顺序依次保存，存满后再擦除，然后从扇区起始地址继续存储
***************************************************************************************/
void My_STMFlash_SaveUseSector(void);
//读取使用整个扇区保存若干个数据的数据内容，返回值代表Flash扇区中有没有保存数据
bool My_STMFlash_ReadSectorSave(void);
//初始化使用整个扇区保存若干个数据，并读取保存的数据，返回值代表Flash扇区中有没有保存数据
bool My_STMFlash_SectorSaveInit(u32 sectorAddr, u16 *buffer, u16 length);		   
#endif

