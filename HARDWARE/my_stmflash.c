#include "my_stmflash.h"
 // STM32F030F4P6 16KB FLASH,4KB RAM,1������1KB,������ʼ0x8000000
// STM32F103C8T6 64KB FLASH,20KB RAM,1������1KB,������ʼ0x8000000 
//flash����ʱ������ɽ������
//��ȡָ����ַ�İ���(16λ����)
//faddr:����ַ(�˵�ַ����Ϊ2�ı���!!)
//����ֵ:��Ӧ����.
u16 My_STMFlash_ReadHalfWord(u32 addr)
{
	return *(u16*)addr; 
}

#if SVAE_BYTES_ORDE>0
void My_STMFlash_ReadBytes(u32 ReadAddr,u8 *pBuffer,u16 length)
{
	u16 i;
	union_u16 temp;
	for(i=0;i<length;i++)
	{
		temp.value=My_STMFlash_ReadHalfWord(ReadAddr+i);//��ȡ2���ֽ�.
		pBuffer[i] = temp.Bytes.byte_1;
		i++;
		if(i < length)
		{
			pBuffer[i] = temp.Bytes.byte_0;
		}
	}
}
#else
void My_STMFlash_ReadBytes(u32 ReadAddr,u8 *pBuffer,u16 length)
{
	My_STMFlash_Read(ReadAddr,(u16 *)pBuffer,(length/2)+(length%2));
}
#endif
//��ָ����ַ��ʼ����ָ�����ȵ�����
//ReadAddr:��ʼ��ַ
//pBuffer:����ָ��
//length:����(16λ)��
void My_STMFlash_Read(u32 ReadAddr,u16 *pBuffer,u16 length)   	
{
	u16 i;
	for(i=0;i<length;i++)
	{
		pBuffer[i]=My_STMFlash_ReadHalfWord(ReadAddr);//��ȡ2���ֽ�.
		ReadAddr+=2;//ƫ��2���ֽ�.	
	}
}
#ifdef STM32_FLASH_WREN	//���ʹ����д   
//������д��
//WriteAddr:��ʼ��ַ
//pBuffer:����ָ��
//length:����(16λ)��   
void My_STMFlash_Write_NoCheck(u32 WriteAddr,u16 *pBuffer,u16 length)   
{ 			 		 
	u16 i;
	for(i=0;i<length;i++)
	{
#if	!defined (USE_HAL_DRIVER)
		FLASH_ProgramHalfWord(WriteAddr,pBuffer[i]);
#else
		HAL_FLASH_Program(FLASH_TYPEPROGRAM_HALFWORD,WriteAddr,pBuffer[i]);
#endif
	    WriteAddr+=2;//��ַ����2.
	}  
} 
#if SVAE_BYTES_ORDE>0
void My_STMFlash_WriteBytes(u32 WriteAddr,u8 *pBuffer,u16 length)
{
	u16 addr_offset_sector;	   //������ƫ�Ƶ�ַ(16λ�ּ���)
	u16 length_write_sector;	   //��������Ҫд������ݳ���
	u16 buffer_flash[STM_SECTOR_SIZE/2];//�����2K�ֽ�
	while(length>0)
	{
		addr_offset_sector = WriteAddr%STM_SECTOR_SIZE;
		length_write_sector = STM_SECTOR_SIZE-addr_offset_sector;
		if(length>length_write_sector)//������
		{
			ByteArrayToHalfWordArray(pBuffer,buffer_flash,length_write_sector);//
			My_STMFlash_Write(WriteAddr,buffer_flash,length_write_sector/2);
			WriteAddr += length_write_sector;
			pBuffer += length_write_sector;
			length -= length_write_sector;
		}
		else
		{
			ByteArrayToHalfWordArray(pBuffer,buffer_flash,length);//
			My_STMFlash_Write(WriteAddr,buffer_flash,(length/2)+(length%2));
			length = 0;//����ѭ��
		}
	}
}
#else
void My_STMFlash_WriteBytes(u32 WriteAddr,u8 *pBuffer,u16 length)
{
	My_STMFlash_Write(WriteAddr,(u16 *)pBuffer,(length/2)+(length%2));
}
#endif
//��ָ����ַ��ʼд��ָ�����ȵ�����
//WriteAddr:��ʼ��ַ(�˵�ַ����Ϊ2�ı���!!)
//pBuffer:����ָ��
//length:����(16λ)��(����Ҫд���16λ���ݵĸ���.)	 
u16 buffer_flash[STM_SECTOR_SIZE/2];//�����2K�ֽ�
void My_STMFlash_Write(u32 WriteAddr,u16 *pBuffer,u16 length)	
{
	u16 addr_offset_sector;	   //������ƫ�Ƶ�ַ(16λ�ּ���)
	u16 countCanWrite; //������ʣ���ַ(16λ�ּ���)	   
 	u16 i;    
	if(WriteAddr<FLASH_BASE||(WriteAddr>=(FLASH_BASE+1024*STM32_FLASH_SIZE)))return;//�Ƿ���ַ
#if	!defined (USE_HAL_DRIVER)
		FLASH_Unlock();						//����
#else
		HAL_FLASH_Unlock();					//����
#endif
	addr_offset_sector = WriteAddr%STM_SECTOR_SIZE;		//�������ڵ�ƫ��(2���ֽ�Ϊ������λ.)
	while(length>0) 
	{	
		countCanWrite=(STM_SECTOR_SIZE-addr_offset_sector)/2;		//����ʣ��ռ��С 
		if(length<=countCanWrite)
		{
			countCanWrite=length;//�����ڸ�������Χ
		}
		My_STMFlash_Read(WriteAddr-addr_offset_sector,buffer_flash,STM_SECTOR_SIZE/2);//������������������
		//**All notes can be deleted and modified**//
		if(i<countCanWrite)//��Ҫ����
		{	
#if	!defined (USE_HAL_DRIVER)
			FLASH_ErasePage(WriteAddr-addr_offset_sector);//�����������
#else
			FLASH_PageErase(WriteAddr-addr_offset_sector);	//�����������
			FLASH_WaitForLastOperation(FLASH_WAITETIME);            	//�ȴ��ϴβ������
			CLEAR_BIT(FLASH->CR, FLASH_CR_PER);							//���CR�Ĵ�����PERλ���˲���Ӧ����FLASH_PageErase()����ɣ�
																		//����HAL�����沢û������Ӧ����HAL��bug��
#endif
			for(i=0;i<countCanWrite;i++)//����
			{
				buffer_flash[i+addr_offset_sector/2]=pBuffer[i];	  
			}
			My_STMFlash_Write_NoCheck(WriteAddr-addr_offset_sector,buffer_flash,STM_SECTOR_SIZE/2);//д����������  
		}
		else 
		{
			My_STMFlash_Write_NoCheck(WriteAddr,pBuffer,countCanWrite);//д�Ѿ������˵�,ֱ��д������ʣ������. 				   
		}  
		addr_offset_sector=0;				//ƫ��λ��Ϊ0 	 
		pBuffer += (countCanWrite*2);  	//ָ��ƫ��
		WriteAddr += (countCanWrite*2);	//д��ַƫ��	   
		length-=countCanWrite;	//�ֽ�(16λ)���ݼ�
	}
#if	!defined (USE_HAL_DRIVER)
	FLASH_Lock();//����
#else
	HAL_FLASH_Lock();		//����
#endif
}

/***************************************************************************************
���³�����Ϊ�����ɸ����ݱ��浽һ��Flash�����У�һ�����ڱ���仯Ƶ�ʽϸߵ�����
��綯���Ǳ�����̣�,��0.1���ﱣ��һ�Σ�����һ�㷽��������һ�ξͲ���һ�Σ���ֻ�ܱ������10000����
��ʹ�ô˷�������1����1024�ֽ�Ϊ��������0.1�����������6���ֽڣ������ɱ��浽1700000����
�÷����Ļ���ԭ���ǽ�������ͬһ���������յ�ַ˳�����α��棬�������ٲ�����Ȼ���������ʼ��ַ�����洢
***************************************************************************************/
static u16 addrOffset_sectorSave=0;//����ʵ����Ҫ�������ݵ�ƫ�Ƶ�ַ
static u16 *dataBuffer;//��Ҫ���������
static u16 dataSizeToSave = 0;//dataBuffer��16λ���ݳ���
static u32 addr_sector = FLASH_BASE_ADDR+(STM32_FLASH_SIZE-1)*STM_SECTOR_SIZE;

void My_STMFlash_SaveUseSector(void)
{
	My_STMFlash_Write_NoCheck(addr_sector+addrOffset_sectorSave, dataBuffer,dataSizeToSave);
	addrOffset_sectorSave+=dataSizeToSave*2;
	if((STM_SECTOR_SIZE-addrOffset_sectorSave)<(dataSizeToSave*2)){//����д��
#if	!defined (USE_HAL_DRIVER)
		FLASH_ErasePage(addr_sector);//������������
#else
		FLASH_PageErase(addr_sector);	//�����������
		FLASH_WaitForLastOperation(FLASH_WAITETIME);            	//�ȴ��ϴβ������
		CLEAR_BIT(FLASH->CR, FLASH_CR_PER);							//���CR�Ĵ�����PERλ���˲���Ӧ����FLASH_PageErase()����ɣ�
																	//����HAL�����沢û������Ӧ����HAL��bug��
#endif
		addrOffset_sectorSave=0;
		My_STMFlash_SaveUseSector();
	}
}
//��ȡʹ�����������������ɸ����ݵ��������ݣ�����ֵ����Flash��������û�б�������
bool My_STMFlash_ReadSectorSave(void)
{
	u16 index;//���㱣���˶��ٴΣ����������´�Ҫ����ĵ�ַ
	u16 buffer;
	//���㱣�������ͬʱ�������1����
	for(index=STM_SECTOR_SIZE/2;index>0;index--)
	{
		buffer = My_STMFlash_ReadHalfWord(addr_sector+((index-1)*2));
		if(buffer!=0xffff)
		{
			break;
		}
	}
	if(index==0)//��������������(��һ���ϵ�)
	{
		addrOffset_sectorSave = 0;
#if	!defined (USE_HAL_DRIVER)
		FLASH_Unlock();						//����
#else
		HAL_FLASH_Unlock();					//����
#endif
		return false;
	}
	else{
		addrOffset_sectorSave = ((index-1)/dataSizeToSave)*dataSizeToSave*2;
		//**All notes can be deleted and modified**//
#if	!defined (USE_HAL_DRIVER)
		FLASH_Lock();//����
#else
		HAL_FLASH_Lock();		//����
#endif
		return true;
	}
}
//��ʼ��ʹ�����������������ɸ����ݣ�����ȡ��������ݣ�����ֵ����Flash��������û�б�������
bool My_STMFlash_SectorSaveInit(u32 sectorAddr, u16 *buffer, u16 length)
{
	addr_sector = sectorAddr - sectorAddr%STM_SECTOR_SIZE;
	dataBuffer = buffer;
	dataSizeToSave = length;
	return My_STMFlash_ReadSectorSave();
}

#endif


