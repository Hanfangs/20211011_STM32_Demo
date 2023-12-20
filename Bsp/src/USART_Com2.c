/*
********************** (C) COPYRIGHT 2019 SOLAR SUPCON *************************
* File Name          : USART_Com2.c
* Author             : dingyongjian 
* Date First Issued  : 02/01/15
* Description        : ����RS485ͨѶ C�ļ�,��������
********************************************************************************
* History:
* 02/01/19 v1.0
********************************************************************************
*/
#define USART2_GLOBALS

/* Includes-------------------------------------------------------------------*/
#include "include.h"
#include "main.h"
/* Private variables ---------------------------------------------------------*/
#define USART2_RXBUFFER_LEN  256      //USART����������
static u8 s_byUsart2RxBuffer[USART2_RXBUFFER_LEN]; //USART���ջ�����
#define USART2_TXBUFFER_LEN  256       //USART����������
static u8 s_byUsart2TxBuffer[USART2_TXBUFFER_LEN]; //USART���ͻ�����

static FlagStatus s_bUsart2RxEndFlag = RESET;//���ս�����־         0
static FlagStatus s_bUsart2TxEndFlag = SET;//���ͽ�����־           1 

static u8 s_byUsart2StartTxDelay = 0;//��ʼ������ʱ
static u8 s_byUsart2TxEndDelay = 0;//����������ʱ


/*Private functions ---------------------------------------------------------*/
/*
*******************************************************************************
* Function Name  : Init_Usart2
* Description    : ����Usart2
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************
*/
void Init_Usart2(void)
{
    GPIO_InitTypeDef GPIO_InitStructure; //RS485�õ���GPIO 
    NVIC_InitTypeDef NVIC_InitStructure; //�ж���������������ʼ���ж����ȼ�   
    USART_InitTypeDef USART_InitStructure;//USART����
    
    /*����RS485ͨѶʱ�õ���GPIO��ʱ��*/
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB ,ENABLE);     
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA ,ENABLE);  
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO ,ENABLE); 
    
    /*��ʼ��RS485ͨѶʱ�õ���GPIO*/
        //GPIO_PinRemapConfig(GPIO_Remap_Usart2, ENABLE);

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2; 
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz; 
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP; 
    GPIO_Init(GPIOA, &GPIO_InitStructure); 
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3; 
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING; 
    GPIO_Init(GPIOA, &GPIO_InitStructure); 
    /*GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1; 
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 
    GPIO_Init(GPIOA, &GPIO_InitStructure); */           
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;            ///RS485������������ﶨ����
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 
    GPIO_Init(GPIOB, &GPIO_InitStructure);
    
    /*����Usart2��ʱ��*/
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);

    /*����Usart2*/
    USART_InitStructure.USART_BaudRate = BAUDRATE_FRE;   //ͨѶ������
    /*0��һ����ʼλ��8������λ��n��ֹͣλ*/
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    /*00��1��ֹͣλ��*/
    USART_InitStructure.USART_StopBits = USART_StopBits_1;  
    /*0����ֹУ�����*/
    USART_InitStructure.USART_Parity = USART_Parity_No;
    /*0����ֹCTSӲ�������ƣ�0����ֹRTSӲ�������ƣ�*/
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    /*1��ʹ�ܷ��͡�1��ʹ�ܽ��գ�����ʼ��ѰRX�����ϵ���ʼλ��*/
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx; 
    /*��ʼ��USART3*/
    USART_Init(USART2, &USART_InitStructure); 
    
    /*ʹ��Usart2*/
    USART_Cmd(USART2, ENABLE); 
    
    /*����DMA1��ʱ��*/
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);//**
    
    /*�����ж�����������*/
    NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;              ////�����ж�������  �����Ǹ����������˻�Ҫ����һ�����������緢���жϣ���������жϵȵ�
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 5; 
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0; 
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE; 
    NVIC_Init(&NVIC_InitStructure);
    
    /*�����ж�����������*/
    NVIC_InitStructure.NVIC_IRQChannel = DMA1_Channel7_IRQn;//**           DMA�ж�Ҳ������   �ж�������ʲô��һ��GXD  ����Ƿ���DMA����ж�  ���ǽ���DMA����ж�ȴ��û�е�
    NVIC_Init(&NVIC_InitStructure);                                        ///��Ϊ������ɣ��ǿ�IDLE�ж���ʵ�ֵģ���Ϊ�㲻֪������ն��ٸ��ֽڹ�����ֻ���Ǹ������жϲ�֪��
    
    Usart2_RxEnable();                       ////RS485��������
    Usart2_StartRx();      ///
}

/*����ʹ��*/
void Usart2_RxEnable(void)
{
    //GPIO_ResetBits(GPIOA,GPIO_Pin_1);//
    GPIO_ResetBits(GPIOB,GPIO_Pin_11);// ����485
}

/*����ʹ��*/
void Usart2_TxEnable(void)
{
    //GPIO_SetBits(GPIOA,GPIO_Pin_1);// 
    GPIO_SetBits(GPIOB,GPIO_Pin_11);//����485
}

/*
*******************************************************************************
* Function Name  : Usart2_StartRx
* Description    : USART��ʼ����    ����2�Ľ�����DMAͨ��6
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************
*/
void Usart2_StartRx(void)
{
    DMA_InitTypeDef DMA_InitStructure;
  
    s_bUsart2RxEndFlag = RESET;       /////�������Ҫ
      
    DMA_DeInit(DMA1_Channel6);
    DMA_InitStructure.DMA_PeripheralBaseAddr = (u32)&(USART2->DR);              ///����Ǵ��ڵ�����
    DMA_InitStructure.DMA_MemoryBaseAddr = (u32)(s_byUsart2RxBuffer);           ////����Ǵ��ڵĽ��ջ�����   
    DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;
    DMA_InitStructure.DMA_BufferSize = USART2_RXBUFFER_LEN;                   ///256
    DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;           
    DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;                     
    DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;     ///  ���ֵ����0 
    DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
    DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
    DMA_InitStructure.DMA_Priority = DMA_Priority_VeryHigh;  
    DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
    DMA_Init(DMA1_Channel6, &DMA_InitStructure);    

    USART_GetFlagStatus(USART2, USART_FLAG_IDLE);
    USART_ReceiveData(USART2);                        ///��������ڶ�һ�� DR�Ĵ���  �ն�  ��DMA֮ǰ������ն�һ��GXD
    USART_ITConfig(USART2, USART_IT_IDLE,ENABLE);    ////���ڿ����жϱ�����  ������ж�������ʲô�أ�  ������������Ǵ��ڿ����жϣ���û�д����յ��ź�ʱ�������ʱ�����ж�
                                                    
    USART_DMACmd(USART2,USART_DMAReq_Rx,ENABLE);    ////���û�л����һ��ʼ�����IDLE���������Ч��������������֮�������������IDLE�Ż���Ч
    DMA_Cmd(DMA1_Channel6, ENABLE);
}

/*
*******************************************************************************
* Function Name  : Usart2_StartTx
* Description    : USART��ʼ����
* Input          : None                  ����2�ķ���ͨ����DMAͨ��7
* Output         : None
* Return         : None                   �ڷ����У�������DMAͨ��7�жϣ������ڽ���ʱ������Ŀ��Դ����ж�
*******************************************************************************
*/
void Usart2_StartTx(u8 *pbyData, u16 wLength)          ////ע�⵽����������Ⱥ���Ҫ  Ҫ�������ֽھ͵��ö��٣�
{
    DMA_InitTypeDef DMA_InitStructure;
    
    /*Usart2_TxEnable();
    
    s_byUsart2StartTxDelay = 3;
    
    s_bUsart2TxEndFlag = RESET;
    wLength = ( wLength < USART2_TXBUFFER_LEN) ? wLength : USART2_TXBUFFER_LEN;
    memcpy(s_byUsart2TxBuffer, pbyData, wLength);
    
    while(s_byUsart2StartTxDelay != 0)
    {
        ;
    }*/
    //2020-11-19�޸�  Is_Uart2StartTxDelayEnd   s_byUsart2StartTxDelay
    if(s_byUsart2StartTxDelay != 0)//��ʱδ��ɣ��˳�       ʲô�ط��õ������ʱ��?       ����֮ǰ����Ҫ��һ��  ��Ϊ�����û���꣬�����Ͻ���Ļ�������
    {
        return ;
    }
    
    s_bUsart2TxEndFlag = RESET;   ///��һ��Ҫ���������ʼ���ͣ���ʱ�ͽ����������ɱ�־����Ϊ0  ��ʾû�з������
    wLength = ( wLength < USART2_TXBUFFER_LEN) ? wLength : USART2_TXBUFFER_LEN;
    memcpy(s_byUsart2TxBuffer, pbyData, wLength);  
   
    DMA_DeInit(DMA1_Channel7);
    DMA_InitStructure.DMA_PeripheralBaseAddr = (u32)&(USART2->DR);
    DMA_InitStructure.DMA_MemoryBaseAddr = (u32)(s_byUsart2TxBuffer);
    DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralDST;
    DMA_InitStructure.DMA_BufferSize = wLength;                            ////�������Ҫ���͵��ֽڵĳ���
    DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
    DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
    DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
    DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
    DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
    DMA_InitStructure.DMA_Priority = DMA_Priority_High; 
    DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
    DMA_Init(DMA1_Channel7, &DMA_InitStructure); 
    
    DMA_ClearFlag(DMA1_FLAG_GL7);
    DMA_ITConfig(DMA1_Channel7, DMA_IT_TC, ENABLE);  ///����DMA�ж�  �ж���������ʲô���أ�     �������־������ Usart2_TxEnd();  ������ǽ� s_byUsart2TxEndDelay=10
                                                       ///Ҳ����˵�ڷ��ͽ����󣬵�һ����ǽ����� ���Ҫ�ȵ�ԭ���Ǵ��ڻ�û���꣬�ͽ���������صĻ������ͻ�û�н������������û�ܳɹ�����ȥ
    USART_DMACmd(USART2,USART_DMAReq_Tx,ENABLE); 
    DMA_Cmd(DMA1_Channel7, ENABLE);
}

/*���ս�����־    �����ʼ��֮���ֵ��RESET  ��ʾû�н��ս���      ��������п����ж��е��� ��ʾ������      */
void Usart2_RxEnd(void)
{
    s_bUsart2RxEndFlag = SET;
}

/*���ͽ�����־        */
void Usart2_TxEnd(void)
{
    s_byUsart2TxEndDelay = 10;
}

/*��ʼ�����ӳ�            ��ʱ��2�в��ϵĵ����������  */
void Usart2_StartTxDelay(void)
{
    if( s_byUsart2StartTxDelay > 0)
    {
        s_byUsart2StartTxDelay--;
    }
}

/*���ͽ�����ʱ ������ʱ������������ջᱻ����       �������һֱ�����õ� �ڶ�ʱ��2�ж��� */
void Usart2_TxEndDelay(void)
{
    if( s_byUsart2TxEndDelay > 0)
    {
        s_byUsart2TxEndDelay--;
        
        if(s_byUsart2TxEndDelay == 0)
        {
            s_bUsart2TxEndFlag = SET;
            gs_Motor_Param.bySendDataFlag = ACK_IDLE;//Ӧ�����    ��������־��  �ָ���״̬ 
            Usart2_RxEnable();
        }
    }
}

/*
*******************************************************************************
* Function Name  : Is_Uart4StartTxDelayEnd(void)
* Description    : USART����ǰ����
* Input          : None
* Output         : None
* Return         : trun or false
*******************************************************************************
*/
FlagStatus Is_Uart2StartTxDelayEnd(void)
{     
    if(0 == s_byUsart2StartTxDelay)//��ʱ���
    {
        return SET;
    }   
    
    return RESET;
}

/*
*******************************************************************************
* Function Name  : Usart2_ReadForTx
* Description    : USART����ǰ����
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************
*/
void Usart2_ReadForTx(void)
{     
    Usart2_TxEnable();
    
    s_byUsart2StartTxDelay = 6;
    
   // s_bUsart2TxEndFlag = RESET;       
}

/*�жϽ����Ƿ��Ѿ����    ����˵���1     */
FlagStatus Is_Usart2_RxEnd(void)
{
    return ((s_bUsart2RxEndFlag == 1) ?  SET:RESET);
}

/*�жϷ����Ƿ��Ѿ����*/
FlagStatus Is_Usart2_TxEnd(void)
{
    return (s_bUsart2TxEndFlag == 1) ?  SET:RESET;
}

/*������յ������ݵĳ���*/
u16 Usart2_GetRxLength(void)
{
    return ((u16)(USART2_RXBUFFER_LEN - DMA_GetCurrDataCounter(DMA1_Channel6)));
}

/*�Ѵ��ڽ��յ�������s_byUsart2RxBuffer������pbyData*/
void Usart2_GetRxData(u8 *pbyData, u16 wLength)
{
    memcpy(pbyData, s_byUsart2RxBuffer, wLength);
}


/*
*******************************************************************************
* Function Name  : ComMirrorGroup_Recv
* Description    : ���վ��鷢�͵�����
                   �ж����ݽ����Ƿ���ɣ��������������ݵ�pbyData����������һ�ν���
* Input          : None
* Output         : None        �������һֱ�ڱ������ŵ�  һ����ʼ���գ��ͻὫ���s_bUsart2RxEndFlag=0 ���������������һ���������ֱ�ӷ��أ�ֻ�е���������ж��вŻ���Ϊ1 ���Ǵ�����������
                                        �����Ͼ����������  ������˽��ղ���

///  ���ղ��ֱȽϼ�һЩ�����յ�һ���ַ���֮�󣬻����IDLE�жϣ��Ӷ�ʹ�����s_bUsart2RxEndFlag=1  ������һ�����������У����ǲ��ϵĲ�ѯ�����־�ģ�һ������1���ͻὫ������������Ȼ��
///  �����������


///  ���Ͳ����أ����͵Ļ��أ����ǵȽ��պ��˺󣬻������Э���У������Ϳ���ʱ������� gs_Motor_Param.bySendDataFlag = PARAM_SET_ACK;//���ò���֡Ӧ��  
��һ�������ֵ��������ڷ��ͺ���һֱ����Ϊһ������ �����еģ�����
       if(Is_Usart2_TxEnd())//�������     �����s_bUsart2TxEndFlag==1�� s_bUsart2TxEndFlag��ʼ��ʱ����1�ģ������ڷ������˵ȴ�ʱ�������Ҳ����Ϊ1
---ע�⵽����������У�void Usart2_TxEnd(void)������ʱֵ��Ϊ10 ��������ɺ� ��������Ӧ���Ҫ��500����Ż��������գ�����ǹ���Ϊ֮��Ҳ����˵��λ������յ���Ӧ��Ҫ��500������ܷ�����һ
��������Ҫע��ģ�������ȣ��������ͣ���������Ϊ���ǵ��ǵ����RS485

����ǰ�ȵ�һ�ȣ��ȸ�1000us  Ȼ�����оͿ�ʼ�����ˣ�������ɺ�Ҫ��������ʱ����һ����500ms��ʱ����һ����600us��ʱ����ֻ����������ʱ�������ˣ����ǲŻ�
                               Usart2_StartRx();�������գ�����Usart2_TxEnd();���÷��������ʱ�����������ʱ��600us
ע�⵽��� Usart2_TxEnd()�����������DMAͨ��7�е��õģ�ֻ�е�DMA������ɺ�Ż���������������������һ��1000us�Ķ�ʱ���������ʱ���˺󣬲Ż������Is_Usart2_TxEnd()=1

///����ֻ����������Ϳ�ʼ�����У��Ż������s_bUsart2TxEndFlag = RESET; ����������Щ���ˣ��������ܽ�һ�¡�������������͵ģ�
////  �ڶ���������


1.�ڽ��յ�����֮��  if(ACK_IDLE == gs_Motor_Param.bySendDataFlag) ���������ͱ�־�ǿ��еģ��Żᴥ�����������Ȼ�� �� gs_Motor_Param.bySendDataFlag = PARAM_SET_ACK;
�ٿ�һ�������־���ϵ�ʱ���ǵ��������   memset(&gs_Motor_Param,0,sizeof(S_MOTOR_PARAM)); ���������־�أ�Ҳȫ������0�ģ������ڷ��������У���һֱ��IDLE���еġ�


2. ���ڷ�������һֱ�������еģ��ӵ����״̬�� if(Is_Usart2_TxEnd())//�������     �����s_bUsart2TxEndFlag==1�� s_bUsart2TxEndFlag��ʼ��ʱ����1�ģ������ڷ��͵ȴ�ʱ�������Ҳ����Ϊ1
Ҳ����˵�������������ˣ������ϵ���������1 ���⣬�����һ�η�������ˣ����ҵȴ�ʱ��Ҳ���ˣ����Ҳ����Ϊ1��ע������ȵ�ʱ����10��100us  ����ж���������������ж�ε��ã�һ��Ҫ���������
�����ڷ���ǰ��������Ҫ�ж�һ�£������־�Ƿ�Ϊ1 �����Ϊ1�Ļ��������������

3 Ȼ�����Usart2_ReadForTx();//����ǰ׼��   ׼��һ�Ƿ�������������s_byUsart2StartTxDelay=6  ��ʼ����ǰ�Ե�Ƭ��  ����֮ǰΪʲôҪ��һ���أ�����ǰ��û���ͺ����ǣ�
//ע�⵽���Ƿ���ǰ�ǵ�6��100us�ġ�

4 ���ڷ������ˣ�Send_ParamFrameAckData();//���Ͳ���Ӧ��֡   Usart2_StartTx(Ack_Data,4); ����������� ����4���ֽ� ��ͬʱ�������˻������������飺
/// һ����Ҫ�������ǣ�    s_bUsart2TxEndFlag = RESET;   ���ͽ�����־��Ϊ��Ч����δ���ͽ�����
//  ���⣬�����ж�       if(s_byUsart2StartTxDelay != 0)//��ʱδ��ɣ��˳� �������ǰ���6��100us������ �о��������ж�һ���ǲ��Ƕ��డ ��Ϊǰ���Ǹ�
///        Ӧ��ȴ����Ѿ��������if(Is_Uart2StartTxDelayEnd())//����ǰ��ʱ���   �����ָ�һ���е��߼����డ�� ȷʵ����
///  ˵����������ǻ�Ҫע����������͹����� ������DMA�жϣ��Ǹ������DMA�ж�������ʲô�أ�     s_byUsart2TxEndDelay = 10; ����������ͽ�����ʱֵ����һ��10�����������Ҫ��10��100us
///  ����Ҫ���ǣ���DMA����жϺ�����Ҫ��10��100us  �����ʱ����ʱ��������3������ �������6����������������Ҫ������

5 SetSendAck_TimeOut(TIME_500MS)  �������һ��״̬�������������һ��500ms�Ľϳ�ʱ��ĵȴ���

6.if(Is_SendAckTimeout() && (!Is_Usart2_TxEnd())) �����������������Ҫ���㣬�����2�����������10��100us��ɺ���������Ҫ���������飺
            s_bUsart2TxEndFlag = SET;       ����־����һ��  �Ժ�������Is_Usart2_TxEnd�������ܱ�ʾ�����ˡ�
            gs_Motor_Param.bySendDataFlag = ACK_IDLE;//Ӧ�����    ��������־��  �ָ���״̬  ���״̬����ʾ����״̬�����������״̬��
            Usart2_RxEnable();              ///����أ����ǽ�RS485�Ľ�������һ�¡�
///         ��ô�����500ms���������ʲô��Ҫ�����أ����������һ���򵥵���ʱ��
////        ������һ�����������������2���жϱ�־�Ƿǵ�----!Is_Usart2_TxEnd() ע����������ţ����������ڷ��ͺ����н����ֵ��ΪRESET������������϶�������ġ�

7  �������ɺ󣬾������һ���ˣ������������飺Usart2_StartRx();Usart2_TxEnd();
///       �ȿ�����һ����s_byUsart2TxEndDelay = 10; �������������ʱ��ɺ�ͻ�ִ��ǰ��������Ҫ�Ĳ��衣���ǻص�����������ȥ���������̽����ˡ�

* Return         : None      ע����������ǲ��ϵ������������е�
*******************************************************************************
*/
FlagStatus Com2_Recv(u8 *pbyData, u16 *pwLength)
{
    if (Is_Usart2_RxEnd())                    //�жϽ����Ƿ��Ѿ����  s_bUsart2RxEndFlag�����־���������������־��Usart2_RxEnd()������Ϊ1  �����������IDLE�ж��лᱻ���� �ʡ�����
    {
        *pwLength = Usart2_GetRxLength();     //������յ������ݵĳ���
        
        *pwLength = (*pwLength) < MAX_LEN_CMD ? (*pwLength) : MAX_LEN_CMD;  //���*pwLength����MAX_LEN_CMD��ȡMAX_LEN_CMD��

         Usart2_GetRxData(pbyData, *pwLength); //�Ѵ��ڽ��յ�������s_byUsart2RxBuffer������pbyData
        
         Usart2_StartRx();                     //������һ�εĽ���  ע������������У��Ὣ���s_bUsart2RxEndFlag=RESET  �Ӷ�������ֲ��ٳ�������������ص�
        
         return SET;                          //�����������ˣ�����TRUE
    }
    
    return RESET;                              //�������û����ɣ�����RESET
}

/*
*******************************************************************************
* Function Name  : ComMirrorGroup_Send
* Description    : �����鷢������
* Input          : �������ݰ��ĵ�ַ*pbyData
                   ���ݰ��ĳ���wLength
* Output         : None
* Return         : None
*******************************************************************************
*/
void Com2_Send(u8 *pbyData, u16 wLength)
{
    Usart2_StartTx(pbyData, wLength);
}

/************** (C) COPYRIGHT 2015 SOLAR SUPCON ************END OF FILE********/

