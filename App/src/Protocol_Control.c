/*
********************** (C) COPYRIGHT 2020 SOLAR  *************************
* File Name          : Protocol_Control.c
* Author             : dingyongjian 
* Date First Issued  : 
* Description        : bldc�������
********************************************************************************
* History:
* 11/19/2020 v1.0
*
********************************************************************************
*/
#define      PROTOCOL_CONTROL_GLOBALS

#include "include.h"
#include "main.h"

/* ˽�б��� ------------------------------------------------------------------*/
//S_MOTOR_PARAM gs_Motor_Param;
//��ַ����˿�
GPIO_TypeDef *DeviceAddrGPIO_Port[6] = {DEVICE_ADDR1_PORT,DEVICE_ADDR2_PORT,DEVICE_ADDR4_PORT,\
                                        DEVICE_ADDR8_PORT,DEVICE_ADDR16_PORT,DEVICE_ADDR32_PORT};
uint16_t DeviceAddrGPIO_Pin[6] = {DEVICE_ADDR1_PIN,DEVICE_ADDR2_PIN,DEVICE_ADDR4_PIN,\
                                  DEVICE_ADDR8_PIN,DEVICE_ADDR16_PIN,DEVICE_ADDR32_PIN};
/* Private functions ------------------------------------------------------- */

/*
*******************************************************************************
* Function Name  : Device_Info_Init
* Description    : �豸��ַ��С����λ�ų�ʼ��
* Input          : none
* Output         : none
* Return         : none
*******************************************************************************
*/
void Device_Info_Init(void)
{
    u8 i;
    u8 byMod = 0;//����
    u8 byMult = 0;//����
    gs_Motor_Param.byDevAddr = 0;
    for(i = 0; i < ADDR_BIT_LEN; i++)            ///ֻ��6λ
    {
        if(Bit_RESET == GPIO_ReadInputDataBit(DeviceAddrGPIO_Port[i],DeviceAddrGPIO_Pin[i]))//�͵�ƽ
        { 
            gs_Motor_Param.byDevAddr |= (1 << i);
        }
    }
    
    //����2GPIO�쳣
   /* if(Bit_RESET == GPIO_ReadInputDataBit(DeviceAddrGPIO_Port[1],DeviceAddrGPIO_Pin[1]))
    {
        gs_Motor_Param.byDevAddr &= ~(1 << 1);
    }
    else
    {
        gs_Motor_Param.byDevAddr &= ~(1 << 1);
    }*/
      
    byMod  = gs_Motor_Param.byDevAddr % 8;//����%8����
    byMult = (gs_Motor_Param.byDevAddr >> 3);//����/8����
    
    if(0 == byMod)//��ַΪ8�ı�������5��        ֻ��8 16 24 32 ��4�����
    {
        gs_Motor_Param.byGroupNum = M_GROUP_5;//��5��  = 4
        if(byMult <= 4)//��ַ������32
        {
            gs_Motor_Param.byGroupBit = (1 << (byMult - 1));//����λ��        ԭ��Ϊ��(1 << (byMult + 2))   ��Ϊ8�Ƶ�0λ��
        }
        else
        {
            gs_Motor_Param.byGroupBit = 0;
        }
    }
    else//��ַ��Ϊ8�ı�����1~4��
    {
        gs_Motor_Param.byGroupNum = byMult;
        gs_Motor_Param.byGroupBit = (1 << (byMod - 1));//����λ��
    }
}
/*
*******************************************************************************
* Function Name  : Motor_Param_DataInit
* Description    : ������в�����ʼ��
* Input          : none
* Output         : none
* Return         : none
*******************************************************************************
*/
void Motor_Param_DataInit(void)
{
    memset(&gs_Motor_Param,0,sizeof(S_MOTOR_PARAM));
    Device_Info_Init();//�豸��ַ��С����λ�ų�ʼ��
}

/*
*******************************************************************************
* Function Name  : Get_Data_Xor
* Description    : XORУ�麯��
* Input          : u8 *nData, u16 wLength
* Output         : none
* Return         : XorResult
*******************************************************************************
*/
u8 Get_Data_Xor(u8 *nData, u16 wLength) 
{
    u8 XorResult = 0;
    
    if(wLength < 2)//����С��2
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
* Description    : У��������ݸ�ʽ
* Input          : PROTOCOL_CMD_HEAD *p_cmdHead, u16 totalLen
* Output         : none
* Return         : TRUE OR FALSE

�������ҲҪ�޸�һ�£����ںϲ���һ������ҲҪ��ʶ�����

���ںϲ���������һ����У��ͣ����ǾͲ������ˡ�


�����һ�£�����������ȴ���16  ������Ҫ��������������Ƿ�������������ѵĵ�ַ  Ȼ���������ַ����Ϣ��ȡ����


*******************************************************************************
*/
BOOLEAN CheckReceiveData(PROTOCOL_REC_HEAD *p_cmdHead, u16 wLen)
{
    if((wLen != NOMAL_DATA_LEN) && (wLen != (2*NOMAL_DATA_LEN))) //���ݳ��Ȳ�����  ������8���߲�����16
    {
        return FALSE;
    }
    
    if((((PARAM_SET_CMD == p_cmdHead->byCmd)|| (PARAM_SET_NOACK_CMD == p_cmdHead->byCmd))                \
         &&((p_cmdHead->byRecData[DIR_ADDR] & 0x3F)== gs_Motor_Param.byDevAddr))\
       ||(BROADCAST_CMD == p_cmdHead->byCmd))//�������������㲥����
    {
      return TRUE; // modified by hjl, 202110230113
/*      if(Get_Data_Xor(((u8 *)p_cmdHead + 1),6) != p_cmdHead->byXorCode)//XORУ�鲻�ϸ� 2~7�ֽ�
        {
            return FALSE;  ////���ǿ����������������16ʱ���ж�һ�������2�������У����Ƿ���ȷ
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
* Description    : ���ݽ�����������
  �������ķ������в���֡��
  ����������Ӧ��Ȼ���
  Ȼ��������ķ�����������֡��
  ÿ������֡ǰ������һ����֡������������������ ���յ�����֡�����յ��㲥֡����ʱ�������������
* Input          : PROTOCOL_REC_HEAD *p_cmdHead, u16 totalLen
* Output         : none
* Return         : none      �����������һֱ���еģ�ֻ���յ����ݺ�Ż�����һ��

20210805  ����һ������������λ��������������ϲ�����



*******************************************************************************
*/
static void Data_Analysis_Op(u8* data, u16 wLen)
{
    PROTOCOL_REC_HEAD *p_cmdHead = (PROTOCOL_REC_HEAD *) data;
    
    if(CheckReceiveData(p_cmdHead, wLen))//����У��ͨ��
    {
        switch(p_cmdHead->byCmd)
        {
            case PARAM_SET_CMD://������������ ----�����������������ϲ��ģ�Ҫ���ֿ�����
            {
              
              if(wLen == NOMAL_DATA_LEN)   ///�������ͨ������ȣ��Ͱ�����ͨ������
              {
              
                gs_Motor_Param.sMotorParam_Set.byRunDir = ((p_cmdHead->byRecData[DIR_ADDR] >> 6) & (0x01));
                gs_Motor_Param.sMotorParam_Set.byRunSpeed = (p_cmdHead->byRecData[RUN_SPEED] & 0x7F);
                gs_Motor_Param.sMotorParam_Set.byDelayTimer = (p_cmdHead->byRecData[DELAY_L7B] & 0x7F);	// 20211115 hjl               
//                gs_Motor_Param.sMotorParam_Set.byDelayTimer = 0; // modified by hjl, 202110230056
//               gs_Motor_Param.sMotorParam_Set.byDelayTimer =    (p_cmdHead->byRecData[DELAY_L7B] & 0x7F) + (p_cmdHead->byRecData[DELAY_H1B] << 7);               
                gs_Motor_Param.sMotorParam_Set.byRunTimer = (p_cmdHead->byRecData[RUN_TIMER] & 0x7F);
               
                gs_Motor_Param.byRunTrig = GET_PARAM_FRAME;//���ý��յ�����֡
                
                if(PARAM_FRAME_CMD == gs_Motor_Param.byCmdRec_Flag)//ǰһ֡Ϊ����֡
                {
                    Set_MotorErr(M_LOST_ACT);//���ò���֮ǰ�޶���ָ�����       ����Ӱ�����У�ֻ������λ���������
                }
                else
                {
                    Clear_MotorErr(M_LOST_ACT);//�����֮ǰ�޶���ָ�����
                }               
                gs_Motor_Param.byCmdRec_Flag = PARAM_FRAME_CMD;
                
                if(ACK_IDLE == gs_Motor_Param.bySendDataFlag)//Ӧ�����   ��һ�������أ��Ѳ��ɾͲ�Ӧ������  Ҳ����λ������һ��ʱ���ڣ���û���յ�Ӧ�������ٴ����·�����������
                {
                    gs_Motor_Param.bySendDataFlag = PARAM_SET_ACK;//���ò���֡Ӧ��  �����������Ҫ��
                }
              }
              else if(wLen == 2*NOMAL_DATA_LEN)      ///����˵������������������
              {
////�Ƚ�����1�����
                gs_Motor_Param.sMotorParam_Set.byRunDir = ((p_cmdHead->byRecData[DIR_ADDR] >> 6) & (0x01));
                gs_Motor_Param.sMotorParam_Set.byRunSpeed = (p_cmdHead->byRecData[RUN_SPEED] & 0x7F);
                gs_Motor_Param.sMotorParam_Set.byDelayTimer = (p_cmdHead->byRecData[DELAY_L7B] & 0x7F);	// 20211115 hjl               
//                gs_Motor_Param.sMotorParam_Set.byDelayTimer = 0; // modified by hjl, 202110230056
//              gs_Motor_Param.sMotorParam_Set.byDelayTimer =    (p_cmdHead->byRecData[DELAY_L7B] & 0x7F) + (p_cmdHead->byRecData[DELAY_H1B] << 7);               
                gs_Motor_Param.sMotorParam_Set.byRunTimer = (p_cmdHead->byRecData[RUN_TIMER] & 0x7F);
               
                gs_Motor_Param.byRunTrig = GET_PARAM_FRAME;//���ý��յ�����֡
                
                if(PARAM_FRAME_CMD == gs_Motor_Param.byCmdRec_Flag)//ǰһ֡Ϊ����֡
                {
                    Set_MotorErr(M_LOST_ACT);//���ò���֮ǰ�޶���ָ�����                ����Ӱ�����У�ֻ������λ���������
                }
                else
                {
                    Clear_MotorErr(M_LOST_ACT);//�����֮ǰ�޶���ָ�����
                }               
                gs_Motor_Param.byCmdRec_Flag = PARAM_FRAME_CMD;
                
                if(ACK_IDLE == gs_Motor_Param.bySendDataFlag)//Ӧ�����   ��һ�������أ��Ѳ��ɾͲ�Ӧ������  Ҳ����λ������һ��ʱ���ڣ���û���յ�Ӧ�������ٴ����·�����������
                {
                   gs_Motor_Param.bySendDataFlag = PARAM_SET_ACK;//���ò���֡Ӧ��  �����������Ҫ��
                }

  ///�ٽ�����2������              
                p_cmdHead = (PROTOCOL_REC_HEAD *) (data+8);

                if((p_cmdHead->byRecData[gs_Motor_Param.byGroupNum] & gs_Motor_Param.byGroupBit)\
                    == gs_Motor_Param.byGroupBit)//��ǰС����������֡
                {
//                  if( gs_Motor_Param.byRunTrig == TRIG_ILDE)//    ����   

           if(GET_PARAM_FRAME == gs_Motor_Param.byRunTrig)//�ѽ��յ�����֡  û�յ�����֡���򲻻�������д������ñ�־
                    {
                        gs_Motor_Param.byRunTrig = TRIG_MOTOR_RUN;//����������������    ����ӵ���������֡���Ż��д����������  ���������־
                    }
                    
                    if(RUN_FRAME_CMD == gs_Motor_Param.byCmdRec_Flag)//ǰһ֡Ϊ����˶�֡
                    {
                        Set_MotorErr(M_LOST_PARAM);//���ö���֮ǰ�޲���ָ�����  �����������ô�죬�Ѳ��ɵ���Ͳ�ת�˻���ʲô�������Ҳû�У�ֻ������λ������һ�¶���
                    }                                                                                                                     ///����Ӱ������
                    else
                    {
                        Clear_MotorErr(M_LOST_PARAM);//�嶯��֮ǰ�޲���ָ�����
                    }               
                    gs_Motor_Param.byCmdRec_Flag = RUN_FRAME_CMD;     ////�����ʼ������ ע�⵽��ʱ�������־��
                }                                                     ///�´��ٷ���������Ҳû�� �����־��Ŀ�ľ���Ϊ���ȷ�����֡�ٷ��㲥֡
                                                                      
            }
                
              else{}        ////��Ȼʲô��Ҳ����
              }
            break;
            case PARAM_SET_NOACK_CMD:        ///���������Ӧ�������������
              {
             if(wLen == NOMAL_DATA_LEN)   ///�������ͨ������ȣ��Ͱ�����ͨ������
              {
 
                gs_Motor_Param.sMotorParam_Set.byRunDir = ((p_cmdHead->byRecData[DIR_ADDR] >> 6) & (0x01));
                gs_Motor_Param.sMotorParam_Set.byRunSpeed = (p_cmdHead->byRecData[RUN_SPEED] & 0x7F);
                gs_Motor_Param.sMotorParam_Set.byDelayTimer = (p_cmdHead->byRecData[DELAY_L7B] & 0x7F);	// 20211115 hjl               
//                gs_Motor_Param.sMotorParam_Set.byDelayTimer = 0; // modified by hjl, 202110230056
//                  (p_cmdHead->byRecData[DELAY_L7B] & 0x7F) + (p_cmdHead->byRecData[DELAY_H1B] << 7);               
                gs_Motor_Param.sMotorParam_Set.byRunTimer = (p_cmdHead->byRecData[RUN_TIMER] & 0x7F);
               
                gs_Motor_Param.byRunTrig = GET_PARAM_FRAME;//���ý��յ�����֡
                
                if(PARAM_FRAME_CMD == gs_Motor_Param.byCmdRec_Flag)//ǰһ֡Ϊ����֡
                {
                    Set_MotorErr(M_LOST_ACT);//���ò���֮ǰ�޶���ָ�����       ����Ӱ�����У�ֻ������λ���������
                }
                else
                {
                    Clear_MotorErr(M_LOST_ACT);//�����֮ǰ�޶���ָ�����
                }               
                gs_Motor_Param.byCmdRec_Flag = PARAM_FRAME_CMD;
                
               if(ACK_IDLE == gs_Motor_Param.bySendDataFlag)//Ӧ�����   ��һ�������أ��Ѳ��ɾͲ�Ӧ������  Ҳ����λ������һ��ʱ���ڣ���û���յ�Ӧ�������ٴ����·�����������
                {
//                    gs_Motor_Param.bySendDataFlag = PARAM_SET_ACK;//���ò���֡Ӧ��  �����������Ҫ��
               }              
              }
              else if(wLen == 2*NOMAL_DATA_LEN)      ///����˵������������������
              {
////�Ƚ�����1�����
                gs_Motor_Param.sMotorParam_Set.byRunDir = ((p_cmdHead->byRecData[DIR_ADDR] >> 6) & (0x01));
                gs_Motor_Param.sMotorParam_Set.byRunSpeed = (p_cmdHead->byRecData[RUN_SPEED] & 0x7F);
                gs_Motor_Param.sMotorParam_Set.byDelayTimer = (p_cmdHead->byRecData[DELAY_L7B] & 0x7F);	// 20211115 hjl               
//                gs_Motor_Param.sMotorParam_Set.byDelayTimer = 0; // modified by hjl, 202110230021
//                  (p_cmdHead->byRecData[DELAY_L7B] & 0x7F) + (p_cmdHead->byRecData[DELAY_H1B] << 7);               
                gs_Motor_Param.sMotorParam_Set.byRunTimer = (p_cmdHead->byRecData[RUN_TIMER] & 0x7F);
               
                gs_Motor_Param.byRunTrig = GET_PARAM_FRAME;//���ý��յ�����֡
                
                if(PARAM_FRAME_CMD == gs_Motor_Param.byCmdRec_Flag)//ǰһ֡Ϊ����֡
                {
                    Set_MotorErr(M_LOST_ACT);//���ò���֮ǰ�޶���ָ�����                ����Ӱ�����У�ֻ������λ���������
                }
                else
                {
                    Clear_MotorErr(M_LOST_ACT);//�����֮ǰ�޶���ָ�����
                }               
                gs_Motor_Param.byCmdRec_Flag = PARAM_FRAME_CMD;
                
                if(ACK_IDLE == gs_Motor_Param.bySendDataFlag)//Ӧ�����   ��һ�������أ��Ѳ��ɾͲ�Ӧ������  Ҳ����λ������һ��ʱ���ڣ���û���յ�Ӧ�������ٴ����·�����������
                {
//                    gs_Motor_Param.bySendDataFlag = PARAM_SET_ACK;//���ò���֡Ӧ��  �����������Ҫ��
                }

  ///�ٽ�����2������              
                p_cmdHead = (PROTOCOL_REC_HEAD *) (data+8);

                if((p_cmdHead->byRecData[gs_Motor_Param.byGroupNum] & gs_Motor_Param.byGroupBit)\
                    == gs_Motor_Param.byGroupBit)//��ǰС����������֡
                {
//                  if( gs_Motor_Param.byRunTrig == TRIG_ILDE)//    ����   

           if(GET_PARAM_FRAME == gs_Motor_Param.byRunTrig)//�ѽ��յ�����֡  û�յ�����֡���򲻻�������д������ñ�־
                    {
                        gs_Motor_Param.byRunTrig = TRIG_MOTOR_RUN;//����������������    ����ӵ���������֡���Ż��д����������  ���������־
                    }
                    
                    if(RUN_FRAME_CMD == gs_Motor_Param.byCmdRec_Flag)//ǰһ֡Ϊ����˶�֡
                    {
                        Set_MotorErr(M_LOST_PARAM);//���ö���֮ǰ�޲���ָ�����  �����������ô�죬�Ѳ��ɵ���Ͳ�ת�˻���ʲô�������Ҳû�У�ֻ������λ������һ�¶���
                    }                                                                                                                     ///����Ӱ������
                    else
                    {
                        Clear_MotorErr(M_LOST_PARAM);//�嶯��֮ǰ�޲���ָ�����
                    }               
                    gs_Motor_Param.byCmdRec_Flag = RUN_FRAME_CMD;     ////�����ʼ������ ע�⵽��ʱ�������־��
                }                                                     ///�´��ٷ���������Ҳû�� �����־��Ŀ�ľ���Ϊ���ȷ�����֡�ٷ��㲥֡
                                                                      
            }
                
              else{}        ////��Ȼʲô��Ҳ����
             
             
             
              
              }
              break;
        
            case BROADCAST_CMD://�㲥����   �㲥��������Ƕ�ͬһȺ�ĵ����������
            {
                if((p_cmdHead->byRecData[gs_Motor_Param.byGroupNum] & gs_Motor_Param.byGroupBit)\
                    == gs_Motor_Param.byGroupBit)//��ǰС����������֡
                {
//                  if( gs_Motor_Param.byRunTrig == TRIG_ILDE)//    ����   

           if(GET_PARAM_FRAME == gs_Motor_Param.byRunTrig)//�ѽ��յ�����֡  û�յ�����֡���򲻻�������д������ñ�־
                    {
                        gs_Motor_Param.byRunTrig = TRIG_MOTOR_RUN;//����������������    ����ӵ���������֡���Ż��д����������  ���������־
                    }
                    
                    if(RUN_FRAME_CMD == gs_Motor_Param.byCmdRec_Flag)//ǰһ֡Ϊ����˶�֡
                    {
                        Set_MotorErr(M_LOST_PARAM);//���ö���֮ǰ�޲���ָ�����  �����������ô�죬�Ѳ��ɵ���Ͳ�ת�˻���ʲô�������Ҳû�У�ֻ������λ������һ�¶���
                    }                                                                                                                     ///����Ӱ������
                    else
                    {
                        Clear_MotorErr(M_LOST_PARAM);//�嶯��֮ǰ�޲���ָ�����
                    }               
                    gs_Motor_Param.byCmdRec_Flag = RUN_FRAME_CMD;     ////�����ʼ������ ע�⵽��ʱ�������־��
                }                                                     ///�´��ٷ���������Ҳû�� �����־��Ŀ�ľ���Ϊ���ȷ�����֡�ٷ��㲥֡
                                                                      
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
* Description    : Э�����ݽ��մ�������
* Input          : none
* Output         : none  �������һֱ������
* Return         : none
*******************************************************************************
*/
void Protocol_Data_Receive_Op(void)
{
    static u8  byComDataRecBuffer[MAX_LEN_CMD]; //���վ����յ��ľ�������
//    static u8  byComDataRecBuffer_16[16];
    static u16 wComDataRecLength;//�������ݳ���
    u8 i;
    u8 *p_rec;
 
    
        PROTOCOL_REC_HEAD *p_cmdHead; 

    
    if (Com2_Recv(byComDataRecBuffer, &wComDataRecLength))//�����ϲ㷢�͵�����
    {
        if(wComDataRecLength>16)                 //������һ��������Ա�����
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
          return;              ////û���ҵ���ֱ�ӷ�����
          
        }
        else{
        Data_Analysis_Op(byComDataRecBuffer, wComDataRecLength);
        }
    }
}


/*
*******************************************************************************
* Function Name  : Send_ParamFrameAckData
* Description    : ���Ͳ���֡Ӧ���
* Input          : none
* Output         : none  
* Return         : none
*******************************************************************************
*/
void Send_ParamFrameAckData(void)
{
    static u8 Ack_Data[4] = {PARAM_ACK_CMD,0,0,0};                   //��ַ0 �������Ӧ��  
                                                              
    Ack_Data[ADDR_ACK]   = gs_Motor_Param.byDevAddr;                  //��ַ1
    Ack_Data[STATUS_ACK] = gs_Motor_Param.byMotorStatus;              //2     ���Ƿ��д�����������У���Ϊ1�����û�У��������0       
    Ack_Data[XOR_ACK]    = (Ack_Data[ADDR_ACK] ^ Ack_Data[STATUS_ACK]);  ///3
    Usart2_StartTx(Ack_Data,4);                      ////����������У������ͱ�־������־��ΪRESET�ˡ�
}

/*
*******************************************************************************
* Function Name  : SetSendAck_TimeOut
* Description    : ���÷��ͳ�ʱʱ��
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
* Description    : ���ͳ�ʱʱ��--
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
* Description    : �ж��Ƿ�ʱ
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
* Function Name  : Protocol_Data_Send_Op         ��ʵ�ϣ���������������һ�������ݽ�������һ�������ݷ������� �������ʱ�޿������е� 
* Description    : Э�����ݷ��ʹ�������
* Input          : none
* Output         : none      ���Ǹ����񣬲�������  ƽʱһ����IDLE״̬�£�һ�����յ����ݣ��ͻ����һ��״̬��ת����
�����ת���У��в�����ʱ������Ҫ����������ɺ����DMA����жϣ������һ���������ú�������������һ��ֵ����һ�¡�
�ڵȴ�500������Լ�������ɱ�־Ϊ0�������ͬʱ����������-----------����һ�����⣬�������û��ͬʱ���������Ϊһ��ʼ��500����û���㣬�����������Ϊ�����ж��Ѿ�������
�����2�������Ѿ��������˰�����ô���£��㲻�����˰�������������Ϊ�����ж��Ѿ������ˣ���ʱ������һ����ʱ  s_byUsart2TxEndDelay = 10; ���ǿ�ʼ�������� ���һ���Ӿͼ�����
��������s_bUsart2TxEndFlag = SET; ������������Ͳ������ˣ���ô�찡�� ���������ˡ���������û���
///����˰������s_bUsart2TxEndFlag = RESET  �����ڷ��ͺ��������õģ�������������İ�������������ΪʲôҪ�Ӹ�����ж��أ��ǲ��ǿ���ȥ���أ��е����ɣ������ڷ��ͺ����н����RESET�ˣ���Ҫ���һ��

* Return         : none           �������һֱ�������ŵ�  



*******************************************************************************
*/
void Protocol_Data_Send_Op(void)
{     
     switch(gs_Motor_Param.bySendDataFlag)//���ͱ�ʶ        �ϵ�ʱ�� �����־��Ӧ����е�
     {
         case ACK_IDLE://Ӧ�����        ������û������Ҫ���͵�ʱ�����ֵ�������ﷵ����
         {
         }
         break;
         
         case PARAM_SET_ACK://����֡Ӧ��    �ڲ������յ�֮��������У��������־��Ϊ����Ӧ��PARAM_SET_ACK  ������գ���ʲô��Ҳû������λ�����Ƴ�ʱ��ᷢ�ְ׷���
         {
             if(Is_Usart2_TxEnd())//�������     �����s_bUsart2TxEndFlag==1�� s_bUsart2TxEndFlag��ʼ��ʱ����1�ģ������ڷ��͵ȴ�ʱ�������Ҳ����Ϊ1
             {
                 Usart2_ReadForTx();//����ǰ׼��       ׼��һ�Ƿ�������������s_byUsart2StartTxDelay=6  ��ʼ����ǰ�Ե�Ƭ��  ����֮ǰΪʲôҪ��һ���أ�����ǰ��û���ͺ����ǣ�
                 gs_Motor_Param.bySendDataFlag = ACK_WAIT;//Ӧ��ȴ�
             }
         }
         break;
         
         case ACK_WAIT://Ӧ��ȴ�          ��6��100us
         {
             if(Is_Uart2StartTxDelayEnd())//����ǰ��ʱ���
             {
                 Send_ParamFrameAckData();//���Ͳ���Ӧ��֡                    s_byUsart2TxEndDelay=10
                 SetSendAck_TimeOut(TIME_20MS);//����Ӧ��ʱʱ��500ms       gs_Motor_Param.dwSendAckTimeout = timeout;      20210715  ��Ϊ200������һ�¿�
                 gs_Motor_Param.bySendDataFlag = ACK_SEND;//����Ӧ��
             }
         }
         break;
         
         case ACK_SEND://����Ӧ��  ��500����󣬾�Ҫת�����ڽ����ˣ�����500�����ڣ��ǲ������κ������  ���Ҫ����λ�����յ�Ӧ�������Ҫ��500����ŷ������Ȼ�أ�
         {
             if(Is_SendAckTimeout() && (!Is_Usart2_TxEnd()))//���ͳ�ʱ����������  ����������һ����500ms  ��һ��ȴֻ�� 1000us   ע�⵽�����������������DMA��ɵ��жϺ�����
             {
                 Usart2_StartRx();                     //������һ�εĽ���  DMA��ȻҪ���³�ʼ�� ����е����
                 
                 Usart2_TxEnd();    //�÷��������ʱ       s_byUsart2TxEndDelay = 10   ��һ���֮�󣬾ͽ�s_bUsart2TxEndFlag = SET; �ҿ��У������������              
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
* Description    : ��������
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
* Description    : ���õ������λ
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
* Description    : ��������λ
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
* Description    : ����ӳ����к�����ʱ��ݼ�����
* Input          : none
* Output         : none
* Return         : none
*******************************************************************************
*/
void MotorTimerDecrease(void)
{
    if(DECREASE_DELAY == gs_Motor_Param.byRunOrDelayFlag)//��ʱ���еݼ�
    {
        if(gs_Motor_Param.dwMotorDelayTimer > 0)
        {
            gs_Motor_Param.dwMotorDelayTimer--;
        }
    }
    else if(DECREASE_RUN == gs_Motor_Param.byRunOrDelayFlag)//���еݼ�
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


