/*
 * Motor_Control_BLDC.h
 *
 *  Created on: 2017-7-23
 *      Author: dingyongjian
 */
#ifndef __MOTOR_CONTROL_BLDC_H_
#define __MOTOR_CONTROL_BLDC_H_

/* 引用限制说明 ------------------------------------------------------------- */
#ifdef      MOTOR_CONTROL_BLDC_GLOBALS
  #define   MOTOR_CONTROL_BLDC_EXT
#else
  #define   MOTOR_CONTROL_BLDC_EXT    extern
#endif

#define HALL_ERR_CNT_LIMIT 6//6次上报霍尔异常

#define KEY_TEST           0//测试代码启用
#define FORCE_MODE_ENABLE  1//采用三下管到通刹车模式
#define MOTOR_BRAKE_ENABLE 1//1为启用刹车功能
/* 私有类型定义 --------------------------------------------------------------*/
typedef enum
{
    CW = 0,  // 顺时钟方向(对着电机轴看)，一般定义为反转
    CCW = 1  // 逆时针方向(对着电机轴看)，一般定义为正转
}MOTOR_DIR;

typedef enum 
{
    STOP      = 0,  // 停机
    START     = 1,//启动
    RUN       = 2,    // 运行
    INIT      = 3,//初始化
    DELAY_RUN = 4//延迟运行
}MOTOR_STATE;

typedef enum 
{
    MOTOR_IDLE       = 0,  // 停机空闲 
    MOTOR_START      = 1,  // 停机启动 
    MOTOR_RUN_ACC    = 2,    // 加速
    MOTOR_RUN_DEC    = 3,    // 减速
    MOTOR_RUN        = 4,    // 运行
    MOTOR_STOP       = 5,//停机
    MOTOR_STOP_NEXT  = 6,//停机续
    MOTOR_STOP_2NEXT = 7,//停机续
    MOTOR_INIT       = 10,  // 初始化 
    MOTOR_DELAY_RUN  = 11  // 延迟运行
}MOTOR_RUN_STATE;



typedef struct
{
  __IO int          motor_speed;        // 电机转速(RPM):0..3500
  __IO int          motor_setspeed;        // 电机转速(RPM):0..3500
  __IO MOTOR_STATE  motor_state;        // 电机旋转状态
  __IO MOTOR_DIR    motor_direction;    // 电机转动方向
  __IO uint32_t     step_counter;       // 霍尔传感器步数，用于测量电机转速
  __IO uint32_t 	step_counter_prev;	// 上一次霍尔传感器步数，20211115 hjl
  __IO uint16_t     stalling_count;     // 停机标志，如果该值超2000，认为电机停止旋转
  __IO uint16_t		stalling_number;	// 停机标识，20211115 hjl
}MOTOR_DEVICE;
//定义PID结构体
typedef struct 
{
   __IO int      SetPoint;      //设定目标 Desired Value
   __IO double   Proportion;    //比例常数 Proportional Const
   __IO double   Integral;      //积分常数 Integral Const
   __IO double   Derivative;    //微分常数 Derivative Const
   __IO int      LastError;     //Error[-1]
   __IO int      PrevError;     //Error[-2]
}PID;

/* 私有宏定义 ----------------------------------------------------------------*/
/*************************************/
//定义PID相关宏
// 这三个参数设定对电机运行影响非常大
/*************************************/
#define  P_DATA_ACC                   0.4 //0.7              //加速P参数  0.35
#define  I_DATA_ACC                   0.05//0.04        //I参数 0.03
#define  D_DATA_ACC                   0                 //D参数
#define  P_DATA_DEC                   0.8               //减速P参数  0.35
#define  I_DATA_DEC                   0.01//0.06        //I参数 0.03
#define  D_DATA_DEC                   0                 //D参数

#define  MOTOR_RATED_SPEED        500              //额定转速
#define  MOTOR_MAX_SPEED          1600              //空载转速
#define  MOTOR_MIN_SPEED          100               ///20210718  GXD   100        
#define  MOTOR_POLE_PAIR_NUM      4                 //极对数（8 磁极数）
// modified by hjl, 20211025
#define MOTOR_MIN_DUTY_SPEED		120				// PWM占空比
#define MOTOR_DEC_MIN_DUTY_SPEED	100				// 减速时的最小占空比
#define MOTOR_MAX_DUTY_SPEED		900
#define MOTOR_ACC_DELTA_SPEED		1				// original 20
#define MOTOR_DEC_DELTA_SPEED		3				// original 20

//软件过流保护
#define  OVERLOAD_CURRENT_H         44.0  //30A,1ms内->500us           40   
#define  OVERLOAD_CURRENT_L        31.0  //25A,50ms内->5ms             28
#define  SAMP_R_VALUE             0.01     //10毫欧
#define  SAMP_MULT                (32.1/5.1)//32.1k/5.1k
#define  VREF_VALUE               3.3//3.3V
#define  CODE_VALUE               (1 << 12)
#define  LIMIT_CURRENT_L       ((u32)(OVERLOAD_CURRENT_L*SAMP_R_VALUE*SAMP_MULT/VREF_VALUE*CODE_VALUE))     
#define  LIMIT_CURRENT_H        ((u32)(OVERLOAD_CURRENT_H*SAMP_R_VALUE*SAMP_MULT/VREF_VALUE*CODE_VALUE))
#define  OVERLOAD_TIMER_BASE      50//50us
#define  OVERLOAD_TIMER_L     (50000/OVERLOAD_TIMER_BASE)///50ms
#define  OVERLOAD_TIMER_H       (1000/OVERLOAD_TIMER_BASE)//1000us

MOTOR_CONTROL_BLDC_EXT void HALL_TIMx_Callback(void);
MOTOR_CONTROL_BLDC_EXT void HALL_Abnormal_Callback(void);
MOTOR_CONTROL_BLDC_EXT void HAL_SYSTICK_Callback(void);
MOTOR_CONTROL_BLDC_EXT void Motor_Run_Control(void);
MOTOR_CONTROL_BLDC_EXT void Set_Motor_Stop_Delay(uint32_t delay_time);
MOTOR_CONTROL_BLDC_EXT void Motor_Stop_Delay(void);
MOTOR_CONTROL_BLDC_EXT void IncPIDInit(void);
MOTOR_CONTROL_BLDC_EXT void Set_Motor_Stop(void);
MOTOR_CONTROL_BLDC_EXT void Set_Motor_Start(void);
MOTOR_CONTROL_BLDC_EXT void Set_Motor_Dec(void);
MOTOR_CONTROL_BLDC_EXT void Key_Test(void);
MOTOR_CONTROL_BLDC_EXT void Motor_PWM_IDLE(void);
MOTOR_CONTROL_BLDC_EXT void Motor_PWM_READY(void);
MOTOR_CONTROL_BLDC_EXT void MotorOverLoad_Check(void);
#endif /* MOTOR_CONTROL_BLDC_H_ */
