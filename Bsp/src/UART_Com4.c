/*
********************** (C) COPYRIGHT 2019 SOLAR SUPCON *************************
* File Name          : UART_Com4.c
* Author             : dingyongjian 
* Date First Issued  : 02/01/15
* Description        : ����RS485ͨѶ C�ļ�
********************************************************************************
* History:
* 02/01/19 v1.0
********************************************************************************
*/
#define UART4_GLOBALS

/* Includes-------------------------------------------------------------------*/
#include "include.h"
#include "main.h"
#include "stm32f10x_gpio.h"
/* Private variables ---------------------------------------------------------*/
#define UART4_RXBUFFER_LEN  256      //USART����������
static u8 s_byUart4RxBuffer[UART4_RXBUFFER_LEN]; //USART���ջ�����
#define UART4_TXBUFFER_LEN  256       //USART����������
static u8 s_byUart4TxBuffer[UART4_TXBUFFER_LEN]; //USART���ͻ�����

static FlagStatus s_bUart4RxEndFlag = RESET;//���ս�����־
static FlagStatus s_bUart4TxEndFlag = SET;//���ͽ�����־

static u8 s_byUart4StartTxDelay = 0;//��ʼ������ʱ
static u8 s_byUart4TxEndDelay = 0;//����������ʱ

/*Private functions ---------------------------------------------------------*/
/*
*******************************************************************************
* Function Name  : Init_Uart4
* Description    : ����UART4
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************
*/
void Init_Uart4(void)
{
    GPIO_InitTypeDef GPIO_InitStructure; //RS485�õ���GPIO 
    USART_InitTypeDef USART_InitStructure;//USART����
    NVIC_InitTypeDef NVIC_InitStructure; 
    
    /*����RS485ͨѶʱ�õ���GPIO��ʱ��*/
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC ,ENABLE);     
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB ,ENABLE);  
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOD ,ENABLE);
    
    /*��ʼ��RS485ͨѶʱ�õ���GPIO*/

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10; 
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz; 
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP; 
    GPIO_Init(GPIOC, &GPIO_InitStructure); 
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11; 
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING; 
    GPIO_Init(GPIOC, &GPIO_InitStructure); 
    
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2; 
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 
    GPIO_Init(GPIOD, &GPIO_InitStructure); 
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12; 
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 
    GPIO_Init(GPIOC, &GPIO_InitStructure); 
    //GPIO_ResetBits(GPIOC,GPIO_Pin_12);// 
   // GPIO_ResetBits(GPIOD,GPIO_Pin_2);// 
    
    /*����USART1��ʱ��*/
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_UART4, ENABLE);

//USART ��ʼ������
     /*����UART4*/
    USART_InitStructure.USART_BaudRate = BAUDRATE_FRE_DRV;//BAUDRATE_FRE;   //ͨѶ������115200->117187
    /*0��һ����ʼλ��8������λ��n��ֹͣλ*/
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    /*00��1��ֹͣλ��*/
    USART_InitStructure.USART_StopBits = USART_StopBits_1;  
    /*0����ֹУ�����*/
    USART_InitStructure.USART_Parity = USART_Parity_No;
    /*0����ֹCTSӲ�������ƣ�0����ֹRTSӲ�������ƣ�*/
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    /*1��ʹ�ܷ��͡�1��ʹ�ܽ��գ�����ʼ��ѰRX�����ϵ���ʼλ��*/
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx; 
    /*��ʼ��USART4*/
    USART_Init(UART4, &USART_InitStructure); 
   
    /*ʹ��UART4*/
    USART_Cmd(UART4, ENABLE); 
    
    /*����DMA2��ʱ��*/
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA2, ENABLE);//**
    
    /*�����ж�����������*/
    NVIC_InitStructure.NVIC_IRQChannel = UART4_IRQn; 
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 4; 
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0; 
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE; 
    NVIC_Init(&NVIC_InitStructure);
    
    /*�����ж�����������*/
    NVIC_InitStructure.NVIC_IRQChannel = DMA2_Channel4_5_IRQn;//**
    NVIC_Init(&NVIC_InitStructure);
	
    Uart4_RxEnable();
    Uart4_StartRx();

}

/*����ʹ��*/
void Uart4_RxEnable(void)
{
    GPIO_ResetBits(GPIOD,GPIO_Pin_2);// 
    GPIO_ResetBits(GPIOC,GPIO_Pin_12);// 
}

/*����ʹ��*/
void Uart4_TxEnable(void)
{
    GPIO_SetBits(GPIOD,GPIO_Pin_2);// 
    GPIO_SetBits(GPIOC,GPIO_Pin_12);// 
}

/*
*******************************************************************************
* Function Name  : Uart4_StartRx
* Description    : USART��ʼ����
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************
*/
void Uart4_StartRx(void)
{
    DMA_InitTypeDef DMA_InitStructure;
  
    s_bUart4RxEndFlag = RESET;
      
    DMA_DeInit(DMA2_Channel3);
    DMA_InitStructure.DMA_PeripheralBaseAddr = (u32)&(UART4->DR);
    DMA_InitStructure.DMA_MemoryBaseAddr = (u32)(s_byUart4RxBuffer);
    DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;
    DMA_InitStructure.DMA_BufferSize = UART4_RXBUFFER_LEN;
    DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
    DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
    DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
    DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
    DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
    DMA_InitStructure.DMA_Priority = DMA_Priority_VeryHigh;  
    DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
    DMA_Init(DMA2_Channel3, &DMA_InitStructure);    

    USART_GetFlagStatus(UART4, USART_FLAG_IDLE);
    USART_ReceiveData(UART4);
    USART_ITConfig(UART4, USART_IT_IDLE,ENABLE);
    
    USART_DMACmd(UART4,USART_DMAReq_Rx,ENABLE); 
    DMA_Cmd(DMA2_Channel3, ENABLE);
}


/*
*******************************************************************************
* Function Name  : Uart4_StartTx
* Description    : USART��ʼ����
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************
*/
void Uart4_StartTx(u8 *pbyData, u16 wLength)
{  
    DMA_InitTypeDef DMA_InitStructure;
    
    Uart4_TxEnable();
    
    s_byUart4StartTxDelay = 6;//3->5
    
    s_bUart4TxEndFlag = RESET;
    wLength = ( wLength < UART4_TXBUFFER_LEN) ? wLength : UART4_TXBUFFER_LEN;
    memcpy(s_byUart4TxBuffer, pbyData, wLength);
    
    while(s_byUart4StartTxDelay != 0)
    {
        ;
    }
   
    DMA_DeInit(DMA2_Channel5);
    DMA_InitStructure.DMA_PeripheralBaseAddr = (u32)&(UART4->DR);
    DMA_InitStructure.DMA_MemoryBaseAddr = (u32)(s_byUart4TxBuffer);
    DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralDST;
    DMA_InitStructure.DMA_BufferSize = wLength;
    DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
    DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
    DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
    DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
    DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
    DMA_InitStructure.DMA_Priority = DMA_Priority_High; 
    DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
    DMA_Init(DMA2_Channel5, &DMA_InitStructure); 
    
    DMA_ClearFlag(DMA2_FLAG_GL5);
    DMA_ITConfig(DMA2_Channel5, DMA_IT_TC, ENABLE);
    
    USART_DMACmd(UART4,USART_DMAReq_Tx,ENABLE); 
    DMA_Cmd(DMA2_Channel5, ENABLE);
    
}

//2020-7-6����
/*
*******************************************************************************
* Function Name  : Uart4_ReadForTx
* Description    : USART����ǰ����
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************
*/
void Uart4_ReadForTx(void)
{     
    Uart4_TxEnable();
    
    s_byUart4StartTxDelay = 6;
    
    s_bUart4TxEndFlag = RESET;       
}

/*
*******************************************************************************
* Function Name  : Is_Uart4StartTxDelayEnd(void)
* Description    : USART����ǰ����
* Input          : None
* Output         : None
* Return         : trun or false
*******************************************************************************
*/
FlagStatus Is_Uart4StartTxDelayEnd(void)
{     
    if(s_byUart4StartTxDelay == 0)//��ʱ���
    {
        return SET;
    }   
    
    return RESET;
}

/*
*******************************************************************************
* Function Name  : Uart4_TripTx
* Description    : USART��������
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************
*/
void Uart4_TripTx(u8 *pbyData, u16 wLength)
{   
    DMA_InitTypeDef DMA_InitStructure;          
    
    if(s_byUart4StartTxDelay != 0)//��ʱδ��ɣ��˳�
    {
        return ;
    }
    
    wLength = ( wLength < UART4_TXBUFFER_LEN) ? wLength : UART4_TXBUFFER_LEN;
    memcpy(s_byUart4TxBuffer, pbyData, wLength);  
    
    DMA_DeInit(DMA2_Channel5);
    DMA_InitStructure.DMA_PeripheralBaseAddr = (u32)&(UART4->DR);
    DMA_InitStructure.DMA_MemoryBaseAddr = (u32)(s_byUart4TxBuffer);
    DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralDST;
    DMA_InitStructure.DMA_BufferSize = wLength;
    DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
    DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
    DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
    DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
    DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
    DMA_InitStructure.DMA_Priority = DMA_Priority_High; 
    DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
    DMA_Init(DMA2_Channel5, &DMA_InitStructure); 
    
    DMA_ClearFlag(DMA2_FLAG_GL5);
    DMA_ITConfig(DMA2_Channel5, DMA_IT_TC, ENABLE);
    
    USART_DMACmd(UART4,USART_DMAReq_Tx,ENABLE); 
    DMA_Cmd(DMA2_Channel5, ENABLE);
    
}
//--------------------------//

/*���ս�����־*/
void Uart4_RxEnd(void)
{
    s_bUart4RxEndFlag = SET;
}

/*���ͽ�����־*/
void Uart4_TxEnd(void)
{
    s_byUart4TxEndDelay = 3;
}

/*��ʼ�����ӳ�*/
void Uart4_StartTxDelay(void)
{
    if( s_byUart4StartTxDelay > 0)
    {
        s_byUart4StartTxDelay--;
    }
}

/*���ͽ�����ʱ*/
void Uart4_TxEndDelay(void)
{
    if( s_byUart4TxEndDelay > 0)
    {
        s_byUart4TxEndDelay--;
        
        if(s_byUart4TxEndDelay == 0)
        {
            s_bUart4TxEndFlag = SET;
            Uart4_RxEnable();
        }
    }
}



/*�жϽ����Ƿ��Ѿ����*/
FlagStatus Is_Uart4_RxEnd(void)
{
    return ((s_bUart4RxEndFlag == 1) ?  SET:RESET);
}

/*�жϷ����Ƿ��Ѿ����*/
FlagStatus Is_Uart4_TxEnd(void)
{
    return (s_bUart4TxEndFlag == 1) ?  SET:RESET;
}

/*������յ������ݵĳ���*/
u16 Uart4_GetRxLength(void)
{
    return ((u16)(UART4_RXBUFFER_LEN - DMA_GetCurrDataCounter(DMA2_Channel3)));;
}

/*�Ѵ��ڽ��յ�������s_byUart4RxBuffer������pbyData*/
void Uart4_GetRxData(u8 *pbyData, u16 wLength)
{
    memcpy(pbyData, s_byUart4RxBuffer, wLength);
}


/*
*******************************************************************************
* Function Name  : Com4_Recv
* Description    : �����ֲ������͵�����
                   �ж����ݽ����Ƿ���ɣ��������������ݵ�pbyData����������һ�ν���
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************
*/
FlagStatus Com4_Recv(u8 *pbyData, u16 *pwLength)
{
    if (Is_Uart4_RxEnd())                    //�жϽ����Ƿ��Ѿ����
    {
        *pwLength = Uart4_GetRxLength();     //������յ������ݵĳ���
        
        *pwLength = (*pwLength) < MAX_LEN_CMD ? (*pwLength) : MAX_LEN_CMD;  //���*pwLength����MAX_LEN_CMD��ȡMAX_LEN_CMD��

         Uart4_GetRxData(pbyData, *pwLength); //�Ѵ��ڽ��յ�������s_byUart4RxBuffer������pbyData
        
         Uart4_StartRx();                     //������һ�εĽ���
        
         return SET;                          //�����������ˣ�����TRUE
    }
    
    return RESET;                              //�������û����ɣ�����RESET
}

/*
*******************************************************************************
* Function Name  : Com4_Send
* Description    : ��������
* Input          : �������ݰ��ĵ�ַ*pbyData
                   ���ݰ��ĳ���wLength
* Output         : None
* Return         : None
*******************************************************************************
*/
void Com4_Send(u8 *pbyData, u16 wLength)
{
#if(COM4_TXMODE_CHANGE_ENABLE == 0) //Ĭ��ԭ��ģʽ
    Uart4_StartTx(pbyData, wLength);
#endif
    
#if(COM4_TXMODE_CHANGE_ENABLE == 1) //2020-7-6�������޸�  
    //2020-7-6�޸�
    Uart4_TripTx(pbyData, wLength);
#endif
}



/*
*******************************************************************************
* Function Name  : UART4_IRQHandler
* Description    : uart4 �������/��������жϴ�����
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************
*/
void UART4_IRQHandler(void)  
{      
    USART_ITConfig(UART4,USART_IT_IDLE,DISABLE);
    
    Uart4_RxEnd();
}

/*
*******************************************************************************
* Function Name  : DMA2_Channel4_5_IRQHandler
* Description    : uart4 DMA��������жϴ�����
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************
*/
void DMA2_Channel4_5_IRQHandler(void)
{ 
    DMA_ClearITPendingBit(DMA2_IT_TC5);  
    
    Uart4_TxEnd();
}
/*
*******************************************************************************
* Function Name  : Uart4_Test_Op
* Description    : uart4 ����
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************
*/
/*static u8 s_byManual_ControlData[UART4_RXBUFFER_LEN];
static u16 s_wManual_DataLen = 0;
void Uart4_Test_Op(void)
{
    if(Com4_Recv(s_byManual_ControlData,&s_wManual_DataLen))
    {
        Com4_Send(s_byManual_ControlData,s_wManual_DataLen);
    }
}*/
/************** (C) COPYRIGHT 2015 SOLAR SUPCON ************END OF FILE********/

