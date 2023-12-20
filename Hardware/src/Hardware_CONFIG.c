/*
********************** (C) COPYRIGHT 2019 SOLAR SUPCON *************************
* File Name          : Hardware_CONFIG.c
* Author             : dingyongjian 
* Date First Issued  : 02/01/19
* Description        : Ӳ�������ʼ�� C�ļ�
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
#define TEMP_V25      143000//�¶ȴ�������25��ʱ�������ѹ������ֵ1.43 V
//#define ADC_TURN_VOL  ((s32)(330000/4096))

u16 g_wADC_ConvertedValue[4];


/* Private functions ------------------------------------------------------- */
/*
******************************************************************************
* �� �� �� ��  STM32_GPIO_CONF
* ��    �� ��  �����ʼ��
* Ӳ��ƽ̨ ��  
* Ӳ�����ӣ�   -------------------------
                |  PA4  -  Mcu_LED1   |//���е�
                |  PA6  -  Mcu_LED2   |//����ָʾ��
                |  PA7  -  Mcu_LED3   |
                |  PC4  -  Mcu_LED4   |
                |  PC5  -  Mcu_LED5   |
                |  PB0  -  Mcu_LED6   |
               -------------------------
* �� �� �� ��  ST3.5.0
* ��    �� ��  
* ��    �� ��  2013��3��25��  ���ֻ��ͨ�õ�IO�� �Ķ���
* ��    �� ��  0.10
******************************************************************************
*/
void STM32_GPIO_Config(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    
    /*����GPIOAʱ���ź�*/
    RCC_APB2PeriphClockCmd( RCC_APB2Periph_GPIOA,ENABLE );
    /*����GPIOBʱ���ź�*/
    RCC_APB2PeriphClockCmd( RCC_APB2Periph_GPIOB,ENABLE );
    /*����GPIOCʱ���ź�*/
    RCC_APB2PeriphClockCmd( RCC_APB2Periph_GPIOC,ENABLE );
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO ,ENABLE);
    
    
    GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable, ENABLE);//GPIOB3,B4ΪTDO�� NRJRST
    /*��ַ����*/
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


////��ʱ��2��һ��1MHz�ļ�ʱ��GXD  100us  ��һ����ʱ������˺ü������ܡ���Ϊ�������
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
    TIM_ITConfig(TIM2, TIM_IT_Update, ENABLE);          ////100us�ж�һ��
    
    TIM_Cmd(TIM2, ENABLE);
}

void Init_TIMER4(void)//1ms��ʱ��  ��ʱ���õĵ�Ҳ�Ǽ�
{
    TIM_TimeBaseInitTypeDef TIM_BaseInitStructure;
    
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE);
     
    TIM_BaseInitStructure.TIM_Period = 1000-1;           
    TIM_BaseInitStructure.TIM_Prescaler = 71;//
    TIM_BaseInitStructure.TIM_ClockDivision = 0;     
    TIM_BaseInitStructure.TIM_CounterMode = TIM_CounterMode_Down; 
    TIM_TimeBaseInit(TIM4, &TIM_BaseInitStructure); 
    
    TIM_ClearFlag(TIM4, TIM_FLAG_Update);
    TIM_ITConfig(TIM4, TIM_IT_Update, ENABLE);         ///�ж�����
    
    TIM_Cmd(TIM4, ENABLE);
}
void NVIC_Configuration(void)//100us
{
    NVIC_InitTypeDef NVIC_InitStructure; 
    
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);           //�����ж����ȼ���
     
    /* Enable the TIM2 Interrupt    ����100us��ʱ�õ�  */
    NVIC_InitStructure.NVIC_IRQChannel = TIM2_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);   
    
    /* Enable the TIM4 Interrupt        ��ʱ��4������ʲô��?   1ms��ʱ�õ�      */
    NVIC_InitStructure.NVIC_IRQChannel = TIM4_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
     
    /*TIM1_CC_IRQn               ��ʱ��1�Ƚ��ж�     ע�⵽��ʱ��1�Ƚ����⣬����ͨ�ö�ʱ����ͨ�ö�ʱ���ж�ֻ��1��������ʱ��1��4���������ж�ʸ��        */
    NVIC_InitStructure.NVIC_IRQChannel = TIM1_CC_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
     
    /*ɲ��TIM1_BRK_IRQn*/
#if(MOTOR_BRAKE_ENABLE == 1)//�ر�ɲ������   
    NVIC_InitStructure.NVIC_IRQChannel = TIM1_BRK_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
#endif
   
}



/*  
* ��������ADC1_GPIO_Config  
* ����  ��ʹ�� ADC1 �� DMA1 ��ʱ�ӣ���ʼ��   
* ����  : ��  
* ���  ����  
* ����  ���ڲ�����  
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
    GPIO_Init(GPIOC, &GPIO_InitStructure);   // PC0,����ʱ������������ 
    
    /* Configure P13  as analog input */  //TEMP   
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13;   
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;   
    GPIO_Init(GPIOC, &GPIO_InitStructure);   // PC0,����ʱ������������ 
}

/* ��������ADC1_Mode_Config  
* ����  ������ ADC1 �Ĺ���ģʽΪ DMA ģʽ  
* ����  : ��  
* ���  ����  
* ����  ���ڲ�����  
*cpu�¶�ֵ= ��1.43V - ADC16ֵ/4096*3.3��/Avg_Slope(4.3mV/���϶�) + 25
*/ 
static void ADC1_Mode_Config(void)
{
    DMA_InitTypeDef DMA_InitStructure;   
    ADC_InitTypeDef ADC_InitStructure;   
       
    /* DMA channel1 configuration */  
    DMA_DeInit(DMA1_Channel1);   
    DMA_InitStructure.DMA_PeripheralBaseAddr = ADC1_DR_Address;  /*ADC��ַ*/   
    DMA_InitStructure.DMA_MemoryBaseAddr = (u32)g_wADC_ConvertedValue;/*�ڴ��ַ*/   
    DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC; //����Ϊ����Դ  
    DMA_InitStructure.DMA_BufferSize = 4;   
    DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;/*�����ַ�̶�*/   
    DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;  /*�ڴ��ַ�̶�*/   
    DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord; //����   
    DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;   
    DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;     //ѭ������   
    DMA_InitStructure.DMA_Priority = DMA_Priority_High;   
    DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;   
    DMA_Init(DMA1_Channel1, &DMA_InitStructure); 
    
    /* Enable DMA channel1 */  
    DMA_Cmd(DMA1_Channel1, ENABLE);   
     
    /* ADC1 configuration */  
       
    ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;  /*���� ADC ģʽ*/   
    ADC_InitStructure.ADC_ScanConvMode = ENABLE ;   /*��ֹɨ��ģʽ��ɨ��ģʽ���ڶ�ͨ���ɼ�*/   
    ADC_InitStructure.ADC_ContinuousConvMode = ENABLE;  /*��������ת��ģʽ������ͣ�ؽ��� ADC ת��*/   
    ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None; /*��ʹ���ⲿ����ת��*/   
    ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;  /*�ɼ������Ҷ���*/   
    ADC_InitStructure.ADC_NbrOfChannel = 4;     /*Ҫת����ͨ����Ŀ 1*/   
    ADC_Init(ADC1, &ADC_InitStructure);   
       
    /*���� ADC ʱ�ӣ�Ϊ PCLK2 �� 4 ��Ƶ���� 50/8Hz*/
    RCC_ADCCLKConfig(RCC_PCLK2_Div4); 
    /*���� ADC1 ��ͨ�� 0 Ϊ 55.5 ���������� */    
    ADC_RegularChannelConfig(ADC1, ADC_Channel_16, 1, ADC_SampleTime_55Cycles5); 
    ADC_RegularChannelConfig(ADC1, ADC_Channel_16, 2, ADC_SampleTime_55Cycles5); 
    ADC_RegularChannelConfig(ADC1, ADC_Channel_16, 3, ADC_SampleTime_55Cycles5);  
    ADC_RegularChannelConfig(ADC1, ADC_Channel_16, 4, ADC_SampleTime_55Cycles5);  
     
    
    /* �����ڲ��¶ȴ������� Vrefint ͨ�� */
    ADC_TempSensorVrefintCmd(ENABLE);
    
    /* Enable ADC1 DMA */  
    ADC_DMACmd(ADC1, ENABLE);   
    DMA_Cmd(DMA1_Channel1, ENABLE);
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
       
    /* ����û�в����ⲿ����������ʹ��������� ADC ת�� */    
    ADC_SoftwareStartConvCmd(ADC1, ENABLE);
}

/*  
* ��������ADC1_Init  
* ����  �� ADC1 ��ʼ��  
* ����  : ��  
* ���  ����  
* ����  ���ڲ�����  
*/
void ADC1_Init(void)
{
    ADC1_GPIO_Config();
    ADC1_Mode_Config();
}

/* ��������u16 Get_Cpu_Temp(void)  
* ����  �����ȡcpu�¶�  
* ����  : ��  
* ���  ����  
* ����  ���ڲ�����  
*cpu�¶�ֵ= ��1.43V - ADC16ֵ/4096*3.3��/Avg_Slope(4.3mV/���϶�) + 25��С����1λ
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


/* ��������DebugUART_Configuration  
* ����  ������ ���Դ���  
* ����  : ��  
* ���  ����              ����õ�CPUû��C�˿ڣ��������ԭ�ȵ����õĴ�����
* ����  ���ڲ�����  
*cpu�¶�ֵ= ��1.43V - ADC16ֵ/4096*3.3��/Avg_Slope(4.3mV/���϶�) + 25
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
    /*����USART4��ʱ��*/
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_UART4, ENABLE);

    USART_InitStructure.USART_BaudRate = 115200;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;//9λ����
    USART_InitStructure.USART_StopBits = USART_StopBits_1;//1λֹͣλ
    USART_InitStructure.USART_Parity = USART_Parity_No;//��У��
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None; //Ӳ��������ʧ��
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx; //���ͺͽ���ʹ��
    USART_Init(UART4, &USART_InitStructure); 
    USART_ITConfig(UART4, USART_IT_RXNE, ENABLE);
    USART_Cmd(UART4, ENABLE); 
    USART_ClearITPendingBit(UART4, USART_IT_TC);//����ж�TCλ
    
    //GPIO_SetBits(GPIOC,GPIO_Pin_12);//
}
/************** (C) COPYRIGHT 2015 SOLAR SUPCON ************END OF FILE********/
