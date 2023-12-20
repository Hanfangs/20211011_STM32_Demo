/*********************************************************************************
 * �ļ���  ��main.c
 * ����    ��     
 * ʵ��ƽ̨������STM32F103CBT6
 * ��汾  ��ST1.0.0
 *���ߣ�dingyongjian
 *���ڣ�2020-11-11           2021/03/08 GXD ע����
*******************************************
*2020-11-11 STM32 BLDC_500W
*ע��
*2020-11-18�����ת���������Ӧ��ϵ����ȷ��
*2020-12-7������Ӳ��������GPIO�ܽ�
*ʿ��΢SDH21263 ��HIN LIN ��Ϊ+ʱH���  HIN LIN ��Ϊ-ʱL���
*2020-12-11
*brkɲ�����޸�դ������������ʱǿ��Ϊ��
*GPIO-B3����2�쳣��TDO����ӳ�䣬
Limit_speed_duty = 500; //������������ռ�ձ�30.0%����Ϊ50.0%

�����˶��ֽ���������˹���оƬ��ϣ��������Զ����е�  GXD  2021
//------------------------------
******************************************************************************/

#include "main.h"
#include "include.h"
#include <string.h>


u8 edition[18] = {"2019.11.25.Rain\r\n"};
/* Private typedef -----------------------------------------------------------*/
//�����б�
typedef struct
{
    void (*fun)(void);
    u32 dwT_100us;         ///�������ʱʱ�䣬ÿ��һ��ʱ��������
    u32 dwTime[2];         ///����Ƕ�ʱ������   �ڲ�������ʱ��
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
  #define PUTCHAR_PROTOTYPE int fputc(int ch, FILE *f)//printf��ӳ��
#endif /* __GNUC__ */
/* Private functions ---------------------------------------------------------*/
static void Init_TaskList(void);//�����б����
static BOOLEAN Reg_Task(u32 T_100us, void (*fun)(void));//ע������
static void Run_Tasks(void);//��������
void Hardware_Init(void);//�����ʼ��
void AppData_Init(void);//Ӧ�ò����ݳ�ʼ��

//----------------������-------------------//
int main()
{
    __disable_irq();//�����ж�
    NVIC_SetVectorTable(NVIC_VectTab_FLASH, 0x0000); //�ж�����ƫ����0x08000000  �����������������ǰ������� GXD 2021 04 21
     
	 //����ʱ�����ڲ����Ź�
    DBGMCU_Config(DBGMCU_IWDG_STOP,ENABLE); 
    DBGMCU_Config(DBGMCU_TIM1_STOP,ENABLE); 
    
    Delay_ms(100);//��ʱ100ms���ȴ���������
    
    Hardware_Init();//�����ʼ��
    AppData_Init();//Ӧ�ò����ݳ�ʼ��
    Init_TaskList();//��ʼ�������б�
    
    __enable_irq();//�����ж�
   
    IWDG_Init(4,625);//���Ź�Լ1S           ��Ƶϵ��=4  ��ʱ��625    ʱ���Ƕ�����  40Kʱ�ӣ�������10K����625�Ļ�����62.5ms
    //if(Is_Usart2_TxEnd())Usart2_StartTx(edition, 18);
    
    //��������
    while(!Reg_Task(FEED_DOG_TIME, FeedWatchDog_Op))		//ι������ 20ms 1��
		Wdt_StopFeedDog();    
     
    while(!Reg_Task(NO_TIME, Protocol_Data_Receive_Op))		//Э�����ݽ��մ�������
		Wdt_StopFeedDog();
    
    while(!Reg_Task(NO_TIME, Protocol_Data_Send_Op))		//Э�����ݷ��ʹ�������
		Wdt_StopFeedDog();
    
    while(!Reg_Task(MOTOR_CTRL_TIME, Motor_Run_Control))		//������п�������          1ms�Ϳ���1�� 
		Wdt_StopFeedDog();
   
    while(!Reg_Task(LED_FLASH_TIME, LED_Status_Control))		//��������         500����
		Wdt_StopFeedDog();
    //if(Is_Usart2_TxEnd())Usart2_StartTx(edition, 18);
    
    while (1)
    {
     /* Host Task handler */
        Run_Tasks();//��������  
    }
}
//----------------������-------------------//

/*Function*/
/*
*******************************************************************************
* Function Name  : Hardware_Init
* Description    : Ӳ�������ʼ��
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************
*/
void Hardware_Init(void)//�����ʼ��
{
    STM32_GPIO_Config();//GPIO�ڳ�ʼ��
    
    Init_TIMER2();//100us��ʱ����ʼ��
    Init_TIMER4();//1ms��ʱ����ʼ�� 
    
    Init_Usart2();//com2��ʼ��������λ��
    
    HALL_HallTimerInit();          //�����ӿڳ�ʼ��
    BLDC_TIMx_PWM_Init();           //BLDC��� PWM��ʼ��
    //DebugUART_Configuration();//���Դ�������
    MotorCurrentAdcChannel_Init();
   // ADC1_Init();//ADC��ʼ��
    NVIC_Configuration();//�жϳ�ʼ��
}

/*
*******************************************************************************
* Function Name  : AppData_Init
* Description    : App���ݳ�ʼ��
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************
*/
void AppData_Init(void)//Ӧ�ò����ݳ�ʼ��
{
    Motor_Param_DataInit();//������в�����ʼ�������豸��ַ��
    IncPIDInit();//PID��ʼ��
}

/*
*******************************************************************************
* Function Name  : Init_TaskList
* Description    : �����б����
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************
*/
static void Init_TaskList(void)//�����б����
{
    memset((u8 *)g_TaskList, 0x00000000, MAX_TASK_CNT * sizeof(FUN_PERIODICALLY));
}

/*
*******************************************************************************
* Function Name  : Reg_Task
* Description    : ����ע��
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************
*/
static BOOLEAN Reg_Task(u32 T_100us, void (*fun)(void))//ע������
{
    static u32 TaskCnt = 0;

    if((NULL == fun) || (MAX_TASK_CNT <= TaskCnt))//�Ƿ�������������
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
* Description    : ��������
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************
*/
static void Run_Tasks(void)//��������         
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

        g_TaskList[i].dwTime[0] = g_dwTimer_100us/g_TaskList[i].dwT_100us;    ////������5000��������λ       һ��������λ=0.1ms

        if(g_TaskList[i].dwTime[0] != g_TaskList[i].dwTime[1])               //����ִ�����ڵ���ִ������
        {
            g_TaskList[i].fun();

            g_TaskList[i].dwTime[1] = g_TaskList[i].dwTime[0];          ////������������������ dwTime[0]��dwTime[1]
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
