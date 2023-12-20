/*
********************** (C) COPYRIGHT 2020 SOLAR  *************************
* File Name          : queue.c
* Author             : dingyongjian 
* Date First Issued  : 
* Description        : ���д���
********************************************************************************
* History:
* 02/02/20 v1.0
********************************************************************************
*/
#define      QUEUE_GLOBALS

#include "include.h"
#include "main.h"

u8 g_byComTurnFlag = 0;//���У�1Ϊæµʱ���

u8 COM_MIRROR_PORT = COM_2;
static STRUCT_QUEUE queue[QUEUE_CNT];

static u32 s_dwHead = 0;
static u32 s_dwTail = 0;
static u16 s_wTimeout = 0;//��ʱ����

//���ջ���
/*static STRUCT_QUEUE rec_queue[REC_QUEUE_CNT];

static u32 rec_s_dwHead = 0;
static u32 rec_s_dwTail = 0;*/
u8 g_byComBuf[MAX_LEN_CMD];
u16 g_wComDataLen = 0;
static COM_MASTER_STATUS comStatus = COM_MASTER_IDLE;

/*
*******************************************************************************
* Function Name  : init_queue
* Description    : ��ʼ������
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************
*/
void init_queue(void)
{
    memset(&queue, 0x00, QUEUE_CNT * sizeof(STRUCT_QUEUE));
}

/*
*******************************************************************************
* Function Name  : is_retry_over
* Description    : �ж��ط������Ƿ���
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************
*/
static BOOLEAN is_retry_over(STRUCT_QUEUE *p_queue)
{
    if(0 == p_queue->byRetryCnt)
    {
        return TRUE;
    }
    else
    {
        p_queue->byRetryCnt--;
        return FALSE;
    }
}

/*
*******************************************************************************
* Function Name  : set_timeout
* Description    : ���ó�ʱʱ��
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************
*/
static void set_timeout(STRUCT_QUEUE *p_queue)
{
    s_wTimeout = p_queue->wTimeout;
}

/*
*******************************************************************************
* Function Name  : is_timeout
* Description    : �ж��Ƿ�ʱ
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************
*/
static BOOLEAN is_timeout(void)
{
    if (0 == s_wTimeout)
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
* Function Name  : Decrease_TimeoutCnt
* Description    : ��ʱ����--
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************
*/
void Decrease_TimeoutCnt(void)
{
    if(0 < s_wTimeout)
    {
        s_wTimeout--;
    }
}
/*
*******************************************************************************
* Function Name  : insert_queue
* Description    : �������
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************
*/
BOOLEAN insert_queue(u8* buf, u16 len, u16 timeout, u8 retryCnt, BOOLEAN (*p_cmdAckFun)(u8 *data, u16 len, COM_ERR err))
{
    if ((len > QUEUE_BUF_LEN) || (0 == len))//���ȳ��޻���Ϊ��
    {
        return FALSE;
    }

    if (s_dwHead == (s_dwTail + 1) % QUEUE_CNT)//��������
    {
        return FALSE;
    }

    memcpy(queue[s_dwTail].byBuf, buf, len);
    queue[s_dwTail].wLen = len;
    queue[s_dwTail].byRetryCnt = retryCnt;
    queue[s_dwTail].wTimeout = timeout;
    queue[s_dwTail].p_cmdAckFun = p_cmdAckFun;

    s_dwTail = (s_dwTail + 1) % QUEUE_CNT;
    return TRUE;
}

/*
*******************************************************************************
* Function Name  : get_queue
* Description    : ��ȡ��ǰ�������
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************
*/
STRUCT_QUEUE* get_queue(void)
{
    if (s_dwHead == s_dwTail)
    {
        return NULL;
    }

    return &queue[s_dwHead];
}

/*
*******************************************************************************
* Function Name  : del_queue
* Description    : ɾ����ǰ�������
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************
*/
void del_queue(void)
{
    if (s_dwHead == s_dwTail)
    {
        return ;
    }

    s_dwHead = (s_dwHead + 1) % QUEUE_CNT;
}

//���ջ���
/*void init_rec_queue(void)
{
  memset(&rec_queue, 0x00, REC_QUEUE_CNT * sizeof(STRUCT_QUEUE));
}

BOOLEAN insert_rec_queue(u8* rec_buf, u16 rec_len, u16 rec_timeout, u8 rec_retryCnt, BOOLEAN (*rec_p_cmdAckFun)(u8 *data, u16 len, COM_ERR err))
{
    if ((rec_len > QUEUE_BUF_LEN) || (0==rec_len))
            return FALSE;

    if (rec_s_dwHead == (rec_s_dwTail + 1) % REC_QUEUE_CNT)
            return FALSE;

    memcpy(rec_queue[rec_s_dwTail].byBuf, rec_buf, rec_len);
    rec_queue[rec_s_dwTail].wLen = rec_len;
    rec_queue[rec_s_dwTail].retryCnt = rec_retryCnt;
    rec_queue[rec_s_dwTail].timeout = rec_timeout;
    rec_queue[rec_s_dwTail].p_cmdAckFun = rec_p_cmdAckFun;

    rec_s_dwTail = (rec_s_dwTail + 1) % REC_QUEUE_CNT;
    return TRUE;
}

STRUCT_QUEUE* get_rec_queue(void)
{
    if (rec_s_dwHead == rec_s_dwTail)
            return NULL;

    return &rec_queue[rec_s_dwHead];
}

void del_rec_queue(void)
{
    if (rec_s_dwHead == rec_s_dwTail)
            return ;

    rec_s_dwHead = (rec_s_dwHead + 1) % REC_QUEUE_CNT;
}*/

/*
*******************************************************************************
* Function Name  : ComData_send
* Description    : ���ڷ�������
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************
*/
static BOOLEAN ComData_send(STRUCT_QUEUE *send)
{
    if(COM_MIRROR_PORT == COM_2)
    {
        Com2_Send(send->byBuf, send->wLen);
    }
    
    return TRUE;
}

/*
*******************************************************************************
* Function Name  : Com_Send_Op
* Description    : ���������ڷ��ʹ���
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************
*/
void Com_Send_Op(void)
{
    STRUCT_QUEUE *p_queue = get_queue();
    
    volatile FlagStatus txEnd = RESET;
    volatile FlagStatus rxEnd = RESET;

    if(COM_MIRROR_PORT == COM_2)
    {
        txEnd = Is_Usart2_TxEnd();
        rxEnd = Is_Usart2_RxEnd();
    }
    
    if((COM_MASTER_IDLE == comStatus) && (NULL != p_queue))
    {//Ҫ��������
        if((!rxEnd) && (txEnd))
        {
            if(ComData_send(p_queue))
            {//������ʱ��ʱ������״̬��Ϊ���ȴ���Ӧ��               
                
                set_timeout(p_queue);
                comStatus = COM_MASTER_WAIT;
            }
        }
    }
    else if(COM_MASTER_WAIT == comStatus)
    {//�ڡ��ȴ���Ӧ����       
        if(is_timeout())
        {//��ʱ��
                      
            if(is_retry_over(p_queue))
            {//�ط�����Ҳ����
             //֪ͨӦ�ò㴦��
             //ɾ������
                p_queue->p_cmdAckFun(p_queue->byBuf, p_queue->wLen, COM_ERR_TIMEOUT);
                del_queue();
            }

            //����״̬��Ϊ�����С�
            comStatus = COM_MASTER_IDLE;
        }
    }
    else
    {
            ;
    }
}

/*
*******************************************************************************
* Function Name  : Com_Receive_Op
* Description    : ���������ڽ��մ���
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************
*/
void Com_Receive_Op(void)
{
    if(Is_Usart2_RxEnd() && (COM_MIRROR_PORT == COM_2))//com��2
    {//���ջ����������ݵ���
        STRUCT_QUEUE *p_queue = get_queue();	//ȡ��Ӧ����������

        if(Is_Usart2_RxEnd())//���ݽ������
        {
            Com2_Recv(g_byComBuf, &g_wComDataLen);//ȡ��������
        }
        
        if(COM_MASTER_WAIT != comStatus)
        {//ȷ�����ڴ��ڽ���״̬
            comStatus = COM_MASTER_IDLE;
            return;
        }

        if(NULL == p_queue)
        {//û�з��������������������Ӧ�������
            comStatus = COM_MASTER_IDLE;
            return;
        }
        
        if(p_queue->p_cmdAckFun(g_byComBuf, g_wComDataLen, COM_ERR_OK))
        {//֪ͨӦ�ò㴦���Ҵ�������ȷ
            //Set_ComLed_MirrorLink(COM_TIMEOUT);                    
            del_queue();
        }
        else
        {//����������˵���ظ�����������
         //��Ҫ���ط�����                   
            if(is_retry_over(p_queue))
            {//�ط�����Ҳ����
             //֪ͨӦ�ò㴦��
             //ɾ�����У������������
                p_queue->p_cmdAckFun(NULL, 0, COM_ERR_TIMEOUT);
                del_queue();
            }
        }

        comStatus = COM_MASTER_IDLE;
    }
}


//���ڶ��в������-2020-6-15
/*
*******************************************************************************
* Function Name  : CheckComStatus
* Description    : �ж϶�����������״̬
* Input          : COM_MASTER_STATUS status
* Output         : None
* Return         : None
*******************************************************************************
*/
BOOLEAN CheckComStatus(COM_MASTER_STATUS status)
{
    if(comStatus == status)//�ж�״̬ʮ��һ��
    {
        return TRUE;
    }
    
    return FALSE;
}

/*
*******************************************************************************
* Function Name  : insert_first_queue
* Description    : �������ȣ���һ������
* Input          : u8* buf, u16 len, u16 timeout, u8 retryCnt, BOOLEAN (*p_cmdAckFun)(u8 *data, u16 len, COM_ERR err)
* Output         : None
* Return         : TRUE /FALSE
*******************************************************************************
*/
BOOLEAN insert_first_queue(u8* buf, u16 len, u16 timeout, u8 retryCnt, BOOLEAN (*p_cmdAckFun)(u8 *data, u16 len, COM_ERR err))
{
    u32 dwHead = 0;
      
    if ((len > QUEUE_BUF_LEN) || (0 == len))//���ȳ��޻���Ϊ��
    {
        return FALSE;
    }

    if (s_dwHead == (s_dwTail + 1) % QUEUE_CNT)//��������
    {
        return FALSE;
    }

    s_dwHead = (s_dwHead + QUEUE_CNT - 1)% QUEUE_CNT;//�����е�����ǰһ����
      
    if(CheckComStatus(COM_MASTER_WAIT))//����æ���ȴ���ǰ���д������ٴ�������
    {
        dwHead = (s_dwHead + 1) % QUEUE_CNT;  //͸������������һ����  
       
        memcpy(queue[s_dwHead].byBuf,queue[dwHead].byBuf,queue[dwHead].wLen);
        queue[s_dwHead].wLen        = queue[dwHead].wLen;
        queue[s_dwHead].byRetryCnt  = queue[dwHead].byRetryCnt;
        queue[s_dwHead].wTimeout    = queue[dwHead].wTimeout;
        queue[s_dwHead].p_cmdAckFun = queue[dwHead].p_cmdAckFun;
        
        g_byComTurnFlag = 1;//æµʱ���
    }
    else//���ڿ���
    {
        dwHead = s_dwHead;     
        g_byComTurnFlag = 0;//����ʱ���
    }
    
    memcpy(queue[dwHead].byBuf, buf, len);
    queue[dwHead].wLen = len;
    queue[dwHead].byRetryCnt = retryCnt;
    queue[dwHead].wTimeout = timeout;
    queue[dwHead].p_cmdAckFun = p_cmdAckFun;

    return TRUE;
}
