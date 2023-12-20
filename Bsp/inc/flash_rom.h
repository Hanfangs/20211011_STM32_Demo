/*
********************** (C) COPYRIGHT 2015 SOLAR SUPCON *************************
* File Name          : FlashRom.h
* Author             : puhuafeng 
* Date First Issued  : 02/02/15
* Description        : FLASH驱动 h文件
********************************************************************************
* History:
* 02/02/15 v1.1
********************************************************************************
*/
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __FLASHROM_H_
  #define __FLASHROM_H_

/* 引用限制说明 ------------------------------------------------------------- */
#ifdef      FLASHROM_GLOBALS
  #define   FLASHROM_EXT
#else
  #define   FLASHROM_EXT    extern
#endif

/* Private function prototypes -----------------------------------------------*/
FLASHROM_EXT void FlashRead(u32 Addr, u8 *pBuf, u16 wlen);
FLASHROM_EXT void FlashWrite(u32 WriteAddr, u8 *pBuffer, u16 NumToWrite);
FLASHROM_EXT void RamhModify(u8 *pdes, u8 *psrc, u16 wlen);

#endif
/************** (C) COPYRIGHT 2015 SOLAR SUPCON ************END OF FILE********/