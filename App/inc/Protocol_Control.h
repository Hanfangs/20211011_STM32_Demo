/*
 * Protocol_Control.h
 *
 *  Created on: 2020-11-19
 *      Author: dingyongjian
 */
#ifndef __PROTOCOL_CONTROL_H_
#define __PROTOCOL_CONTROL_H_

/* 引用限制说明 ------------------------------------------------------------- */
#ifdef      PROTOCOL_CONTROL_GLOBALS
  #define   PROTOCOL_CONTROL_EXT
#else
  #define   PROTOCOL_CONTROL_EXT    extern
#endif
/* 私有类型定义 --------------------------------------------------------------*/
typedef struct
{
    u8	byCmd;    //命令码 0x85参数配置  0x99小车应答，0x8A，广播数据
    /*u8	byDirAddr; //方向、地址编号：B7=0 ，B6= 方向， B5 -B0= 编号
    u8  byRunSpeed;//运行速度*6
    u8  byDelay_L7b;//延迟运行时间低7位 B7=0 ，B6 -B0=0~127  *0.01s
    u8  byRunTimer;//运行时间 B7=0 ，B6 -B0=0~127  *0.01s
    u8  byDelay_H1b;//延迟运行时间高第8位，B7 -B1=0 ，B0= 延迟时间第8位
    */
    u8  byRecData[5];
    u8  byCmdSeq;//序列号，B7=0 ，B6 -B0= 递增
    u8	byXorCode;//异或校验码，Byte 2 -7 XOR 
}PROTOCOL_REC_HEAD;//接收帧数据结构

typedef struct
{
    u8	byRunDir; //方向编号：B7=0 ，B6= 方向，
    u8  byRunSpeed;//运行速度 （B6~0）*6
    u8  byDelayTimer;//延迟运行时间低7位 B7=0 ，B6 -B0=0~127  *0.01s
    u8  byRunTimer;//运行时间 B7=0 ，B6 -B0=0~127  *0.01s
}S_RUN_PARAM;//运行参数

typedef struct
{
    u8	byDevAddr;    //设备地址B5 -B0= 编号
    u8  byCmdSeq;//序列号，B7=0 ，B6 -B0= 递增
    u8  byCmdSeq_Last;//之前序列号，B7=0 ，B6 -B0= 递增
    u8	byMotorStatus;//电机状态 
    S_RUN_PARAM sMotorParam_Set;//参数设置值
    S_RUN_PARAM sMotorParam_Local;//当前参数值
    u8  byRunTrig;//运行触发，置1表示接收到参数帧，置2表示触发驱动器运行
    u8  byGroupNum;//小车组号,0~4分别表示小车组1~5
    u8  byGroupBit;//小车组内位号
    u8  bySendDataFlag;//发送数据表示
    u32 dwSendAckTimeout;//发送超时
    u32 dwLedRun_Cnt;//LED运行计数
    u8  byLedFlashCtrlFlag;//LED闪灯控制
    u8  byCmdRec_Flag;//指令接收标识，用于判定参数或动作交替状态
    u8  byCmdRec_LastFlag;//前一次指令接收标识，用于判定参数或动作交替状态
    u8  byRunOrDelayFlag;//1为运行，2位延迟运行，0为空闲
    u32 dwMotorRunTimer;//电机运行时间/100us
    u32 dwMotorDelayTimer;//电机延迟运行时间/100us
    s32 dnMotorSpeed;//运行速度
    u16 wDec_Duty;//减速占空比
    u16 wNow_Duty;
    s32 dnMotor_NowSpeed;//当前转速
    u32 dwMotor_Current;//电机当前电流
    u32 dwOverLoadCnt_H;//短时高过流计数
    u32 dwOverLoadCnt_L;//短时低过流计数
    u32 dwLimitCurrent_H;
    u32 dwLimitCurrent_L;
    u8  byCurrentDownL_Flag;//电流低标识
    u8  byCurrentDownH_Flag;//电流高标识
    u8  rsv[2];
}S_MOTOR_PARAM;//电机参数

typedef struct
{
    u8	byAckCmd;    //命令码
    u8	byAckAddr;  //
}PROTOCOL_ACK_HEAD;//应答帧数据格式

typedef enum 
{
   M_RUN_FAULT    = 0x20,//电机运作失败
   M_LOST_ACT     = 0x10,//参数之前无动作指令
   M_LOST_PARAM   = 0x08,//动作之前无参数指令
   M_HALL_ERR     = 0x04,//霍尔错误
   M_OVERLOAD     = 0x02//过流保护，过载
}MOTOR_ERR_STATE;

/* 私有宏定义 ----------------------------------------------------------------*/
#define TIMER_US           100//100us
#define M_TIMER_BASE      (u32)((1000000/100) / TIMER_US)//100us->1s*0.01/M_TIMER_BASE   10毫秒为单位  0.1ms运行1次
/*gs_Motor_Param.byRunOrDelayFlag 递减模式标识*/         ///  故如果是100ms  则程序中设的是10   10*100=1000次          
#define DECREASE_IDLE       0//递减空闲                 设为x  相当于延时x*10000 微秒，则要求的是100us会计多少次？   
#define DECREASE_RUN        1//运行时间递减
#define DECREASE_DELAY      2//延迟运行时间递减
/*设备地址管脚*/
#define LED_PORT                        GPIOB//GPIOA //LED
#define LED_PIN                         GPIO_Pin_10//GPIO_Pin_5
#define ADDR_BIT_LEN                    6//6位
//拨码开关1~6 1为低位6为高位
#define DEVICE_ADDR1_PORT               GPIOB //地址最低位，第0位
#define DEVICE_ADDR1_PIN                GPIO_Pin_9//原理图中D1  //GPIO_Pin_5
#define DEVICE_ADDR2_PORT               GPIOB
#define DEVICE_ADDR2_PIN                GPIO_Pin_3//原理图中D7 //GPIO_Pin_8
#define DEVICE_ADDR4_PORT               GPIOB 
#define DEVICE_ADDR4_PIN                GPIO_Pin_4//原理图中D6 //GPIO_Pin_9
#define DEVICE_ADDR8_PORT               GPIOB
#define DEVICE_ADDR8_PIN                GPIO_Pin_5//原理图中D5//GPIO_Pin_10
#define DEVICE_ADDR16_PORT              GPIOB 
#define DEVICE_ADDR16_PIN               GPIO_Pin_6//原理图中D4//GPIO_Pin_11
#define DEVICE_ADDR32_PORT              GPIOB  //地址第5位
#define DEVICE_ADDR32_PIN               GPIO_Pin_7//原理图中D3//GPIO_Pin_12

#define DEVICE_ADDR64_PORT              GPIOB  //地址第6位，预留测试用
#define DEVICE_ADDR64_PIN               GPIO_Pin_8//原理图中D2//GPIO_Pin_12
//参数帧byRecData【0~4】
#define DIR_ADDR    0  //方向、地址编号
#define RUN_SPEED   1  //运行速度
#define DELAY_L7B   2  //延迟运行时间低7位
#define RUN_TIMER   3  //运行时间
#define DELAY_H1B   4  //迟运行时间高第8位
//命令码

#define PARAM_SET_NOACK_CMD   0x95 //参数设置命令
#define PARAM_SET_CMD   0x85 //参数设置命令
#define BROADCAST_CMD   0x8A //广播命令
#define PARAM_ACK_CMD   0x99 //参数设置应答命令
#define NOMAL_DATA_LEN  8//常规数据长度

/*u8  byRunTrig运行触发，置1表示接收到参数帧，置2表示触发驱动器运行*/
#define TRIG_ILDE        0//空闲
#define GET_PARAM_FRAME  1//接收到参数帧
#define TRIG_MOTOR_RUN   2//触发驱动器运行

//gs_Motor_Param.bySendDataFlag应答
#define ACK_IDLE       0//应答空闲
#define PARAM_SET_ACK  1//参数帧应答
#define ACK_WAIT       2//应答等待
#define ACK_SEND       3//发送应答

//参数应答包定义
#define ADDR_ACK     1  //方向、地址编号
#define STATUS_ACK   2  //小车状态
#define XOR_ACK      3  //XOR校验码 字节2~3

/*gs_Motor_Param.byGroupNum 小车组号*/
#define M_GROUP_1    0//第1组
#define M_GROUP_2    1//第2组
#define M_GROUP_3    2//第3组
#define M_GROUP_4    3//第4组
#define M_GROUP_5    4//第5组

/*gs_Motor_Param.byLedFlashCtrlFlag  LED闪灯控制*/
#define LED_IDLE       0//待机
#define LED_HALL_ERR   1//霍尔异常
#define LED_OVERLOAD   3//过载
#define LED_RUN_MODE   10//正常，转速

/*gs_Motor_Param.byCmdRec_Flag 用于判定参数或动作交替状态*/
#define IDLE_CMD         0x00   //空闲帧
#define PARAM_FRAME_CMD  0x01  //参数配置帧
#define RUN_FRAME_CMD    0x02  //电机运动帧

/*霍尔异常检测 Hall_Check_Flag*/
#define HALL_CHECK_IDLE     0//空闲
#define HALL_CHECK_START    1//启动检测
#define HALL_CHECK          2//检测中
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
