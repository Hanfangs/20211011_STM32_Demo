/*
********************** (C) COPYRIGHT 2019 SOLAR SUPCON *************************
* File Name          : USART_CONFIG.h
* Author             : dingyongjian 
* Date First Issued  : 02/01/19
* Description        : 串口RS485通讯 H文件
********************************************************************************
* History:
* 02/01/19 v1.0
********************************************************************************
*/
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __USART_COM1_H
  #define __USART_COM1_H

/* 引用限制说明 ------------------------------------------------------------- */
#ifdef      USART1_GLOBALS
  #define   USART1_EXT
#else
  #define   USART1_EXT    extern
#endif

/* Private function prototypes -----------------------------------------------*/ 
USART1_EXT void Init_Usart1(void);
USART1_EXT void Usart1_RxEnable(void);
USART1_EXT void Usart1_TxEnable(void);
USART1_EXT void Usart1_StartRx(void);
USART1_EXT void Usart1_StartTx(u8 *pbyData, u16 wLength);
USART1_EXT void Usart1_RxEnd(void);
USART1_EXT void Usart1_TxEnd(void);
USART1_EXT void Usart1_StartTxDelay(void);
USART1_EXT void Usart1_TxEndDelay(void);
USART1_EXT FlagStatus Is_Usart1_RxEnd(void);
USART1_EXT FlagStatus Is_Usart1_TxEnd(void);
USART1_EXT u16 Usart1_GetRxLength(void);
USART1_EXT void Usart1_GetRxData(u8 *pbyData, u16 wLength);
USART1_EXT FlagStatus Com1_Recv(u8 *pbyData, u16 *pwLength);
USART1_EXT void Com1_Send(u8 *pbyData, u16 wLength);

#endif /*_USART_CONFIG_H*/

/************** (C) COPYRIGHT 2015 SOLAR SUPCON ************END OF FILE********/
