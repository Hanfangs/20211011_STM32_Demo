/*
********************** (C) COPYRIGHT 2015 SOLAR  *************************
* File Name          : delay_time.c
* Author             : dingyongjian 
* Date First Issued  : 02/02/15
* Description        : 延时函数 C文件
********************************************************************************
* History:
* 02/02/15 v1.1
********************************************************************************
*/
#define DELAY_TIME_GLOBALS

/* Includes ------------------------------------------------------------------*/
#include "include.h"
#include "delay_time.h"

/* Private functions ---------------------------------------------------------*/
/*
******************************************************************************
* 函 数 名 ：  Delay_us
* 描    述 ：  微妙级延时函数
******************************************************************************
*/
void Delay_us(u16 t)
{
   while(t--)
   {
       __ASM("nop"); 
       __ASM("nop");
       __ASM("nop");
       __ASM("nop");
       __ASM("nop");
       __ASM("nop");
       __ASM("nop");
       __ASM("nop");
       __ASM("nop"); 
       __ASM("nop");
       __ASM("nop");
       __ASM("nop");
       __ASM("nop");
       __ASM("nop");
       __ASM("nop");
       __ASM("nop");
       __ASM("nop"); 
       __ASM("nop");
       __ASM("nop");
       __ASM("nop");
       __ASM("nop");
       __ASM("nop");
       __ASM("nop");
       __ASM("nop");
       __ASM("nop"); 
       __ASM("nop");
       __ASM("nop");
       __ASM("nop");
       __ASM("nop");
       __ASM("nop");
       __ASM("nop");
       __ASM("nop");
       __ASM("nop"); 
       __ASM("nop");
       __ASM("nop");
       __ASM("nop");
       __ASM("nop");
       __ASM("nop");
       __ASM("nop");
       __ASM("nop");
       __ASM("nop"); 
       __ASM("nop");
       __ASM("nop");
       __ASM("nop");
       __ASM("nop");
       __ASM("nop");
       __ASM("nop");
       __ASM("nop");
       __ASM("nop"); 
       __ASM("nop");
       __ASM("nop");
       __ASM("nop");
       __ASM("nop");
       __ASM("nop");
       __ASM("nop");
       __ASM("nop");
       __ASM("nop"); 
       __ASM("nop");
       __ASM("nop");
       __ASM("nop");
       __ASM("nop");
   }
}

/*
******************************************************************************
* 函 数 名 ：  Delay_us
* 描    述 ：  毫秒级延时函数
******************************************************************************
*/
void Delay_ms(u16 y)
{
   while(y--)
   {
      Delay_us(1000);
   }
}

/************** (C) COPYRIGHT 2015 SOLAR SUPCON ************END OF FILE********/

