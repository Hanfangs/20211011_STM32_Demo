/*
********************** (C) COPYRIGHT 2019 SOLAR SUPCON *************************
* File Name          : Hardware_CONFIG.h
* Author             : dingyongjian 
* Date First Issued  : 02/01/19
* Description        : 硬件外设初始化 H文件
********************************************************************************
* History:
* 02/01/19 v1.0
********************************************************************************
*/
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __Hardware_CONFIG_H
  #define __Hardware_CONFIG_H

/* 引用限制说明 ------------------------------------------------------------- */
#ifdef      Hardware_CONFIG_GLOBALS
  #define   Hardware_CONFIG_EXT
#else
  #define   Hardware_CONFIG_EXT    extern
#endif
   
/* Private function prototypes -----------------------------------------------*/
Hardware_CONFIG_EXT void STM32_GPIO_Config(void);
Hardware_CONFIG_EXT void Init_TIMER2(void);
Hardware_CONFIG_EXT void Init_TIMER4(void);//5ms定时器
Hardware_CONFIG_EXT void NVIC_Configuration(void);//100us
Hardware_CONFIG_EXT void ADC1_Init(void);

Hardware_CONFIG_EXT u16 g_wADC_ConvertedValue[4];
Hardware_CONFIG_EXT s16 Get_Cpu_Temp(void);
Hardware_CONFIG_EXT void DebugUART_Configuration(void);
#endif /*__GPIO_CONFIG_H*/

/******************* (C) COPYRIGHT 2011 STMicroelectronics *****END OF FILE****/
