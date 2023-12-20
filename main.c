/*********************************************************************************
 * 文件名  ：main.c
 * 描述    ：     
 * 实验平台：基于STM32F103CBT6
 * 库版本  ：ST1.0.0
 *作者：dingyongjian
 *日期：2020-11-11           2021/03/08 GXD 注释用
*******************************************
*2020-11-11 STM32 BLDC_500W
*注：
*2020-11-18电机可转，但换向对应关系还需确认
*2020-12-7基于新硬件，调整GPIO管脚
*士兰微SDH21263 ，HIN LIN 均为+时H输出  HIN LIN 均为-时L输出
*2020-12-11
*brk刹车，修改栅极驱动器空闲时强制为低
*GPIO-B3拨码2异常，TDO需重映射，
Limit_speed_duty = 500; //低速启动限制占空比30.0%调整为50.0%

增加了多字节命令。增加了国产芯片配合，增加了自动运行等  GXD  2021
//------------------------------
******************************************************************************/

#include "main.h"
#include "include.h"
#include <string.h>


u8 edition[18] = {"2019.11.25.Rain\r\n"};
/* Private typedef -----------------------------------------------------------*/
//任务列表
typedef struct
{
    void (*fun)(void);
    u32 dwT_100us;         ///这个是延时时间，每隔一段时间再运行
    u32 dwTime[2];         ///这个是定时器计数   内部用来定时的
}FUN_PERIODICALLY;
/* Private define ------------------------------------------------------------*/
static FUN_PERIODICALLY g_TaskList[MAX_TASK_CNT];          
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
u32 g_dwTimer_100us = 0;
/* Private function prototypes -----------------------------------------------*/
#ifdef __GNUC__
  /* With GCC/RAISONANCE, small printf (option LD Linker->Libraries->Small printf set to 'Yes') calls __io_putchar() */
  #define PUTCHAR_PROTOTYPE int __io_putchar(int ch)
#else
  #define PUTCHAR_PROTOTYPE int fputc(int ch, FILE *f)//printf重映射
#endif /* __GNUC__ */
/* Private functions ---------------------------------------------------------*/
static void Init_TaskList(void);//任务列表清空
static BOOLEAN Reg_Task(u32 T_100us, void (*fun)(void));//注册任务
static void Run_Tasks(void);//运行任务
void Hardware_Init(void);//外设初始化
void AppData_Init(void);//应用层数据初始化

//----------------主函数-------------------//
int main()
{
    __disable_irq();//关总中断
    NVIC_SetVectorTable(NVIC_VectTab_FLASH, 0x0000); //中断向量偏移至0x08000000  本来就在这里，这个不是白移了吗？ GXD 2021 04 21
     
	 //调试时禁用内部看门狗
    DBGMCU_Config(DBGMCU_IWDG_STOP,ENABLE); 
    DBGMCU_Config(DBGMCU_TIM1_STOP,ENABLE); 
    
    Delay_ms(100);//延时100ms，等待外设启动
    
    Hardware_Init();//外设初始化
    AppData_Init();//应用层数据初始化
    Init_TaskList();//初始化任务列表
    
    __enable_irq();//开总中断
   
    IWDG_Init(4,625);//看门狗约1S           分频系数=4  计时器625    时钟是多少呢  40K时钟，则它是10K，而625的话就是62.5ms
    //if(Is_Usart2_TxEnd())Usart2_StartTx(edition, 18);
    
    //加载任务
    while(!Reg_Task(FEED_DOG_TIME, FeedWatchDog_Op))		//喂狗任务 20ms 1次
		Wdt_StopFeedDog();    
     
    while(!Reg_Task(NO_TIME, Protocol_Data_Receive_Op))		//协议数据接收处理任务
		Wdt_StopFeedDog();
    
    while(!Reg_Task(NO_TIME, Protocol_Data_Send_Op))		//协议数据发送处理任务
		Wdt_StopFeedDog();
    
    while(!Reg_Task(MOTOR_CTRL_TIME, Motor_Run_Control))		//电机运行控制任务          1ms就控制1次 
		Wdt_StopFeedDog();
   
    while(!Reg_Task(LED_FLASH_TIME, LED_Status_Control))		//闪灯任务         500毫秒
		Wdt_StopFeedDog();
    //if(Is_Usart2_TxEnd())Usart2_StartTx(edition, 18);
    
    while (1)
    {
     /* Host Task handler */
        Run_Tasks();//运行任务  
    }
}
//----------------主函数-------------------//

/*Function*/
/*
*******************************************************************************
* Function Name  : Hardware_Init
* Description    : 硬件外设初始化
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************
*/
void Hardware_Init(void)//外设初始化
{
    STM32_GPIO_Config();//GPIO口初始化
    
    Init_TIMER2();//100us定时器初始化
    Init_TIMER4();//1ms定时器初始化 
    
    Init_Usart2();//com2初始化，对上位机
    
    HALL_HallTimerInit();          //霍尔接口初始化
    BLDC_TIMx_PWM_Init();           //BLDC电机 PWM初始化
    //DebugUART_Configuration();//调试串口配置
    MotorCurrentAdcChannel_Init();
   // ADC1_Init();//ADC初始化
    NVIC_Configuration();//中断初始化
}

/*
*******************************************************************************
* Function Name  : AppData_Init
* Description    : App数据初始化
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************
*/
void AppData_Init(void)//应用层数据初始化
{
    Motor_Param_DataInit();//电机运行参数初始化（含设备地址）
    IncPIDInit();//PID初始化
}

/*
*******************************************************************************
* Function Name  : Init_TaskList
* Description    : 任务列表清空
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************
*/
static void Init_TaskList(void)//任务列表清空
{
    memset((u8 *)g_TaskList, 0x00000000, MAX_TASK_CNT * sizeof(FUN_PERIODICALLY));
}

/*
*******************************************************************************
* Function Name  : Reg_Task
* Description    : 任务注册
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************
*/
static BOOLEAN Reg_Task(u32 T_100us, void (*fun)(void))//注册任务
{
    static u32 TaskCnt = 0;

    if((NULL == fun) || (MAX_TASK_CNT <= TaskCnt))//是否孔任务或任务超限
    {
        return FALSE;
    }

    g_TaskList[TaskCnt].fun = fun;
    g_TaskList[TaskCnt].dwT_100us = T_100us;

    TaskCnt++;

    return TRUE;
}

/*
*******************************************************************************
* Function Name  : Run_Tasks
* Description    : 任务运行
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************
*/
static void Run_Tasks(void)//运行任务         
{
    u32 i;

    for(i = 0; i < MAX_TASK_CNT; i++)
    {
        if(NULL == g_TaskList[i].fun)
        {
            break;
        }

        if(0 == g_TaskList[i].dwT_100us)
        {
            g_TaskList[i].fun();
            continue;
        }

        g_TaskList[i].dwTime[0] = g_dwTimer_100us/g_TaskList[i].dwT_100us;    ////比如是5000个计数单位       一个计数单位=0.1ms

        if(g_TaskList[i].dwTime[0] != g_TaskList[i].dwTime[1])               //任务执行周期到，执行任务
        {
            g_TaskList[i].fun();

            g_TaskList[i].dwTime[1] = g_TaskList[i].dwTime[0];          ////这里用了两个计数器 dwTime[0]和dwTime[1]
        }
        else
        {
          ;
        }
    }
}
/**
  * @brief  Retargets the C library printf function to the USART.
  * @param  None
  * @retval None
  */
PUTCHAR_PROTOTYPE
{
    /* Place your implementation of fputc here */
    /* e.g. write a character to the USART */
    USART_SendData(UART4, (uint8_t) ch);
   //USART_SendData(USART2, (uint8_t) ch);
    /* Loop until the end of transmission */
    while (USART_GetFlagStatus(UART4, USART_FLAG_TC) == RESET)
     //while (USART_GetFlagStatus(USART2, USART_FLAG_TC) == RESET)
    {}

    return ch;
}


#ifdef  USE_FULL_ASSERT

/**
  * @brief  Reports the name of the source file and the source line number
  *   where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t* file, uint32_t line)
{ 
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */

  /* Infinite loop */
    while (1)
    {
    }
}
#endif
