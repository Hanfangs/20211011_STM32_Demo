/*
********************** (C) COPYRIGHT 2019 SOLAR SUPCON *************************
* File Name          : USART_COM3.h
* Author             : dingyongjian 
* Date First Issued  : 02/01/15
* Description        : 串口RS485通讯 H文件
********************************************************************************
* History:
* 02/01/19 v1.0
********************************************************************************
*/
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __USART_COM3_H
  #define __USART_COM3_H

/* 引用限制说明 ------------------------------------------------------------- */
#ifdef      USART3_GLOBALS
  #define   USART3_EXT
#else
  #define   USART3_EXT    extern
#endif

/* Private function prototypes -----------------------------------------------*/ 
USART3_EXT void Init_Usart3(void);
USART3_EXT void Usart3_RxEnable(void);
USART3_EXT void Usart3_TxEnable(void);
USART3_EXT void Usart3_StartRx(void);
USART3_EXT void Usart3_StartTx(u8 *pbyData, u16 wLength);
USART3_EXT void Usart3_RxEnd(void);
USART3_EXT void Usart3_TxEnd(void);
USART3_EXT void Usart3_StartTxDelay(void);
USART3_EXT void Usart3_TxEndDelay(void);
USART3_EXT FlagStatus Is_Usart3_RxEnd(void);
USART3_EXT FlagStatus Is_Usart3_TxEnd(void);
USART3_EXT u16 Usart3_GetRxLength(void);
USART3_EXT void Usart3_GetRxData(u8 *pbyData, u16 wLength);
USART3_EXT FlagStatus Com3_Recv(u8 *pbyData, u16 *pwLength);
USART3_EXT void Com3_Send(u8 *pbyData, u16 wLength);

#endif /*_USART_CONFIG_H*/

/************** (C) COPYRIGHT 2015 SOLAR SUPCON ************END OF FILE********/
