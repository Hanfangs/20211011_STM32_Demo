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
void TIM1_CC_IRQHandler(void)     ////这个好象啥事也没做似的  是不是就没有允许中断过啊，看一下
{
    if ( TIM_GetITStatus ( TIM1, TIM_IT_CC4 ) != RESET )              //CCR4中断
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
void TIM1_BRK_IRQHandler(void)//刹车
{  
    TIM_ClearFlag(TIM1, TIM_FLAG_Break);
    //设置堵转故障
    Set_MotorErr(M_RUN_FAULT);//设置电机运作失败故障
    Set_MotorErr(M_OVERLOAD);//设置电机过流故障
    Set_Motor_Stop();//电机停机
    TIM_ITConfig(TIM1, TIM_IT_Break,DISABLE);
}
/*
*******************************************************************************
* Function Name  : TIM2_IRQHandler
* Description    : 100us定时器中断处理函数
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************
*/
void TIM2_IRQHandler(void)
{  
    TIM_ClearITPendingBit(TIM2, TIM_IT_Update);  
    
    g_dwTimer_100us++; 
    if(g_dwTimer_100us > TIME_ONE_DAY)//超过一天清零
    {
        g_dwTimer_100us = 0;
    }
    
    Decrease_TimeoutCnt();   
    Decrease_SendAcTimeoutCnt();//发送超时递减
    Usart2_StartTxDelay();//启动发送延时---------这个开始发送的延时 =6*100us
    Usart2_TxEndDelay(); //发送完成延时 
    MotorTimerDecrease();//电机延迟运行和运行时间
}
/*
*******************************************************************************
* Function Name  : TIM3_IRQHandler
* Description    : HALL处理中断
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************
*/
void TIM3_IRQHandler(void)
{  
     /* 确保是否产生了EXTI Line中断 */
    if ( TIM_GetITStatus ( HALL_TIMx, TIM_IT_Trigger ) != RESET )              //捕获中断
    {
        /* 清除中断标志位	*/
        TIM_ClearITPendingBit (HALL_TIMx,TIM_IT_Trigger);
        Led1_RunTask_Op(LED_FLASH);//运行状态灯闪烁
        HALL_TIMx_Callback();
    }       
}


/*
*******************************************************************************
* Function Name  : TIM4_IRQHandler
* Description    : 1ms定时器中断处理函数
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
* Description    : usart1 接收完成中断处理函数
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
* Description    : usart1 DMA发送完成中断处理函数
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
* Description    : usart2 接收完成中断处理函数
* Input          : None
* Output         : None         这个是总线空闲中断  但有一个问题，就是总线空闲在初始化之后是不是一直都是空闲的呢，那也就是说这个中断随时会出现？
* Return         : None      只要调用这个 Usart2_StartRx(void)   而在初始化这个串口时，就已经调用了这个函数了。
///                                                              难不成，这个总线空闲只有在总线忙了之后，再进入空闲时，才会被触发吗？  知道了，当RXNE=1之后且2个数据字节位后，才会IDLE=1   
*******************************************************************************                            否则，这个位不会置1的。
*/
void USART2_IRQHandler(void)  
{
    USART_ITConfig(USART2,USART_IT_IDLE,DISABLE);
    
    Usart2_RxEnd();
}

/*
*******************************************************************************
* Function Name  : DMA1_Channel7_IRQHandler
* Description    : usart2 DMA发送完成中断处理函数
* Input          : None
* Output         : None
* Return         : None     我们用到这个通道7
*******************************************************************************
*/
void DMA1_Channel7_IRQHandler(void)
{ 
    DMA_ClearITPendingBit(DMA1_IT_TC7);  
    
    Usart2_TxEnd();     ////s_byUsart2TxEndDelay = 10; 将这个值给一个
}

/*
*******************************************************************************
* Function Name  : USART2_IRQHandler
* Description    : usart2 接收完成中断处理函数
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
* Description    : usart3 DMA发送完成中断处理函数
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
