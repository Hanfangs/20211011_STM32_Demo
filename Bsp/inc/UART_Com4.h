/*
********************** (C) COPYRIGHT 2019 SOLAR SUPCON *************************
* File Name          : UART_COM4.h
* Author             : dingyongjian 
* Date First Issued  : 02/01/15
* Description        : 串口RS485通讯 H文件
********************************************************************************
* History:
* 02/01/19 v1.0
********************************************************************************
*/
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __UART_COM4_H
  #define __UART_COM4_H

/* 引用限制说明 ------------------------------------------------------------- */
#ifdef      UART4_GLOBALS
  #define   UART4_EXT
#else
  #define   UART4_EXT    extern
#endif

/* Private function prototypes -----------------------------------------------*/ 
UART4_EXT void Init_Uart4(void);
UART4_EXT void Uart4_RxEnable(void);
UART4_EXT void Uart4_TxEnable(void);
UART4_EXT void Uart4_StartRx(void);
UART4_EXT void Uart4_StartTx(u8 *pbyData, u16 wLength);
UART4_EXT void Uart4_RxEnd(void);
UART4_EXT void Uart4_TxEnd(void);
UART4_EXT void Uart4_StartTxDelay(void);
UART4_EXT void Uart4_TxEndDelay(void);
UART4_EXT FlagStatus Is_Uart4_RxEnd(void);
UART4_EXT FlagStatus Is_Uart4_TxEnd(void);
UART4_EXT u16 Uart4_GetRxLength(void);
UART4_EXT void Uart4_GetRxData(u8 *pbyData, u16 wLength);
UART4_EXT FlagStatus Com4_Recv(u8 *pbyData, u16 *pwLength);
UART4_EXT void Com4_Send(u8 *pbyData, u16 wLength);

//UART4_EXT void Uart4_Test_Op(void);
UART4_EXT FlagStatus Is_Uart4StartTxDelayEnd(void);
UART4_EXT void Uart4_ReadForTx(void);
UART4_EXT void Uart4_TripTx(u8 *pbyData, u16 wLength);
#endif /*_USART_CONFIG_H*/

/************** (C) COPYRIGHT 2015 SOLAR SUPCON ************END OF FILE********/
