/****************************************Copyright (c)**************************************************
**
**                                
**
**--------------File Info-------------------------------------------------------------------------------
** File Name: 			W25Qxx.h
** Last modified Date: 	2010-10-29
** Last Version: 		1.0
** Descriptions: 		SPI flash 操作函数 
**
**------------------------------------------------------------------------------------------------------
** Created 	 By: 		
** Created date: 		2010-10-29
** Version: 1.0
** Descriptions: 		First version
**
**------------------------------------------------------------------------------------------------------
** Modified by: 	
** Modified date: 	
** Version:
** Descriptions:  	
**
********************************************************************************************************/

#ifndef __W25QXX_H 
#define __W25QXX_H

/* Includes ------------------------------------------------------------------*/
#include "stm32f10x.h"
#include "stm32f10x_spi.h"
/* Private typedef -----------------------------------------------------------*/
/*
******************************************************************************
*引用限制说明
******************************************************************************
*/
#ifdef      W25QXX_GLOBALS
  #define   W25QXX_EXT
#else
  #define   W25QXX_EXT    extern
#endif


//W25X系列/Q系列芯片列表	   
//W25Q80  ID  0XEF13
//W25Q16  ID  0XEF14
//W25Q32  ID  0XEF15
//W25Q64  ID  0XEF16	
//W25Q128 ID  0XEF17	
#define W25Q80 	0XEF13 	
#define W25Q16 	0XEF14
#define W25Q32 	0XEF15
#define W25Q64 	0XEF16
#define W25Q128	0XEF17
#define W25X40  0xEF12
	   

#define SPI_FLASH_CS_LOW()       GPIO_ResetBits(GPIOC, GPIO_Pin_7)
#define SPI_FLASH_CS_HIGH()      GPIO_SetBits(GPIOC, GPIO_Pin_7)


				 
////////////////////////////////////////////////////////////////////////////
 
//指令表
#define W25X_WriteEnable		0x06 
#define W25X_WriteDisable		0x04 
#define W25X_ReadStatusReg		0x05 
#define W25X_WriteStatusReg		0x01 
#define W25X_ReadData			0x03 
#define W25X_FastReadData		0x0B 
#define W25X_FastReadDual		0x3B 
#define W25X_PageProgram		0x02 
#define W25X_BlockErase			0xD8 
#define W25X_SectorErase		0x20 
#define W25X_ChipErase			0xC7 
#define W25X_PowerDown			0xB9 
#define W25X_ReleasePowerDown	0xAB 
#define W25X_DeviceID			0xAB 
#define W25X_ManufactDeviceID	0x90 
#define W25X_JedecDeviceID		0x9F 


W25QXX_EXT u16 W25QXX_TYPE;					//定义W25QXX芯片型号	
W25QXX_EXT void W25QXX_Init(void);
W25QXX_EXT u16 W25QXX_ReadID(void);  	    		//读取FLASH ID
W25QXX_EXT u8 W25QXX_ReadSR(void);        		//读取状态寄存器 
W25QXX_EXT void W25QXX_Write_SR(u8 sr);  			//写状态寄存器
W25QXX_EXT void W25QXX_Write_Enable(void);  		//写使能 
W25QXX_EXT void W25QXX_Write_Disable(void);		//写保护
W25QXX_EXT void W25QXX_Write_NoCheck(u8* pBuffer,u32 WriteAddr,u16 NumByteToWrite);
W25QXX_EXT void W25QXX_Read(u8* pBuffer,u32 ReadAddr,u16 NumByteToRead);   //读取flash
W25QXX_EXT void W25QXX_Write(u8* pBuffer,u32 WriteAddr,u16 NumByteToWrite);//写入flash
W25QXX_EXT void W25QXX_Erase_Chip(void);    	  	//整片擦除
W25QXX_EXT void W25QXX_Erase_Sector(u32 Dst_Addr);	//扇区擦除
W25QXX_EXT void W25QXX_Wait_Busy(void);           	//等待空闲
W25QXX_EXT void W25QXX_PowerDown(void);        	//进入掉电模式
W25QXX_EXT void W25QXX_WAKEUP(void);				//唤醒
W25QXX_EXT void SPI2_Init(void);

//将4k内容全部读取，替换相应偏移地址内容后，再将4K重新写入，需修改函数
W25QXX_EXT void Spi_Flash_MWrite(u8 *pbuf,u32 Addr,u16 wlen);
W25QXX_EXT u8 g_byW25Qxx_WriteBuf[4096];
W25QXX_EXT u8 g_byW25Qxx_ReadBuf[4096];
#endif



/*********************************************************************************************************
      END FILE
*********************************************************************************************************/

