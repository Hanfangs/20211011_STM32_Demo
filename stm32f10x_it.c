/**
  ******************************************************************************
  * @file    Project/STM32F10x_StdPeriph_Template/stm32f10x_it.c 
  * @author  MCD Application Team
  * @version V3.4.0
  * @date    10/15/2010
  * @brief   Main Interrupt Service Routines.
  *          This file provides template for all exceptions handler and 
  *          peripherals interrupt service routine.
  ******************************************************************************
  * @copy
  *
  * THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
  * WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE
  * TIME. AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY
  * DIRECT, INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING
  * FROM THE CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE
  * CODING INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
  *
  * <h2><center>&copy; COPYRIGHT 2010 STMicroelectronics</center></h2>
  */ 

/* Includes ------------------------------------------------------------------*/


#include "stm32f10x_it.h"
#include "main.h"
#include "stm32f10x_adc.h"
//#include "USART_Com1.h"
//#include "USART_Com2.h"
/** @addtogroup STM32F10x_StdPeriph_Template
  * @{
  */

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/

/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

/******************************************************************************/
/*            Cortex-M3 Processor Exceptions Handlers                         */
/******************************************************************************/

/**
  * @brief   This function handles NMI exception.
  * @param  None
  * @retval None
  */
void NMI_Handler(void)
{
}

/**
  * @brief  This function handles Hard Fault exception.
  * @param  None
  * @retval None
  */
void HardFault_Handler(void)
{
  /* Go to infinite loop when Hard Fault exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles Memory Manage exception.
  * @param  None
  * @retval None
  */
void MemManage_Handler(void)
{
  /* Go to infinite loop when Memory Manage exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles Bus Fault exception.
  * @param  None
  * @retval None
  */
void BusFault_Handler(void)
{
  /* Go to infinite loop when Bus Fault exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles Usage Fault exception.
  * @param  None
  * @retval None
  */
void UsageFault_Handler(void)
{
  /* Go to infinite loop when Usage Fault exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles SVCall exception.
  * @param  None
  * @retval None
  */
void SVC_Handler(void)
{
}

/**
  * @brief  This function handles Debug Monitor exception.
  * @param  None
  * @retval None
  */
void DebugMon_Handler(void)
{
}

/**
  * @brief  This function handles PendSVC exception.
  * @param  None
  * @retval None
  */
void PendSV_Handler(void)
{
}

/**
  * @brief  This function handles SysTick Handler.
  * @param  None
  * @retval None
  */
void SysTick_Handler(void)
{
}

/**
  * @brief  This function handles ADC1 and ADC2 global interrupts requests.
  * @param  None
  * @retval None
  */
u32  TestADC_IRQCnt = 0;
//u8  RecordCnt = 0;
//u32  RecordValue[256];

void ADC1_2_IRQHandler(void)
{  
    if(ADC_GetFlagStatus(ADC1,ADC_FLAG_JEOC) == SET)
    {
         ADC_ClearFlag(ADC1, ADC_FLAG_JEOC);  
        
        g_wADC_ConvertedValue[TestADC_IRQCnt % 4]  = \
          ADC_GetInjectedConversionValue(ADC1, ADC_InjectedChannel_1);
        
        g_dwAdc_CurrentValue[TestADC_IRQCnt % 4] =  \
          ADC_GetInjectedConversionValue(ADC1,ADC_InjectedChannel_2);
         
       // RecordValue[RecordCnt++] = g_dwAdc_CurrentValue[TestADC_IRQCnt % 4];//gs_Motor_Param.dwMotor_Current;
        TestADC_IRQCnt++;
        MotorOverLoad_Check();
    }
}

u32  TestCCR4_IRQCnt = 0;
void TIM1_CC_IRQHandler(void)     ////�������ɶ��Ҳû���Ƶ�  �ǲ��Ǿ�û�������жϹ�������һ��
{
    if ( TIM_GetITStatus ( TIM1, TIM_IT_CC4 ) != RESET )              //CCR4�ж�
    {
        TIM_ClearFlag(TIM1, TIM_FLAG_CC4);
        TestCCR4_IRQCnt++;
    }
}
/**
  * @brief  TIM2_IRQHandler
  *         This function handles Timer2 Handler.
  * @param  None
  * @retval None
  */
void TIM1_BRK_IRQHandler(void)//ɲ��
{  
    TIM_ClearFlag(TIM1, TIM_FLAG_Break);
    //���ö�ת����
    Set_MotorErr(M_RUN_FAULT);//���õ������ʧ�ܹ���
    Set_MotorErr(M_OVERLOAD);//���õ����������
    Set_Motor_Stop();//���ͣ��
    TIM_ITConfig(TIM1, TIM_IT_Break,DISABLE);
}
/*
*******************************************************************************
* Function Name  : TIM2_IRQHandler
* Description    : 100us��ʱ���жϴ�����
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************
*/
void TIM2_IRQHandler(void)
{  
    TIM_ClearITPendingBit(TIM2, TIM_IT_Update);  
    
    g_dwTimer_100us++; 
    if(g_dwTimer_100us > TIME_ONE_DAY)//����һ������
    {
        g_dwTimer_100us = 0;
    }
    
    Decrease_TimeoutCnt();   
    Decrease_SendAcTimeoutCnt();//���ͳ�ʱ�ݼ�
    Usart2_StartTxDelay();//����������ʱ---------�����ʼ���͵���ʱ =6*100us
    Usart2_TxEndDelay(); //���������ʱ 
    MotorTimerDecrease();//����ӳ����к�����ʱ��
}
/*
*******************************************************************************
* Function Name  : TIM3_IRQHandler
* Description    : HALL�����ж�
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************
*/
void TIM3_IRQHandler(void)
{  
     /* ȷ���Ƿ������EXTI Line�ж� */
    if ( TIM_GetITStatus ( HALL_TIMx, TIM_IT_Trigger ) != RESET )              //�����ж�
    {
        /* ����жϱ�־λ	*/
        TIM_ClearITPendingBit (HALL_TIMx,TIM_IT_Trigger);
        Led1_RunTask_Op(LED_FLASH);//����״̬����˸
        HALL_TIMx_Callback();
    }       
}


/*
*******************************************************************************
* Function Name  : TIM4_IRQHandler
* Description    : 1ms��ʱ���жϴ�����
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************
*/
void TIM4_IRQHandler(void)
{  
    TIM_ClearITPendingBit(TIM4, TIM_IT_Update);   
    HAL_SYSTICK_Callback();
    Motor_Stop_Delay();
}

/*
*******************************************************************************
* Function Name  : USART1_IRQHandler
* Description    : usart1 ��������жϴ�����
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************
*/
void USART1_IRQHandler(void)
{
    USART_ITConfig(USART1,USART_IT_IDLE,DISABLE);
    
   // Usart1_RxEnd();
}

/*
*******************************************************************************
* Function Name  : DMA1_Channel4_IRQHandler
* Description    : usart1 DMA��������жϴ�����
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************
*/
void DMA1_Channel4_IRQHandler(void)
{ 
    DMA_ClearITPendingBit(DMA1_IT_TC4);  
    
    //Usart1_TxEnd();
}

/*
*******************************************************************************
* Function Name  : USART2_IRQHandler
* Description    : usart2 ��������жϴ�����
* Input          : None
* Output         : None         ��������߿����ж�  ����һ�����⣬�������߿����ڳ�ʼ��֮���ǲ���һֱ���ǿ��е��أ���Ҳ����˵����ж���ʱ����֣�
* Return         : None      ֻҪ������� Usart2_StartRx(void)   ���ڳ�ʼ���������ʱ�����Ѿ���������������ˡ�
///                                                              �Ѳ��ɣ�������߿���ֻ��������æ��֮���ٽ������ʱ���Żᱻ������  ֪���ˣ���RXNE=1֮����2�������ֽ�λ�󣬲Ż�IDLE=1   
*******************************************************************************                            �������λ������1�ġ�
*/
void USART2_IRQHandler(void)  
{
    USART_ITConfig(USART2,USART_IT_IDLE,DISABLE);
    
    Usart2_RxEnd();
}

/*
*******************************************************************************
* Function Name  : DMA1_Channel7_IRQHandler
* Description    : usart2 DMA��������жϴ�����
* Input          : None
* Output         : None
* Return         : None     �����õ����ͨ��7
*******************************************************************************
*/
void DMA1_Channel7_IRQHandler(void)
{ 
    DMA_ClearITPendingBit(DMA1_IT_TC7);  
    
    Usart2_TxEnd();     ////s_byUsart2TxEndDelay = 10; �����ֵ��һ��
}

/*
*******************************************************************************
* Function Name  : USART2_IRQHandler
* Description    : usart2 ��������жϴ�����
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************
*/
void USART3_IRQHandler(void)  
{
    USART_ITConfig(USART3,USART_IT_IDLE,DISABLE);
    
    //Usart3_RxEnd();;
}

/*
*******************************************************************************
* Function Name  : DMA1_Channel2_IRQHandler
* Description    : usart3 DMA��������жϴ�����
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************
*/
void DMA1_Channel2_IRQHandler(void)
{ 
    DMA_ClearITPendingBit(DMA1_IT_TC2);  
    
   // Usart3_TxEnd();
}


/******************************************************************************/
/*                 STM32F10x Peripherals Interrupt Handlers                   */
/*  Add here the Interrupt Handler for the used peripheral(s) (PPP), for the  */
/*  available peripheral interrupt handler's name please refer to the startup */
/*  file (startup_stm32f10x_xx.s).                                            */
/******************************************************************************/


/**
  * @brief  This function handles PPP interrupt request.
  * @param  None
  * @retval None
  */
/*void PPP_IRQHandler(void)
{
}*/

/**
  * @}
  */ 


/******************* (C) COPYRIGHT 2010 STMicroelectronics *****END OF FILE****/
