/*
 * Protocol_Control.h
 *
 *  Created on: 2020-11-19
 *      Author: dingyongjian
 */
#ifndef __PROTOCOL_CONTROL_H_
#define __PROTOCOL_CONTROL_H_

/* ��������˵�� ------------------------------------------------------------- */
#ifdef      PROTOCOL_CONTROL_GLOBALS
  #define   PROTOCOL_CONTROL_EXT
#else
  #define   PROTOCOL_CONTROL_EXT    extern
#endif
/* ˽�����Ͷ��� --------------------------------------------------------------*/
typedef struct
{
    u8	byCmd;    //������ 0x85��������  0x99С��Ӧ��0x8A���㲥����
    /*u8	byDirAddr; //���򡢵�ַ��ţ�B7=0 ��B6= ���� B5 -B0= ���
    u8  byRunSpeed;//�����ٶ�*6
    u8  byDelay_L7b;//�ӳ�����ʱ���7λ B7=0 ��B6 -B0=0~127  *0.01s
    u8  byRunTimer;//����ʱ�� B7=0 ��B6 -B0=0~127  *0.01s
    u8  byDelay_H1b;//�ӳ�����ʱ��ߵ�8λ��B7 -B1=0 ��B0= �ӳ�ʱ���8λ
    */
    u8  byRecData[5];
    u8  byCmdSeq;//���кţ�B7=0 ��B6 -B0= ����
    u8	byXorCode;//���У���룬Byte 2 -7 XOR 
}PROTOCOL_REC_HEAD;//����֡���ݽṹ

typedef struct
{
    u8	byRunDir; //�����ţ�B7=0 ��B6= ����
    u8  byRunSpeed;//�����ٶ� ��B6~0��*6
    u8  byDelayTimer;//�ӳ�����ʱ���7λ B7=0 ��B6 -B0=0~127  *0.01s
    u8  byRunTimer;//����ʱ�� B7=0 ��B6 -B0=0~127  *0.01s
}S_RUN_PARAM;//���в���

typedef struct
{
    u8	byDevAddr;    //�豸��ַB5 -B0= ���
    u8  byCmdSeq;//���кţ�B7=0 ��B6 -B0= ����
    u8  byCmdSeq_Last;//֮ǰ���кţ�B7=0 ��B6 -B0= ����
    u8	byMotorStatus;//���״̬ 
    S_RUN_PARAM sMotorParam_Set;//��������ֵ
    S_RUN_PARAM sMotorParam_Local;//��ǰ����ֵ
    u8  byRunTrig;//���д�������1��ʾ���յ�����֡����2��ʾ��������������
    u8  byGroupNum;//С�����,0~4�ֱ��ʾС����1~5
    u8  byGroupBit;//С������λ��
    u8  bySendDataFlag;//�������ݱ�ʾ
    u32 dwSendAckTimeout;//���ͳ�ʱ
    u32 dwLedRun_Cnt;//LED���м���
    u8  byLedFlashCtrlFlag;//LED���ƿ���
    u8  byCmdRec_Flag;//ָ����ձ�ʶ�������ж�������������״̬
    u8  byCmdRec_LastFlag;//ǰһ��ָ����ձ�ʶ�������ж�������������״̬
    u8  byRunOrDelayFlag;//1Ϊ���У�2λ�ӳ����У�0Ϊ����
    u32 dwMotorRunTimer;//�������ʱ��/100us
    u32 dwMotorDelayTimer;//����ӳ�����ʱ��/100us
    s32 dnMotorSpeed;//�����ٶ�
    u16 wDec_Duty;//����ռ�ձ�
    u16 wNow_Duty;
    s32 dnMotor_NowSpeed;//��ǰת��
    u32 dwMotor_Current;//�����ǰ����
    u32 dwOverLoadCnt_H;//��ʱ�߹�������
    u32 dwOverLoadCnt_L;//��ʱ�͹�������
    u32 dwLimitCurrent_H;
    u32 dwLimitCurrent_L;
    u8  byCurrentDownL_Flag;//�����ͱ�ʶ
    u8  byCurrentDownH_Flag;//�����߱�ʶ
    u8  rsv[2];
}S_MOTOR_PARAM;//�������

typedef struct
{
    u8	byAckCmd;    //������
    u8	byAckAddr;  //
}PROTOCOL_ACK_HEAD;//Ӧ��֡���ݸ�ʽ

typedef enum 
{
   M_RUN_FAULT    = 0x20,//�������ʧ��
   M_LOST_ACT     = 0x10,//����֮ǰ�޶���ָ��
   M_LOST_PARAM   = 0x08,//����֮ǰ�޲���ָ��
   M_HALL_ERR     = 0x04,//��������
   M_OVERLOAD     = 0x02//��������������
}MOTOR_ERR_STATE;

/* ˽�к궨�� ----------------------------------------------------------------*/
#define TIMER_US           100//100us
#define M_TIMER_BASE      (u32)((1000000/100) / TIMER_US)//100us->1s*0.01/M_TIMER_BASE   10����Ϊ��λ  0.1ms����1��
/*gs_Motor_Param.byRunOrDelayFlag �ݼ�ģʽ��ʶ*/         ///  �������100ms  ������������10   10*100=1000��          
#define DECREASE_IDLE       0//�ݼ�����                 ��Ϊx  �൱����ʱx*10000 ΢�룬��Ҫ�����100us��ƶ��ٴΣ�   
#define DECREASE_RUN        1//����ʱ��ݼ�
#define DECREASE_DELAY      2//�ӳ�����ʱ��ݼ�
/*�豸��ַ�ܽ�*/
#define LED_PORT                        GPIOB//GPIOA //LED
#define LED_PIN                         GPIO_Pin_10//GPIO_Pin_5
#define ADDR_BIT_LEN                    6//6λ
//���뿪��1~6 1Ϊ��λ6Ϊ��λ
#define DEVICE_ADDR1_PORT               GPIOB //��ַ���λ����0λ
#define DEVICE_ADDR1_PIN                GPIO_Pin_9//ԭ��ͼ��D1  //GPIO_Pin_5
#define DEVICE_ADDR2_PORT               GPIOB
#define DEVICE_ADDR2_PIN                GPIO_Pin_3//ԭ��ͼ��D7 //GPIO_Pin_8
#define DEVICE_ADDR4_PORT               GPIOB 
#define DEVICE_ADDR4_PIN                GPIO_Pin_4//ԭ��ͼ��D6 //GPIO_Pin_9
#define DEVICE_ADDR8_PORT               GPIOB
#define DEVICE_ADDR8_PIN                GPIO_Pin_5//ԭ��ͼ��D5//GPIO_Pin_10
#define DEVICE_ADDR16_PORT              GPIOB 
#define DEVICE_ADDR16_PIN               GPIO_Pin_6//ԭ��ͼ��D4//GPIO_Pin_11
#define DEVICE_ADDR32_PORT              GPIOB  //��ַ��5λ
#define DEVICE_ADDR32_PIN               GPIO_Pin_7//ԭ��ͼ��D3//GPIO_Pin_12

#define DEVICE_ADDR64_PORT              GPIOB  //��ַ��6λ��Ԥ��������
#define DEVICE_ADDR64_PIN               GPIO_Pin_8//ԭ��ͼ��D2//GPIO_Pin_12
//����֡byRecData��0~4��
#define DIR_ADDR    0  //���򡢵�ַ���
#define RUN_SPEED   1  //�����ٶ�
#define DELAY_L7B   2  //�ӳ�����ʱ���7λ
#define RUN_TIMER   3  //����ʱ��
#define DELAY_H1B   4  //������ʱ��ߵ�8λ
//������

#define PARAM_SET_NOACK_CMD   0x95 //������������
#define PARAM_SET_CMD   0x85 //������������
#define BROADCAST_CMD   0x8A //�㲥����
#define PARAM_ACK_CMD   0x99 //��������Ӧ������
#define NOMAL_DATA_LEN  8//�������ݳ���

/*u8  byRunTrig���д�������1��ʾ���յ�����֡����2��ʾ��������������*/
#define TRIG_ILDE        0//����
#define GET_PARAM_FRAME  1//���յ�����֡
#define TRIG_MOTOR_RUN   2//��������������

//gs_Motor_Param.bySendDataFlagӦ��
#define ACK_IDLE       0//Ӧ�����
#define PARAM_SET_ACK  1//����֡Ӧ��
#define ACK_WAIT       2//Ӧ��ȴ�
#define ACK_SEND       3//����Ӧ��

//����Ӧ�������
#define ADDR_ACK     1  //���򡢵�ַ���
#define STATUS_ACK   2  //С��״̬
#define XOR_ACK      3  //XORУ���� �ֽ�2~3

/*gs_Motor_Param.byGroupNum С�����*/
#define M_GROUP_1    0//��1��
#define M_GROUP_2    1//��2��
#define M_GROUP_3    2//��3��
#define M_GROUP_4    3//��4��
#define M_GROUP_5    4//��5��

/*gs_Motor_Param.byLedFlashCtrlFlag  LED���ƿ���*/
#define LED_IDLE       0//����
#define LED_HALL_ERR   1//�����쳣
#define LED_OVERLOAD   3//����
#define LED_RUN_MODE   10//������ת��

/*gs_Motor_Param.byCmdRec_Flag �����ж�������������״̬*/
#define IDLE_CMD         0x00   //����֡
#define PARAM_FRAME_CMD  0x01  //��������֡
#define RUN_FRAME_CMD    0x02  //����˶�֡

/*�����쳣��� Hall_Check_Flag*/
#define HALL_CHECK_IDLE     0//����
#define HALL_CHECK_START    1//�������
#define HALL_CHECK          2//�����
/*************************************/
PROTOCOL_CONTROL_EXT S_MOTOR_PARAM gs_Motor_Param;
PROTOCOL_CONTROL_EXT void Protocol_Data_Receive_Op(void);
PROTOCOL_CONTROL_EXT void Protocol_Data_Send_Op(void);
PROTOCOL_CONTROL_EXT void Decrease_SendAcTimeoutCnt(void);
PROTOCOL_CONTROL_EXT void Motor_Param_DataInit(void);
PROTOCOL_CONTROL_EXT void Clear_MotorErr(MOTOR_ERR_STATE ErrBit);
PROTOCOL_CONTROL_EXT void Set_MotorErr(MOTOR_ERR_STATE ErrBit);
PROTOCOL_CONTROL_EXT void Clear_MotorFault(void);
PROTOCOL_CONTROL_EXT void MotorTimerDecrease(void);

#endif /* PROTOCOL_CONTROL_H */
