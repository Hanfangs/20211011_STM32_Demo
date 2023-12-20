/*
********************** (C) COPYRIGHT 2015 SOLAR SUPCON *************************
* File Name          : delay_time.h
* Author             : dingyongjian 
* Date First Issued  : 02/02/15
* Description        : 延时函数 C文件
********************************************************************************
* History:
* 02/02/15 v1.1
********************************************************************************
*/
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __DELAY_TIME_H
#define __DELAY_TIME_H

/* 引用限制说明 ------------------------------------------------------------- */
#ifdef      DELAY_TIME_GLOBALS
  #define   DELAY_TIME_EXT
#else
  #define   DELAY_TIME_EXT    extern
#endif

/* Private function prototypes -----------------------------------------------*/
DELAY_TIME_EXT void Delay_us(u16 t);
DELAY_TIME_EXT void Delay_ms(u16 y);

#endif /*__DELAY_TIME_H*/
/************** (C) COPYRIGHT 2015 SOLAR SUPCON ************END OF FILE********/

