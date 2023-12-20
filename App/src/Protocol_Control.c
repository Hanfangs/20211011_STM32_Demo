/*
********************** (C) COPYRIGHT 2020 SOLAR  *************************
* File Name          : Protocol_Control.c
* Author             : dingyongjian 
* Date First Issued  : 
* Description        : bldc电机控制
********************************************************************************
* History:
* 11/19/2020 v1.0
*
********************************************************************************
*/
#define      PROTOCOL_CONTROL_GLOBALS

#include "include.h"
#include "main.h"

/* 私有变量 ------------------------------------------------------------------*/
//S_MOTOR_PARAM gs_Motor_Param;
//地址拨码端口
GPIO_TypeDef *DeviceAddrGPIO_Port[6] = {DEVICE_ADDR1_PORT,DEVICE_ADDR2_PORT,DEVICE_ADDR4_PORT,\
                                        DEVICE_ADDR8_PORT,DEVICE_ADDR16_PORT,DEVICE_ADDR32_PORT};
uint16_t DeviceAddrGPIO_Pin[6] = {DEVICE_ADDR1_PIN,DEVICE_ADDR2_PIN,DEVICE_ADDR4_PIN,\
                                  DEVICE_ADDR8_PIN,DEVICE_ADDR16_PIN,DEVICE_ADDR32_PIN};
/* Private functions ------------------------------------------------------- */

/*
*******************************************************************************
* Function Name  : Device_Info_Init
* Description    : 设备地址及小车组位号初始化
* Input          : none
* Output         : none
* Return         : none
*******************************************************************************
*/
void Device_Info_Init(void)
{
    u8 i;
    u8 byMod = 0;//余数
    u8 byMult = 0;//倍数
    gs_Motor_Param.byDevAddr = 0;
    for(i = 0; i < ADDR_BIT_LEN; i++)            ///只有6位
    {
        if(Bit_RESET == GPIO_ReadInputDataBit(DeviceAddrGPIO_Port[i],DeviceAddrGPIO_Pin[i]))//低电平
        { 
            gs_Motor_Param.byDevAddr |= (1 << i);
        }
    }
    
    //拨码2GPIO异常
   /* if(Bit_RESET == GPIO_ReadInputDataBit(DeviceAddrGPIO_Port[1],DeviceAddrGPIO_Pin[1]))
    {
        gs_Motor_Param.byDevAddr &= ~(1 << 1);
    }
    else
    {
        gs_Motor_Param.byDevAddr &= ~(1 << 1);
    }*/
      
    byMod  = gs_Motor_Param.byDevAddr % 8;//计算%8余数
    byMult = (gs_Motor_Param.byDevAddr >> 3);//计算/8倍数
    
    if(0 == byMod)//地址为8的倍数，第5组        只有8 16 24 32 这4种情况
    {
        gs_Motor_Param.byGroupNum = M_GROUP_5;//第5组  = 4
        if(byMult <= 4)//地址不超过32
        {
            gs_Motor_Param.byGroupBit = (1 << (byMult - 1));//组内位号        原先为：(1 << (byMult + 2))   改为8移到0位置
        }
        else
        {
            gs_Motor_Param.byGroupBit = 0;
        }
    }
    else//地址不为8的倍数，1~4组
    {
        gs_Motor_Param.byGroupNum = byMult;
        gs_Motor_Param.byGroupBit = (1 << (byMod - 1));//组内位号
    }
}
/*
*******************************************************************************
* Function Name  : Motor_Param_DataInit
* Description    : 电机运行参数初始化
* Input          : none
* Output         : none
* Return         : none
*******************************************************************************
*/
void Motor_Param_DataInit(void)
{
    memset(&gs_Motor_Param,0,sizeof(S_MOTOR_PARAM));
    Device_Info_Init();//设备地址及小车组位号初始化
}

/*
*******************************************************************************
* Function Name  : Get_Data_Xor
* Description    : XOR校验函数
* Input          : u8 *nData, u16 wLength
* Output         : none
* Return         : XorResult
*******************************************************************************
*/
u8 Get_Data_Xor(u8 *nData, u16 wLength) 
{
    u8 XorResult = 0;
    
    if(wLength < 2)//长度小于2
    {
        return 0;
    }
    
    while(wLength--)
    {
        XorResult ^= *nData++;
    }
    return (u8)XorResult;
}

/*
*******************************************************************************
* Function Name  : CheckReceiveData
* Description    : 校验接收数据格式
* Input          : PROTOCOL_CMD_HEAD *p_cmdHead, u16 totalLen
* Output         : none
* Return         : TRUE OR FALSE

这个函数也要修改一下，对于合并的一条命令也要能识别出来

至于合并后的命令，后一条的校验和，我们就不管它了。


这里改一下，如果参数长度大于16  则我们要搜索这个命令中是否包含有我们自已的地址  然后把这条地址的信息提取出来


*******************************************************************************
*/
BOOLEAN CheckReceiveData(PROTOCOL_REC_HEAD *p_cmdHead, u16 wLen)
{
    if((wLen != NOMAL_DATA_LEN) && (wLen != (2*NOMAL_DATA_LEN))) //数据长度不满足  不等于8或者不等于16
    {
        return FALSE;
    }
    
    if((((PARAM_SET_CMD == p_cmdHead->byCmd)|| (PARAM_SET_NOACK_CMD == p_cmdHead->byCmd))                \
         &&((p_cmdHead->byRecData[DIR_ADDR] & 0x3F)== gs_Motor_Param.byDevAddr))\
       ||(BROADCAST_CMD == p_cmdHead->byCmd))//参数设置命令或广播命令
    {
      return TRUE; // modified by hjl, 202110230113
/*      if(Get_Data_Xor(((u8 *)p_cmdHead + 1),6) != p_cmdHead->byXorCode)//XOR校验不合格 2~7字节
        {
            return FALSE;  ////我们可以在这里，当长度是16时，判断一下这个第2条命令的校验和是否正确
        }    
*/       /* 
        if(wLen==16)
        {
          if(Get_Data_Xor(((u8 *)p_cmdHead + 1 + 8),6)!= (p_cmdHead + 8)->byXorCode)
            return FALSE;
        }
       */ 
    }
    else
    {
        return FALSE;
    }
    
    return TRUE;
}

/*
*******************************************************************************
* Function Name  : Data_Analysis_Op
* Description    : 数据解析处理任务，
  控制中心发送运行参数帧后，
  驱动器返回应答然命令。
  然后控制中心发生运行命令帧，
  每个命令帧前必须有一参数帧，否则驱动器不作。 先收到参数帧，再收到广播帧，此时，电机方可运行
* Input          : PROTOCOL_REC_HEAD *p_cmdHead, u16 totalLen
* Output         : none
* Return         : none      这个并不是在一直运行的，只有收到数据后才会运行一次

20210805  增加一条命令，让这个上位机可以两条命令合并发送



*******************************************************************************
*/
static void Data_Analysis_Op(u8* data, u16 wLen)
{
    PROTOCOL_REC_HEAD *p_cmdHead = (PROTOCOL_REC_HEAD *) data;
    
    if(CheckReceiveData(p_cmdHead, wLen))//数据校验通过
    {
        switch(p_cmdHead->byCmd)
        {
            case PARAM_SET_CMD://参数设置命令 ----这里可能是两条命令合并的，要区分开来。
            {
              
              if(wLen == NOMAL_DATA_LEN)   ///如果是普通的命令长度，就按照普通的来了
              {
              
                gs_Motor_Param.sMotorParam_Set.byRunDir = ((p_cmdHead->byRecData[DIR_ADDR] >> 6) & (0x01));
                gs_Motor_Param.sMotorParam_Set.byRunSpeed = (p_cmdHead->byRecData[RUN_SPEED] & 0x7F);
                gs_Motor_Param.sMotorParam_Set.byDelayTimer = (p_cmdHead->byRecData[DELAY_L7B] & 0x7F);	// 20211115 hjl               
//                gs_Motor_Param.sMotorParam_Set.byDelayTimer = 0; // modified by hjl, 202110230056
//               gs_Motor_Param.sMotorParam_Set.byDelayTimer =    (p_cmdHead->byRecData[DELAY_L7B] & 0x7F) + (p_cmdHead->byRecData[DELAY_H1B] << 7);               
                gs_Motor_Param.sMotorParam_Set.byRunTimer = (p_cmdHead->byRecData[RUN_TIMER] & 0x7F);
               
                gs_Motor_Param.byRunTrig = GET_PARAM_FRAME;//设置接收到参数帧
                
                if(PARAM_FRAME_CMD == gs_Motor_Param.byCmdRec_Flag)//前一帧为参数帧
                {
                    Set_MotorErr(M_LOST_ACT);//设置参数之前无动作指令故障       并不影响运行，只是向上位机报告而已
                }
                else
                {
                    Clear_MotorErr(M_LOST_ACT);//清参数之前无动作指令故障
                }               
                gs_Motor_Param.byCmdRec_Flag = PARAM_FRAME_CMD;
                
                if(ACK_IDLE == gs_Motor_Param.bySendDataFlag)//应答空闲   万一不空闲呢，难不成就不应答了吗？  也许上位机看到一定时间内，还没有收到应答，它就再次重新发送这个命令吧
                {
                    gs_Motor_Param.bySendDataFlag = PARAM_SET_ACK;//设置参数帧应答  这个才是最重要的
                }
              }
              else if(wLen == 2*NOMAL_DATA_LEN)      ///这里说明是连续的两条命令
              {
////先解析第1条命令：
                gs_Motor_Param.sMotorParam_Set.byRunDir = ((p_cmdHead->byRecData[DIR_ADDR] >> 6) & (0x01));
                gs_Motor_Param.sMotorParam_Set.byRunSpeed = (p_cmdHead->byRecData[RUN_SPEED] & 0x7F);
                gs_Motor_Param.sMotorParam_Set.byDelayTimer = (p_cmdHead->byRecData[DELAY_L7B] & 0x7F);	// 20211115 hjl               
//                gs_Motor_Param.sMotorParam_Set.byDelayTimer = 0; // modified by hjl, 202110230056
//              gs_Motor_Param.sMotorParam_Set.byDelayTimer =    (p_cmdHead->byRecData[DELAY_L7B] & 0x7F) + (p_cmdHead->byRecData[DELAY_H1B] << 7);               
                gs_Motor_Param.sMotorParam_Set.byRunTimer = (p_cmdHead->byRecData[RUN_TIMER] & 0x7F);
               
                gs_Motor_Param.byRunTrig = GET_PARAM_FRAME;//设置接收到参数帧
                
                if(PARAM_FRAME_CMD == gs_Motor_Param.byCmdRec_Flag)//前一帧为参数帧
                {
                    Set_MotorErr(M_LOST_ACT);//设置参数之前无动作指令故障                并不影响运行，只是向上位机报告而已
                }
                else
                {
                    Clear_MotorErr(M_LOST_ACT);//清参数之前无动作指令故障
                }               
                gs_Motor_Param.byCmdRec_Flag = PARAM_FRAME_CMD;
                
                if(ACK_IDLE == gs_Motor_Param.bySendDataFlag)//应答空闲   万一不空闲呢，难不成就不应答了吗？  也许上位机看到一定时间内，还没有收到应答，它就再次重新发送这个命令吧
                {
                   gs_Motor_Param.bySendDataFlag = PARAM_SET_ACK;//设置参数帧应答  这个才是最重要的
                }

  ///再解析第2条命令              
                p_cmdHead = (PROTOCOL_REC_HEAD *) (data+8);

                if((p_cmdHead->byRecData[gs_Motor_Param.byGroupNum] & gs_Motor_Param.byGroupBit)\
                    == gs_Motor_Param.byGroupBit)//当前小车运行命令帧
                {
//                  if( gs_Motor_Param.byRunTrig == TRIG_ILDE)//    空闲   

           if(GET_PARAM_FRAME == gs_Motor_Param.byRunTrig)//已接收到参数帧  没收到参数帧，则不会出现运行触发设置标志
                    {
                        gs_Motor_Param.byRunTrig = TRIG_MOTOR_RUN;//设置启动参数运行    必须接到参数设置帧，才会有触发马达运行  这个触发标志
                    }
                    
                    if(RUN_FRAME_CMD == gs_Motor_Param.byCmdRec_Flag)//前一帧为电机运动帧
                    {
                        Set_MotorErr(M_LOST_PARAM);//设置动作之前无参数指令故障  有这个故障怎么办，难不成电机就不转了还是什么？这个倒也没有，只是往上位机报告一下而已
                    }                                                                                                                     ///并不影响运行
                    else
                    {
                        Clear_MotorErr(M_LOST_PARAM);//清动作之前无参数指令故障
                    }               
                    gs_Motor_Param.byCmdRec_Flag = RUN_FRAME_CMD;     ////电机开始运行了 注意到此时将这个标志后
                }                                                     ///下次再发运行命令也没用 这个标志的目的就是为了先发配置帧再发广播帧
                                                                      
            }
                
              else{}        ////不然什么事也不做
              }
            break;
            case PARAM_SET_NOACK_CMD:        ///这个就是无应答参数设置命令
              {
             if(wLen == NOMAL_DATA_LEN)   ///如果是普通的命令长度，就按照普通的来了
              {
 
                gs_Motor_Param.sMotorParam_Set.byRunDir = ((p_cmdHead->byRecData[DIR_ADDR] >> 6) & (0x01));
                gs_Motor_Param.sMotorParam_Set.byRunSpeed = (p_cmdHead->byRecData[RUN_SPEED] & 0x7F);
                gs_Motor_Param.sMotorParam_Set.byDelayTimer = (p_cmdHead->byRecData[DELAY_L7B] & 0x7F);	// 20211115 hjl               
//                gs_Motor_Param.sMotorParam_Set.byDelayTimer = 0; // modified by hjl, 202110230056
//                  (p_cmdHead->byRecData[DELAY_L7B] & 0x7F) + (p_cmdHead->byRecData[DELAY_H1B] << 7);               
                gs_Motor_Param.sMotorParam_Set.byRunTimer = (p_cmdHead->byRecData[RUN_TIMER] & 0x7F);
               
                gs_Motor_Param.byRunTrig = GET_PARAM_FRAME;//设置接收到参数帧
                
                if(PARAM_FRAME_CMD == gs_Motor_Param.byCmdRec_Flag)//前一帧为参数帧
                {
                    Set_MotorErr(M_LOST_ACT);//设置参数之前无动作指令故障       并不影响运行，只是向上位机报告而已
                }
                else
                {
                    Clear_MotorErr(M_LOST_ACT);//清参数之前无动作指令故障
                }               
                gs_Motor_Param.byCmdRec_Flag = PARAM_FRAME_CMD;
                
               if(ACK_IDLE == gs_Motor_Param.bySendDataFlag)//应答空闲   万一不空闲呢，难不成就不应答了吗？  也许上位机看到一定时间内，还没有收到应答，它就再次重新发送这个命令吧
                {
//                    gs_Motor_Param.bySendDataFlag = PARAM_SET_ACK;//设置参数帧应答  这个才是最重要的
               }              
              }
              else if(wLen == 2*NOMAL_DATA_LEN)      ///这里说明是连续的两条命令
              {
////先解析第1条命令：
                gs_Motor_Param.sMotorParam_Set.byRunDir = ((p_cmdHead->byRecData[DIR_ADDR] >> 6) & (0x01));
                gs_Motor_Param.sMotorParam_Set.byRunSpeed = (p_cmdHead->byRecData[RUN_SPEED] & 0x7F);
                gs_Motor_Param.sMotorParam_Set.byDelayTimer = (p_cmdHead->byRecData[DELAY_L7B] & 0x7F);	// 20211115 hjl               
//                gs_Motor_Param.sMotorParam_Set.byDelayTimer = 0; // modified by hjl, 202110230021
//                  (p_cmdHead->byRecData[DELAY_L7B] & 0x7F) + (p_cmdHead->byRecData[DELAY_H1B] << 7);               
                gs_Motor_Param.sMotorParam_Set.byRunTimer = (p_cmdHead->byRecData[RUN_TIMER] & 0x7F);
               
                gs_Motor_Param.byRunTrig = GET_PARAM_FRAME;//设置接收到参数帧
                
                if(PARAM_FRAME_CMD == gs_Motor_Param.byCmdRec_Flag)//前一帧为参数帧
                {
                    Set_MotorErr(M_LOST_ACT);//设置参数之前无动作指令故障                并不影响运行，只是向上位机报告而已
                }
                else
                {
                    Clear_MotorErr(M_LOST_ACT);//清参数之前无动作指令故障
                }               
                gs_Motor_Param.byCmdRec_Flag = PARAM_FRAME_CMD;
                
                if(ACK_IDLE == gs_Motor_Param.bySendDataFlag)//应答空闲   万一不空闲呢，难不成就不应答了吗？  也许上位机看到一定时间内，还没有收到应答，它就再次重新发送这个命令吧
                {
//                    gs_Motor_Param.bySendDataFlag = PARAM_SET_ACK;//设置参数帧应答  这个才是最重要的
                }

  ///再解析第2条命令              
                p_cmdHead = (PROTOCOL_REC_HEAD *) (data+8);

                if((p_cmdHead->byRecData[gs_Motor_Param.byGroupNum] & gs_Motor_Param.byGroupBit)\
                    == gs_Motor_Param.byGroupBit)//当前小车运行命令帧
                {
//                  if( gs_Motor_Param.byRunTrig == TRIG_ILDE)//    空闲   

           if(GET_PARAM_FRAME == gs_Motor_Param.byRunTrig)//已接收到参数帧  没收到参数帧，则不会出现运行触发设置标志
                    {
                        gs_Motor_Param.byRunTrig = TRIG_MOTOR_RUN;//设置启动参数运行    必须接到参数设置帧，才会有触发马达运行  这个触发标志
                    }
                    
                    if(RUN_FRAME_CMD == gs_Motor_Param.byCmdRec_Flag)//前一帧为电机运动帧
                    {
                        Set_MotorErr(M_LOST_PARAM);//设置动作之前无参数指令故障  有这个故障怎么办，难不成电机就不转了还是什么？这个倒也没有，只是往上位机报告一下而已
                    }                                                                                                                     ///并不影响运行
                    else
                    {
                        Clear_MotorErr(M_LOST_PARAM);//清动作之前无参数指令故障
                    }               
                    gs_Motor_Param.byCmdRec_Flag = RUN_FRAME_CMD;     ////电机开始运行了 注意到此时将这个标志后
                }                                                     ///下次再发运行命令也没用 这个标志的目的就是为了先发配置帧再发广播帧
                                                                      
            }
                
              else{}        ////不然什么事也不做
             
             
             
              
              }
              break;
        
            case BROADCAST_CMD://广播命令   广播命令好象是对同一群的电机进行启动
            {
                if((p_cmdHead->byRecData[gs_Motor_Param.byGroupNum] & gs_Motor_Param.byGroupBit)\
                    == gs_Motor_Param.byGroupBit)//当前小车运行命令帧
                {
//                  if( gs_Motor_Param.byRunTrig == TRIG_ILDE)//    空闲   

           if(GET_PARAM_FRAME == gs_Motor_Param.byRunTrig)//已接收到参数帧  没收到参数帧，则不会出现运行触发设置标志
                    {
                        gs_Motor_Param.byRunTrig = TRIG_MOTOR_RUN;//设置启动参数运行    必须接到参数设置帧，才会有触发马达运行  这个触发标志
                    }
                    
                    if(RUN_FRAME_CMD == gs_Motor_Param.byCmdRec_Flag)//前一帧为电机运动帧
                    {
                        Set_MotorErr(M_LOST_PARAM);//设置动作之前无参数指令故障  有这个故障怎么办，难不成电机就不转了还是什么？这个倒也没有，只是往上位机报告一下而已
                    }                                                                                                                     ///并不影响运行
                    else
                    {
                        Clear_MotorErr(M_LOST_PARAM);//清动作之前无参数指令故障
                    }               
                    gs_Motor_Param.byCmdRec_Flag = RUN_FRAME_CMD;     ////电机开始运行了 注意到此时将这个标志后
                }                                                     ///下次再发运行命令也没用 这个标志的目的就是为了先发配置帧再发广播帧
                                                                      
            }
            break;
            
            default:
            {
            }
            break;
        }
    }
    
}
/*
*******************************************************************************
* Function Name  : Protocol_Data_Receive_Op
* Description    : 协议数据接收处理任务
* Input          : none
* Output         : none  这个任务一直在运行
* Return         : none
*******************************************************************************
*/
void Protocol_Data_Receive_Op(void)
{
    static u8  byComDataRecBuffer[MAX_LEN_CMD]; //定日镜接收到的镜组数据
//    static u8  byComDataRecBuffer_16[16];
    static u16 wComDataRecLength;//镜组数据长度
    u8 i;
    u8 *p_rec;
 
    
        PROTOCOL_REC_HEAD *p_cmdHead; 

    
    if (Com2_Recv(byComDataRecBuffer, &wComDataRecLength))//接收上层发送的数据
    {
        if(wComDataRecLength>16)                 //搜索那一个串是面对本机的
        {
                 p_rec =  byComDataRecBuffer;
                 p_cmdHead = (PROTOCOL_REC_HEAD *) p_rec;

         for(i=0;i<wComDataRecLength/16;i++)
        {
            if((p_cmdHead->byRecData[DIR_ADDR] & 0x3F)== gs_Motor_Param.byDevAddr)
             {Data_Analysis_Op(p_rec, 16);
             return;}
            p_rec+=16;
            p_cmdHead = (PROTOCOL_REC_HEAD *) p_rec;
            
        }
          return;              ////没有找到，直接返回了
          
        }
        else{
        Data_Analysis_Op(byComDataRecBuffer, wComDataRecLength);
        }
    }
}


/*
*******************************************************************************
* Function Name  : Send_ParamFrameAckData
* Description    : 发送参数帧应答包
* Input          : none
* Output         : none  
* Return         : none
*******************************************************************************
*/
void Send_ParamFrameAckData(void)
{
    static u8 Ack_Data[4] = {PARAM_ACK_CMD,0,0,0};                   //地址0 就是这个应答  
                                                              
    Ack_Data[ADDR_ACK]   = gs_Motor_Param.byDevAddr;                  //地址1
    Ack_Data[STATUS_ACK] = gs_Motor_Param.byMotorStatus;              //2     看是否有错误发生，如果有，就为1，如果没有，这个等于0       
    Ack_Data[XOR_ACK]    = (Ack_Data[ADDR_ACK] ^ Ack_Data[STATUS_ACK]);  ///3
    Usart2_StartTx(Ack_Data,4);                      ////在这个函数中，将发送标志结束标志设为RESET了。
}

/*
*******************************************************************************
* Function Name  : SetSendAck_TimeOut
* Description    : 设置发送超时时间
* Input          : u32 timeout
* Output         : none
* Return         : none
*******************************************************************************
*/
void SetSendAck_TimeOut(u32 timeout)
{
    gs_Motor_Param.dwSendAckTimeout = timeout;
}

/*
*******************************************************************************
* Function Name  : Decrease_SendAcTimeoutCnt
* Description    : 发送超时时间--
* Input          : none
* Output         : none
* Return         : none
*******************************************************************************
*/
void Decrease_SendAcTimeoutCnt(void)
{
    if(gs_Motor_Param.dwSendAckTimeout > 0)
    {
        gs_Motor_Param.dwSendAckTimeout--;
    }
}

/*
*******************************************************************************
* Function Name  : Is_SendAckTimeout
* Description    : 判断是否超时
* Input          : None
* Output         : None
* Return         : TRUE/FALSE
*******************************************************************************
*/
static BOOLEAN Is_SendAckTimeout(void)
{
    if (0 == gs_Motor_Param.dwSendAckTimeout)
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

/*
*******************************************************************************
* Function Name  : Protocol_Data_Send_Op         事实上，运行了两个任务，一个是数据接收任务，一个是数据发送任务 这个是无时无刻在运行的 
* Description    : 协议数据发送处理任务
* Input          : none
* Output         : none      这是个任务，不断运行  平时一般在IDLE状态下，一旦接收到数据，就会进入一个状态机转换中
在这个转换中，有不少延时的事情要做，发送完成后，配合DMA完成中断，会调用一个函数，该函数将计数器的一个值设置一下。
在等待500毫秒后以及发送完成标志为0的情况下同时满足的情况下-----------但有一个问题，这个可能没法同时满足好象，因为一开始是500毫秒没满足，后来是这个因为发送中断已经产生了
这个第2个条件已经不满足了啊，怎么回事，搞不明白了啊？？？？？因为发送中断已经产生了，此时产生了一个计时  s_byUsart2TxEndDelay = 10; 于是开始减计数了 这个一下子就减完了
减完后，这个s_bUsart2TxEndFlag = SET; 于是这个条件就不满足了，怎么办啊？ 碰到难题了。。。。还没解决
///解决了啊，这个s_bUsart2TxEndFlag = RESET  就是在发送函数里设置的，本来就是满足的啊。。。。但是为什么要加个这个判断呢？是不是可以去掉呢？有点多余吧，明明在发送函数中将这个RESET了，还要多此一举

* Return         : none           这个任务一直在运行着的  



*******************************************************************************
*/
void Protocol_Data_Send_Op(void)
{     
     switch(gs_Motor_Param.bySendDataFlag)//发送标识        上电时， 这个标志是应答空闲的
     {
         case ACK_IDLE://应答空闲        所以在没有数据要发送的时候，这个值就在这里返回了
         {
         }
         break;
         
         case PARAM_SET_ACK://参数帧应答    在参数接收到之后，如果空闲，则将这个标志设为参数应答PARAM_SET_ACK  如果不空，则什么事也没做，上位机估计超时后会发现白发了
         {
             if(Is_Usart2_TxEnd())//发送完成     看这个s_bUsart2TxEndFlag==1否？ s_bUsart2TxEndFlag初始化时就是1的，另外在发送等待时间过后，它也会置为1
             {
                 Usart2_ReadForTx();//发送前准备       准备一是发送允许，另外是s_byUsart2StartTxDelay=6  开始发送前稍等片刻  发送之前为什么要等一下呢？担心前面没发送好吗还是？
                 gs_Motor_Param.bySendDataFlag = ACK_WAIT;//应答等待
             }
         }
         break;
         
         case ACK_WAIT://应答等待          等6个100us
         {
             if(Is_Uart2StartTxDelayEnd())//发送前延时完成
             {
                 Send_ParamFrameAckData();//发送参数应答帧                    s_byUsart2TxEndDelay=10
                 SetSendAck_TimeOut(TIME_20MS);//设置应答超时时间500ms       gs_Motor_Param.dwSendAckTimeout = timeout;      20210715  改为200毫秒试一下看
                 gs_Motor_Param.bySendDataFlag = ACK_SEND;//发送应答
             }
         }
         break;
         
         case ACK_SEND://发送应答  等500毫秒后，就要转到串口接收了，在这500毫秒内，是不接收任何命令的  这就要求上位机在收到应答后，至少要等500毫秒才发命令。不然呢？
         {
             if(Is_SendAckTimeout() && (!Is_Usart2_TxEnd()))//发送超时，重启串口  等了两个，一个是500ms  另一个却只有 1000us   注意到这个变量的设置是在DMA完成的中断函数中
             {
                 Usart2_StartRx();                     //启动下一次的接收  DMA居然要重新初始化 这个有点奇怪
                 
                 Usart2_TxEnd();    //置发送完成延时       s_byUsart2TxEndDelay = 10   过一会儿之后，就将s_bUsart2TxEndFlag = SET; 且空闲，且允许接收了              
             }
         }
         break;
           
         
         default:
         {
         }
         break;
     }
     
}

/*
*******************************************************************************
* Function Name  : Clear_MotorFault
* Description    : 清电机故障
* Input          : none
* Output         : none
* Return         : none
*******************************************************************************
*/
void Clear_MotorFault(void)
{
    gs_Motor_Param.byMotorStatus = 0;
}

/*
*******************************************************************************
* Function Name  : Set_MotorErr
* Description    : 设置电机故障位
* Input          : ErrBit
* Output         : none
* Return         : none
*******************************************************************************
*/
void Set_MotorErr(MOTOR_ERR_STATE ErrBit)
{
    gs_Motor_Param.byMotorStatus |= ErrBit;
#if (KEY_TEST == 1)   
    gs_Motor_Param.bySendDataFlag = 1;
#endif
}

/*
*******************************************************************************
* Function Name  : Clear_MotorErr
* Description    : 清电机故障位
* Input          : ErrBit
* Output         : none
* Return         : none
*******************************************************************************
*/
void Clear_MotorErr(MOTOR_ERR_STATE ErrBit)
{
    gs_Motor_Param.byMotorStatus &= (~ErrBit);
}

/*
*******************************************************************************
* Function Name  : MotorTimerDecrease
* Description    : 电机延迟运行和运行时间递减函数
* Input          : none
* Output         : none
* Return         : none
*******************************************************************************
*/
void MotorTimerDecrease(void)
{
    if(DECREASE_DELAY == gs_Motor_Param.byRunOrDelayFlag)//延时运行递减
    {
        if(gs_Motor_Param.dwMotorDelayTimer > 0)
        {
            gs_Motor_Param.dwMotorDelayTimer--;
        }
    }
    else if(DECREASE_RUN == gs_Motor_Param.byRunOrDelayFlag)//运行递减
    {
        if(gs_Motor_Param.dwMotorRunTimer > 0)
        {
            gs_Motor_Param.dwMotorRunTimer--;
        }
    }
    else
    {
    }
}


