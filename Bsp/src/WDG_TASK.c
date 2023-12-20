/*
********************** (C) COPYRIGHT 2020 SOLAR  *************************
* File Name          : WDG_TASK.c
* Author             : dingyongjian 
* Date First Issued  : 
* Description        : 看门狗
********************************************************************************
* History:
* 02/02/20 v1.0
********************************************************************************
*/

#define WDG_TASK_GLOBALS

/* Includes ------------------------------------------------------------------*/
#include "stm32f10x.h"

#include "LED_TASK.h"
#include "WDG_TASK.h"


u8 g_byStopFeedDog = 0;
/*
******************************************************************************
* 函 数 名 ：  FeedWatchDog_Op
* 描    述 ：  喂狗处理
* 硬件连接：   -------------------------
                |  PB12 -  LED        | 
               -------------------------
******************************************************************************
*/
/*void FeedWatchDog_Op(void)
{
    uint8_t byBitVal;
    
    byBitVal = GPIO_ReadOutputDataBit(GPIOB,GPIO_Pin_12);
    
    GPIO_WriteBit(GPIOB,GPIO_Pin_12,(BitAction)(!byBitVal));
}*/
void IWDG_Init(u8 prer,u16 rlr)
{
    IWDG->KR=0X5555;//使能对IWDG->PR和IWDG->RLR的写
    IWDG->PR=prer; //设置分频系数
    IWDG->RLR=rlr; //从加载寄存器 IWDG->RLR
    IWDG->KR=0XAAAA;//reload
    IWDG->KR=0XCCCC;//使能看门狗
}

void FeedWatchDog_Op(void)
{
   IWDG->KR=0XAAAA;//reload
}

/*void IWDG_Init()
{
  IWDG->KR=0XAAAA;//reload
  IWDG->KR=0XCCCC;//使能看门狗
}*/

//喂独立看门狗
void IWDG_Feed(void)
{
   if(0 == g_byStopFeedDog)
   {
       IWDG->KR=0XAAAA;//reload
   }
}


//停止喂狗
void Wdt_StopFeedDog(void)
{
    g_byStopFeedDog = 1;
}