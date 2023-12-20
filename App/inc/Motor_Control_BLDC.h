/*
 * Motor_Control_BLDC.h
 *
 *  Created on: 2017-7-23
 *      Author: dingyongjian
 */
#ifndef __MOTOR_CONTROL_BLDC_H_
#define __MOTOR_CONTROL_BLDC_H_

/* ��������˵�� ------------------------------------------------------------- */
#ifdef      MOTOR_CONTROL_BLDC_GLOBALS
  #define   MOTOR_CONTROL_BLDC_EXT
#else
  #define   MOTOR_CONTROL_BLDC_EXT    extern
#endif

#define HALL_ERR_CNT_LIMIT 6//6���ϱ������쳣

#define KEY_TEST           0//���Դ�������
#define FORCE_MODE_ENABLE  1//�������¹ܵ�ͨɲ��ģʽ
#define MOTOR_BRAKE_ENABLE 1//1Ϊ����ɲ������
/* ˽�����Ͷ��� --------------------------------------------------------------*/
typedef enum
{
    CW = 0,  // ˳ʱ�ӷ���(���ŵ���ῴ)��һ�㶨��Ϊ��ת
    CCW = 1  // ��ʱ�뷽��(���ŵ���ῴ)��һ�㶨��Ϊ��ת
}MOTOR_DIR;

typedef enum 
{
    STOP      = 0,  // ͣ��
    START     = 1,//����
    RUN       = 2,    // ����
    INIT      = 3,//��ʼ��
    DELAY_RUN = 4//�ӳ�����
}MOTOR_STATE;

typedef enum 
{
    MOTOR_IDLE       = 0,  // ͣ������ 
    MOTOR_START      = 1,  // ͣ������ 
    MOTOR_RUN_ACC    = 2,    // ����
    MOTOR_RUN_DEC    = 3,    // ����
    MOTOR_RUN        = 4,    // ����
    MOTOR_STOP       = 5,//ͣ��
    MOTOR_STOP_NEXT  = 6,//ͣ����
    MOTOR_STOP_2NEXT = 7,//ͣ����
    MOTOR_INIT       = 10,  // ��ʼ�� 
    MOTOR_DELAY_RUN  = 11  // �ӳ�����
}MOTOR_RUN_STATE;



typedef struct
{
  __IO int          motor_speed;        // ���ת��(RPM):0..3500
  __IO int          motor_setspeed;        // ���ת��(RPM):0..3500
  __IO MOTOR_STATE  motor_state;        // �����ת״̬
  __IO MOTOR_DIR    motor_direction;    // ���ת������
  __IO uint32_t     step_counter;       // �������������������ڲ������ת��
  __IO uint32_t 	step_counter_prev;	// ��һ�λ���������������20211115 hjl
  __IO uint16_t     stalling_count;     // ͣ����־�������ֵ��2000����Ϊ���ֹͣ��ת
  __IO uint16_t		stalling_number;	// ͣ����ʶ��20211115 hjl
}MOTOR_DEVICE;
//����PID�ṹ��
typedef struct 
{
   __IO int      SetPoint;      //�趨Ŀ�� Desired Value
   __IO double   Proportion;    //�������� Proportional Const
   __IO double   Integral;      //���ֳ��� Integral Const
   __IO double   Derivative;    //΢�ֳ��� Derivative Const
   __IO int      LastError;     //Error[-1]
   __IO int      PrevError;     //Error[-2]
}PID;

/* ˽�к궨�� ----------------------------------------------------------------*/
/*************************************/
//����PID��غ�
// �����������趨�Ե������Ӱ��ǳ���
/*************************************/
#define  P_DATA_ACC                   0.4 //0.7              //����P����  0.35
#define  I_DATA_ACC                   0.05//0.04        //I���� 0.03
#define  D_DATA_ACC                   0                 //D����
#define  P_DATA_DEC                   0.8               //����P����  0.35
#define  I_DATA_DEC                   0.01//0.06        //I���� 0.03
#define  D_DATA_DEC                   0                 //D����

#define  MOTOR_RATED_SPEED        500              //�ת��
#define  MOTOR_MAX_SPEED          1600              //����ת��
#define  MOTOR_MIN_SPEED          100               ///20210718  GXD   100        
#define  MOTOR_POLE_PAIR_NUM      4                 //��������8 �ż�����
// modified by hjl, 20211025
#define MOTOR_MIN_DUTY_SPEED		120				// PWMռ�ձ�
#define MOTOR_DEC_MIN_DUTY_SPEED	100				// ����ʱ����Сռ�ձ�
#define MOTOR_MAX_DUTY_SPEED		900
#define MOTOR_ACC_DELTA_SPEED		1				// original 20
#define MOTOR_DEC_DELTA_SPEED		3				// original 20

//�����������
#define  OVERLOAD_CURRENT_H         44.0  //30A,1ms��->500us           40   
#define  OVERLOAD_CURRENT_L        31.0  //25A,50ms��->5ms             28
#define  SAMP_R_VALUE             0.01     //10��ŷ
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
