/*
********************** (C) COPYRIGHT 2015 SOLAR SUPCON *************************
* File Name          : USART_Com1.c
* Author             : dingyongjian 
* Date First Issued  : 02/01/15
* Description        : 串口RS485通讯 C文件
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
#define USART1_RXBUFFER_LEN  256      //USART接收区容量
static u8 s_byUsart1RxBuffer[USART1_RXBUFFER_LEN]; //USART接收缓冲区
#define USART1_TXBUFFER_LEN  256       //USART发送区容量
static u8 s_byUsart1TxBuffer[USART1_TXBUFFER_LEN]; //USART发送缓冲区

static FlagStatus s_bUsart1RxEndFlag = RESET;//接收结束标志
static FlagStatus s_bUsart1TxEndFlag = SET;//发送结束标志

static u8 s_byUsart1StartTxDelay = 0;//开始发送延时
static u8 s_byUsart1TxEndDelay = 0;//结束发送延时


/*Private functions ---------------------------------------------------------*/
/*
*******************************************************************************
* Function Name  : Init_Usart1
* Description    : 配置USART1
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************
*/
void Init_Usart1(void)
{
    GPIO_InitTypeDef GPIO_InitStructure; //RS485用到的GPIO 
    NVIC_InitTypeDef NVIC_InitStructure; //中断向量控制器，初始化中断优先级   
    USART_InitTypeDef USART_InitStructure;//USART配置
    
    /*开启RS485通讯时用到的GPIO的时钟*/
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB ,ENABLE);     
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA ,ENABLE);  
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO ,ENABLE); 
    
    /*初始化RS485通讯时用到的GPIO*/
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
    
    /*开启USART1的时钟*/
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);

    /*配置USART1*/
    USART_InitStructure.USART_BaudRate = BAUDRATE_FRE;   //通讯波特率
    /*0：一个起始位，8个数据位，n个停止位*/
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    /*00：1个停止位；*/
    USART_InitStructure.USART_StopBits = USART_StopBits_1;  
    /*0：禁止校验控制*/
    USART_InitStructure.USART_Parity = USART_Parity_No;
    /*0：禁止CTS硬件流控制；0：禁止RTS硬件流控制；*/
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    /*1：使能发送。1：使能接收，并开始搜寻RX引脚上的起始位。*/
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx; 
    /*初始化USART3*/
    USART_Init(USART1, &USART_InitStructure); 
    
    /*使能USART1*/
    USART_Cmd(USART1, ENABLE); 
    
    /*开启DMA1的时钟*/
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);//**
    
    /*设置中断向量控制器*/
    NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn; 
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 5; 
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0; 
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE; 
    NVIC_Init(&NVIC_InitStructure);
    
    /*设置中断向量控制器*/
    NVIC_InitStructure.NVIC_IRQChannel = DMA1_Channel4_IRQn;//**
    NVIC_Init(&NVIC_InitStructure);
    
    Usart1_RxEnable();
    Usart1_StartRx();
}

/*接收使能*/
void Usart1_RxEnable(void)
{
    //GPIO_SetBits(GPIOA,GPIO_Pin_11);// 
   GPIO_ResetBits(GPIOA,GPIO_Pin_12);// 隔离485
}

/*发送使能*/
void Usart1_TxEnable(void)
{
   // GPIO_ResetBits(GPIOA,GPIO_Pin_11);// 
    GPIO_SetBits(GPIOA,GPIO_Pin_12);// 隔离485
}

/*
*******************************************************************************
* Function Name  : Usart1_StartRx
* Description    : USART开始接收
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
* Description    : USART开始发送
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

/*接收结束标志*/
void Usart1_RxEnd(void)
{
    s_bUsart1RxEndFlag = SET;
}

/*发送结束标志*/
void Usart1_TxEnd(void)
{
    s_byUsart1TxEndDelay = 3;
}

/*开始发送延迟*/
void Usart1_StartTxDelay(void)
{
    if( s_byUsart1StartTxDelay > 0)
    {
        s_byUsart1StartTxDelay--;
    }
}

/*发送结束延时*/
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

/*判断接收是否已经完成*/
FlagStatus Is_Usart1_RxEnd(void)
{
    return ((s_bUsart1RxEndFlag == 1) ?  SET:RESET);
}

/*判断发送是否已经完成*/
FlagStatus Is_Usart1_TxEnd(void)
{
    return (s_bUsart1TxEndFlag == 1) ?  SET:RESET;
}

/*计算接收到的数据的长度*/
u16 Usart1_GetRxLength(void)
{
    return ((u16)(USART1_RXBUFFER_LEN - DMA_GetCurrDataCounter(DMA1_Channel5)));
}

/*把串口接收到的数据s_byUsart1RxBuffer拷贝到pbyData*/
void Usart1_GetRxData(u8 *pbyData, u16 wLength)
{
    memcpy(pbyData, s_byUsart1RxBuffer, wLength);
}


/*
*******************************************************************************
* Function Name  : ComMirrorGroup_Recv
* Description    : 接收镜组发送的数据
                   判断数据接收是否完成，并拷贝镜组数据到pbyData，再启动下一次接收
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************
*/
FlagStatus Com1_Recv(u8 *pbyData, u16 *pwLength)
{
    if (Is_Usart1_RxEnd())                    //判断接收是否已经完成
    {
        *pwLength = Usart1_GetRxLength();     //计算接收到的数据的长度
        
        *pwLength = (*pwLength) < MAX_LEN_CMD ? (*pwLength) : MAX_LEN_CMD;  //如果*pwLength大于MAX_LEN_CMD就取MAX_LEN_CMD；

         Usart1_GetRxData(pbyData, *pwLength); //把串口接收到的数据s_byUsart1RxBuffer拷贝到pbyData
        
         Usart1_StartRx();                     //启动下一次的接收
        
         return SET;                          //如果接收完成了，返回TRUE
    }
    
    return RESET;                              //如果接收没有完成，返回RESET
}

/*
*******************************************************************************
* Function Name  : ComMirrorGroup_Send
* Description    : 给镜组发送数据
* Input          : 发送数据包的地址*pbyData
                   数据包的长度wLength
* Output         : None
* Return         : None
*******************************************************************************
*/
void Com1_Send(u8 *pbyData, u16 wLength)
{
    Usart1_StartTx(pbyData, wLength);
}

/************** (C) COPYRIGHT 2015 SOLAR SUPCON ************END OF FILE********/

