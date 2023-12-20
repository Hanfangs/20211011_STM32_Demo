/*
********************** (C) COPYRIGHT 2015 SOLAR *************************
* File Name          : Motor_Init.c
* Author             : dingyongjian 
* Date First Issued  : 02/02/15
* Description        : LED���� C�ļ�
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
  * ��������: ����Ƕ�������жϿ�����NVIC
  * �������: ��
  * �� �� ֵ: ��
  * ˵    ������
  */
static void HALL_TIMx_NVIC_Configuration(void)
{
    NVIC_InitTypeDef NVIC_InitStructure;
    
    /* ����TIMΪ�ж�Դ */
    NVIC_InitStructure.NVIC_IRQChannel = TIM3_IRQn;
          /* ������ռʽ���ȼ�Ϊ0 */
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;
          /* ���������ȼ�Ϊ0 */
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
          /* ʹ���ж�ͨ�� */
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
}

/*
******************************************************************************
* �� �� �� ��  Hall_HallTimerInit
* ��    �� ��  HALL�ӿڳ�ʼ��  ���ڿ���������ʱ��3��ΪHALL��ʱ����Ŀ���ǽ������������������ÿ�ο�ʼ�������м��� ÿ���жϣ����ǿ���ͨ��������񵽵ļ����ó��ٶ�
* Ӳ�����ӣ�   
******************************************************************************
*/
void HALL_HallTimerInit(void)          ///�����õ���ʱ��Ӧ�����ڲ�ʱ�ӣ������￴�ó����أ�
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

      /* ʹ�ܶ�ʱ��ʱ�� */
    HALL_TIM_APBxClock_FUN(HALL_TIM_CLK,ENABLE);          ////����Ƕ�ʱ��3  TIMER3  �������ʹ�ܶ�ʱ��3��ʱ�� 
    /* ��ʱ����������ʼ�� */		 
    /* ��ʱ����: HALL_TIM_Period+1 */
    TIM_TimeBaseStructure.TIM_Period = HALL_TIM_PERIOD;         ///  0xFFFF  ��Ϊ���
    /* ����Ԥ��Ƶ��HALL_TIM_Prescaler,�������Ƶ�ʣ�72MHz/(HALL_TIM_Prescaler+1)/(HALL_TIM_Period+1) */
    TIM_TimeBaseStructure.TIM_Prescaler = HALL_TIM_PRESCALER;    ////Ԥ��ƵΪ72 
    /* ����ʱ�ӷ�Ƶϵ��������Ƶ(�����ò���) */
    TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1 ;     ////ʱ�ӾͲ���Ƶ��   
    /* ���ϼ���ģʽ */
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_CenterAligned1;     /////�������ģʽ1----���Ǹ�ʲô����ģʽ�أ� �ڼƵ�ĳ��ֵʱ �����������ź� ����������Զ���ᵽ�Ǹ�ֵȥ��
    TIM_TimeBaseInit(HALL_TIMx, &TIM_TimeBaseStructure);                      

 ///ǰ�漸������⣬���������⼸��Ϊ�ؼ��ˣ�   
    
    /* ��ʼ��TIM5���벶����� */
    /* CC1S=01 	ѡ������� IC1ӳ�䵽TI1�� */
    TIM_ICInitStructure.TIM_Channel     = HALL_TIM_Channel_x;        /// ͨ��1   TIM_Channel_1
    /* �����ز��� */
    TIM_ICInitStructure.TIM_ICPolarity  = TIM_ICPolarity_BothEdge;   ////0x000A  ����ʲô��˼�أ�˫���� û������   0x000A  �о����������⣬û�����˫���ش��� ��ʵ�����������ɵ�����Ҳ���԰ɣ�û�Թ��л�����һ��
    /* ӳ�䵽TI1�� */
    TIM_ICInitStructure.TIM_ICSelection = TIM_ICSelection_TRC;    ////   ���Դ�ģʽ������ TRC      CC1S=11  CC1ͨ��������Ϊ���룬IC1ӳ����TRC�ϡ���ģʽ���������ڲ����������뱻ѡ��ʱ(��
                                                                  ////                                  TIMx_SMCR�Ĵ�����TSλѡ��)
    /* ���������Ƶ,����Ƶ  */
    TIM_ICInitStructure.TIM_ICPrescaler = TIM_ICPSC_DIV1;	           
    /* IC1F=0000 ���������˲��� ���˲� */
    TIM_ICInitStructure.TIM_ICFilter    = 0x08;                         
    TIM_ICInit(HALL_TIMx, &TIM_ICInitStructure);  
    
    /* ����NVIC       ��������������ʱ��3���ܵ��ж�����   NVIC�ж�   */
    HALL_TIMx_NVIC_Configuration();          
    
    TIM_SelectHallSensor(HALL_TIMx,ENABLE);                      //ʹ��TIMx�Ļ����������ӿ�       ����TISλ��ѡ����
    TIM_SelectInputTrigger(HALL_TIMx, TIM_TS_TI1F_ED);         	//���봥��Դѡ��     ----------TI1�ı��ؼ��  0x0040  ������������TS[2:0]=100  ������Դ��TI1F_ED 
    
          TIM_SelectSlaveMode(HALL_TIMx, TIM_SlaveMode_Reset);           /////   ��ģʽѡ��  �������ź���ʱ����λ��������� SMSλ=100  ע�⵽TS3��λֻ����SMS=100ʱ�ò�����
    TIM_SelectMasterSlaveMode(HALL_TIMx, TIM_MasterSlaveMode_Enable);	//       ����ģʽѡ��    MSM ���λ���岻�Ǻܴ�
     /* ��������ж� ,����CC1IE�����ж�	 */
    TIM_ITConfig(HALL_TIMx, TIM_IT_Trigger, ENABLE);         ////       ע�⵽ʹ���˴����жϣ�Ҳ����HALLÿ������һ�ζ����ж� ---�жϳ���������ʲô��
    /* ʹ�ܶ�ʱ�� */
    TIM_Cmd(HALL_TIMx, ENABLE);
    TIM_ClearITPendingBit (HALL_TIMx,TIM_IT_Trigger);
    
/*
    // Timer configuration in Clear on capture mode
    TIM_DeInit(HALL_TIMER);
    
    TIM_TimeBaseStructInit(&TIM_HALLTimeBaseInitStructure);
    // Set full 16-bit working range
    TIM_HALLTimeBaseInitStructure.TIM_Period = U16_MAX;
    TIM_HALLTimeBaseInitStructure.TIM_ClockDivision = TIM_CKD_DIV1;	//���������˲�ʱ���Ƶ��
    TIM_TimeBaseInit(HALL_TIMER,&TIM_HALLTimeBaseInitStructure);
    
    TIM_ICStructInit(&TIM_HALLICInitStructure);
    TIM_HALLICInitStructure.TIM_Channel = TIM_Channel_1;
    TIM_HALLICInitStructure.TIM_ICPolarity = TIM_ICPolarity_Falling; //ÿת120��Ƕȣ�����һ���½��ء�
    TIM_HALLICInitStructure.TIM_ICFilter = ICx_FILTER;				 //����ÿ����һ�β����жϣ��൱�����ת��120��Ƕȡ�
    																 //���ǻ�����ÿ60��ǶȾ�ִ��һ�Σ���Ϊ���������ź�
    TIM_ICInit(HALL_TIMER,&TIM_HALLICInitStructure);				 //��XOR��Ȼ������CHANEL1�ġ�
    
    // Force the HALL_TIMER prescaler with immediate access (no need of an update event) 
    TIM_PrescalerConfig(HALL_TIMER, (u16) HALL_MAX_RATIO, 
                       TIM_PSCReloadMode_Immediate);
    TIM_InternalClockConfig(HALL_TIMER);
    
    //Enables the XOR of channel 1, channel2 and channel3
    TIM_SelectHallSensor(HALL_TIMER, ENABLE);
    
    TIM_SelectInputTrigger(HALL_TIMER, TIM_TS_TI1FP1);
    TIM_SelectSlaveMode(HALL_TIMER,TIM_SlaveMode_Reset);
   
    // Source of Update event is only counter overflow/underflow
	//�����ж�Դֻ�Ǽ���������������硣
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
	//ʹ��2���жϣ���÷ֿ�2��д��
    TIM_ITConfig(HALL_TIMER, TIM_IT_CC1, ENABLE);
    TIM_ITConfig(HALL_TIMER, TIM_IT_Update, ENABLE);

    TIM_SetCounter(HALL_TIMER, HALL_COUNTER_RESET);
    TIM_Cmd(HALL_TIMER, ENABLE);*/
}


/**
  * ��������: ����TIMx�������PWMʱ�õ���I/O
  * �������: ��
  * �� �� ֵ: ��
  * ˵    ������
  */
static void BLDC_TIMx_GPIO_Config(void) 
{
    GPIO_InitTypeDef GPIO_InitStructure;
  
          /* ʹ�ܶ�ʱ��ʼ�� */
    BLDC_TIM_APBxClock_FUN(BLDC_TIM_CLK, ENABLE);
  
    /* ʹ�ܶ�ʱ��ͨ������GPIOʱ�� */
    RCC_APB2PeriphClockCmd(BLDC_TIM_GPIO_CLK|RCC_APB2Periph_AFIO, ENABLE); 
  
    /* ���ö�ʱ��ͨ��1�������ģʽ�������������ģʽ */
    GPIO_InitStructure.GPIO_Pin =  BLDC_TIM_CH1_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(BLDC_TIM_CH1_PORT, &GPIO_InitStructure);
  
    /* ���ö�ʱ��ͨ��2�������ģʽ */
    GPIO_InitStructure.GPIO_Pin =  BLDC_TIM_CH2_PIN;
    GPIO_Init(BLDC_TIM_CH2_PORT, &GPIO_InitStructure);
  
    /* ���ö�ʱ��ͨ��3�������ģʽ */
    GPIO_InitStructure.GPIO_Pin =  BLDC_TIM_CH3_PIN;
    GPIO_Init(BLDC_TIM_CH3_PORT, &GPIO_InitStructure);

    // ���ö�ʱ������ͨ��1�������ģʽ     
    GPIO_InitStructure.GPIO_Pin =  BLDC_TIM_CH1N_PIN;
    GPIO_Init(BLDC_TIM_CH1N_PORT, &GPIO_InitStructure); 
     
    // ���ö�ʱ������ͨ��2�������ģʽ 
    GPIO_InitStructure.GPIO_Pin =  BLDC_TIM_CH2N_PIN;
    GPIO_Init(BLDC_TIM_CH2N_PORT, &GPIO_InitStructure); 
     
    // ���ö�ʱ������ͨ��3�������ģʽ 
    GPIO_InitStructure.GPIO_Pin =  BLDC_TIM_CH3N_PIN;
    GPIO_Init(BLDC_TIM_CH3N_PORT, &GPIO_InitStructure);    
    
    /* Configuration: BKIN pin */
    GPIO_InitStructure.GPIO_Pin = BLDC_TIM_BKIN_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(BLDC_TIM_BKIN_PORT, &GPIO_InitStructure); 
    
     
  
}


/*�������ADC������ʼ��         */
void MotorCurrentAdcChannel_Init(void)
{
    u8 i = 0;
    GPIO_InitTypeDef GPIO_InitStructure;
    //DMA_InitTypeDef DMA_InitStructure;
    ADC_InitTypeDef ADC_InitStructure; 
    NVIC_InitTypeDef NVIC_InitStructure; //�ж���������������ʼ���ж����ȼ�
    
    /* Enable DMA clock */  
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);   
       
    /* Enable ADC1 and GPIOC clock */  
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1 | RCC_APB2Periph_GPIOB \
                           | RCC_APB2Periph_GPIOC,ENABLE);  
    
      /****** Configure PB,01(ADC Channels [.9]) as analog input ****/
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1 ;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
    
     /* Configure P13  as analog input */  //TEMP    û������¶Ȱ� ������ܽ�    ���������û������ܰ�����ô����      
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13;   
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;   
    GPIO_Init(GPIOC, &GPIO_InitStructure);   // PC0,����ʱ������������
      /* ADC1 registers reset ----------------------------------------------------*/
    ADC_DeInit(ADC1);
  
    /* Enable ADC1 */
    ADC_Cmd(ADC1, ENABLE);
     
    /* ADC1 configuration */  
       
    ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;                        /*���� ADC ģʽ*/   
    ADC_InitStructure.ADC_ScanConvMode = ENABLE ;                             /*��ֹɨ��ģʽ��ɨ��ģʽ���ڶ�ͨ���ɼ�*/   
    ADC_InitStructure.ADC_ContinuousConvMode = DISABLE;                       /*��������ת��ģʽ������ͣ�ؽ��� ADC ת��*/   
    ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;        /*��ʹ���ⲿ����ת��*/   
    ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;                      /*�ɼ������Ҷ���*/   
    ADC_InitStructure.ADC_NbrOfChannel = 1;                            /*Ҫת����ͨ����Ŀ 1*/   
    ADC_Init(ADC1, &ADC_InitStructure);   
       
   
    /* Enable ADC1 */  
    ADC_Cmd(ADC1, ENABLE);   
        
     /*��λУ׼�Ĵ��� */      
    ADC_ResetCalibration(ADC1);
    /*�ȴ�У׼�Ĵ�����λ��� */  
    while(ADC_GetResetCalibrationStatus(ADC1));   
       
    /* ADC У׼ */  
    ADC_StartCalibration(ADC1);   
    /* �ȴ�У׼���*/  
    while(ADC_GetCalibrationStatus(ADC1));   

    /* ADC1 Injected group of conversions end interrupt disabling   ע��ͨ�� AD �ж�û������   ������Ϊһ��ʼ�ǲ���һ�������������ʽ*/
    ADC_ITConfig(ADC1, ADC_IT_JEOC, DISABLE);
  
    /* �����ڲ��¶ȴ������� Vrefint ͨ��   ��Ȼ���û���ϵ�  */
    ADC_TempSensorVrefintCmd(ENABLE);
    
    /*���� ADC1 ��ͨ�� 0 Ϊ 55.5 ���������� */    
   /* ADC_RegularChannelConfig(ADC1, ADC_Channel_16, 1, ADC_SampleTime_55Cycles5); 
    ADC_RegularChannelConfig(ADC1, ADC_Channel_16, 2, ADC_SampleTime_55Cycles5); 
    ADC_RegularChannelConfig(ADC1, ADC_Channel_16, 3, ADC_SampleTime_55Cycles5);  
    ADC_RegularChannelConfig(ADC1, ADC_Channel_16, 4, ADC_SampleTime_55Cycles5);*/
    ADC_ExternalTrigInjectedConvConfig(ADC1, ADC_ExternalTrigInjecConv_None);           ///û��ָ������ͨ��
    ADC_ExternalTrigInjectedConvCmd(ADC1,ENABLE);                                    ///��������ⲿ����ȴ��������  �����������ҽ�ȥ����һ�£�һ����Ϊ�ޣ�һ����Ϊ�У��񾭲�һ����
    
    ADC_InjectedSequencerLengthConfig(ADC1,2);
    ADC_InjectedChannelConfig(ADC1, ADC_Channel_16, 1, ADC_SampleTime_239Cycles5);  ////�������¶Ⱥ��Ǹ�����
    ADC_InjectedChannelConfig(ADC1, ADC_Channel_9, 2, ADC_SampleTime_239Cycles5);
    
    /* �����ڲ��¶ȴ������� Vrefint ͨ�� */
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
    
    
    
     /*���� ADC ʱ�ӣ�Ϊ PCLK2 �� 8 ��Ƶ���� 9Hz*/
    RCC_ADCCLKConfig(RCC_PCLK2_Div8); 
    /*���� ADC1 ��ͨ�� 0 Ϊ 55.5 ���������� */    
    
    ADC_InjectedSequencerLengthConfig(ADC1,2);
    ADC_InjectedChannelConfig(ADC1, ADC_Channel_16, 1, ADC_SampleTime_71Cycles5);           ///ͨ��16
    ADC_InjectedChannelConfig(ADC1, ADC_Channel_9, 2, ADC_SampleTime_71Cycles5);            ////ͨ��9����PB1
    
     /* ADC1 Injected conversions trigger is TIM1 TRGO */ 
    ADC_ExternalTrigInjectedConvConfig(ADC1, ADC_ExternalTrigInjecConv_T1_CC4);         //// ��ʱ��1��CC4����AD�Ĵ���ʱ��
    ADC_ITConfig(ADC1, ADC_IT_JEOC, ENABLE);                                          //// ADC1��ע��ADת���ж��������
    //ADC_SoftwareStartInjectedConvCmd(ADC1, ENABLE);
    /* ����û�в����ⲿ����������ʹ��������� ADC ת�� */    
    //ADC_SoftwareStartConvCmd(ADC1, ENABLE);
   /* Enable the ADC1_2 Interrupt */
    NVIC_InitStructure.NVIC_IRQChannel =  ADC1_2_IRQn;		//��ADC1�ж�
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;    
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
     //Enable the DMA1_4 Interrupt 
   /* NVIC_InitStructure.NVIC_IRQChannel =  DMA1_Channel4_IRQn;		//��dma1_4�ж�
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;    
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);*/
}
/**
  * ��������: ����TIMx�����PWM�źŵ�ģʽ�������ڡ����ԡ�ռ�ձ�
  * �������: ��
  * �� �� ֵ: ��
  * ˵    ������
  */
/*
 * TIMxCLK/CK_PSC --> TIMxCNT --> TIMx_ARR --> TIMxCNT ���¼���
 *                    TIMx_CCR(��ƽ�����仯)
 * �ź�����=(TIMx_ARR +1 ) * ʱ������
 * ռ�ձ�=TIMx_CCR/(TIMx_ARR +1)
 */
static void BLDC_TIMx_Configuration(void)
{
    TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
    TIM_OCInitTypeDef  TIM_OCInitStructure;
    TIM_BDTRInitTypeDef TIM_BDTRInitStructure;
    
     
    /* ��ʱ����������ʼ�� */		 
    TIM_TimeBaseStructure.TIM_Period = BLDC_TIM_PERIOD-1;            ////���ֵ��ʵ��ֵΪ3600-1           ��������20KƵ��  ���ʱ����50us
    /* ����Ԥ��Ƶ����Ԥ��Ƶ����Ϊ72MHz */
    TIM_TimeBaseStructure.TIM_Prescaler = BLDC_TIM_PRESCALER;        ///����Ƶ
    /* ����ʱ�ӷ�Ƶϵ��������Ƶ(�����ò���) */
    TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1 ;        ///����Ƶ
    /* ���ϼ���ģʽ */
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;     ///��������ϼ���    �����Ǹ�HALL�ɲ���  
    /* �ظ������� */
    TIM_TimeBaseStructure.TIM_RepetitionCounter = BLDC_TIM_REPETITIONCOUNTER;    ////�������0 û�õ�
    TIM_TimeBaseInit(BLDC_TIMx, &TIM_TimeBaseStructure);
  
    /* ��ʱ�����ͨ��1ģʽ���� */
    /* ģʽ���ã�PWMģʽ1 */
    TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;              ////TIMx_CNT<TIMx_CCR1ʱͨ��1Ϊ��Ч��ƽ
    /* ���״̬���ã�ʹ����� */
    TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;	
    /* ����ͨ�����״̬���ã�ʹ����� */
    TIM_OCInitStructure.TIM_OutputNState = TIM_OutputNState_Enable; ///���������ʹ����

    /* ��������ֵ�������������������ֵʱ����ƽ��������    ����Ϊʲô����Ϊ0  ����Ϊ0 ������Ч��ƽ  */
    TIM_OCInitStructure.TIM_Pulse = 0;      ///���ֵ����0�Ļ�������ζ�������Զ�������Ǹ���Ч��ƽ��
    /* ����ʱ������ֵС��CCR1_ValʱΪ�ߵ�ƽ */
    TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;          ///����������Ǹߵ�ƽ��Ч��������������ǵ͵�ƽ��Ч ���Ե�ʱ����ʱ��һ�����һ����ʹӶ������·
    TIM_OCInitStructure.TIM_OCNPolarity= TIM_OCNPolarity_Low;         //// �м����ֵ=0008  Ҳ���� CCxNP=1  �ʼ�ס����������ֵ��ɵ͵�ƽ��Ч
    TIM_OCInitStructure.TIM_OCIdleState = TIM_OCIdleState_Reset;     ///�����������ʲô����  ����ʱ�����Ǹ�λ�� Ҳ���ǵ͵�ƽ    0����MOE=0ʱ�����ʵ����OC1N����������OC1=0��
    TIM_OCInitStructure.TIM_OCNIdleState = TIM_OCNIdleState_Set;//0x0200    /// ����ʱ ���NΪ�ߵ�ƽ ��������MOE�ֵ���0 ����������ʼ�����״̬   //  1����MOE=0ʱ�����ʵ����OC1N����������OC1=1
    /* ��ʼ����ʱ��ͨ��1���PWM */                              /// 1����MOE=0ʱ��������OC1N=1�� Ҳ����˵����MOE=0ʱ�����N������Ǹߵ�ƽ��Ҳ������Ч��ƽ  �����������Ӷ�����ͨ��
    TIM_OC1Init(BLDC_TIMx, &TIM_OCInitStructure);                   //// һ����CCNP��Ϊ�͵�ƽ��Ч��Ҳ���ǽ����CCxNP=1 ��ô��CC1E��CC1NE=0�������:
                                                                     ////                                                    OCx=CCxP=0   OCxN=CCxNP=1   �ʴ�ʱ���Ϲܹرգ����¹�Ҳ�ǹر��ˡ�
    /* ��ʱ�����ͨ��2ģʽ���� */                                    ////�����ڳ�ʼ��������£��������ȫ���ǹرյģ�������������֮�����ֵ�͵��� OCxREF + ���� + ������OCxREF���� + ���� + ������
    /* ����ͨ��2�ĵ�ƽ����ֵ���������һ��ռ�ձȵ�PWM */            ////��������ʱ����� �ڻ���ʱ������6�����ȫ��������ġ�Ψ�����е�����������������4������DISABLE��
    TIM_OCInitStructure.TIM_Pulse = 0;                                              ////��Ҳ����˵��������4�����������Ϊ����ͨ��״̬�ġ�ֻ��������ͨ������һ����һֱͨ����һ���ǰ�ռ�ձ�ͨ��
    /* ��ʼ����ʱ��ͨ��2���PWM */                                                  ///
    TIM_OC2Init(BLDC_TIMx, &TIM_OCInitStructure);
  
    /* ��ʱ�����ͨ��3ģʽ���� */
    /* ����ͨ��3�ĵ�ƽ����ֵ���������һ��ռ�ձȵ�PWM */
    TIM_OCInitStructure.TIM_Pulse = 0;
    /* ��ʼ����ʱ��ͨ��3���PWM */
    TIM_OC3Init(BLDC_TIMx, &TIM_OCInitStructure);
    
    // �趨TIM1_CH4�Ĺ���״̬ Ϊ�Ƚ����ģʽ,����ADת����ͨ��4�����GPIO������� �����������Ҫ��İ� ��ΪAD������������ ADC_ExternalTrigInjecConv_T1_CC4
  /* Channel 4 Configuration in OC */                               ///PA11ȷʵ�����յ�û�õ����TIM1_CH4   
                
   TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;  
   TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable; 
   TIM_OCInitStructure.TIM_OutputNState = TIM_OutputNState_Disable;                  
   TIM_OCInitStructure.TIM_Pulse = BLDC_TIM_PERIOD - 1;          ///��������ڽ�����ʱ��  ���CCֵ��ĺ��Ǹ�����ֵ����һ����  ����˼
  
   TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High; 
   TIM_OCInitStructure.TIM_OCNPolarity = TIM_OCNPolarity_Low;        ///�������Nû�õ���  
   TIM_OCInitStructure.TIM_OCIdleState = TIM_OCIdleState_Reset;
   TIM_OCInitStructure.TIM_OCNIdleState = TIM_OCNIdleState_Reset;   ///�������Nû�õ���
   TIM_OC4Init(BLDC_TIMx, &TIM_OCInitStructure);
      // �趨TIM1_CH4�Ĺ���״̬ Ϊ�Ƚ����ģʽ,����ADת����
 
   
 ////######################################################################################################   
    /* Automatic Output enable, Break, dead time and lock configuration*/                              //##
    TIM_BDTRInitStructure.TIM_OSSRState = TIM_OSSRState_Enable;                                        //##   OSSR=1 
    TIM_BDTRInitStructure.TIM_OSSIState = TIM_OSSIState_Enable;                                        //##   OSSI=1 
    TIM_BDTRInitStructure.TIM_LOCKLevel = TIM_LOCKLevel_OFF;                                           //## 
    TIM_BDTRInitStructure.TIM_DeadTime = 20;//1/72M = 13.89ns                                          //##   ���� 0.3us
                                                                                                       //##
#if(MOTOR_BRAKE_ENABLE == 0)//�ر�ɲ������        ������������������ɲ�����ܵ�                       //##
    TIM_BDTRInitStructure.TIM_Break = TIM_Break_Disable;                                               //## 
    TIM_BDTRInitStructure.TIM_BreakPolarity = TIM_BreakPolarity_High;                                  //##
#else//����ɲ������                                                                                    //##
    TIM_BDTRInitStructure.TIM_Break = TIM_Break_Enable;                                                //##
    TIM_BDTRInitStructure.TIM_BreakPolarity = TIM_BreakPolarity_High;                                  //##   ɲ������Ϊ��
#endif                                                                                                 //##
                                                                                                       //##
    TIM_BDTRInitStructure.TIM_AutomaticOutput = TIM_AutomaticOutput_Enable;                            //##     1��MOE�ܱ�����á�1��������һ�������¼����Զ��á�1��(���ɲ��������Ч)
    TIM_BDTRConfig(BLDC_TIMx, &TIM_BDTRInitStructure);                                                 //##
 ////######################################################################################################   
    TIM_OC1PreloadConfig(BLDC_TIMx,TIM_OCPreload_Enable);           ////�������CCֵ��Ԥװ�ع��� �����¼�����ʱ���Ÿ������CC��ֵ 
    TIM_OC2PreloadConfig(BLDC_TIMx,TIM_OCPreload_Enable);          
    TIM_OC3PreloadConfig(BLDC_TIMx,TIM_OCPreload_Enable);
    
    // �趨TIM1_CH4�Ĺ���״̬ Ϊ�Ƚ����ģʽ,����ADת����
    TIM_OC4PreloadConfig(BLDC_TIMx,TIM_OCPreload_Enable);
   /*�������ADC������ʼ��*/
   // MotorCurrentAdcChannel_Init();

    /* ʹ�ܶ�ʱ�����ؼĴ���ARR */
    TIM_ARRPreloadConfig(BLDC_TIMx, ENABLE);
  
    TIM_ITConfig(BLDC_TIMx,TIM_IT_CC4,DISABLE);                ////�ر����CC4���ж���
    
    //ɲ��
    TIM_ClearITPendingBit(BLDC_TIMx,TIM_IT_Break);
    TIM_ITConfig(BLDC_TIMx,TIM_IT_Break,DISABLE);              ///ɲ���ж�Ҳ�й���
    /* ʹ�ܶ�ʱ�� */
    TIM_Cmd(BLDC_TIMx, ENABLE);
    
    /* TIM�����ʹ�� */
    TIM_CtrlPWMOutputs(BLDC_TIMx, ENABLE);                ////�������MOE =1   ע�⵽��ʼ��֮�����ֵ����1 ������Ҳû�и�λ���� ����һ�еĵ�ƽ���㣬������MOE=1Ϊ��׼
    
    TIM_CCxCmd(BLDC_TIMx,TIM_Channel_1,TIM_CCx_Disable);         ///CCxE�����Ƚ�ֹ��������Ϊ��������  CCE=0
    TIM_CCxNCmd(BLDC_TIMx,TIM_Channel_1,TIM_CCxN_Disable);      ///CCNE  Ҳû��ֹ����                ��CCNE=0
    TIM_CCxCmd(BLDC_TIMx,TIM_Channel_2,TIM_CCx_Disable);
    TIM_CCxNCmd(BLDC_TIMx,TIM_Channel_2,TIM_CCxN_Disable);
    TIM_CCxCmd(BLDC_TIMx,TIM_Channel_3,TIM_CCx_Disable);
    TIM_CCxNCmd(BLDC_TIMx,TIM_Channel_3,TIM_CCxN_Disable);
    //TIM_CCxCmd(BLDC_TIMx,TIM_Channel_4,TIM_CCx_Disable);   ����ֹ�ˣ��ʴ�ʱ6���ܽŵ����ֵȡ����   MOE=1��OSSI=1��OSSR=1��OIS1�޹���ΪMOE=1�ˡ�OIS1N�޹�   ��CC1E=1 CC1NE=1 λ��ֵ
    //TIM_CCxNCmd(BLDC_TIMx,TIM_Channel_4,TIM_CCxN_Disable);  ///  ��ʱ���Ϊ  OCx=CCxP=0   OCxN=CCxNP=1   �ʴ�ʱ���Ϲܹرգ����¹�Ҳ�ǹر��ˡ�  
}

/**
  * ��������: TIMx ���PWM�źų�ʼ��
  * �������: ��
  * �� �� ֵ: ��
  * ˵    ����ֻҪ�����������TIMx���ĸ�ͨ���ͻ���PWM�ź����
  */
void BLDC_TIMx_PWM_Init(void)
{
    BLDC_TIMx_GPIO_Config();
    BLDC_TIMx_Configuration();	
}

/************** (C) COPYRIGHT 2015 SOLAR SUPCON ************END OF FILE********/
