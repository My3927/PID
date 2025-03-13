#ifndef __MY_FLASH_H__
#define __MY_FLASH_H__
#include "my_include.h"

//////////////////////////////////////////////////////////////////////////////////////////////////////
#define FLASH_BASE_ADDR			FLASH_BASE 	 		//��stm32f10x.h���ж���
//�û������Լ�����Ҫ����
#define STM32_FLASH_SIZE 		64 	 		//��ѡSTM32��FLASH������С(��λΪK)
#define STM32_FLASH_WREN 		1              //ʹ��FLASHд��(0��������;1��ʹ��)
#define FLASH_WAITETIME  		50000          	//FLASH�ȴ���ʱʱ��
//////////////////////////////////////////////////////////////////////////////////////////////////////
#define SVAE_BYTES_ORDE			0//ʹ�ö�д�ֽں���ʱ���ֽڵĴ洢˳��0Ϊ16λ���ݵĸ�8λ��flash�͵�ַ
									//1Ϊ16λ���ݵĸ�8λ��flash�ߵ�ַ��Ϊ0ʱ����ִ��Ч�ʽϸ�
									
#if STM32_FLASH_SIZE<256
#define STM_SECTOR_SIZE 		1024 //�ֽ�
#else 
#define STM_SECTOR_SIZE			2048
#endif	
#ifndef	FLASH_FLAG_WRPRTERR
#define FLASH_FLAG_WRPRTERR		FLASH_FLAG_WRPERR
#endif
 
#if	defined (USE_HAL_DRIVER) 
extern void    FLASH_PageErase(uint32_t PageAddress);
#endif
//��ȡָ����ַ�İ���(16λ����)
//faddr:����ַ(�˵�ַ����Ϊ2�ı���!!)
//����ֵ:��Ӧ����.
uint16_t My_STMFlash_ReadHalfWord(u32 addr);
void My_STMFlash_ReadBytes(u32 ReadAddr,u8 *pBuffer,u16 length);
//��ָ����ַ��ʼ����ָ�����ȵ�����
//ReadAddr:��ʼ��ַ
//pBuffer:����ָ��
//NumToWrite:����(16λ)��
void My_STMFlash_Read(u32 ReadAddr,u16 *pBuffer,u16 length);
#ifdef STM32_FLASH_WREN	//���ʹ����д  
void My_STMFlash_WriteBytes(u32 WriteAddr,u8 *pBuffer,u16 length);
//������д��
//WriteAddr:��ʼ��ַ
//pBuffer:����ָ��
//NumToWrite:����(16λ)��   
void My_STMFlash_Write_NoCheck(u32 WriteAddr,u16 *pBuffer,u16 length);
//��ָ����ַ��ʼд��ָ�����ȵ�����
//WriteAddr:��ʼ��ַ(�˵�ַ����Ϊ2�ı���!!)
//pBuffer:����ָ��
//NumToWrite:����(16λ)��(����Ҫд���16λ���ݵĸ���.)	
void My_STMFlash_Write(u32 WriteAddr,u16 *pBuffer,u16 length);
#endif	
/***************************************************************************************
���³�����Ϊ�����ɸ����ݱ��浽һ��Flash�����У�һ�����ڱ���仯Ƶ�ʽϸߵ�����
��綯���Ǳ�����̣�,��0.1���ﱣ��һ�Σ�����һ�㷽��������һ�ξͲ���һ�Σ���ֻ�ܱ������10000����
��ʹ�ô˷�������1����1024�ֽ�Ϊ��������0.1�����������6���ֽڣ������ɱ��浽1700000����
�÷����Ļ���ԭ���ǽ�������ͬһ���������յ�ַ˳�����α��棬�������ٲ�����Ȼ���������ʼ��ַ�����洢
***************************************************************************************/
void My_STMFlash_SaveUseSector(void);
//��ȡʹ�����������������ɸ����ݵ��������ݣ�����ֵ����Flash��������û�б�������
bool My_STMFlash_ReadSectorSave(void);
//��ʼ��ʹ�����������������ɸ����ݣ�����ȡ��������ݣ�����ֵ����Flash��������û�б�������
bool My_STMFlash_SectorSaveInit(u32 sectorAddr, u16 *buffer, u16 length);		   
#endif

