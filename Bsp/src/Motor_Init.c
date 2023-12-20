/*
********************** (C) COPYRIGHT 2015 SOLAR *************************
* File Name          : Motor_Init.c
* Author             : dingyongjian 
* Date First Issued  : 02/02/15
* Description        : LED驱动 C文件
********************************************************************************
* History:
* 02/02/15 v1.1
********************************************************************************
*/
#define MOTOR_INIT_GLOBALS

/* Includes ------------------------------------------------------------------*/
#include "include.h"
#include "stm32f10x_adc.h"
#include "main.h"

u16 g_dwAdc_CurrentValue[4] = {0,0,0,0};
u16 g_dwAdc_CurrentValueOffset[4] = {0,0,0,0};
/* Private functions ------------------------------------------------------- */
/**
  * 函数功能: 配置嵌套向量中断控制器NVIC
  * 输入参数: 无
  * 返 回 值: 无
  * 说    明：无
  */
static void HALL_TIMx_NVIC_Configuration(void)
{
    NVIC_InitTypeDef NVIC_InitStructure;
    
    /* 配置TIM为中断源 */
    NVIC_InitStructure.NVIC_IRQChannel = TIM3_IRQn;
          /* 设置抢占式优先级为0 */
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;
          /* 设置子优先级为0 */
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
          /* 使能中断通道 */
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
}

/*
******************************************************************************
* 函 数 名 ：  Hall_HallTimerInit
* 描    述 ：  HALL接口初始化  现在看起来，定时器3作为HALL定时器，目的是将三个输入异或起来，每次开始，都进行计数 每次中断，我们可以通过这个捕获到的计数得出速度
* 硬件连接：   
******************************************************************************
*/
void HALL_HallTimerInit(void)          ///这里用到的时钟应该是内部时钟，在那里看得出来呢？
{
   // TIM_TimeBaseInitTypeDef TIM_HALLTimeBaseInitStructure;
   // TIM_ICInitTypeDef       TIM_HALLICInitStructure;
    //NVIC_InitTypeDef        NVIC_InitHALLStructure;
    TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
    TIM_ICInitTypeDef        TIM_ICInitStructure;
    GPIO_InitTypeDef        GPIO_InitStructure;
  
    /* TIM3 clock source enable */
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);
    /* Enable GPIOA, clock */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
    /* Enable GPIOB, clock */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);

    GPIO_StructInit(&GPIO_InitStructure);
    /* Configure PA.06,07  PB.00 as Hall sensors input */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7;
//    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
//    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOB, &GPIO_InitStructure);

      /* 使能定时器时钟 */
    HALL_TIM_APBxClock_FUN(HALL_TIM_CLK,ENABLE);          ////这个是定时器3  TIMER3  这里就是使能定时器3的时钟 
    /* 定时器基本参数始终 */		 
    /* 定时周期: HALL_TIM_Period+1 */
    TIM_TimeBaseStructure.TIM_Period = HALL_TIM_PERIOD;         ///  0xFFFF  设为最大
    /* 设置预分频：HALL_TIM_Prescaler,输出脉冲频率：72MHz/(HALL_TIM_Prescaler+1)/(HALL_TIM_Period+1) */
    TIM_TimeBaseStructure.TIM_Prescaler = HALL_TIM_PRESCALER;    ////预分频为72 
    /* 设置时钟分频系数：不分频(这里用不到) */
    TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1 ;     ////时钟就不分频了   
    /* 向上计数模式 */
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_CenterAligned1;     /////中央计数模式1----这是个什么计数模式呢？ 在计到某个值时 将触发更新信号 可是我们永远不会到那个值去的
    TIM_TimeBaseInit(HALL_TIMx, &TIM_TimeBaseStructure);                      

 ///前面几个好理解，就是下面这几行为关键了：   
    
    /* 初始化TIM5输入捕获参数 */
    /* CC1S=01 	选择输入端 IC1映射到TI1上 */
    TIM_ICInitStructure.TIM_Channel     = HALL_TIM_Channel_x;        /// 通道1   TIM_Channel_1
    /* 上升沿捕获 */
    TIM_ICInitStructure.TIM_ICPolarity  = TIM_ICPolarity_BothEdge;   ////0x000A  这是什么意思呢？双边沿 没理解清楚   0x000A  感觉这里有问题，没法设成双边沿触发 事实上这里可能设成单边沿也可以吧？没试过有机会试一下
    /* 映射到TI1上 */
    TIM_ICInitStructure.TIM_ICSelection = TIM_ICSelection_TRC;    ////   来自从模式控制器 TRC      CC1S=11  CC1通道被配置为输入，IC1映射在TRC上。此模式仅工作在内部触发器输入被选中时(由
                                                                  ////                                  TIMx_SMCR寄存器的TS位选择)
    /* 配置输入分频,不分频  */
    TIM_ICInitStructure.TIM_ICPrescaler = TIM_ICPSC_DIV1;	           
    /* IC1F=0000 配置输入滤波器 不滤波 */
    TIM_ICInitStructure.TIM_ICFilter    = 0x08;                         
    TIM_ICInit(HALL_TIMx, &TIM_ICInitStructure);  
    
    /* 配置NVIC       这个就是让这个定时器3的总的中断允许   NVIC中断   */
    HALL_TIMx_NVIC_Configuration();          
    
    TIM_SelectHallSensor(HALL_TIMx,ENABLE);                      //使能TIMx的霍尔传感器接口       就是TIS位被选择了
    TIM_SelectInputTrigger(HALL_TIMx, TIM_TS_TI1F_ED);         	//输入触发源选择     ----------TI1的边沿检测  0x0040  在这里设置了TS[2:0]=100  即触发源是TI1F_ED 
    
          TIM_SelectSlaveMode(HALL_TIMx, TIM_SlaveMode_Reset);           /////   从模式选择  当触发信号来时，复位这个计数器 SMS位=100  注意到TS3个位只有在SMS=100时用不到吗？
    TIM_SelectMasterSlaveMode(HALL_TIMx, TIM_MasterSlaveMode_Enable);	//       主从模式选择    MSM 这个位意义不是很大
     /* 允许更新中断 ,允许CC1IE捕获中断	 */
    TIM_ITConfig(HALL_TIMx, TIM_IT_Trigger, ENABLE);         ////       注意到使能了触发中断，也就是HALL每被触发一次都会中断 ---中断程序里做了什么？
    /* 使能定时器 */
    TIM_Cmd(HALL_TIMx, ENABLE);
    TIM_ClearITPendingBit (HALL_TIMx,TIM_IT_Trigger);
    
/*
    // Timer configuration in Clear on capture mode
    TIM_DeInit(HALL_TIMER);
    
    TIM_TimeBaseStructInit(&TIM_HALLTimeBaseInitStructure);
    // Set full 16-bit working range
    TIM_HALLTimeBaseInitStructure.TIM_Period = U16_MAX;
    TIM_HALLTimeBaseInitStructure.TIM_ClockDivision = TIM_CKD_DIV1;	//设置数字滤波时间分频数
    TIM_TimeBaseInit(HALL_TIMER,&TIM_HALLTimeBaseInitStructure);
    
    TIM_ICStructInit(&TIM_HALLICInitStructure);
    TIM_HALLICInitStructure.TIM_Channel = TIM_Channel_1;
    TIM_HALLICInitStructure.TIM_ICPolarity = TIM_ICPolarity_Falling; //每转120电角度，产生一次下降沿。
    TIM_HALLICInitStructure.TIM_ICFilter = ICx_FILTER;				 //所以每产生一次捕获中断，相当于马达转了120电角度。
    																 //但是换向是每60电角度就执行一次，因为三个输入信号
    TIM_ICInit(HALL_TIMER,&TIM_HALLICInitStructure);				 //是XOR后然后输入CHANEL1的。
    
    // Force the HALL_TIMER prescaler with immediate access (no need of an update event) 
    TIM_PrescalerConfig(HALL_TIMER, (u16) HALL_MAX_RATIO, 
                       TIM_PSCReloadMode_Immediate);
    TIM_InternalClockConfig(HALL_TIMER);
    
    //Enables the XOR of channel 1, channel2 and channel3
    TIM_SelectHallSensor(HALL_TIMER, ENABLE);
    
    TIM_SelectInputTrigger(HALL_TIMER, TIM_TS_TI1FP1);
    TIM_SelectSlaveMode(HALL_TIMER,TIM_SlaveMode_Reset);
   
    // Source of Update event is only counter overflow/underflow
	//更新中断源只是计数器上溢或者下溢。
    TIM_UpdateRequestConfig(HALL_TIMER, TIM_UpdateSource_Regular);
    
    // Enable the HALL_TIMER IRQChannel
    NVIC_InitHALLStructure.NVIC_IRQChannel = TIM3_IRQn;

  
    NVIC_InitHALLStructure.NVIC_IRQChannelPreemptionPriority = 
                                                      TIMx_PRE_EMPTION_PRIORITY;
    NVIC_InitHALLStructure.NVIC_IRQChannelSubPriority = TIMx_SUB_PRIORITY;
    NVIC_InitHALLStructure.NVIC_IRQChannelCmd = ENABLE;
    
    NVIC_Init(&NVIC_InitHALLStructure);

    // Clear the TIMx's pending flags
    TIM_ClearFlag(HALL_TIMER, TIM_FLAG_Update + TIM_FLAG_CC1 + TIM_FLAG_CC2 + \
                  TIM_FLAG_CC3 + TIM_FLAG_CC4 + TIM_FLAG_Trigger + TIM_FLAG_CC1OF + \
                  TIM_FLAG_CC2OF + TIM_FLAG_CC3OF + TIM_FLAG_CC4OF);
  
    // Selected input capture and Update (overflow) events generate interrupt
	//使能2个中断，最好分开2行写。
    TIM_ITConfig(HALL_TIMER, TIM_IT_CC1, ENABLE);
    TIM_ITConfig(HALL_TIMER, TIM_IT_Update, ENABLE);

    TIM_SetCounter(HALL_TIMER, HALL_COUNTER_RESET);
    TIM_Cmd(HALL_TIMER, ENABLE);*/
}


/**
  * 函数功能: 配置TIMx复用输出PWM时用到的I/O
  * 输入参数: 无
  * 返 回 值: 无
  * 说    明：无
  */
static void BLDC_TIMx_GPIO_Config(void) 
{
    GPIO_InitTypeDef GPIO_InitStructure;
  
          /* 使能定时器始终 */
    BLDC_TIM_APBxClock_FUN(BLDC_TIM_CLK, ENABLE);
  
    /* 使能定时器通道引脚GPIO时钟 */
    RCC_APB2PeriphClockCmd(BLDC_TIM_GPIO_CLK|RCC_APB2Periph_AFIO, ENABLE); 
  
    /* 配置定时器通道1输出引脚模式：复用推挽输出模式 */
    GPIO_InitStructure.GPIO_Pin =  BLDC_TIM_CH1_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(BLDC_TIM_CH1_PORT, &GPIO_InitStructure);
  
    /* 配置定时器通道2输出引脚模式 */
    GPIO_InitStructure.GPIO_Pin =  BLDC_TIM_CH2_PIN;
    GPIO_Init(BLDC_TIM_CH2_PORT, &GPIO_InitStructure);
  
    /* 配置定时器通道3输出引脚模式 */
    GPIO_InitStructure.GPIO_Pin =  BLDC_TIM_CH3_PIN;
    GPIO_Init(BLDC_TIM_CH3_PORT, &GPIO_InitStructure);

    // 配置定时器互补通道1输出引脚模式     
    GPIO_InitStructure.GPIO_Pin =  BLDC_TIM_CH1N_PIN;
    GPIO_Init(BLDC_TIM_CH1N_PORT, &GPIO_InitStructure); 
     
    // 配置定时器互补通道2输出引脚模式 
    GPIO_InitStructure.GPIO_Pin =  BLDC_TIM_CH2N_PIN;
    GPIO_Init(BLDC_TIM_CH2N_PORT, &GPIO_InitStructure); 
     
    // 配置定时器互补通道3输出引脚模式 
    GPIO_InitStructure.GPIO_Pin =  BLDC_TIM_CH3N_PIN;
    GPIO_Init(BLDC_TIM_CH3N_PORT, &GPIO_InitStructure);    
    
    /* Configuration: BKIN pin */
    GPIO_InitStructure.GPIO_Pin = BLDC_TIM_BKIN_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(BLDC_TIM_BKIN_PORT, &GPIO_InitStructure); 
    
     
  
}


/*电机电流ADC采样初始化         */
void MotorCurrentAdcChannel_Init(void)
{
    u8 i = 0;
    GPIO_InitTypeDef GPIO_InitStructure;
    //DMA_InitTypeDef DMA_InitStructure;
    ADC_InitTypeDef ADC_InitStructure; 
    NVIC_InitTypeDef NVIC_InitStructure; //中断向量控制器，初始化中断优先级
    
    /* Enable DMA clock */  
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);   
       
    /* Enable ADC1 and GPIOC clock */  
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1 | RCC_APB2Periph_GPIOB \
                           | RCC_APB2Periph_GPIOC,ENABLE);  
    
      /****** Configure PB,01(ADC Channels [.9]) as analog input ****/
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1 ;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
    
     /* Configure P13  as analog input */  //TEMP    没有这个温度吧 有这个管脚    但是这个脚没这个功能啊，怎么回事      
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13;   
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;   
    GPIO_Init(GPIOC, &GPIO_InitStructure);   // PC0,输入时不用设置速率
      /* ADC1 registers reset ----------------------------------------------------*/
    ADC_DeInit(ADC1);
  
    /* Enable ADC1 */
    ADC_Cmd(ADC1, ENABLE);
     
    /* ADC1 configuration */  
       
    ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;                        /*独立 ADC 模式*/   
    ADC_InitStructure.ADC_ScanConvMode = ENABLE ;                             /*禁止扫描模式，扫描模式用于多通道采集*/   
    ADC_InitStructure.ADC_ContinuousConvMode = DISABLE;                       /*开启连续转换模式，即不停地进行 ADC 转换*/   
    ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;        /*不使用外部触发转换*/   
    ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;                      /*采集数据右对齐*/   
    ADC_InitStructure.ADC_NbrOfChannel = 1;                            /*要转换的通道数目 1*/   
    ADC_Init(ADC1, &ADC_InitStructure);   
       
   
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

    /* ADC1 Injected group of conversions end interrupt disabling   注入通道 AD 中断没有允许   这是因为一开始是测试一下用软件启动方式*/
    ADC_ITConfig(ADC1, ADC_IT_JEOC, DISABLE);
  
    /* 开启内部温度传感器和 Vrefint 通道   不然这个没有上电  */
    ADC_TempSensorVrefintCmd(ENABLE);
    
    /*配置 ADC1 的通道 0 为 55.5 个采样周期 */    
   /* ADC_RegularChannelConfig(ADC1, ADC_Channel_16, 1, ADC_SampleTime_55Cycles5); 
    ADC_RegularChannelConfig(ADC1, ADC_Channel_16, 2, ADC_SampleTime_55Cycles5); 
    ADC_RegularChannelConfig(ADC1, ADC_Channel_16, 3, ADC_SampleTime_55Cycles5);  
    ADC_RegularChannelConfig(ADC1, ADC_Channel_16, 4, ADC_SampleTime_55Cycles5);*/
    ADC_ExternalTrigInjectedConvConfig(ADC1, ADC_ExternalTrigInjecConv_None);           ///没有指明触发通道
    ADC_ExternalTrigInjectedConvCmd(ADC1,ENABLE);                                    ///但是这个外部触发却是允许了  这两个函数我进去看了一下，一会设为无，一会设为有，神经病一样。
    
    ADC_InjectedSequencerLengthConfig(ADC1,2);
    ADC_InjectedChannelConfig(ADC1, ADC_Channel_16, 1, ADC_SampleTime_239Cycles5);  ////配置了温度和那个电流
    ADC_InjectedChannelConfig(ADC1, ADC_Channel_9, 2, ADC_SampleTime_239Cycles5);
    
    /* 开启内部温度传感器和 Vrefint 通道 */
   // ADC_TempSensorVrefintCmd(ENABLE);
   /* Clear the ADC1 JEOC pending flag */
    ADC_ClearFlag(ADC1, ADC_FLAG_JEOC);  
    ADC_SoftwareStartInjectedConvCmd(ADC1,ENABLE);
    
    for(i = 0; i < 4; i++)
    {
        g_wADC_ConvertedValue[i]      = ADC_GetInjectedConversionValue(ADC1,ADC_InjectedChannel_1);
        g_dwAdc_CurrentValueOffset[i] = ADC_GetInjectedConversionValue(ADC1,ADC_InjectedChannel_2);
        
        /* Clear the ADC1 JEOC pending flag */
        ADC_ClearFlag(ADC1, ADC_FLAG_JEOC);    
        ADC_SoftwareStartInjectedConvCmd(ADC1,ENABLE);
    }
    
    
    
     /*配置 ADC 时钟，为 PCLK2 的 8 分频，即 9Hz*/
    RCC_ADCCLKConfig(RCC_PCLK2_Div8); 
    /*配置 ADC1 的通道 0 为 55.5 个采样周期 */    
    
    ADC_InjectedSequencerLengthConfig(ADC1,2);
    ADC_InjectedChannelConfig(ADC1, ADC_Channel_16, 1, ADC_SampleTime_71Cycles5);           ///通道16
    ADC_InjectedChannelConfig(ADC1, ADC_Channel_9, 2, ADC_SampleTime_71Cycles5);            ////通道9就是PB1
    
     /* ADC1 Injected conversions trigger is TIM1 TRGO */ 
    ADC_ExternalTrigInjectedConvConfig(ADC1, ADC_ExternalTrigInjecConv_T1_CC4);         //// 定时器1的CC4就是AD的触发时钟
    ADC_ITConfig(ADC1, ADC_IT_JEOC, ENABLE);                                          //// ADC1的注入AD转换中断是允许的
    //ADC_SoftwareStartInjectedConvCmd(ADC1, ENABLE);
    /* 由于没有采用外部触发，所以使用软件触发 ADC 转换 */    
    //ADC_SoftwareStartConvCmd(ADC1, ENABLE);
   /* Enable the ADC1_2 Interrupt */
    NVIC_InitStructure.NVIC_IRQChannel =  ADC1_2_IRQn;		//开ADC1中断
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;    
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
     //Enable the DMA1_4 Interrupt 
   /* NVIC_InitStructure.NVIC_IRQChannel =  DMA1_Channel4_IRQn;		//开dma1_4中断
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;    
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);*/
}
/**
  * 函数功能: 配置TIMx输出的PWM信号的模式，如周期、极性、占空比
  * 输入参数: 无
  * 返 回 值: 无
  * 说    明：无
  */
/*
 * TIMxCLK/CK_PSC --> TIMxCNT --> TIMx_ARR --> TIMxCNT 重新计数
 *                    TIMx_CCR(电平发生变化)
 * 信号周期=(TIMx_ARR +1 ) * 时钟周期
 * 占空比=TIMx_CCR/(TIMx_ARR +1)
 */
static void BLDC_TIMx_Configuration(void)
{
    TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
    TIM_OCInitTypeDef  TIM_OCInitStructure;
    TIM_BDTRInitTypeDef TIM_BDTRInitStructure;
    
     
    /* 定时器基本参数始终 */		 
    TIM_TimeBaseStructure.TIM_Period = BLDC_TIM_PERIOD-1;            ////这个值的实际值为3600-1           经计算是20K频率  这个时间是50us
    /* 设置预分频：不预分频，即为72MHz */
    TIM_TimeBaseStructure.TIM_Prescaler = BLDC_TIM_PRESCALER;        ///不分频
    /* 设置时钟分频系数：不分频(这里用不到) */
    TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1 ;        ///不分频
    /* 向上计数模式 */
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;     ///这个是向上计数    但是那个HALL可不是  
    /* 重复计算器 */
    TIM_TimeBaseStructure.TIM_RepetitionCounter = BLDC_TIM_REPETITIONCOUNTER;    ////这个等于0 没用到
    TIM_TimeBaseInit(BLDC_TIMx, &TIM_TimeBaseStructure);
  
    /* 定时器输出通道1模式配置 */
    /* 模式配置：PWM模式1 */
    TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;              ////TIMx_CNT<TIMx_CCR1时通道1为有效电平
    /* 输出状态设置：使能输出 */
    TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;	
    /* 互补通道输出状态设置：使能输出 */
    TIM_OCInitStructure.TIM_OutputNState = TIM_OutputNState_Enable; ///两个输出都使能了

    /* 设置跳变值，当计数器计数到这个值时，电平发生跳变    这里为什么设置为0  先设为0 总是无效电平  */
    TIM_OCInitStructure.TIM_Pulse = 0;      ///这个值等于0的话，则意味着这个永远不会有那个有效电平了
    /* 当定时器计数值小于CCR1_Val时为高电平 */
    TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;          ///正极性输出是高电平有效，而负极性输出是低电平有效 所以当时间来时，一个变高一个变低从而不会短路
    TIM_OCInitStructure.TIM_OCNPolarity= TIM_OCNPolarity_Low;         //// 切记这个值=0008  也就是 CCxNP=1  故记住在这里，将这个值设成低电平有效
    TIM_OCInitStructure.TIM_OCIdleState = TIM_OCIdleState_Reset;     ///这个是用来干什么的呢  空闲时，它是复位的 也就是低电平    0：当MOE=0时，如果实现了OC1N，则死区后OC1=0；
    TIM_OCInitStructure.TIM_OCNIdleState = TIM_OCNIdleState_Set;//0x0200    /// 空闲时 这个N为高电平 由于我们MOE现等于0 故这个是设初始化后的状态   //  1：当MOE=0时，如果实现了OC1N，则死区后OC1=1
    /* 初始化定时器通道1输出PWM */                              /// 1：当MOE=0时，死区后OC1N=1。 也就是说，当MOE=0时，这个N的输出是高电平，也就是无效电平  这样两个管子都不导通。
    TIM_OC1Init(BLDC_TIMx, &TIM_OCInitStructure);                   //// 一旦将CCNP设为低电平有效，也就是将这个CCxNP=1 那么在CC1E和CC1NE=0的情况下:
                                                                     ////                                                    OCx=CCxP=0   OCxN=CCxNP=1   故此时是上管关闭，而下管也是关闭了。
    /* 定时器输出通道2模式配置 */                                    ////所以在初始化的情况下，这个管子全部是关闭的，至于真正运行之后，这个值就等于 OCxREF + 极性 + 死区和OCxREF反相 + 极性 + 死区了
    /* 设置通道2的电平跳变值，输出另外一个占空比的PWM */            ////真正运行时的情况 在换相时，我们6个输出全都是设过的。唯有其中的两个是允许，其它的4个都是DISABLE的
    TIM_OCInitStructure.TIM_Pulse = 0;                                              ////这也就是说，其它的4个输出都是置为不导通的状态的。只有两个导通，其中一个是一直通，另一个是按占空比通。
    /* 初始化定时器通道2输出PWM */                                                  ///
    TIM_OC2Init(BLDC_TIMx, &TIM_OCInitStructure);
  
    /* 定时器输出通道3模式配置 */
    /* 设置通道3的电平跳变值，输出另外一个占空比的PWM */
    TIM_OCInitStructure.TIM_Pulse = 0;
    /* 初始化定时器通道3输出PWM */
    TIM_OC3Init(BLDC_TIMx, &TIM_OCInitStructure);
    
    // 设定TIM1_CH4的工作状态 为比较输出模式,触发AD转换。通道4的输出GPIO设过了吗？ 或者这个不需要设的吧 因为AD触发的输入是 ADC_ExternalTrigInjecConv_T1_CC4
  /* Channel 4 Configuration in OC */                               ///PA11确实是悬空的没用到这个TIM1_CH4   
                
   TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;  
   TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable; 
   TIM_OCInitStructure.TIM_OutputNState = TIM_OutputNState_Disable;                  
   TIM_OCInitStructure.TIM_Pulse = BLDC_TIM_PERIOD - 1;          ///这个在周期结束的时候  这个CC值设的和那个周期值还是一样的  有意思
  
   TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High; 
   TIM_OCInitStructure.TIM_OCNPolarity = TIM_OCNPolarity_Low;        ///估计这个N没用到吧  
   TIM_OCInitStructure.TIM_OCIdleState = TIM_OCIdleState_Reset;
   TIM_OCInitStructure.TIM_OCNIdleState = TIM_OCNIdleState_Reset;   ///估计这个N没用到吧
   TIM_OC4Init(BLDC_TIMx, &TIM_OCInitStructure);
      // 设定TIM1_CH4的工作状态 为比较输出模式,触发AD转换。
 
   
 ////######################################################################################################   
    /* Automatic Output enable, Break, dead time and lock configuration*/                              //##
    TIM_BDTRInitStructure.TIM_OSSRState = TIM_OSSRState_Enable;                                        //##   OSSR=1 
    TIM_BDTRInitStructure.TIM_OSSIState = TIM_OSSIState_Enable;                                        //##   OSSI=1 
    TIM_BDTRInitStructure.TIM_LOCKLevel = TIM_LOCKLevel_OFF;                                           //## 
    TIM_BDTRInitStructure.TIM_DeadTime = 20;//1/72M = 13.89ns                                          //##   不到 0.3us
                                                                                                       //##
#if(MOTOR_BRAKE_ENABLE == 0)//关闭刹车功能        这个程序是启用了这个刹车功能的                       //##
    TIM_BDTRInitStructure.TIM_Break = TIM_Break_Disable;                                               //## 
    TIM_BDTRInitStructure.TIM_BreakPolarity = TIM_BreakPolarity_High;                                  //##
#else//开启刹车功能                                                                                    //##
    TIM_BDTRInitStructure.TIM_Break = TIM_Break_Enable;                                                //##
    TIM_BDTRInitStructure.TIM_BreakPolarity = TIM_BreakPolarity_High;                                  //##   刹车极性为高
#endif                                                                                                 //##
                                                                                                       //##
    TIM_BDTRInitStructure.TIM_AutomaticOutput = TIM_AutomaticOutput_Enable;                            //##     1：MOE能被软件置’1’或在下一个更新事件被自动置’1’(如果刹车输入无效)
    TIM_BDTRConfig(BLDC_TIMx, &TIM_BDTRInitStructure);                                                 //##
 ////######################################################################################################   
    TIM_OC1PreloadConfig(BLDC_TIMx,TIM_OCPreload_Enable);           ////开启这个CC值的预装载功能 更新事件到来时，才更新这个CC的值 
    TIM_OC2PreloadConfig(BLDC_TIMx,TIM_OCPreload_Enable);          
    TIM_OC3PreloadConfig(BLDC_TIMx,TIM_OCPreload_Enable);
    
    // 设定TIM1_CH4的工作状态 为比较输出模式,触发AD转换。
    TIM_OC4PreloadConfig(BLDC_TIMx,TIM_OCPreload_Enable);
   /*电机电流ADC采样初始化*/
   // MotorCurrentAdcChannel_Init();

    /* 使能定时器重载寄存器ARR */
    TIM_ARRPreloadConfig(BLDC_TIMx, ENABLE);
  
    TIM_ITConfig(BLDC_TIMx,TIM_IT_CC4,DISABLE);                ////关闭这个CC4的中断了
    
    //刹车
    TIM_ClearITPendingBit(BLDC_TIMx,TIM_IT_Break);
    TIM_ITConfig(BLDC_TIMx,TIM_IT_Break,DISABLE);              ///刹车中断也中关了
    /* 使能定时器 */
    TIM_Cmd(BLDC_TIMx, ENABLE);
    
    /* TIM主输出使能 */
    TIM_CtrlPWMOutputs(BLDC_TIMx, ENABLE);                ////这个就是MOE =1   注意到初始化之后这个值等于1 后面再也没有复位过了 所以一切的电平计算，都是以MOE=1为基准
    
    TIM_CCxCmd(BLDC_TIMx,TIM_Channel_1,TIM_CCx_Disable);         ///CCxE这里先禁止掉，这是为了启动吧  CCE=0
    TIM_CCxNCmd(BLDC_TIMx,TIM_Channel_1,TIM_CCxN_Disable);      ///CCNE  也没禁止掉，                即CCNE=0
    TIM_CCxCmd(BLDC_TIMx,TIM_Channel_2,TIM_CCx_Disable);
    TIM_CCxNCmd(BLDC_TIMx,TIM_Channel_2,TIM_CCxN_Disable);
    TIM_CCxCmd(BLDC_TIMx,TIM_Channel_3,TIM_CCx_Disable);
    TIM_CCxNCmd(BLDC_TIMx,TIM_Channel_3,TIM_CCxN_Disable);
    //TIM_CCxCmd(BLDC_TIMx,TIM_Channel_4,TIM_CCx_Disable);   都禁止了，故此时6个管脚的输出值取决于   MOE=1、OSSI=1、OSSR=1、OIS1无关因为MOE=1了、OIS1N无关   和CC1E=1 CC1NE=1 位的值
    //TIM_CCxNCmd(BLDC_TIMx,TIM_Channel_4,TIM_CCxN_Disable);  ///  此时输出为  OCx=CCxP=0   OCxN=CCxNP=1   故此时是上管关闭，而下管也是关闭了。  
}

/**
  * 函数功能: TIMx 输出PWM信号初始化
  * 输入参数: 无
  * 返 回 值: 无
  * 说    明：只要调用这个函数TIMx的四个通道就会有PWM信号输出
  */
void BLDC_TIMx_PWM_Init(void)
{
    BLDC_TIMx_GPIO_Config();
    BLDC_TIMx_Configuration();	
}

/************** (C) COPYRIGHT 2015 SOLAR SUPCON ************END OF FILE********/
