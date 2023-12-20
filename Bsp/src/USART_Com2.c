/*
********************** (C) COPYRIGHT 2019 SOLAR SUPCON *************************
* File Name          : USART_Com2.c
* Author             : dingyongjian 
* Date First Issued  : 02/01/15
* Description        : 串口RS485通讯 C文件,与驱动器
********************************************************************************
* History:
* 02/01/19 v1.0
********************************************************************************
*/
#define USART2_GLOBALS

/* Includes-------------------------------------------------------------------*/
#include "include.h"
#include "main.h"
/* Private variables ---------------------------------------------------------*/
#define USART2_RXBUFFER_LEN  256      //USART接收区容量
static u8 s_byUsart2RxBuffer[USART2_RXBUFFER_LEN]; //USART接收缓冲区
#define USART2_TXBUFFER_LEN  256       //USART发送区容量
static u8 s_byUsart2TxBuffer[USART2_TXBUFFER_LEN]; //USART发送缓冲区

static FlagStatus s_bUsart2RxEndFlag = RESET;//接收结束标志         0
static FlagStatus s_bUsart2TxEndFlag = SET;//发送结束标志           1 

static u8 s_byUsart2StartTxDelay = 0;//开始发送延时
static u8 s_byUsart2TxEndDelay = 0;//结束发送延时


/*Private functions ---------------------------------------------------------*/
/*
*******************************************************************************
* Function Name  : Init_Usart2
* Description    : 配置Usart2
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************
*/
void Init_Usart2(void)
{
    GPIO_InitTypeDef GPIO_InitStructure; //RS485用到的GPIO 
    NVIC_InitTypeDef NVIC_InitStructure; //中断向量控制器，初始化中断优先级   
    USART_InitTypeDef USART_InitStructure;//USART配置
    
    /*开启RS485通讯时用到的GPIO的时钟*/
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB ,ENABLE);     
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA ,ENABLE);  
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO ,ENABLE); 
    
    /*初始化RS485通讯时用到的GPIO*/
        //GPIO_PinRemapConfig(GPIO_Remap_Usart2, ENABLE);

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2; 
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz; 
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP; 
    GPIO_Init(GPIOA, &GPIO_InitStructure); 
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3; 
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING; 
    GPIO_Init(GPIOA, &GPIO_InitStructure); 
    /*GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1; 
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 
    GPIO_Init(GPIOA, &GPIO_InitStructure); */           
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;            ///RS485的允许端在这里定义了
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 
    GPIO_Init(GPIOB, &GPIO_InitStructure);
    
    /*开启Usart2的时钟*/
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);

    /*配置Usart2*/
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
    USART_Init(USART2, &USART_InitStructure); 
    
    /*使能Usart2*/
    USART_Cmd(USART2, ENABLE); 
    
    /*开启DMA1的时钟*/
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);//**
    
    /*设置中断向量控制器*/
    NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;              ////串口中断允许了  具体那个特性允许了还要看另一个函数，比如发送中断，比如接收中断等等
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 5; 
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0; 
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE; 
    NVIC_Init(&NVIC_InitStructure);
    
    /*设置中断向量控制器*/
    NVIC_InitStructure.NVIC_IRQChannel = DMA1_Channel7_IRQn;//**           DMA中断也允许了   中断中做了什么看一下GXD  这个是发送DMA完成中断  但是接收DMA完成中断却是没有的
    NVIC_Init(&NVIC_InitStructure);                                        ///因为接收完成，是靠IDLE中断来实现的，因为你不知道会接收多少个字节过来，只有那个空闲中断才知道
    
    Usart2_RxEnable();                       ////RS485控制引脚
    Usart2_StartRx();      ///
}

/*接收使能*/
void Usart2_RxEnable(void)
{
    //GPIO_ResetBits(GPIOA,GPIO_Pin_1);//
    GPIO_ResetBits(GPIOB,GPIO_Pin_11);// 隔离485
}

/*发送使能*/
void Usart2_TxEnable(void)
{
    //GPIO_SetBits(GPIOA,GPIO_Pin_1);// 
    GPIO_SetBits(GPIOB,GPIO_Pin_11);//隔离485
}

/*
*******************************************************************************
* Function Name  : Usart2_StartRx
* Description    : USART开始接收    串口2的接收是DMA通道6
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************
*/
void Usart2_StartRx(void)
{
    DMA_InitTypeDef DMA_InitStructure;
  
    s_bUsart2RxEndFlag = RESET;       /////这个很重要
      
    DMA_DeInit(DMA1_Channel6);
    DMA_InitStructure.DMA_PeripheralBaseAddr = (u32)&(USART2->DR);              ///这个是串口的数据
    DMA_InitStructure.DMA_MemoryBaseAddr = (u32)(s_byUsart2RxBuffer);           ////这个是串口的接收缓冲区   
    DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;
    DMA_InitStructure.DMA_BufferSize = USART2_RXBUFFER_LEN;                   ///256
    DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;           
    DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;                     
    DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;     ///  这个值等于0 
    DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
    DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
    DMA_InitStructure.DMA_Priority = DMA_Priority_VeryHigh;  
    DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
    DMA_Init(DMA1_Channel6, &DMA_InitStructure);    

    USART_GetFlagStatus(USART2, USART_FLAG_IDLE);
    USART_ReceiveData(USART2);                        ///把这个串口读一下 DR寄存器  空读  在DMA之前把这个空读一下GXD
    USART_ITConfig(USART2, USART_IT_IDLE,ENABLE);    ////串口空闲中断被允许  在这个中断中做了什么呢？  串口中允许的是串口空闲中断，即没有串口收到信号时，变空闲时会有中断
                                                    
    USART_DMACmd(USART2,USART_DMAReq_Rx,ENABLE);    ////如果没有活动，则一开始，这个IDLE并不会变有效，而是在有数据之后，无数据了这个IDLE才会有效
    DMA_Cmd(DMA1_Channel6, ENABLE);
}

/*
*******************************************************************************
* Function Name  : Usart2_StartTx
* Description    : USART开始发送
* Input          : None                  串口2的发送通道是DMA通道7
* Output         : None
* Return         : None                   在发送中，是允许DMA通道7中断，但是在接收时，允许的可以串口中断
*******************************************************************************
*/
void Usart2_StartTx(u8 *pbyData, u16 wLength)          ////注意到这个参数长度很重要  要发多少字节就调用多少？
{
    DMA_InitTypeDef DMA_InitStructure;
    
    /*Usart2_TxEnable();
    
    s_byUsart2StartTxDelay = 3;
    
    s_bUsart2TxEndFlag = RESET;
    wLength = ( wLength < USART2_TXBUFFER_LEN) ? wLength : USART2_TXBUFFER_LEN;
    memcpy(s_byUsart2TxBuffer, pbyData, wLength);
    
    while(s_byUsart2StartTxDelay != 0)
    {
        ;
    }*/
    //2020-11-19修改  Is_Uart2StartTxDelayEnd   s_byUsart2StartTxDelay
    if(s_byUsart2StartTxDelay != 0)//延时未完成，退出       什么地方用到这个延时呢?       发送之前，先要等一下  因为如果还没发完，又马上进入的话。。。
    {
        return ;
    }
    
    s_bUsart2TxEndFlag = RESET;   ///你一旦要调用这个开始发送，此时就将这个发送完成标志设置为0  表示没有发送完成
    wLength = ( wLength < USART2_TXBUFFER_LEN) ? wLength : USART2_TXBUFFER_LEN;
    memcpy(s_byUsart2TxBuffer, pbyData, wLength);  
   
    DMA_DeInit(DMA1_Channel7);
    DMA_InitStructure.DMA_PeripheralBaseAddr = (u32)&(USART2->DR);
    DMA_InitStructure.DMA_MemoryBaseAddr = (u32)(s_byUsart2TxBuffer);
    DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralDST;
    DMA_InitStructure.DMA_BufferSize = wLength;                            ////这里就是要发送的字节的长度
    DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
    DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
    DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
    DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
    DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
    DMA_InitStructure.DMA_Priority = DMA_Priority_High; 
    DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
    DMA_Init(DMA1_Channel7, &DMA_InitStructure); 
    
    DMA_ClearFlag(DMA1_FLAG_GL7);
    DMA_ITConfig(DMA1_Channel7, DMA_IT_TC, ENABLE);  ///允许DMA中断  中断里面做了什么事呢？     除了清标志，就是 Usart2_TxEnd();  而这个是将 s_byUsart2TxEndDelay=10
                                                       ///也就是说在发送结束后，等一会就是接收了 这个要等的原因是串口还没发完，就将发送允许关的话，则发送还没有结束，结果发送没能成功发出去
    USART_DMACmd(USART2,USART_DMAReq_Tx,ENABLE); 
    DMA_Cmd(DMA1_Channel7, ENABLE);
}

/*接收结束标志    这个初始化之后的值是RESET  表示没有接收结束      这个函数中空闲中断中调用 表示结束了      */
void Usart2_RxEnd(void)
{
    s_bUsart2RxEndFlag = SET;
}

/*发送结束标志        */
void Usart2_TxEnd(void)
{
    s_byUsart2TxEndDelay = 10;
}

/*开始发送延迟            定时器2中不断的调用这个函数  */
void Usart2_StartTxDelay(void)
{
    if( s_byUsart2StartTxDelay > 0)
    {
        s_byUsart2StartTxDelay--;
    }
}

/*发送结束延时 发送延时结束，这个接收会被允许       这个函数一直被调用的 在定时器2中断中 */
void Usart2_TxEndDelay(void)
{
    if( s_byUsart2TxEndDelay > 0)
    {
        s_byUsart2TxEndDelay--;
        
        if(s_byUsart2TxEndDelay == 0)
        {
            s_bUsart2TxEndFlag = SET;
            gs_Motor_Param.bySendDataFlag = ACK_IDLE;//应答空闲    即给个标志，  又给个状态 
            Usart2_RxEnable();
        }
    }
}

/*
*******************************************************************************
* Function Name  : Is_Uart4StartTxDelayEnd(void)
* Description    : USART发送前操作
* Input          : None
* Output         : None
* Return         : trun or false
*******************************************************************************
*/
FlagStatus Is_Uart2StartTxDelayEnd(void)
{     
    if(0 == s_byUsart2StartTxDelay)//延时完成
    {
        return SET;
    }   
    
    return RESET;
}

/*
*******************************************************************************
* Function Name  : Usart2_ReadForTx
* Description    : USART发送前操作
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************
*/
void Usart2_ReadForTx(void)
{     
    Usart2_TxEnable();
    
    s_byUsart2StartTxDelay = 6;
    
   // s_bUsart2TxEndFlag = RESET;       
}

/*判断接收是否已经完成    完成了等于1     */
FlagStatus Is_Usart2_RxEnd(void)
{
    return ((s_bUsart2RxEndFlag == 1) ?  SET:RESET);
}

/*判断发送是否已经完成*/
FlagStatus Is_Usart2_TxEnd(void)
{
    return (s_bUsart2TxEndFlag == 1) ?  SET:RESET;
}

/*计算接收到的数据的长度*/
u16 Usart2_GetRxLength(void)
{
    return ((u16)(USART2_RXBUFFER_LEN - DMA_GetCurrDataCounter(DMA1_Channel6)));
}

/*把串口接收到的数据s_byUsart2RxBuffer拷贝到pbyData*/
void Usart2_GetRxData(u8 *pbyData, u16 wLength)
{
    memcpy(pbyData, s_byUsart2RxBuffer, wLength);
}


/*
*******************************************************************************
* Function Name  : ComMirrorGroup_Recv
* Description    : 接收镜组发送的数据
                   判断数据接收是否完成，并拷贝镜组数据到pbyData，再启动下一次接收
* Input          : None
* Output         : None        这个函数一直在被运行着的  一旦开始接收，就会将这个s_bUsart2RxEndFlag=0 于是下面这个函数一般情况下是直接返回，只有当这个空闲中断中才会置为1 于是触发调用条件
                                        基本上就是这个过程  搞清楚了接收部分

///  接收部分比较简单一些，在收到一串字符串之后，会产生IDLE中断，从而使得这个s_bUsart2RxEndFlag=1  于是在一个接收任务中，它是不断的查询这个标志的，一旦等于1，就会将数据收下来，然后
///  分析这个数据


///  发送部分呢，发送的话呢，就是等接收好了后，会在这个协议中，当发送空闲时，将这个 gs_Motor_Param.bySendDataFlag = PARAM_SET_ACK;//设置参数帧应答  
而一旦将这个值设过后，由于发送函数一直是作为一个任务 在运行的，于是
       if(Is_Usart2_TxEnd())//发送完成     看这个s_bUsart2TxEndFlag==1否？ s_bUsart2TxEndFlag初始化时就是1的，另外在发送完了等待时间过后，它也会置为1
---注意到在这个函数中，void Usart2_TxEnd(void)它将延时值设为10 即发送完成后 即发送完应答后，要等500毫秒才会重启接收，这个是故意为之，也就是说上位机如果收到了应答，要等500毫秒才能发送下一
条命令，这个要注意的，如果不等，立即发送，则会出错，因为我们的是单向的RS485

发送前先等一等，等个1000us  然后呢有就开始发送了，发送完成后，要等两个定时器，一个是500ms定时器，一个是600us定时器，只有这两个定时器都到了，我们才会
                               Usart2_StartRx();启动接收，并且Usart2_TxEnd();设置发送完成延时，这里这个延时是600us
注意到这个 Usart2_TxEnd()这个函数是在DMA通道7中调用的，只有当DMA发送完成后才会调用这个函数，进而启运一个1000us的定时，等这个定时到了后，才会有这个Is_Usart2_TxEnd()=1

///但是只有在这个发送开始函数中，才会让这个s_bUsart2TxEndFlag = RESET; 所以上面有些错了，我们再总结一下。是如何启动发送的：
////  第二次整理了


1.在接收到参数之后  if(ACK_IDLE == gs_Motor_Param.bySendDataFlag) 如果这个发送标志是空闲的，才会触发这个条件：然后 将 gs_Motor_Param.bySendDataFlag = PARAM_SET_ACK;
再看一下这个标志，上电时我们调用了这个   memset(&gs_Motor_Param,0,sizeof(S_MOTOR_PARAM)); 于是这个标志呢，也全部都是0的，所以在发送任务中，它一直是IDLE运行的。


2. 由于发送任务一直是在运行的，接到这个状态后 if(Is_Usart2_TxEnd())//发送完成     看这个s_bUsart2TxEndFlag==1否？ s_bUsart2TxEndFlag初始化时就是1的，另外在发送等待时间过后，它也会置为1
也就是说，如果发送完成了（比如上电后，这个就是1 另外，如果上一次发送完成了，并且等待时间也到了，这个也将置为1）注意这个等等时间是10个100us  这个判断在这个发送任务中多次调用，一定要把它搞清楚
所以在发送前，我们先要判断一下，这个标志是否为1 如果不为1的话，就在这里等着

3 然后进到Usart2_ReadForTx();//发送前准备   准备一是发送允许，另外是s_byUsart2StartTxDelay=6  开始发送前稍等片刻  发送之前为什么要等一下呢？担心前面没发送好吗还是？
//注意到我们发送前是等6个100us的。

4 终于发数据了，Send_ParamFrameAckData();//发送参数应答帧   Usart2_StartTx(Ack_Data,4); 在这个函数中 发送4个字节 但同时别忘记了还做了下列事情：
/// 一个重要的事情是：    s_bUsart2TxEndFlag = RESET;   发送结束标志设为无效，即未发送结束，
//  此外，它会判断       if(s_byUsart2StartTxDelay != 0)//延时未完成，退出 这个就是前面的6个100us结束否 感觉这里又判断一次是不是多余啊 因为前面那个
///        应答等待中已经有这个：if(Is_Uart2StartTxDelayEnd())//发送前延时完成   现在又搞一次有点逻辑多余啊？ 确实多余
///  说完这个，我们还要注意在这个发送过程中 允许了DMA中断，那个在这个DMA中断中做了什么呢？     s_byUsart2TxEndDelay = 10; 即将这个发送结束延时值给了一个10，即发送完后要等10个100us
///  很重要的是，在DMA完成中断后，至少要等10个100us  在这个时间延时后，它会做3件事情 见下面的6中描述了这三件重要的事情

5 SetSendAck_TimeOut(TIME_500MS)  后进入下一个状态，这个就是启动一个500ms的较长时间的等待。

6.if(Is_SendAckTimeout() && (!Is_Usart2_TxEnd())) 在这里，我两个条件都要满足，先香第2个，它在这个10个100us完成后会做这很重要的三件事情：
            s_bUsart2TxEndFlag = SET;       将标志设置一下  以后调用这个Is_Usart2_TxEnd（）就能表示结束了。
            gs_Motor_Param.bySendDataFlag = ACK_IDLE;//应答空闲    即给个标志，  又给个状态  这个状态，表示发送状态机将进入空闲状态中
            Usart2_RxEnable();              ///这个呢，就是将RS485的接收允许一下。
///         那么，这个500ms结束后会做什么重要事情呢？这个倒就是一个简单的延时。
////        这里有一个误区，就是这个第2个判断标志是非的----!Is_Usart2_TxEnd() 注意这个！符号，我们由于在发送函数中将这个值设为RESET，故这个条件肯定是满足的。

7  在这个完成后，就是最后一步了，做了下列事情：Usart2_StartRx();Usart2_TxEnd();
///       先看后面一个，s_byUsart2TxEndDelay = 10; 这个调用它，延时完成后就会执行前面三个重要的步骤。于是回到接收允许中去。整个过程结束了。

* Return         : None      注意这个函数是不断的在任务中运行的
*******************************************************************************
*/
FlagStatus Com2_Recv(u8 *pbyData, u16 *pwLength)
{
    if (Is_Usart2_RxEnd())                    //判断接收是否已经完成  s_bUsart2RxEndFlag这个标志来决定，而这个标志呢Usart2_RxEnd()中设置为1  而这个函数在IDLE中断中会被调用 故。。。
    {
        *pwLength = Usart2_GetRxLength();     //计算接收到的数据的长度
        
        *pwLength = (*pwLength) < MAX_LEN_CMD ? (*pwLength) : MAX_LEN_CMD;  //如果*pwLength大于MAX_LEN_CMD就取MAX_LEN_CMD；

         Usart2_GetRxData(pbyData, *pwLength); //把串口接收到的数据s_byUsart2RxBuffer拷贝到pbyData
        
         Usart2_StartRx();                     //启动下一次的接收  注意在这个函数中，会将这个s_bUsart2RxEndFlag=RESET  从而这个部分不再成立。这个就是重点
        
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
void Com2_Send(u8 *pbyData, u16 wLength)
{
    Usart2_StartTx(pbyData, wLength);
}

/************** (C) COPYRIGHT 2015 SOLAR SUPCON ************END OF FILE********/

