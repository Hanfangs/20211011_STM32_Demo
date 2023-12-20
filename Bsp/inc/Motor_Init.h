/*
********************** (C) COPYRIGHT 2015 SOLAR  *************************
* File Name          : Motor_Init.h
* Author             : dingyongjian 
* Date First Issued  : 02/02/15
* Description        : MOTOR驱动 H文件
********************************************************************************
* History:
* 02/02/15 v1.1
********************************************************************************
*/
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MOTOR_INIT_H
  #define __MOTOR_INIT_H

/*
******************************************************************************
*引用限制说明
******************************************************************************
*/
#ifdef      MOTOR_INIT_GLOBALS
  #define   MOTOR_INIT_EXT
#else
  #define   MOTOR_INIT_EXT    extern
#endif


#define U8_MAX     ((u8)255)
#define S8_MAX     ((s8)127)
#define S8_MIN     ((s8)-128)
#define U16_MAX    ((u16)65535u)
#define S16_MAX    ((s16)32767)
#define S16_MIN    ((s16)-32768)
#define U32_MAX    ((u32)4294967295uL)
#define S32_MAX    ((s32)2147483647)
#define S32_MIN    ((s32)-2147483648)

#define HALL_TIMER TIM3
#define HALL_MAX_RATIO		((u16)800u)
/* Private define ------------------------------------------------------------*/
#define HALL_MAX_SPEED_FDBK (u16)(HALL_MAX_SPEED_FDBK_RPM/6 * POLE_PAIR_NUM) //2500
#define HALL_MIN_SPEED_FDBK (u16)(HALL_MIN_SPEED_FDBK_RPM/6* POLE_PAIR_NUM)	 //5
#define LOW_RES_THRESHOLD   ((u16)0x5500u)// If capture below, ck prsc decreases
#define	ROTOR_SPEED_FACTOR  ((u32)((CKTIM*10)) / 3)	   // =240,000,000
#define PSEUDO_FREQ_CONV    ((u32)(ROTOR_SPEED_FACTOR / (SAMPLING_FREQ * 10)) * 0x10000uL)
#define SPEED_OVERFLOW      ((u32)(ROTOR_SPEED_FACTOR / HALL_MAX_SPEED_FDBK)) //96000
#define MAX_PERIOD          ((u32)(ROTOR_SPEED_FACTOR / HALL_MIN_SPEED_FDBK)) //48000000
#define HALL_COUNTER_RESET  ((u16) 0)
#define S16_PHASE_SHIFT     (s16)(HALL_PHASE_SHIFT * 65536/360)
#define S16_120_PHASE_SHIFT (s16)(65536/3)
#define S16_60_PHASE_SHIFT  (s16)(65536/6)
//=================HALL信号输入相位======================
//不使用反相器，如：74HC14，同相输入

#define STATE_0 (u8)0
#define STATE_1 (u8)1
#define STATE_2 (u8)2
#define STATE_3 (u8)3
#define STATE_4 (u8)4
#define STATE_5 (u8)5
#define STATE_6 (u8)6
#define STATE_7 (u8)7

//使用74HC14反相输入信号
/*
#define STATE_0 (u8)7
#define STATE_1 (u8)6
#define STATE_2 (u8)5
#define STATE_3 (u8)4
#define STATE_4 (u8)3
#define STATE_5 (u8)2
#define STATE_6 (u8)1
#define STATE_7 (u8)0
*/
//=======================================================

#define NEGATIVE          (s8)-1
#define POSITIVE          (s8)1
#define NEGATIVE_SWAP     (s8)-2
#define POSITIVE_SWAP     (s8)2
#define ERROR             (s8)127

#define GPIO_MSK (u8)0x07
//#define ICx_FILTER (u8) 0x0B // 11 <-> 1333 nsec 
#define ICx_FILTER (u8) 0x0F // 11 <-> 1333 nsec 

#define TIMx_PRE_EMPTION_PRIORITY 2
#define TIMx_SUB_PRIORITY 0


/* 宏定义 --------------------------------------------------------------------*/
#define HALL_TIMx                        TIM3
#define HALL_TIM_APBxClock_FUN           RCC_APB1PeriphClockCmd
#define HALL_TIM_CLK                     RCC_APB1Periph_TIM3
#define HALL_TIM_PERIOD                  0xFFFF                       //定时器重载值
#define HALL_TIM_PRESCALER               72
#define HALL_TIM_Channel_x               TIM_Channel_1                 //通道

#define HALL_TIM_CH1_PIN                 GPIO_Pin_6
#define HALL_TIM_CH1_GPIO                GPIOA
#define HALL_TIM_CH2_PIN                 GPIO_Pin_7
#define HALL_TIM_CH2_GPIO                GPIOA
#define HALL_TIM_CH3_PIN                 GPIO_Pin_0
#define HALL_TIM_CH3_GPIO                GPIOB
#define HALL_TIM_IRQn                    TIM3_IRQn
#define HALL_TIM_IRQHANDLER              TIM3_IRQHandler

/* 宏定义 --------------------------------------------------------------------*/
/********************高级定时器TIM参数定义，只限TIM1 & TIM8************/
#define BLDC_TIMx                       TIM1
#define BLDC_TIM_APBxClock_FUN          RCC_APB2PeriphClockCmd
#define BLDC_TIM_CLK                    RCC_APB2Periph_TIM1

#define BLDC_TIM_GPIO_CLK               (RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB)
#define BLDC_TIM_CH1_PORT               GPIOA
#define BLDC_TIM_CH1_PIN                GPIO_Pin_8
#define BLDC_TIM_CH2_PORT               GPIOA
#define BLDC_TIM_CH2_PIN                GPIO_Pin_9
#define BLDC_TIM_CH3_PORT               GPIOA
#define BLDC_TIM_CH3_PIN                GPIO_Pin_10

#define BLDC_TIM_CH1N_PORT              GPIOB
#define BLDC_TIM_CH1N_PIN               GPIO_Pin_13
#define BLDC_TIM_CH2N_PORT              GPIOB
#define BLDC_TIM_CH2N_PIN               GPIO_Pin_14
#define BLDC_TIM_CH3N_PORT              GPIOB
#define BLDC_TIM_CH3N_PIN               GPIO_Pin_15

#define BLDC_TIM_BKIN_PORT              GPIOB
#define BLDC_TIM_BKIN_PIN               GPIO_Pin_12  ///这个脚好象没有用

#define BLDC_TIM_PWM_FREQ               20000 // PWM频率  这个是20K啊   周期为72000M/20=3600个脉冲   20K的周期为50us  这个值是ARR值

// 定义定时器预分频，定时器实际时钟频率为：72MHz/（BLDC_TIMx_PRESCALER+1）
#define BLDC_TIM_PRESCALER               0  // 实际时钟频率为：72MHz

// 定义定时器周期，当定时器开始计数到BLDC_TIMx_PERIOD值并且重复计数寄存器为0时更新定时器并生成对应事件和中断    这个是PWM频率
#define BLDC_TIM_PERIOD                  (uint16_t)(SystemCoreClock/(BLDC_TIM_PRESCALER+1)/BLDC_TIM_PWM_FREQ)

// 定义高级定时器重复计数寄存器值，
#define BLDC_TIM_REPETITIONCOUNTER       0


MOTOR_INIT_EXT u16 g_dwAdc_CurrentValue[4];
/* Private function prototypes -----------------------------------------------*/
MOTOR_INIT_EXT void HALL_HallTimerInit(void);
MOTOR_INIT_EXT void BLDC_TIMx_PWM_Init(void);
MOTOR_INIT_EXT void MotorCurrentAdcChannel_Init(void);
#endif
/************** (C) COPYRIGHT 2015 SOLAR SUPCON ************END OF FILE********/
