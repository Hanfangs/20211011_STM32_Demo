/***********************************************************************
文件名称main.h
功    能：
编写时间：
编 写 人：
注    意：
***********************************************************************/
#ifndef __MAIN_H
#define __MAIN_H

#include "queue.h"
#include "CRC16.h"
#include "USART_Com2.h"
//#include "USART_Com3.h"
//#include "UART_Com4.h"
#include "WDG_TASK.h"
#include "flash_rom.h"
#include "delay_time.h"
#include "HARDWARE_CONFIG.h"	
#include "LED_TASK.h"
//#include "W25Qxx.h"
#include "Motor_Init.h"
#include "Motor_Control_BLDC.h"
#include "Protocol_Control.h"

/* Exported constants --------------------------------------------------------*/
#define TIME_500US  5
#define TIME_1MS    10
#define TIME_10MS    100
#define TIME_20MS   200
#define TIME_50MS   500
#define TIME_100MS  1000
#define TIME_200MS  2000
#define TIME_250MS  2500
#define TIME_500MS  5000
#define TIME_1S     10000
#define TIME_5S     50000
#define TIME_2S     20000
#define TIME_120MS  1200
#define TIME_3S     30000
#define TIME_10S    100000
#define TIME_30S    300000
#define TIME_60S    300000
#define TIME_75S    7500000
#define TIME_9S     90000//9s
#define TIME_ONE_DAY 24*3600*10000
/***************************可配置的宏定义***************************************/
#define MAX_TASK_CNT				20			//最大任务数
/*******************************************************************************/
#define MOTOR_CTRL_TIME       TIME_1MS
#define NO_TIME 	      0
#define FEED_DOG_TIME         TIME_20MS   
#define LED_FLASH_TIME        TIME_500MS
#define STEP_MOTOR_TIME       TIME_1MS  
#define MIRROR_TRACK_TIME     TIME_1MS
#define UPDATA_FLASH_TIME     TIME_200MS
#define CYCLE_TASK_LOAD_TIME  TIME_5S
#define LCD_SEND_DELAY_TIME   TIME_50MS
#define SDC_INQUIRE_TIME      TIME_250MS
#define LED_CONTROL_TIME      TIME_200MS

#define COM_TIMEOUT	      TIME_3S	

#define ADC1_DR_Address    ((u32)0x4001244c)
//-------------------------------
//

#define COM_1 1
#define COM_2 2
#define COM_3 3
#define COM_4 4
//总线电压检测
//低压报警
#define LIMIT_U_MIN          22.0//单位v
#define LIMIT_MIN_VALUE    ((u32)((LIMIT_U_MIN /21) /3.3*4096))

//高压报警
#define LIMIT_U_MAX          36.0//单位v
#define LIMIT_MAX_VALUE    ((u32)((LIMIT_U_MAX /21) /3.3*4096))
//----------------------------


/* Private define ------------------------------------------------------------*/




#define BAUDRATE_FRE          38400

#define BAUDRATE_FRE_DRV      117187

#define FLASH_MIRROR_ADDR     0x08004000 //地址标识区 4K
#define STM_SECTOR_SIZE 2048
#define STM32_FLASH_BASE      0x08000000     //STM32 FLASH的起始地址 
#define STM32_FLASH_SIZE      512
#define IFLASH_ADDR           0x08000000  //BOOT区 8K
#define FIRM_ADDR             0x08005000 //升级标识区 4K
#define MIRROR_ADDR_ZONE      0x08008000 //设备地址区
#define M_UPDATA_ADDR         0x08020000 //下载区64K
#define M_UPDATA_END          0x0802FFFC //5A5A5A5A
#define WRITE_INDEX           16  //一次写Flash的包数
#define Flash_Page_Size       2048 //Flash每页空间


#define SETBIT(x,y)		{(x) |= (y);}											
#define CLRBIT(x,y)		{(x) &= ~(y);}								
#define TESTBIT(x,y)		((x) & (y))

extern u32 g_dwTimer_100us;
extern u8 COM_MIRROR_PORT;
#endif
