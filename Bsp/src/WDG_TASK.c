/*
********************** (C) COPYRIGHT 2020 SOLAR  *************************
* File Name          : WDG_TASK.c
* Author             : dingyongjian 
* Date First Issued  : 
* Description        : ���Ź�
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
* �� �� �� ��  FeedWatchDog_Op
* ��    �� ��  ι������
* Ӳ�����ӣ�   -------------------------
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
    IWDG->KR=0X5555;//ʹ�ܶ�IWDG->PR��IWDG->RLR��д
    IWDG->PR=prer; //���÷�Ƶϵ��
    IWDG->RLR=rlr; //�Ӽ��ؼĴ��� IWDG->RLR
    IWDG->KR=0XAAAA;//reload
    IWDG->KR=0XCCCC;//ʹ�ܿ��Ź�
}

void FeedWatchDog_Op(void)
{
   IWDG->KR=0XAAAA;//reload
}

/*void IWDG_Init()
{
  IWDG->KR=0XAAAA;//reload
  IWDG->KR=0XCCCC;//ʹ�ܿ��Ź�
}*/

//ι�������Ź�
void IWDG_Feed(void)
{
   if(0 == g_byStopFeedDog)
   {
       IWDG->KR=0XAAAA;//reload
   }
}


//ֹͣι��
void Wdt_StopFeedDog(void)
{
    g_byStopFeedDog = 1;
}