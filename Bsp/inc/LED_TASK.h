/*
********************** (C) COPYRIGHT 2015 SOLAR  *************************
* File Name          : LED_TASK.h
* Author             : puhuafeng 
* Date First Issued  : 02/02/15
* Description        : LED���� H�ļ�
********************************************************************************
* History:
* 02/02/15 v1.1
********************************************************************************
*/
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __LED_TASK_H
  #define __LED_TASK_H

/*
******************************************************************************
*��������˵��
******************************************************************************
*/
#ifdef      LED_TASK_GLOBALS
  #define   LED_TASK_EXT
#else
  #define   LED_TASK_EXT    extern
#endif


#define LED_LIGHTUP     0  //����
#define LED_FLASH       1  //����
#define LED_LIGHTDOWM   2  //���

/* Private function prototypes -----------------------------------------------*/
LED_TASK_EXT void LED_Status_Control(void);
LED_TASK_EXT void Led1_RunTask_Op(u8 Led_Status);//����״̬��

#endif
/************** (C) COPYRIGHT 2015 SOLAR SUPCON ************END OF FILE********/
