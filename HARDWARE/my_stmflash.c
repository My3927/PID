#include "my_stmflash.h"
 // STM32F030F4P6 16KB FLASH,4KB RAM,1个扇区1KB,程序起始0x8000000
// STM32F103C8T6 64KB FLASH,20KB RAM,1个扇区1KB,程序起始0x8000000 
//flash擦除时是整个山区擦除
//读取指定地址的半字(16位数据)
//faddr:读地址(此地址必须为2的倍数!!)
//返回值:对应数据.
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
		temp.value=My_STMFlash_ReadHalfWord(ReadAddr+i);//读取2个字节.
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
//从指定地址开始读出指定长度的数据
//ReadAddr:起始地址
//pBuffer:数据指针
//length:半字(16位)数
void My_STMFlash_Read(u32 ReadAddr,u16 *pBuffer,u16 length)   	
{
	u16 i;
	for(i=0;i<length;i++)
	{
		pBuffer[i]=My_STMFlash_ReadHalfWord(ReadAddr);//读取2个字节.
		ReadAddr+=2;//偏移2个字节.	
	}
}
#ifdef STM32_FLASH_WREN	//如果使能了写   
//不检查的写入
//WriteAddr:起始地址
//pBuffer:数据指针
//length:半字(16位)数   
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
	    WriteAddr+=2;//地址增加2.
	}  
} 
#if SVAE_BYTES_ORDE>0
void My_STMFlash_WriteBytes(u32 WriteAddr,u8 *pBuffer,u16 length)
{
	u16 addr_offset_sector;	   //扇区内偏移地址(16位字计算)
	u16 length_write_sector;	   //扇区内需要写入的数据长度
	u16 buffer_flash[STM_SECTOR_SIZE/2];//最多是2K字节
	while(length>0)
	{
		addr_offset_sector = WriteAddr%STM_SECTOR_SIZE;
		length_write_sector = STM_SECTOR_SIZE-addr_offset_sector;
		if(length>length_write_sector)//跨扇区
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
			length = 0;//结束循环
		}
	}
}
#else
void My_STMFlash_WriteBytes(u32 WriteAddr,u8 *pBuffer,u16 length)
{
	My_STMFlash_Write(WriteAddr,(u16 *)pBuffer,(length/2)+(length%2));
}
#endif
//从指定地址开始写入指定长度的数据
//WriteAddr:起始地址(此地址必须为2的倍数!!)
//pBuffer:数据指针
//length:半字(16位)数(就是要写入的16位数据的个数.)	 
u16 buffer_flash[STM_SECTOR_SIZE/2];//最多是2K字节
void My_STMFlash_Write(u32 WriteAddr,u16 *pBuffer,u16 length)	
{
	u16 addr_offset_sector;	   //扇区内偏移地址(16位字计算)
	u16 countCanWrite; //扇区内剩余地址(16位字计算)	   
 	u16 i;    
	if(WriteAddr<FLASH_BASE||(WriteAddr>=(FLASH_BASE+1024*STM32_FLASH_SIZE)))return;//非法地址
#if	!defined (USE_HAL_DRIVER)
		FLASH_Unlock();						//解锁
#else
		HAL_FLASH_Unlock();					//解锁
#endif
	addr_offset_sector = WriteAddr%STM_SECTOR_SIZE;		//在扇区内的偏移(2个字节为基本单位.)
	while(length>0) 
	{	
		countCanWrite=(STM_SECTOR_SIZE-addr_offset_sector)/2;		//扇区剩余空间大小 
		if(length<=countCanWrite)
		{
			countCanWrite=length;//不大于该扇区范围
		}
		My_STMFlash_Read(WriteAddr-addr_offset_sector,buffer_flash,STM_SECTOR_SIZE/2);//读出整个扇区的内容
		//**All notes can be deleted and modified**//
		if(i<countCanWrite)//需要擦除
		{	
#if	!defined (USE_HAL_DRIVER)
			FLASH_ErasePage(WriteAddr-addr_offset_sector);//擦除这个扇区
#else
			FLASH_PageErase(WriteAddr-addr_offset_sector);	//擦除这个扇区
			FLASH_WaitForLastOperation(FLASH_WAITETIME);            	//等待上次操作完成
			CLEAR_BIT(FLASH->CR, FLASH_CR_PER);							//清除CR寄存器的PER位，此操作应该在FLASH_PageErase()中完成！
																		//但是HAL库里面并没有做，应该是HAL库bug！
#endif
			for(i=0;i<countCanWrite;i++)//复制
			{
				buffer_flash[i+addr_offset_sector/2]=pBuffer[i];	  
			}
			My_STMFlash_Write_NoCheck(WriteAddr-addr_offset_sector,buffer_flash,STM_SECTOR_SIZE/2);//写入整个扇区  
		}
		else 
		{
			My_STMFlash_Write_NoCheck(WriteAddr,pBuffer,countCanWrite);//写已经擦除了的,直接写入扇区剩余区间. 				   
		}  
		addr_offset_sector=0;				//偏移位置为0 	 
		pBuffer += (countCanWrite*2);  	//指针偏移
		WriteAddr += (countCanWrite*2);	//写地址偏移	   
		length-=countCanWrite;	//字节(16位)数递减
	}
#if	!defined (USE_HAL_DRIVER)
	FLASH_Lock();//上锁
#else
	HAL_FLASH_Lock();		//上锁
#endif
}

/***************************************************************************************
以下程序功能为将若干个数据保存到一个Flash扇区中，一般用于保存变化频率较高的数据
如电动车仪表保存里程，,若0.1公里保存一次，按照一般方法，保存一次就擦除一次，则只能保存最大10000公里
而使用此方法，以1扇区1024字节为例，保存0.1公里的数据是6个字节，则最大可保存到1700000公里
该方法的基本原理是将数据在同一个扇区按照地址顺序依次保存，存满后再擦除，然后从扇区起始地址继续存储
***************************************************************************************/
static u16 addrOffset_sectorSave=0;//计算实际需要保存数据的偏移地址
static u16 *dataBuffer;//需要保存的数据
static u16 dataSizeToSave = 0;//dataBuffer的16位数据长度
static u32 addr_sector = FLASH_BASE_ADDR+(STM32_FLASH_SIZE-1)*STM_SECTOR_SIZE;

void My_STMFlash_SaveUseSector(void)
{
	My_STMFlash_Write_NoCheck(addr_sector+addrOffset_sectorSave, dataBuffer,dataSizeToSave);
	addrOffset_sectorSave+=dataSizeToSave*2;
	if((STM_SECTOR_SIZE-addrOffset_sectorSave)<(dataSizeToSave*2)){//扇区写满
#if	!defined (USE_HAL_DRIVER)
		FLASH_ErasePage(addr_sector);//擦除整个扇区
#else
		FLASH_PageErase(addr_sector);	//擦除这个扇区
		FLASH_WaitForLastOperation(FLASH_WAITETIME);            	//等待上次操作完成
		CLEAR_BIT(FLASH->CR, FLASH_CR_PER);							//清除CR寄存器的PER位，此操作应该在FLASH_PageErase()中完成！
																	//但是HAL库里面并没有做，应该是HAL库bug！
#endif
		addrOffset_sectorSave=0;
		My_STMFlash_SaveUseSector();
	}
}
//读取使用整个扇区保存若干个数据的数据内容，返回值代表Flash扇区中有没有保存数据
bool My_STMFlash_ReadSectorSave(void)
{
	u16 index;//计算保存了多少次，进而计算下次要保存的地址
	u16 buffer;
	//计算保存次数，同时将结果加1保存
	for(index=STM_SECTOR_SIZE/2;index>0;index--)
	{
		buffer = My_STMFlash_ReadHalfWord(addr_sector+((index-1)*2));
		if(buffer!=0xffff)
		{
			break;
		}
	}
	if(index==0)//整个扇区被擦除(第一次上电)
	{
		addrOffset_sectorSave = 0;
#if	!defined (USE_HAL_DRIVER)
		FLASH_Unlock();						//解锁
#else
		HAL_FLASH_Unlock();					//解锁
#endif
		return false;
	}
	else{
		addrOffset_sectorSave = ((index-1)/dataSizeToSave)*dataSizeToSave*2;
		//**All notes can be deleted and modified**//
#if	!defined (USE_HAL_DRIVER)
		FLASH_Lock();//上锁
#else
		HAL_FLASH_Lock();		//上锁
#endif
		return true;
	}
}
//初始化使用整个扇区保存若干个数据，并读取保存的数据，返回值代表Flash扇区中有没有保存数据
bool My_STMFlash_SectorSaveInit(u32 sectorAddr, u16 *buffer, u16 length)
{
	addr_sector = sectorAddr - sectorAddr%STM_SECTOR_SIZE;
	dataBuffer = buffer;
	dataSizeToSave = length;
	return My_STMFlash_ReadSectorSave();
}

#endif


