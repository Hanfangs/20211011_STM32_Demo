/*
********************** (C) COPYRIGHT 2015 SOLAR *************************
* File Name          : LED_TASK.c
* Author             : dingyongjian 
* Date First Issued  : 02/02/15
* Description        : LED驱动 C文件
********************************************************************************
* History:
* 02/02/15 v1.1
********************************************************************************
*/
#define LED_TASK_GLOBALS

/* Includes ------------------------------------------------------------------*/
#include "include.h"
#include "main.h"
//#include "USART2_RS422.h"
/* Private functions ------------------------------------------------------- */
/*
******************************************************************************
* 函 数 名 ：  Ledx_RunTask_Op
* 描    述 ：  LED处理
* 硬件连接：    -------------------------
                
               -------------------------
******************************************************************************
*/
void Led1_RunTask_Op(u8 Led_Status)//运行状态灯
{
    u8 byBitVal;
    if(Led_Status == LED_FLASH)//闪烁
    {
        byBitVal = GPIO_ReadOutputDataBit(LED_PORT,LED_PIN);
    
        GPIO_WriteBit(LED_PORT,LED_PIN,(BitAction)(!byBitVal));
    }
    else if(Led_Status == LED_LIGHTUP)
    {
    	GPIO_SetBits(LED_PORT, LED_PIN);//LED1亮
    }
    else if(Led_Status == LED_LIGHTDOWM)
    {
        GPIO_ResetBits(LED_PORT, LED_PIN);//LED1灭
    }
    
} 

void Led2_RunTask_Op(u8 Led_Status)//电源故障指示灯 -》镜组通讯故障灯
{
    u8 byBitVal;
    if(Led_Status == LED_FLASH)//闪烁
    {
        byBitVal = GPIO_ReadOutputDataBit(GPIOA,GPIO_Pin_6);
    
        GPIO_WriteBit(GPIOA,GPIO_Pin_6,(BitAction)(!byBitVal));
    }
    else if(Led_Status == LED_LIGHTUP)
    {
    	GPIO_ResetBits(GPIOA, GPIO_Pin_6);//LED2亮
    }
    else if(Led_Status == LED_LIGHTDOWM)
    {
        GPIO_SetBits(GPIOA, GPIO_Pin_6);//LED2灭
    }
    
} 

void Led3_RunTask_Op(u8 Led_Status)//镜组通讯灯
{
    u8 byBitVal;
    if(Led_Status == LED_FLASH)//闪烁
    {
        byBitVal = GPIO_ReadOutputDataBit(GPIOA,GPIO_Pin_7);
    
        GPIO_WriteBit(GPIOA,GPIO_Pin_7,(BitAction)(!byBitVal));
    }
    else if(Led_Status == LED_LIGHTUP)
    {
    	GPIO_ResetBits(GPIOA, GPIO_Pin_7);//LED3亮
    }
    else if(Led_Status == LED_LIGHTDOWM)
    {
        GPIO_SetBits(GPIOA, GPIO_Pin_7);//LED3灭
    }
   
} 

void Led4_RunTask_Op(u8 Led_Status)//预留
{
    u8 byBitVal;
    if(Led_Status == LED_FLASH)//闪烁
    {
        byBitVal = GPIO_ReadOutputDataBit(GPIOC,GPIO_Pin_4);
    
        GPIO_WriteBit(GPIOC,GPIO_Pin_4,(BitAction)(!byBitVal));
    }
    else if(Led_Status == LED_LIGHTUP)
    {
    	GPIO_ResetBits(GPIOC, GPIO_Pin_4);//LED4亮
    }
    else if(Led_Status == LED_LIGHTDOWM)
    {
        GPIO_SetBits(GPIOC, GPIO_Pin_4);//LED4灭
    }
   
} 

void Led5_RunTask_Op(u8 Led_Status)//方位驱动器指示灯
{
    u8 byBitVal;
    if(Led_Status == LED_FLASH)//闪烁
    {
        byBitVal = GPIO_ReadOutputDataBit(GPIOC,GPIO_Pin_5);
    
        GPIO_WriteBit(GPIOC,GPIO_Pin_5,(BitAction)(!byBitVal));
    }
    else if(Led_Status == LED_LIGHTUP)
    {
    	GPIO_ResetBits(GPIOC, GPIO_Pin_5);//LED5亮
    }
    else if(Led_Status == LED_LIGHTDOWM)
    {
        GPIO_SetBits(GPIOC, GPIO_Pin_5);//LED5灭
    }
       
} 

void Led6_RunTask_Op(u8 Led_Status)//水平驱动器指示灯
{
    u8 byBitVal;
    if(Led_Status == LED_FLASH)//闪烁
    {
        byBitVal = GPIO_ReadOutputDataBit(GPIOB,GPIO_Pin_0);
    
        GPIO_WriteBit(GPIOB,GPIO_Pin_0,(BitAction)(!byBitVal));
    }
    else if(Led_Status == LED_LIGHTUP)
    {
    	GPIO_ResetBits(GPIOB, GPIO_Pin_0);//LED6亮
    }
    else if(Led_Status == LED_LIGHTDOWM)
    {
        GPIO_SetBits(GPIOB, GPIO_Pin_0);//LED6灭
    }
    
} 
/*
*******************************************************************************
* Function Name  : LED_Status_Control
* Description    : LED闪灯功能控制
* Input          : None
                
* Output         : None
* Return         : None
*******************************************************************************
*/
void LED_Status_Control(void)
{ 
    gs_Motor_Param.dwLedRun_Cnt++;
    
    switch(gs_Motor_Param.byLedFlashCtrlFlag)
    {      
        case LED_IDLE://待机状态，每秒闪1次
        {
            gs_Motor_Param.dwLedRun_Cnt = 0;
            Led1_RunTask_Op(LED_FLASH);//运行状态灯闪烁
        }
        break;
        
        case LED_HALL_ERR://霍尔错误，每8秒闪1次 1
        {
            if((7 == gs_Motor_Param.dwLedRun_Cnt)\
                ||(8 == gs_Motor_Param.dwLedRun_Cnt))//第3.5s~4.5s亮灯
            {
                Led1_RunTask_Op(LED_LIGHTUP);//亮灯
            }
            else
            {
                Led1_RunTask_Op(LED_LIGHTDOWM);//灭灯
            }
            
            if(gs_Motor_Param.dwLedRun_Cnt >= 16)    
            {
                gs_Motor_Param.dwLedRun_Cnt = 0;
            }
        }
        break;
        
        case LED_OVERLOAD://过流保护，每8秒闪3次 3
        {
            if((3 == gs_Motor_Param.dwLedRun_Cnt)\
               ||(4 == gs_Motor_Param.dwLedRun_Cnt)\
               ||(7 == gs_Motor_Param.dwLedRun_Cnt)\
               ||(8 == gs_Motor_Param.dwLedRun_Cnt)\
               ||(11 == gs_Motor_Param.dwLedRun_Cnt)\
               ||(12 == gs_Motor_Param.dwLedRun_Cnt))//第1.5~2.5 3.5s~4.5s,5.5s~6.5s亮灯
            {
                Led1_RunTask_Op(LED_LIGHTUP);//亮灯
            }
            else
            {
                Led1_RunTask_Op(LED_LIGHTDOWM);//灭灯
            }
                
            if(gs_Motor_Param.dwLedRun_Cnt >= 16)    
            {
                gs_Motor_Param.dwLedRun_Cnt = 0;
            }
        }
        break;
        
        case LED_RUN_MODE://正常，周期闪烁，转速  10
        {
            gs_Motor_Param.dwLedRun_Cnt = 0;
            Led1_RunTask_Op(LED_FLASH);//运行状态灯闪烁
        }
        break;
        
        default:
        {
            gs_Motor_Param.dwLedRun_Cnt = 0;
            Led1_RunTask_Op(LED_FLASH);//运行状态灯闪烁
        }
        break;
    }
    
}

/************** (C) COPYRIGHT 2015 SOLAR SUPCON ************END OF FILE********/
