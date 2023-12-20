/*
********************** (C) COPYRIGHT 2019 SOLAR SUPCON *************************
* File Name          : USART_COM2.h
* Author             : dingyongjian 
* Date First Issued  : 02/01/15
* Description        : 串口RS485通讯 H文件
********************************************************************************
* History:
* 02/01/19 v1.0
********************************************************************************
*/
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __USART_COM2_H
  #define __USART_COM2_H

/* 引用限制说明 ------------------------------------------------------------- */
#ifdef      USART2_GLOBALS
  #define   USART2_EXT
#else
  #define   USART2_EXT    extern
#endif

/* Private function prototypes -----------------------------------------------*/ 
USART2_EXT void Init_Usart2(void);
USART2_EXT void Usart2_RxEnable(void);
USART2_EXT void Usart2_TxEnable(void);
USART2_EXT void Usart2_StartRx(void);
USART2_EXT void Usart2_StartTx(u8 *pbyData, u16 wLength);
USART2_EXT void Usart2_RxEnd(void);
USART2_EXT void Usart2_TxEnd(void);
USART2_EXT void Usart2_StartTxDelay(void);
USART2_EXT void Usart2_TxEndDelay(void);
USART2_EXT FlagStatus Is_Usart2_RxEnd(void);
USART2_EXT FlagStatus Is_Usart2_TxEnd(void);
USART2_EXT u16 Usart2_GetRxLength(void);
USART2_EXT void Usart2_GetRxData(u8 *pbyData, u16 wLength);
USART2_EXT FlagStatus Com2_Recv(u8 *pbyData, u16 *pwLength);
USART2_EXT void Com2_Send(u8 *pbyData, u16 wLength);

USART2_EXT FlagStatus Is_Uart2StartTxDelayEnd(void);
USART2_EXT void Usart2_ReadForTx(void);
#endif /*_USART_CONFIG_H*/

/************** (C) COPYRIGHT 2015 SOLAR SUPCON ************END OF FILE********/
