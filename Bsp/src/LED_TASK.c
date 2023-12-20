/*
********************** (C) COPYRIGHT 2015 SOLAR *************************
* File Name          : LED_TASK.c
* Author             : dingyongjian 
* Date First Issued  : 02/02/15
* Description        : LED���� C�ļ�
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
* �� �� �� ��  Ledx_RunTask_Op
* ��    �� ��  LED����
* Ӳ�����ӣ�    -------------------------
                
               -------------------------
******************************************************************************
*/
void Led1_RunTask_Op(u8 Led_Status)//����״̬��
{
    u8 byBitVal;
    if(Led_Status == LED_FLASH)//��˸
    {
        byBitVal = GPIO_ReadOutputDataBit(LED_PORT,LED_PIN);
    
        GPIO_WriteBit(LED_PORT,LED_PIN,(BitAction)(!byBitVal));
    }
    else if(Led_Status == LED_LIGHTUP)
    {
    	GPIO_SetBits(LED_PORT, LED_PIN);//LED1��
    }
    else if(Led_Status == LED_LIGHTDOWM)
    {
        GPIO_ResetBits(LED_PORT, LED_PIN);//LED1��
    }
    
} 

void Led2_RunTask_Op(u8 Led_Status)//��Դ����ָʾ�� -������ͨѶ���ϵ�
{
    u8 byBitVal;
    if(Led_Status == LED_FLASH)//��˸
    {
        byBitVal = GPIO_ReadOutputDataBit(GPIOA,GPIO_Pin_6);
    
        GPIO_WriteBit(GPIOA,GPIO_Pin_6,(BitAction)(!byBitVal));
    }
    else if(Led_Status == LED_LIGHTUP)
    {
    	GPIO_ResetBits(GPIOA, GPIO_Pin_6);//LED2��
    }
    else if(Led_Status == LED_LIGHTDOWM)
    {
        GPIO_SetBits(GPIOA, GPIO_Pin_6);//LED2��
    }
    
} 

void Led3_RunTask_Op(u8 Led_Status)//����ͨѶ��
{
    u8 byBitVal;
    if(Led_Status == LED_FLASH)//��˸
    {
        byBitVal = GPIO_ReadOutputDataBit(GPIOA,GPIO_Pin_7);
    
        GPIO_WriteBit(GPIOA,GPIO_Pin_7,(BitAction)(!byBitVal));
    }
    else if(Led_Status == LED_LIGHTUP)
    {
    	GPIO_ResetBits(GPIOA, GPIO_Pin_7);//LED3��
    }
    else if(Led_Status == LED_LIGHTDOWM)
    {
        GPIO_SetBits(GPIOA, GPIO_Pin_7);//LED3��
    }
   
} 

void Led4_RunTask_Op(u8 Led_Status)//Ԥ��
{
    u8 byBitVal;
    if(Led_Status == LED_FLASH)//��˸
    {
        byBitVal = GPIO_ReadOutputDataBit(GPIOC,GPIO_Pin_4);
    
        GPIO_WriteBit(GPIOC,GPIO_Pin_4,(BitAction)(!byBitVal));
    }
    else if(Led_Status == LED_LIGHTUP)
    {
    	GPIO_ResetBits(GPIOC, GPIO_Pin_4);//LED4��
    }
    else if(Led_Status == LED_LIGHTDOWM)
    {
        GPIO_SetBits(GPIOC, GPIO_Pin_4);//LED4��
    }
   
} 

void Led5_RunTask_Op(u8 Led_Status)//��λ������ָʾ��
{
    u8 byBitVal;
    if(Led_Status == LED_FLASH)//��˸
    {
        byBitVal = GPIO_ReadOutputDataBit(GPIOC,GPIO_Pin_5);
    
        GPIO_WriteBit(GPIOC,GPIO_Pin_5,(BitAction)(!byBitVal));
    }
    else if(Led_Status == LED_LIGHTUP)
    {
    	GPIO_ResetBits(GPIOC, GPIO_Pin_5);//LED5��
    }
    else if(Led_Status == LED_LIGHTDOWM)
    {
        GPIO_SetBits(GPIOC, GPIO_Pin_5);//LED5��
    }
       
} 

void Led6_RunTask_Op(u8 Led_Status)//ˮƽ������ָʾ��
{
    u8 byBitVal;
    if(Led_Status == LED_FLASH)//��˸
    {
        byBitVal = GPIO_ReadOutputDataBit(GPIOB,GPIO_Pin_0);
    
        GPIO_WriteBit(GPIOB,GPIO_Pin_0,(BitAction)(!byBitVal));
    }
    else if(Led_Status == LED_LIGHTUP)
    {
    	GPIO_ResetBits(GPIOB, GPIO_Pin_0);//LED6��
    }
    else if(Led_Status == LED_LIGHTDOWM)
    {
        GPIO_SetBits(GPIOB, GPIO_Pin_0);//LED6��
    }
    
} 
/*
*******************************************************************************
* Function Name  : LED_Status_Control
* Description    : LED���ƹ��ܿ���
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
        case LED_IDLE://����״̬��ÿ����1��
        {
            gs_Motor_Param.dwLedRun_Cnt = 0;
            Led1_RunTask_Op(LED_FLASH);//����״̬����˸
        }
        break;
        
        case LED_HALL_ERR://��������ÿ8����1�� 1
        {
            if((7 == gs_Motor_Param.dwLedRun_Cnt)\
                ||(8 == gs_Motor_Param.dwLedRun_Cnt))//��3.5s~4.5s����
            {
                Led1_RunTask_Op(LED_LIGHTUP);//����
            }
            else
            {
                Led1_RunTask_Op(LED_LIGHTDOWM);//���
            }
            
            if(gs_Motor_Param.dwLedRun_Cnt >= 16)    
            {
                gs_Motor_Param.dwLedRun_Cnt = 0;
            }
        }
        break;
        
        case LED_OVERLOAD://����������ÿ8����3�� 3
        {
            if((3 == gs_Motor_Param.dwLedRun_Cnt)\
               ||(4 == gs_Motor_Param.dwLedRun_Cnt)\
               ||(7 == gs_Motor_Param.dwLedRun_Cnt)\
               ||(8 == gs_Motor_Param.dwLedRun_Cnt)\
               ||(11 == gs_Motor_Param.dwLedRun_Cnt)\
               ||(12 == gs_Motor_Param.dwLedRun_Cnt))//��1.5~2.5 3.5s~4.5s,5.5s~6.5s����
            {
                Led1_RunTask_Op(LED_LIGHTUP);//����
            }
            else
            {
                Led1_RunTask_Op(LED_LIGHTDOWM);//���
            }
                
            if(gs_Motor_Param.dwLedRun_Cnt >= 16)    
            {
                gs_Motor_Param.dwLedRun_Cnt = 0;
            }
        }
        break;
        
        case LED_RUN_MODE://������������˸��ת��  10
        {
            gs_Motor_Param.dwLedRun_Cnt = 0;
            Led1_RunTask_Op(LED_FLASH);//����״̬����˸
        }
        break;
        
        default:
        {
            gs_Motor_Param.dwLedRun_Cnt = 0;
            Led1_RunTask_Op(LED_FLASH);//����״̬����˸
        }
        break;
    }
    
}

/************** (C) COPYRIGHT 2015 SOLAR SUPCON ************END OF FILE********/
