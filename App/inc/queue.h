/*
 * queue.h
 *
 *  Created on: 2017-7-23
 *      Author: dingyongjian
 */
#ifndef __QUEUE_H_
#define __QUEUE_H_

/* 引用限制说明 ------------------------------------------------------------- */
#ifdef      QUEUE_GLOBALS
  #define   QUEUE_EXT
#else
  #define   QUEUE_EXT    extern
#endif

#include "stm32f10x.h"
#include <string.h>

#define QUEUE_CNT 		12//12
#define REC_QUEUE_CNT 		8//12
#define QUEUE_BUF_LEN 	        152//152
#define ADDR_MIRROR_GROUP		0x80

#define MAX_LEN_CMD	        128+8//64 GXD20210826
#define MAX_NUM_DRIVER		2//12
#define CRC_LEN			2
#define MIN_CMD_LEN		(sizeof(SDC_CMD_HEAD)+CRC_LEN)


#define MAX_CFG_LEN		256			//组态最大长度
#define DEFAULT_RUN_CNT		1

typedef unsigned char	U8;
typedef U8              BOOLEAN;
#ifndef FALSE
#define FALSE 0
#endif

#ifndef TRUE
#define TRUE 1
#endif

typedef enum
{
    COM_ERR_OK      = 0,
    COM_ERR_TIMEOUT = 1
}COM_ERR;

typedef struct
{
    u16 wLen;
    u16 wTimeout;
    u8  byRetryCnt;
    u8  byRsv[3];
    BOOLEAN (*p_cmdAckFun)(u8 *data, u16 len, COM_ERR err);
    u8 byBuf[QUEUE_BUF_LEN];
} STRUCT_QUEUE;

typedef enum
{
    COM_MASTER_IDLE       = 0,
    COM_MASTER_WAIT       = 1,
    COM_MASTER_WAIT_SEND  = 2//等待发送2020-7-6新增
}COM_MASTER_STATUS;

typedef enum
{
    SDC_SLAVE   = 0,
    SDC_MASTER  = 1
}SDC_M_S;


typedef enum
{
    CMD_RUN_TIMEOUT     = 400,//超时由20ms改为40ms，2020-4-16
    CMD_NORUN_TIMEOUT   = 400,//超时由20ms改为40ms，2020-4-16
    CMD_INQUIRE_TIMEOUT = 400,
    CMD_UPDATA_TIMEOUT  = 400,
    CMD_READ_DRVANGLE_TIMEOUT  = 400
}CMD_TIMEOUT;

typedef enum
{
    CMD_NORETRY           = 0,
    CMD_RUN_RETRY_CNT     = 1,
    CMD_NORUN_RETRY_CNT   = 1,
    CMD_INQUIRE_RETRY_CNT = 0
}CMD_RETRY_CNT;

typedef enum
{
    SDC_UPDATA_STATUS_BUSY    = 0x00,		//不能接收数据包
    SDC_UPDATA_STATUS_READY   = 0x01,		//可以接收数据包
    SDC_UPDATA_STATUS_RECVING = 0x02	//正在接收数据包中
}SDC_UPDATA_STATUS;



QUEUE_EXT void init_queue(void);
QUEUE_EXT STRUCT_QUEUE* get_queue(void);
QUEUE_EXT void del_queue(void);
QUEUE_EXT BOOLEAN insert_queue(u8* buf, u16 len, u16 timeout, u8 retryCnt, BOOLEAN (*p_cmdAckFun)(u8 *data, u16 len, COM_ERR err));

//接收缓存
//extern void init_rec_queue(void);
//extern STRUCT_QUEUE* get_rec_queue(void);
//extern void del_rec_queue(void);
//extern BOOLEAN insert_rec_queue(u8* rec_buf, u16 rec_len, u16 rec_timeout, u8 rec_retryCnt, BOOLEAN (*rec_p_cmdAckFun)(u8 *data, u16 len, COM_ERR err));

QUEUE_EXT void Com_Send_Op(void);
QUEUE_EXT void Com_Receive_Op(void);
QUEUE_EXT void Decrease_TimeoutCnt(void);

QUEUE_EXT u8 COM_MIRROR_PORT;

//串口队列插队新增-2020-6-15
QUEUE_EXT BOOLEAN CheckComStatus(COM_MASTER_STATUS status);
QUEUE_EXT BOOLEAN insert_first_queue(u8* buf, u16 len, u16 timeout, u8 retryCnt, BOOLEAN (*p_cmdAckFun)(u8 *data, u16 len, COM_ERR err));
#endif /* QUEUE_H_ */
