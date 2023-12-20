/*
********************** (C) COPYRIGHT 2019 SOLAR SUPCON *************************
* File Name          : Hardware_CONFIG.c
* Author             : dingyongjian 
* Date First Issued  : 02/01/19
* Description        : 硬件外设初始化 C文件
********************************************************************************
* History:
* 02/01/19 v1.0
********************************************************************************
*/
#define Hardware_CONFIG_GLOBALS

/* Includes ------------------------------------------------------------------*/
#include "include.h"
#include "stm32f10x_adc.h"
#include "stm32f10x_gpio.h"
#include "main.h"

//#define ADC1_DR_Address    ((u32)0x4001244c)
#define AVG_SLOPE     43 //4.3mv
#define TEMP_V25      143000//温度传感器在25℃时的输出电压，典型值1.43 V
//#define ADC_TURN_VOL  ((s32)(330000/4096))

u16 g_wADC_ConvertedValue[4];


/* Private functions ------------------------------------------------------- */
/*
******************************************************************************
* 函 数 名 ：  STM32_GPIO_CONF
* 描    述 ：  外设初始化
* 硬件平台 ：  
* 硬件连接：   -------------------------
                |  PA4  -  Mcu_LED1   |//运行灯
                |  PA6  -  Mcu_LED2   |//故障指示灯
                |  PA7  -  Mcu_LED3   |
                |  PC4  -  Mcu_LED4   |
                |  PC5  -  Mcu_LED5   |
                |  PB0  -  Mcu_LED6   |
               -------------------------
* 库 版 本 ：  ST3.5.0
* 作    者 ：  
* 日    期 ：  2013年3月25日  这个只管通用的IO脚 的定义
* 版    本 ：  0.10
******************************************************************************
*/
void STM32_GPIO_Config(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    
    /*开启GPIOA时钟信号*/
    RCC_APB2PeriphClockCmd( RCC_APB2Periph_GPIOA,ENABLE );
    /*开启GPIOB时钟信号*/
    RCC_APB2PeriphClockCmd( RCC_APB2Periph_GPIOB,ENABLE );
    /*开启GPIOC时钟信号*/
    RCC_APB2PeriphClockCmd( RCC_APB2Periph_GPIOC,ENABLE );
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO ,ENABLE);
    
    
    GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable, ENABLE);//GPIOB3,B4为TDO和 NRJRST
    /*地址拨码*/
    GPIO_InitStructure.GPIO_Pin = DEVICE_ADDR1_PIN | DEVICE_ADDR2_PIN | DEVICE_ADDR4_PIN\
                                  | DEVICE_ADDR8_PIN | DEVICE_ADDR16_PIN | DEVICE_ADDR32_PIN | DEVICE_ADDR64_PIN; 
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING; 
    GPIO_Init(DEVICE_ADDR1_PORT, &GPIO_InitStructure); 
    /*LED  PB10*/
    GPIO_InitStructure.GPIO_Pin = LED_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;
    GPIO_Init(LED_PORT, &GPIO_InitStructure);
    GPIO_ResetBits(LED_PORT, LED_PIN);
}


////定时器2是一个1MHz的计时器GXD  100us  这一个定时器完成了好几个功能。作为任务调度
void Init_TIMER2(void)  
{
    TIM_TimeBaseInitTypeDef TIM_BaseInitStructure;
   
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);
    
    TIM_BaseInitStructure.TIM_Period = 100 - 1;           
    TIM_BaseInitStructure.TIM_Prescaler = 71;      
    TIM_BaseInitStructure.TIM_ClockDivision = 0;     
    TIM_BaseInitStructure.TIM_CounterMode = TIM_CounterMode_Down; 
    TIM_TimeBaseInit(TIM2, &TIM_BaseInitStructure); 
    
    
    TIM_ClearFlag(TIM2, TIM_FLAG_Update);
    TIM_ITConfig(TIM2, TIM_IT_Update, ENABLE);          ////100us中断一次
    
    TIM_Cmd(TIM2, ENABLE);
}

void Init_TIMER4(void)//1ms定时器  定时器用的倒也是简单
{
    TIM_TimeBaseInitTypeDef TIM_BaseInitStructure;
    
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE);
     
    TIM_BaseInitStructure.TIM_Period = 1000-1;           
    TIM_BaseInitStructure.TIM_Prescaler = 71;//
    TIM_BaseInitStructure.TIM_ClockDivision = 0;     
    TIM_BaseInitStructure.TIM_CounterMode = TIM_CounterMode_Down; 
    TIM_TimeBaseInit(TIM4, &TIM_BaseInitStructure); 
    
    TIM_ClearFlag(TIM4, TIM_FLAG_Update);
    TIM_ITConfig(TIM4, TIM_IT_Update, ENABLE);         ///中断允许
    
    TIM_Cmd(TIM4, ENABLE);
}
void NVIC_Configuration(void)//100us
{
    NVIC_InitTypeDef NVIC_InitStructure; 
    
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);           //设置中断优先级组
     
    /* Enable the TIM2 Interrupt    用来100us定时用的  */
    NVIC_InitStructure.NVIC_IRQChannel = TIM2_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);   
    
    /* Enable the TIM4 Interrupt        定时器4用来干什么的?   1ms定时用的      */
    NVIC_InitStructure.NVIC_IRQChannel = TIM4_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
     
    /*TIM1_CC_IRQn               定时器1比较中断     注意到定时器1比较特殊，不象通用定时器，通用定时器中断只有1个，而定时器1有4个单独的中断矢量        */
    NVIC_InitStructure.NVIC_IRQChannel = TIM1_CC_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
     
    /*刹车TIM1_BRK_IRQn*/
#if(MOTOR_BRAKE_ENABLE == 1)//关闭刹车功能   
    NVIC_InitStructure.NVIC_IRQChannel = TIM1_BRK_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
#endif
   
}



/*  
* 函数名：ADC1_GPIO_Config  
* 描述  ：使能 ADC1 和 DMA1 的时钟，初始化   
* 输入  : 无  
* 输出  ：无  
* 调用  ：内部调用  
*/
static void ADC1_GPIO_Config(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;   
       
    /* Enable DMA clock */  
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);   
       
    /* Enable ADC1 and GPIOC clock */  
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1 | RCC_APB2Periph_GPIOC,ENABLE);   
       
    /* Configure PC0  as analog input */     
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;   
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;   
    GPIO_Init(GPIOC, &GPIO_InitStructure);   // PC0,输入时不用设置速率 
    
    /* Configure P13  as analog input */  //TEMP   
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13;   
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;   
    GPIO_Init(GPIOC, &GPIO_InitStructure);   // PC0,输入时不用设置速率 
}

/* 函数名：ADC1_Mode_Config  
* 描述  ：配置 ADC1 的工作模式为 DMA 模式  
* 输入  : 无  
* 输出  ：无  
* 调用  ：内部调用  
*cpu温度值= （1.43V - ADC16值/4096*3.3）/Avg_Slope(4.3mV/摄氏度) + 25
*/ 
static void ADC1_Mode_Config(void)
{
    DMA_InitTypeDef DMA_InitStructure;   
    ADC_InitTypeDef ADC_InitStructure;   
       
    /* DMA channel1 configuration */  
    DMA_DeInit(DMA1_Channel1);   
    DMA_InitStructure.DMA_PeripheralBaseAddr = ADC1_DR_Address;  /*ADC地址*/   
    DMA_InitStructure.DMA_MemoryBaseAddr = (u32)g_wADC_ConvertedValue;/*内存地址*/   
    DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC; //外设为数据源  
    DMA_InitStructure.DMA_BufferSize = 4;   
    DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;/*外设地址固定*/   
    DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;  /*内存地址固定*/   
    DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord; //半字   
    DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;   
    DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;     //循环传输   
    DMA_InitStructure.DMA_Priority = DMA_Priority_High;   
    DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;   
    DMA_Init(DMA1_Channel1, &DMA_InitStructure); 
    
    /* Enable DMA channel1 */  
    DMA_Cmd(DMA1_Channel1, ENABLE);   
     
    /* ADC1 configuration */  
       
    ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;  /*独立 ADC 模式*/   
    ADC_InitStructure.ADC_ScanConvMode = ENABLE ;   /*禁止扫描模式，扫描模式用于多通道采集*/   
    ADC_InitStructure.ADC_ContinuousConvMode = ENABLE;  /*开启连续转换模式，即不停地进行 ADC 转换*/   
    ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None; /*不使用外部触发转换*/   
    ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;  /*采集数据右对齐*/   
    ADC_InitStructure.ADC_NbrOfChannel = 4;     /*要转换的通道数目 1*/   
    ADC_Init(ADC1, &ADC_InitStructure);   
       
    /*配置 ADC 时钟，为 PCLK2 的 4 分频，即 50/8Hz*/
    RCC_ADCCLKConfig(RCC_PCLK2_Div4); 
    /*配置 ADC1 的通道 0 为 55.5 个采样周期 */    
    ADC_RegularChannelConfig(ADC1, ADC_Channel_16, 1, ADC_SampleTime_55Cycles5); 
    ADC_RegularChannelConfig(ADC1, ADC_Channel_16, 2, ADC_SampleTime_55Cycles5); 
    ADC_RegularChannelConfig(ADC1, ADC_Channel_16, 3, ADC_SampleTime_55Cycles5);  
    ADC_RegularChannelConfig(ADC1, ADC_Channel_16, 4, ADC_SampleTime_55Cycles5);  
     
    
    /* 开启内部温度传感器和 Vrefint 通道 */
    ADC_TempSensorVrefintCmd(ENABLE);
    
    /* Enable ADC1 DMA */  
    ADC_DMACmd(ADC1, ENABLE);   
    DMA_Cmd(DMA1_Channel1, ENABLE);
    /* Enable ADC1 */  
    ADC_Cmd(ADC1, ENABLE);   
        
     /*复位校准寄存器 */      
    ADC_ResetCalibration(ADC1);
    /*等待校准寄存器复位完成 */  
    while(ADC_GetResetCalibrationStatus(ADC1));   
       
    /* ADC 校准 */  
    ADC_StartCalibration(ADC1);   
    /* 等待校准完成*/  
    while(ADC_GetCalibrationStatus(ADC1));   
       
    /* 由于没有采用外部触发，所以使用软件触发 ADC 转换 */    
    ADC_SoftwareStartConvCmd(ADC1, ENABLE);
}

/*  
* 函数名：ADC1_Init  
* 描述  ： ADC1 初始化  
* 输入  : 无  
* 输出  ：无  
* 调用  ：内部调用  
*/
void ADC1_Init(void)
{
    ADC1_GPIO_Config();
    ADC1_Mode_Config();
}

/* 函数名：u16 Get_Cpu_Temp(void)  
* 描述  ：配获取cpu温度  
* 输入  : 无  
* 输出  ：无  
* 调用  ：内部调用  
*cpu温度值= （1.43V - ADC16值/4096*3.3）/Avg_Slope(4.3mV/摄氏度) + 25；小数点1位
*/ 
s16 Get_Cpu_Temp(void)
{
    s32 Cpu_TempAdc = 0;
    u8 i = 0;
    
    for(i = 0; i < 4; i++)
    {
        Cpu_TempAdc += g_wADC_ConvertedValue[i];
    }
    
    Cpu_TempAdc = (Cpu_TempAdc >> 2);
    Cpu_TempAdc = TEMP_V25 - ((Cpu_TempAdc*330000) >> 12);
    Cpu_TempAdc = (Cpu_TempAdc / 43) + 250;
      
    return ((s16)Cpu_TempAdc);
    
}


/* 函数名：DebugUART_Configuration  
* 描述  ：配置 调试串口  
* 输入  : 无  
* 输出  ：无              这次用的CPU没有C端口，故这个是原先的无用的代码了
* 调用  ：内部调用  
*cpu温度值= （1.43V - ADC16值/4096*3.3）/Avg_Slope(4.3mV/摄氏度) + 25
*/ 
void DebugUART_Configuration(void)
{ 
    GPIO_InitTypeDef GPIO_InitStructure;
    USART_InitTypeDef USART_InitStructure; 
          
     //RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO,ENABLE); 
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC ,ENABLE); 
  
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10; 
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz; 
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP; 
    GPIO_Init(GPIOC, &GPIO_InitStructure); 
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11; 
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING; 
    GPIO_Init(GPIOC, &GPIO_InitStructure); 
    /*GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12; 
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 
    GPIO_Init(GPIOC, &GPIO_InitStructure);*/
    
     //GPIO_SetBits(GPIOC,GPIO_Pin_12);//
    /*开启USART4的时钟*/
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_UART4, ENABLE);

    USART_InitStructure.USART_BaudRate = 115200;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;//9位数据
    USART_InitStructure.USART_StopBits = USART_StopBits_1;//1位停止位
    USART_InitStructure.USART_Parity = USART_Parity_No;//无校验
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None; //硬件流控制失能
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx; //发送和接受使能
    USART_Init(UART4, &USART_InitStructure); 
    USART_ITConfig(UART4, USART_IT_RXNE, ENABLE);
    USART_Cmd(UART4, ENABLE); 
    USART_ClearITPendingBit(UART4, USART_IT_TC);//清除中断TC位
    
    //GPIO_SetBits(GPIOC,GPIO_Pin_12);//
}
/************** (C) COPYRIGHT 2015 SOLAR SUPCON ************END OF FILE********/
