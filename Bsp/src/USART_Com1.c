/*
********************** (C) COPYRIGHT 2015 SOLAR SUPCON *************************
* File Name          : USART_Com1.c
* Author             : dingyongjian 
* Date First Issued  : 02/01/15
* Description        : ����RS485ͨѶ C�ļ�
********************************************************************************
* History:
* 02/01/19 v1.1
********************************************************************************
*/
#define USART1_GLOBALS

/* Includes-------------------------------------------------------------------*/
#include "include.h"
#include "main.h"
/* Private variables ---------------------------------------------------------*/
#define USART1_RXBUFFER_LEN  256      //USART����������
static u8 s_byUsart1RxBuffer[USART1_RXBUFFER_LEN]; //USART���ջ�����
#define USART1_TXBUFFER_LEN  256       //USART����������
static u8 s_byUsart1TxBuffer[USART1_TXBUFFER_LEN]; //USART���ͻ�����

static FlagStatus s_bUsart1RxEndFlag = RESET;//���ս�����־
static FlagStatus s_bUsart1TxEndFlag = SET;//���ͽ�����־

static u8 s_byUsart1StartTxDelay = 0;//��ʼ������ʱ
static u8 s_byUsart1TxEndDelay = 0;//����������ʱ


/*Private functions ---------------------------------------------------------*/
/*
*******************************************************************************
* Function Name  : Init_Usart1
* Description    : ����USART1
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************
*/
void Init_Usart1(void)
{
    GPIO_InitTypeDef GPIO_InitStructure; //RS485�õ���GPIO 
    NVIC_InitTypeDef NVIC_InitStructure; //�ж���������������ʼ���ж����ȼ�   
    USART_InitTypeDef USART_InitStructure;//USART����
    
    /*����RS485ͨѶʱ�õ���GPIO��ʱ��*/
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB ,ENABLE);     
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA ,ENABLE);  
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO ,ENABLE); 
    
    /*��ʼ��RS485ͨѶʱ�õ���GPIO*/
        //GPIO_PinRemapConfig(GPIO_Remap_USART1, ENABLE);

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9; 
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz; 
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP; 
    GPIO_Init(GPIOA, &GPIO_InitStructure); 
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10; 
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING; 
    GPIO_Init(GPIOA, &GPIO_InitStructure); 
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12; 
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 
    GPIO_Init(GPIOA, &GPIO_InitStructure); 
    
    /*����USART1��ʱ��*/
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);

    /*����USART1*/
    USART_InitStructure.USART_BaudRate = BAUDRATE_FRE;   //ͨѶ������
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
    /*��ʼ��USART3*/
    USART_Init(USART1, &USART_InitStructure); 
    
    /*ʹ��USART1*/
    USART_Cmd(USART1, ENABLE); 
    
    /*����DMA1��ʱ��*/
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);//**
    
    /*�����ж�����������*/
    NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn; 
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 5; 
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0; 
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE; 
    NVIC_Init(&NVIC_InitStructure);
    
    /*�����ж�����������*/
    NVIC_InitStructure.NVIC_IRQChannel = DMA1_Channel4_IRQn;//**
    NVIC_Init(&NVIC_InitStructure);
    
    Usart1_RxEnable();
    Usart1_StartRx();
}

/*����ʹ��*/
void Usart1_RxEnable(void)
{
    //GPIO_SetBits(GPIOA,GPIO_Pin_11);// 
   GPIO_ResetBits(GPIOA,GPIO_Pin_12);// ����485
}

/*����ʹ��*/
void Usart1_TxEnable(void)
{
   // GPIO_ResetBits(GPIOA,GPIO_Pin_11);// 
    GPIO_SetBits(GPIOA,GPIO_Pin_12);// ����485
}

/*
*******************************************************************************
* Function Name  : Usart1_StartRx
* Description    : USART��ʼ����
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************
*/
void Usart1_StartRx(void)
{
    DMA_InitTypeDef DMA_InitStructure;
  
    s_bUsart1RxEndFlag = RESET;
      
    DMA_DeInit(DMA1_Channel5);
    DMA_InitStructure.DMA_PeripheralBaseAddr = (u32)&(USART1->DR);
    DMA_InitStructure.DMA_MemoryBaseAddr = (u32)(s_byUsart1RxBuffer);
    DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;
    DMA_InitStructure.DMA_BufferSize = USART1_RXBUFFER_LEN;
    DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
    DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
    DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
    DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
    DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
    DMA_InitStructure.DMA_Priority = DMA_Priority_VeryHigh;  
    DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
    DMA_Init(DMA1_Channel5, &DMA_InitStructure);    

    USART_GetFlagStatus(USART1, USART_FLAG_IDLE);
    USART_ReceiveData(USART1);
    USART_ITConfig(USART1, USART_IT_IDLE,ENABLE);
    
    USART_DMACmd(USART1,USART_DMAReq_Rx,ENABLE); 
    DMA_Cmd(DMA1_Channel5, ENABLE);
}

/*
*******************************************************************************
* Function Name  : Usart1_StartTx
* Description    : USART��ʼ����
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************
*/
void Usart1_StartTx(u8 *pbyData, u16 wLength)
{
    DMA_InitTypeDef DMA_InitStructure;
    
    Usart1_TxEnable();
    
    s_byUsart1StartTxDelay = 3;
    
    s_bUsart1TxEndFlag = RESET;
    wLength = ( wLength < USART1_TXBUFFER_LEN) ? wLength : USART1_TXBUFFER_LEN;
    memcpy(s_byUsart1TxBuffer, pbyData, wLength);
    
    while(s_byUsart1StartTxDelay != 0)
    {
        ;
    }
   
    DMA_DeInit(DMA1_Channel4);
    DMA_InitStructure.DMA_PeripheralBaseAddr = (u32)&(USART1->DR);
    DMA_InitStructure.DMA_MemoryBaseAddr = (u32)(s_byUsart1TxBuffer);
    DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralDST;
    DMA_InitStructure.DMA_BufferSize = wLength;
    DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
    DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
    DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
    DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
    DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
    DMA_InitStructure.DMA_Priority = DMA_Priority_High; 
    DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
    DMA_Init(DMA1_Channel4, &DMA_InitStructure); 
    
    DMA_ClearFlag(DMA1_FLAG_GL4);
    DMA_ITConfig(DMA1_Channel4, DMA_IT_TC, ENABLE);
    
    USART_DMACmd(USART1,USART_DMAReq_Tx,ENABLE); 
    DMA_Cmd(DMA1_Channel4, ENABLE);
}

/*���ս�����־*/
void Usart1_RxEnd(void)
{
    s_bUsart1RxEndFlag = SET;
}

/*���ͽ�����־*/
void Usart1_TxEnd(void)
{
    s_byUsart1TxEndDelay = 3;
}

/*��ʼ�����ӳ�*/
void Usart1_StartTxDelay(void)
{
    if( s_byUsart1StartTxDelay > 0)
    {
        s_byUsart1StartTxDelay--;
    }
}

/*���ͽ�����ʱ*/
void Usart1_TxEndDelay(void)
{
    if( s_byUsart1TxEndDelay > 0)
    {
        s_byUsart1TxEndDelay--;
        
        if(s_byUsart1TxEndDelay == 0)
        {
            s_bUsart1TxEndFlag = SET;
            Usart1_RxEnable();
        }
    }
}

/*�жϽ����Ƿ��Ѿ����*/
FlagStatus Is_Usart1_RxEnd(void)
{
    return ((s_bUsart1RxEndFlag == 1) ?  SET:RESET);
}

/*�жϷ����Ƿ��Ѿ����*/
FlagStatus Is_Usart1_TxEnd(void)
{
    return (s_bUsart1TxEndFlag == 1) ?  SET:RESET;
}

/*������յ������ݵĳ���*/
u16 Usart1_GetRxLength(void)
{
    return ((u16)(USART1_RXBUFFER_LEN - DMA_GetCurrDataCounter(DMA1_Channel5)));
}

/*�Ѵ��ڽ��յ�������s_byUsart1RxBuffer������pbyData*/
void Usart1_GetRxData(u8 *pbyData, u16 wLength)
{
    memcpy(pbyData, s_byUsart1RxBuffer, wLength);
}


/*
*******************************************************************************
* Function Name  : ComMirrorGroup_Recv
* Description    : ���վ��鷢�͵�����
                   �ж����ݽ����Ƿ���ɣ��������������ݵ�pbyData����������һ�ν���
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************
*/
FlagStatus Com1_Recv(u8 *pbyData, u16 *pwLength)
{
    if (Is_Usart1_RxEnd())                    //�жϽ����Ƿ��Ѿ����
    {
        *pwLength = Usart1_GetRxLength();     //������յ������ݵĳ���
        
        *pwLength = (*pwLength) < MAX_LEN_CMD ? (*pwLength) : MAX_LEN_CMD;  //���*pwLength����MAX_LEN_CMD��ȡMAX_LEN_CMD��

         Usart1_GetRxData(pbyData, *pwLength); //�Ѵ��ڽ��յ�������s_byUsart1RxBuffer������pbyData
        
         Usart1_StartRx();                     //������һ�εĽ���
        
         return SET;                          //�����������ˣ�����TRUE
    }
    
    return RESET;                              //�������û����ɣ�����RESET
}

/*
*******************************************************************************
* Function Name  : ComMirrorGroup_Send
* Description    : �����鷢������
* Input          : �������ݰ��ĵ�ַ*pbyData
                   ���ݰ��ĳ���wLength
* Output         : None
* Return         : None
*******************************************************************************
*/
void Com1_Send(u8 *pbyData, u16 wLength)
{
    Usart1_StartTx(pbyData, wLength);
}

/************** (C) COPYRIGHT 2015 SOLAR SUPCON ************END OF FILE********/

