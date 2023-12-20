/*
********************** (C) COPYRIGHT 2020 SOLAR  *************************
* File Name          : Motor_Control_BLDC.c
* Author             : dingyongjian 
* Date First Issued  : 
* Description        : bldc电机控制
********************************************************************************
* History:
* 02/02/20 v1.0
*
********************************************************************************
*/
#define      MOTOR_CONTROL_BLDC_GLOBALS

#include "include.h"
#include "main.h"
u8 testpin[32]= {0};
/* 私有变量 ------------------------------------------------------------------*/
MOTOR_DEVICE bldc_dev = {200,1000,STOP,CCW,0,0,0,0};        ///转速为200  设置转速为1000; 20211115 hjl
static PID bldc_pid;
__IO uint16_t speed_duty = MOTOR_MIN_DUTY_SPEED; // 速度占空比：0~1000  为1000是占空比为100%
                              // 初始化值必须不小于70，否则电机会堵塞
__IO uint16_t Limit_speed_duty = 500; //低速启动限制占空比
MOTOR_RUN_STATE Motor_Run_State = MOTOR_IDLE;                        ////缺省时，这个状态是空闲

uint32_t g_Motor_Stop_Delay_Timer = 0;
uint16_t Hall_Err_Cnt = 0;//霍尔异常计数
uint16_t Hall_OK_Cnt = 0;//霍尔正常计数
uint8_t  Hall_Check_Seq[6] = {1,3,2,6,4,5};//霍尔判断序列
uint8_t  Hall_Check_Flag = 0;//霍尔检测启动标识
uint8_t  Hall_Check_SeqNum = 0;//序列号

// modified by hjl 20211024
//uint32_t g_Motor_Time_Count = 0;
uint32_t g_Motor_Hall_Count = 0;
uint32_t g_Motor_Hall_Count_Dec = 0;
uint32_t g_Motor_Hall_Count_Stop = 0;

// modified by hjl 20211027, for test
#define HJL_TEST 	1
#define HJL_TEST1 	0
#define HJL_TEST2	0
#define HJL_TEST3	0
#if (HJL_TEST == 1)
int32_t tmp_I;
int32_t tmp_Set_Speed[100];
int32_t tmp_Curr_Speed[100];
int32_t tmp_Curr_Duty[100];
#endif
#if (HJL_TEST1 == 1)		// 20211113 hjl
int32_t tmp_hall_cnt = 0;
uint8_t tmp_pinstate_buff[48];
#endif
#if (HJL_TEST3 == 1)
int32_t tmp_SetSpeed = 0;
int32_t tmp_DeltaSpeed= 0;
#endif
/**************设置停机延时********************************/
void Set_Motor_Stop_Delay(uint32_t delay_time) 
{
    g_Motor_Stop_Delay_Timer = delay_time;
}
/**************电机停机延时********************************/
void Motor_Stop_Delay(void)
{
    if(g_Motor_Stop_Delay_Timer > 0)     ///在1ms定时器中调用它
    {
        g_Motor_Stop_Delay_Timer--;
    }
}
/**************PID参数初始化********************************/
void IncPIDInit(void) 
{
    bldc_dev.motor_speed = MOTOR_MIN_SPEED;//最低转速启动       100   上电时，这个速度是最低转速
    bldc_pid.LastError  = 0;                    //Error[-1]
    bldc_pid.PrevError  = 0;                    //Error[-2]
    bldc_pid.Proportion = P_DATA_ACC;              //比例常数 Proportional Const
    bldc_pid.Integral   = I_DATA_ACC;                //积分常数  Integral Const
    bldc_pid.Derivative = D_DATA_ACC;              //微分常数 Derivative Const
    bldc_pid.SetPoint   = bldc_dev.motor_speed;  //设定目标Desired Value           刚才已经设为100了
    Hall_Err_Cnt = 0;//霍尔异常计数清零
    Hall_Check_Flag = HALL_CHECK_START;//                    启动霍尔异常计数判断
    memset(&g_dwAdc_CurrentValue,0,8);
    gs_Motor_Param.dwOverLoadCnt_H = 0;
    gs_Motor_Param.dwOverLoadCnt_L = 0;
    gs_Motor_Param.dwMotor_Current = 0;
    gs_Motor_Param.dwLimitCurrent_H = LIMIT_CURRENT_H;
    gs_Motor_Param.dwLimitCurrent_L = LIMIT_CURRENT_L;
}
/********************增量式PID控制设计************************************/
int IncPIDCalc(int NextPoint) 
{
    int iError,iIncpid;                                       //当前误差
    
    iError = bldc_pid.SetPoint - NextPoint;                     //增量计算
    iIncpid = (int)((bldc_pid.Proportion * iError)             /*E[k]项*/\
                -(bldc_pid.Integral * bldc_pid.LastError)     /*E[k-1]项*/\
                +(bldc_pid.Derivative * bldc_pid.PrevError));  /*E[k-2]*/
                
    bldc_pid.PrevError = bldc_pid.LastError;                    //存储误差，用于下次计算
    bldc_pid.LastError = iError;
    
    return(iIncpid);                                    //返回增量值
}
/**
  * 函数功能: 系统滴答定时器回调函数     这个实际上就是转速控制  如果转速大了，就利用PID去将占空比减小，如果转速低了，就将这个占空比增加。
  * 输入参数: 无
  * 返 回 值: 无
  * 说    明: 1ms运行该函数一次
  */
void HAL_SYSTICK_Callback(void)
{
    static uint16_t time_count = 0;
    // modified by hjl, 20211028
    uint16_t tmp_Min_Speed_Duty;

	if (bldc_dev.motor_state != RUN)
	{
		time_count = 0;
		return;
	}
	bldc_dev.stalling_count++;
        time_count++;
	if(bldc_dev.stalling_count >= 100)
	{
		bldc_dev.stalling_number++;
		bldc_dev.stalling_count = 0;
		if (bldc_dev.stalling_number >= 30)
		{
            bldc_dev.motor_state = STOP;         ///       这个值设过后，以后就再也不会到这个地方来了，这个每毫秒调用的函数就基本上直接退出了---看前面数第5行。。。
            //Motor_Run_State = MOTOR_STOP;
            //设置堵转故障
            Set_MotorErr(M_RUN_FAULT);  //设置电机运作失败故障
            Set_MotorErr(M_OVERLOAD);   //设置电机过流故障
            Set_Motor_Stop();//电机停机                                             ---电机停机只是设置了一个标志而已---就是它：    Motor_Run_State = MOTOR_STOP;
		}
		else
		{
			HALL_Abnormal_Callback();
		}
	}   
	else
  {
    if(time_count >= 50) //   original is > 50  运行时间大于 50ms  每到50，这个值会被清0     如果不是运行状态，则这里永远不会成立
    {
        static int temp;
        int pid_result;
        // bldc_dev.step_counter记录霍尔传感器在50ms时间内产生的脉冲个数，而电机旋转一圈会有总共
        // (MOTOR_POLE_PAIR_NUM*6)个脉冲,  24个       使用 n=bldc_dev.step_counter/(MOTOR_POLE_PAIR_NUM*6) 为
        // 50ms内电机转动圈数，为换算为【转/分钟(rpm)】
        //  n/50 = x/(60*1000) --> x=bldc_dev.step_counter*50
//        temp = bldc_dev.step_counter * 50;             ////每个HALL感应到一次，这个值就会++  ----这部分在HALL中断中有      就是看50毫秒里面，这个数加到多少了。比如加到10  则此时就是500
        temp = (bldc_dev.step_counter + bldc_dev.step_counter_prev) * 25;     // 20211115 hjl 取均值
        
        gs_Motor_Param.dnMotor_NowSpeed = temp;      ////这里搞不懂是什么意思了 马达的速度就是50ms内的脉冲个数 * 50   刚计算下来这个确实就是每分钟的转速。
        
        pid_result = IncPIDCalc(temp);        //   计算增量
        pid_result = pid_result * 10 / 16;    // *10/25为转速和占空比一个转换，转速（0~2500），占空比（0~1000）
        
#if (HJL_TEST2 == 1)		// 20211113 hjl

#else
#if (HJL_TEST3 == 1)
		if(Motor_Run_State == MOTOR_RUN_ACC) 
		{
			speed_duty += tmp_DeltaSpeed;
			if (speed_duty > tmp_SetSpeed)
				{speed_duty = tmp_SetSpeed;}
		} else
		{
			speed_duty -= tmp_DeltaSpeed;
			if (speed_duty < MOTOR_DEC_MIN_DUTY_SPEED)
				{speed_duty = MOTOR_DEC_MIN_DUTY_SPEED;}
		}
#else
        // modified by hjl, 20211028
        if (Motor_Run_State == MOTOR_RUN_DEC)
        {
        	tmp_Min_Speed_Duty = MOTOR_DEC_MIN_DUTY_SPEED;
        } else
        {
        	tmp_Min_Speed_Duty = MOTOR_MIN_DUTY_SPEED;
        }

        if((pid_result + speed_duty) < tmp_Min_Speed_Duty)
        {
            speed_duty = tmp_Min_Speed_Duty;
        }
        else if((pid_result + speed_duty) > MOTOR_MAX_DUTY_SPEED)//950->600
        {
            speed_duty = MOTOR_MAX_DUTY_SPEED;
        }
        else
        {            
            speed_duty += pid_result;  
            if(temp <= 50)                 ///在转速比较低的情况下，如果输出的占空比又比较大的话，造成电机受不了的
            {
                if(speed_duty >= Limit_speed_duty)         ///  这个值是 500
                {
                    speed_duty = Limit_speed_duty;
                }
            }
        }
#endif
#endif        
        
        time_count = 0;
        bldc_dev.step_counter_prev = bldc_dev.step_counter;
        bldc_dev.step_counter = 0;
//        HALL_TIMx_Callback();       ////不知道为什么这里要调用这个中断函数   ？？？ 再一次看怎么也搞不懂为什么要调这个函数呢？不知道去掉这个会不会有问题呢 20211115 hjl
        
        // for test, hjl, 20211027
#if (HJL_TEST == 1)
        tmp_Set_Speed[tmp_I] = bldc_pid.SetPoint;
        tmp_Curr_Speed[tmp_I] = gs_Motor_Param.dnMotor_NowSpeed;
        tmp_Curr_Duty[tmp_I] = speed_duty;
        tmp_I++;
        if (tmp_I > 99 ) tmp_I = 99;
#endif
    }
  }
}


/*
  * 函数功能: BLDC换相控制
  * 输入参数: 无
  * 返 回 值: 无
  * 说    明: 无
  外接口 5V | HB         5V | HA
         -------  ----> ---------
         HC | HA         HC | HB
  step顺序451326  ---->  462315 只换一处

 */
void BLDC_PHASE_CHANGE(uint8_t step)
{
    switch(step)         ////发现这个上管总是要导通的少一点，而下管则导通的多一点
    {
        case 4: //B+ C-            C- ==BLDC_TIM_PERIOD       B+ ==speed_duty*BLDC_TIM_PERIOD
        {
            /* Next step: Step 2 Configuration -------------------------------------- */ 
            TIM_CCxCmd(BLDC_TIMx,TIM_Channel_1,TIM_CCx_Disable);
            TIM_CCxNCmd(BLDC_TIMx,TIM_Channel_1,TIM_CCxN_Disable);        ////定时器1的通道1禁掉了
        
            /*  Channel1 configuration */
            /*  Channel2 configuration */  
            TIM_CCxNCmd(BLDC_TIMx,TIM_Channel_2,TIM_CCxN_Disable);           ///先把通道2关掉

            TIM_SetCompare2(BLDC_TIMx,BLDC_TIM_PERIOD*speed_duty/1000);     ///再把这个比较器设置一下   但是这个值又是怎么回事呢   这里比较器设的值是 千分之speed_duty占空比
            TIM_CCxCmd(BLDC_TIMx,TIM_Channel_2,TIM_CCx_Enable);           ///  再把这个通道2 允许
            /*  Channel3 configuration */
            TIM_CCxCmd(BLDC_TIMx,TIM_Channel_3,TIM_CCx_Disable);          
            TIM_SetCompare3(BLDC_TIMx,BLDC_TIM_PERIOD);                  ////但是这个通道3N 的比较值设的是就是周期  20K那个，50us
            TIM_CCxNCmd(BLDC_TIMx,TIM_Channel_3,TIM_CCxN_Enable);

        }
        break;
          
        case 5: //B+ A-              A-==BLDC_TIM_PERIOD       B+ ==speed_duty*BLDC_TIM_PERIOD
        {
            /* Next step: Step 3 Configuration -------------------------------------- */      
            TIM_CCxCmd(BLDC_TIMx,TIM_Channel_3,TIM_CCx_Disable);
            TIM_CCxNCmd(BLDC_TIMx,TIM_Channel_3,TIM_CCxN_Disable);
            
            /*  Channel1 configuration */
            TIM_CCxCmd(BLDC_TIMx,TIM_Channel_1,TIM_CCx_Disable);
            TIM_SetCompare1(BLDC_TIMx,BLDC_TIM_PERIOD);                 ///看下来，B+的地方，设的就是占空比*周期，而A-的地方，设的就是周期
            TIM_CCxNCmd(BLDC_TIMx,TIM_Channel_1,TIM_CCxN_Enable);
          
            /*  Channel2 configuration */
            TIM_CCxNCmd(BLDC_TIMx,TIM_Channel_2,TIM_CCxN_Disable);
            TIM_SetCompare2(BLDC_TIMx,BLDC_TIM_PERIOD*speed_duty/1000);
            TIM_CCxCmd(BLDC_TIMx,TIM_Channel_2,TIM_CCx_Enable);
            /*  Channel3 configuration */
        }
        break;
        
        case 1: //C+ A-        A-==BLDC_TIM_PERIOD        C+ ==speed_duty*BLDC_TIM_PERIOD
        {
            /* Next step: Step 4 Configuration -------------------------------------- */
            TIM_CCxCmd(BLDC_TIMx,TIM_Channel_2,TIM_CCx_Disable);
            TIM_CCxNCmd(BLDC_TIMx,TIM_Channel_2,TIM_CCxN_Disable);
            
            /*  Channel1 configuration */
            TIM_CCxCmd(BLDC_TIMx,TIM_Channel_1,TIM_CCx_Disable);
            TIM_SetCompare1(BLDC_TIMx,BLDC_TIM_PERIOD);
            TIM_CCxNCmd(BLDC_TIMx,TIM_Channel_1,TIM_CCxN_Enable);
    
            /*  Channel2 configuration */ 
            /*  Channel3 configuration */
            TIM_CCxNCmd(BLDC_TIMx,TIM_Channel_3,TIM_CCxN_Disable);
            TIM_SetCompare3(BLDC_TIMx,BLDC_TIM_PERIOD*speed_duty/1000);
            TIM_CCxCmd(BLDC_TIMx,TIM_Channel_3,TIM_CCx_Enable);
        }
        break;
        
        case 3: //C+ B-            B- ==BLDC_TIM_PERIOD       C+ ==speed_duty*BLDC_TIM_PERIOD
        {
            /* Next step: Step 5 Configuration -------------------------------------- */
            TIM_CCxCmd(BLDC_TIMx,TIM_Channel_1,TIM_CCx_Disable);
            TIM_CCxNCmd(BLDC_TIMx,TIM_Channel_1,TIM_CCxN_Disable);    
            /*  Channel1 configuration */      
            /*  Channel2 configuration */  

            TIM_CCxCmd(BLDC_TIMx,TIM_Channel_2,TIM_CCx_Disable);
            TIM_SetCompare2(BLDC_TIMx,BLDC_TIM_PERIOD);
            TIM_CCxNCmd(BLDC_TIMx,TIM_Channel_2,TIM_CCxN_Enable);
          
            /*  Channel3 configuration */          
            TIM_CCxNCmd(BLDC_TIMx,TIM_Channel_3,TIM_CCxN_Disable);
            TIM_SetCompare3(BLDC_TIMx,BLDC_TIM_PERIOD*speed_duty/1000);
            TIM_CCxCmd(BLDC_TIMx,TIM_Channel_3,TIM_CCx_Enable);
        }
        break;
        
        case 2: //A+ B-             B- ==BLDC_TIM_PERIOD       A+ ==speed_duty*BLDC_TIM_PERIOD
        {
            /* Next step: Step 6 Configuration -------------------------------------- */
            TIM_CCxCmd(BLDC_TIMx,TIM_Channel_3,TIM_CCx_Disable);
            TIM_CCxNCmd(BLDC_TIMx,TIM_Channel_3,TIM_CCxN_Disable);    
            /*  Channel1 configuration */
            TIM_CCxNCmd(BLDC_TIMx,TIM_Channel_1,TIM_CCxN_Disable);
            TIM_SetCompare1(BLDC_TIMx,BLDC_TIM_PERIOD*speed_duty/1000);
            TIM_CCxCmd(BLDC_TIMx,TIM_Channel_1,TIM_CCx_Enable);
            /*  Channel2 configuration */
            TIM_CCxCmd(BLDC_TIMx,TIM_Channel_2,TIM_CCx_Disable);        
            TIM_SetCompare2(BLDC_TIMx,BLDC_TIM_PERIOD);
            TIM_CCxNCmd(BLDC_TIMx,TIM_Channel_2,TIM_CCxN_Enable);
            /*  Channel3 configuration */
        }
        break;
        
        case 6: //A+ C-
        {
            /* Next step: Step 1 Configuration -------------------------------------- */
            TIM_CCxCmd(BLDC_TIMx,TIM_Channel_2,TIM_CCx_Disable);
            TIM_CCxNCmd(BLDC_TIMx,TIM_Channel_2,TIM_CCxN_Disable);
            
            /*  Channel1 configuration */
            TIM_CCxNCmd(BLDC_TIMx,TIM_Channel_1,TIM_CCxN_Disable);
            TIM_SetCompare1(BLDC_TIMx,BLDC_TIM_PERIOD*speed_duty/1000);
            TIM_CCxCmd(BLDC_TIMx,TIM_Channel_1,TIM_CCx_Enable);
            /*  Channel2 configuration */      
            /*  Channel3 configuration */
            TIM_CCxCmd(BLDC_TIMx,TIM_Channel_3,TIM_CCx_Disable);
            TIM_SetCompare3(BLDC_TIMx,BLDC_TIM_PERIOD);
            TIM_CCxNCmd(BLDC_TIMx,TIM_Channel_3,TIM_CCxN_Enable);
        }
        break;
        
        default:
        {
            TIM_CCxCmd(BLDC_TIMx,TIM_Channel_1,TIM_CCx_Disable);
            TIM_CCxNCmd(BLDC_TIMx,TIM_Channel_1,TIM_CCxN_Disable);
            TIM_CCxCmd(BLDC_TIMx,TIM_Channel_2,TIM_CCx_Disable);
            TIM_CCxNCmd(BLDC_TIMx,TIM_Channel_2,TIM_CCxN_Disable);
            TIM_CCxCmd(BLDC_TIMx,TIM_Channel_3,TIM_CCx_Disable);
            TIM_CCxNCmd(BLDC_TIMx,TIM_Channel_3,TIM_CCxN_Disable);
        }
        break;
    }
    
    TIM_SetCompare4(BLDC_TIMx,BLDC_TIM_PERIOD*speed_duty/1000 - 1);         ////比较器通道4的CC设为 占空比的周期
}

/**
  * 函数功能: 霍尔传感器接口中断回调函数
  * 输入参数: 无
  * 返 回 值: 无
  * 说    明: 无
外接口   5V | HB         5V | HA
         -------  ----> ---------
         HC | HA         HC | HB

1、2、4->2、1、4 只换一处     注意到这个中断是指的是触发中断  即HALL有变化时中断一次  既然这样的话，那电机是怎么启动起来的呢？
  */

u8 tttt = 1;
void HALL_TIMx_Callback(void)
{
    uint8_t pinstate = 0;
    uint8_t i = 0;
    static __IO int8_t pinstate0 = 0; // 保存上次霍尔传感器信号
    static uint8_t ii = 0;
    
#if (HJL_TEST1 == 1)		// 20211113 hjl
    if((HALL_TIM_CH1_GPIO->IDR & HALL_TIM_CH1_PIN) != (uint32_t)Bit_RESET)  //霍尔传感器状态获取 U1 ->W4// V2
    {
        pinstate |= 0x04;//0x01;
    }
    
    if((HALL_TIM_CH2_GPIO->IDR & HALL_TIM_CH2_PIN) != (uint32_t)Bit_RESET)  //霍尔传感器状态获取 V2 -> V2//U1
    {
        pinstate |= 0x02;
    }
    
    if((HALL_TIM_CH3_GPIO->IDR & HALL_TIM_CH3_PIN) != (uint32_t)Bit_RESET)  //霍尔传感器状态获取 W4 -> U1//W4
    {
        pinstate |= 0x01;// 0x04;
    }
	if (tmp_hall_cnt < 48)
	{	
		tmp_pinstate_buff[tmp_hall_cnt] = pinstate;
	}
	tmp_hall_cnt++;
#else 
    //if(bldc_dev.motor_state == STOP)
    if(bldc_dev.motor_state != RUN)
    {
        return;  // 电机处于停止工作状态，退出函数
    }
    //不断获取霍尔位置  shf
    if((HALL_TIM_CH1_GPIO->IDR & HALL_TIM_CH1_PIN) != (uint32_t)Bit_RESET)  //霍尔传感器状态获取 U1 ->W4// V2
    {
        pinstate |= 0x04;//0x01;
    }
    
    if((HALL_TIM_CH2_GPIO->IDR & HALL_TIM_CH2_PIN) != (uint32_t)Bit_RESET)  //霍尔传感器状态获取 V2 -> V2//U1
    {
        pinstate |= 0x02;
    }
    
    if((HALL_TIM_CH3_GPIO->IDR & HALL_TIM_CH3_PIN) != (uint32_t)Bit_RESET)  //霍尔传感器状态获取 W4 -> U1//W4
    {
        pinstate |= 0x01;// 0x04;
    }
    
    //顺时针转CCW 1 3 2 6 4 5，逆时针转CW 6 4 5 1 3 2
    //霍尔异常判断
    if(HALL_CHECK_START == Hall_Check_Flag)//启动霍尔异常检测  第一次总是运行到这里来的
    {
        Hall_Check_SeqNum = 0xFF;
        for(i = 0;i < 6;i++)
        {
            //初始位置检测
            if(Hall_Check_Seq[i] == pinstate)      ////   Hall_Check_Seq[6] = {1,3,2,6,4,5};    这里是判断初始时，HALL的位置到底在那里--见笔记本最后一页画的图
            {
                Hall_Check_SeqNum = i;
                Hall_Check_Flag = HALL_CHECK;           ///这个有点象是个一次性的标志，设了这个值之后，就再也没有设置过了。
                Hall_OK_Cnt = 0;
                Hall_Err_Cnt = 0;
                break;
            }
        }
        
        if(0xFF == Hall_Check_SeqNum)//霍尔异常
        {
            Hall_Err_Cnt++;
            if(Hall_Err_Cnt >= HALL_ERR_CNT_LIMIT )          ////多次出错才会停机，否则，一直在检测
            {
                //故障报警，停机
                Hall_Err_Cnt = 0;
                Set_MotorErr(M_RUN_FAULT);//设置电机运作失败故障
                Set_MotorErr(M_HALL_ERR);//设置霍尔异常故障
                Set_Motor_Stop();//电机停机
            }
        }
    }
    else if(HALL_CHECK == Hall_Check_Flag)//          开始检测    以后是次次运行到这里来了
    {    
         //此处检测有没有切换
         if(Hall_Check_Seq[(Hall_Check_SeqNum + 1) % 6] == pinstate)//下一序列
         {
             Hall_OK_Cnt++;
             Hall_Check_SeqNum = ((Hall_Check_SeqNum + 1) % 6);
         }
         else if(Hall_Check_Seq[(Hall_Check_SeqNum + 5) % 6] == pinstate)//前一序列
         {
             Hall_OK_Cnt++;
             Hall_Check_SeqNum = ((Hall_Check_SeqNum + 5) % 6);
         }
         else                                               ///既不是前一序列，也不是后一序列，这个就麻烦了 估计出问题了吧 
         {
             if(Hall_Check_Seq[Hall_Check_SeqNum] != pinstate)    ////既不是前，也不是后，也不是当前的，说明序列出错了   如果是当前的，则什么事都不用做了
             {
                 Hall_Err_Cnt++;
                 Hall_OK_Cnt = 0;
                 if(Hall_Err_Cnt >= HALL_ERR_CNT_LIMIT)  ////多来几次这样的，才会真正的出错，如果只是偶尔来几次，也算是正常，因为启动过程中可能有偶发性的干扰出现。目的是滤掉这些干扰
                 {
                      //故障报警，停机
                      Set_MotorErr(M_RUN_FAULT);//设置电机运作失败故障
                      Set_MotorErr(M_HALL_ERR);//设置霍尔异常故障
                      Set_Motor_Stop();//电机停机  ----------------------------电机停机，只经设置一个标志即可，以电机运行状态机中会自动停掉的        
                      Hall_Err_Cnt = 0;
                 }
             }
         }
         
         if(Hall_OK_Cnt >= 24)//大于24次（一圈）正常     大于一圈，表示运行好了。 4*6
         {
              Hall_Err_Cnt = 0;
         }
    }
    else
    {
        Hall_Check_Flag = HALL_CHECK_IDLE;
        Hall_Err_Cnt = 0;
        Hall_OK_Cnt = 0;
    }
    //-----------霍尔异常判断-------------------//
    
    if(CW == bldc_dev.motor_direction) // 方向判断（逆时针）  为什么是7减  顺时钟就不要减？
    {
        pinstate = 7 - pinstate;
    }
    //pinstate = 7 - pinstate;
    
    //pinstate = tttt;
    BLDC_PHASE_CHANGE(pinstate);//驱动换相   
    
    if(pinstate0 != pinstate) // 测试发现有时会连续出现两个相同的数据，这里滤掉重复的
    {    
        bldc_dev.step_counter++;    // 步进递增
        bldc_dev.stalling_count = 0;  // 阻塞计数清零
        bldc_dev.stalling_number = 0;	// 20211115 hjl
        g_Motor_Hall_Count++;
    }
    
    pinstate0 = pinstate;
#endif
}

// 20211115 hjl
void HALL_Abnormal_Callback()
{
    uint8_t tmp_pinstate = 0;
    
    //顺时针转CCW 1 3 2 6 4 5，逆时针转CW 6 4 5 1 3 2
	if(CW == bldc_dev.motor_direction)
	{
		Hall_Check_SeqNum = ((Hall_Check_SeqNum + 5) % 6);
		tmp_pinstate = Hall_Check_Seq[Hall_Check_SeqNum];
		tmp_pinstate = 7 - tmp_pinstate;		
	}else
	{
		Hall_Check_SeqNum = ((Hall_Check_SeqNum + 1) % 6);
		tmp_pinstate = Hall_Check_Seq[Hall_Check_SeqNum];
	}
	
    BLDC_PHASE_CHANGE(tmp_pinstate);//驱动换相   
}
/**
  * 函数功能: 电机运行控制函数     这个是做为一个任务来运行的。不象HALL调用，它是在中断中调用的。故那个的优先级显然是最高了
  * 输入参数: 无    
  * 返 回 值: 无          此任务1ms调用一次
  * 说    明: 无
  */
uint8_t Test_pinstate = 0;
void Motor_Run_Control(void)
{
    int32_t startTime;
    Test_pinstate = 0;
    if((HALL_TIM_CH1_GPIO->IDR & HALL_TIM_CH1_PIN) != (uint32_t)Bit_RESET)  //霍尔传感器状态获取 U1->W4
    {
        Test_pinstate |= 0x04;
    }
    
    if((HALL_TIM_CH2_GPIO->IDR & HALL_TIM_CH2_PIN) != (uint32_t)Bit_RESET)  //霍尔传感器状态获取 V2->V2
    {
        Test_pinstate |= 0x02;
    }
    
    if((HALL_TIM_CH3_GPIO->IDR & HALL_TIM_CH3_PIN) != (uint32_t)Bit_RESET)  //霍尔传感器状态获取 W4->U1
    {
        Test_pinstate |= 0x01;
    }
#if (KEY_TEST == 1)
    Key_Test();//按键测试代码
#endif
    
    switch(Motor_Run_State)
    {
        case MOTOR_IDLE://空闲
        {
#if(FORCE_MODE_ENABLE == 1)//采用三下管到通刹车模式
            Motor_PWM_IDLE();
#else
            TIM_CCxCmd(BLDC_TIMx,TIM_Channel_1,TIM_CCx_Disable);
            TIM_CCxNCmd(BLDC_TIMx,TIM_Channel_1,TIM_CCxN_Disable);
            TIM_CCxCmd(BLDC_TIMx,TIM_Channel_2,TIM_CCx_Disable);
            TIM_CCxNCmd(BLDC_TIMx,TIM_Channel_2,TIM_CCxN_Disable);
            TIM_CCxCmd(BLDC_TIMx,TIM_Channel_3,TIM_CCx_Disable);
            TIM_CCxNCmd(BLDC_TIMx,TIM_Channel_3,TIM_CCxN_Disable);
#endif
            //Motor_PWM_IDLE();
            if(TRIG_MOTOR_RUN == gs_Motor_Param.byRunTrig)//参数设置启动  这个只在接到命令后才会成立啊  GXD如果没接到命令，电机就是停止的。
            {
                bldc_dev.motor_state = START;         ///搞两个启动状态，搞的有点脑子坏掉。  一个是全局变量 Motor_Run_State  一个是马达的状态
            }
            
            if(START == bldc_dev.motor_state)// 电机启动
            {
                Motor_Run_State = MOTOR_INIT;//初始化
                gs_Motor_Param.byRunOrDelayFlag = DECREASE_IDLE;//延迟空闲
#if (KEY_TEST == 1)
                gs_Motor_Param.byRunTrig = TRIG_MOTOR_RUN;                   ///这里又搞一个状态 变成运行要搞三个状态来标识了  这里又来一个运行触发状态
                gs_Motor_Param.sMotorParam_Set.byRunSpeed = (696/6);
                gs_Motor_Param.sMotorParam_Set.byDelayTimer = 100;//1S
                gs_Motor_Param.sMotorParam_Set.byRunTimer = 100;//1S
#endif
            }
        }
        break;
        
        case MOTOR_INIT://运行初始化
        {
            Clear_MotorErr(M_HALL_ERR);//清霍尔异常故障
            Clear_MotorErr(M_OVERLOAD);//清电机过流故障
            Clear_MotorErr(M_RUN_FAULT);//清电机运作失败故障
            
            if(TRIG_MOTOR_RUN == gs_Motor_Param.byRunTrig)//参数设置  这个变量两个地方设过，一个是下载参数时，一个是KEY_TEST=1时
            {
                memcpy(&gs_Motor_Param.sMotorParam_Local,&gs_Motor_Param.sMotorParam_Set,sizeof(S_RUN_PARAM));  ////将参数从设置区增长拷贝到本地区
 //20210717  GXD           gs_Motor_Param.byRunTrig = TRIG_ILDE;//    空闲            这个标志起什么作用呢？  没什么作用，多余 GXD
            }
            //gs_Motor_Param.dwMotorRunTimer = (u32)(M_TIMER_BASE * gs_Motor_Param.sMotorParam_Local.byRunTimer);//转换为100us
            
            
            gs_Motor_Param.dwMotorDelayTimer = (u32)(M_TIMER_BASE * gs_Motor_Param.sMotorParam_Local.byDelayTimer);//   转换为100us   这里怎么理解？  这个是下发过来的 估计这个是100us为单位的值
                                                                                                                   ///那么它代表什么意义呢？100us 减1次
            gs_Motor_Param.byRunOrDelayFlag = DECREASE_DELAY;//延迟运行  这是一个标志  =2
            Motor_Run_State = MOTOR_DELAY_RUN;                
#if(FORCE_MODE_ENABLE == 1)//采用三下管到通刹车模式   这个是定义过=1的所以下面这个会执行的
            Motor_PWM_READY();
#endif
        }
        break;
        
        case MOTOR_DELAY_RUN://延迟运行
        {
            if(0 == gs_Motor_Param.dwMotorDelayTimer)//延迟时间结束
            {
                bldc_dev.motor_state = START;//启动运行
                gs_Motor_Param.dnMotorSpeed = (int)(((int)6)*gs_Motor_Param.sMotorParam_Local.byRunSpeed);//6/1.5 设定转速/1.5倍 这里要改成6 20210902 原为4
                startTime = gs_Motor_Param.dnMotorSpeed*5;
                gs_Motor_Param.dwMotorRunTimer = (u32)(M_TIMER_BASE * gs_Motor_Param.sMotorParam_Local.byRunTimer + startTime);//转换为100us*1.5倍       100us为单位的运行时间
                //  modifed by hjl, 20211024
#if ((HJL_TEST2 == 1) || (HJL_TEST3 == 1))		// 20211113 hjl
				g_Motor_Hall_Count_Dec = 200;
				g_Motor_Hall_Count_Stop = 250;
#else
                g_Motor_Hall_Count_Dec = (uint32_t)(gs_Motor_Param.dnMotorSpeed * 24 * gs_Motor_Param.sMotorParam_Local.byRunTimer / 6000.0 + 0.5);
                g_Motor_Hall_Count_Stop = (uint32_t)(g_Motor_Hall_Count_Dec + gs_Motor_Param.sMotorParam_Local.byRunSpeed*0.9 - 18 + 0.5);
                if (g_Motor_Hall_Count_Stop < g_Motor_Hall_Count_Dec)
                {
                  g_Motor_Hall_Count_Stop = g_Motor_Hall_Count_Dec;
                }
#endif
                gs_Motor_Param.byRunOrDelayFlag = DECREASE_RUN;//运行
                Motor_Run_State = MOTOR_START;//电机启动
            }
        }
        break;
        
        case MOTOR_START://开始
        {
            if(START == bldc_dev.motor_state) // 电机启动
            {
                // modified by hjl
                g_Motor_Hall_Count = 0;
                bldc_dev.motor_direction = gs_Motor_Param.sMotorParam_Local.byRunDir;
                bldc_dev.motor_state = RUN;
                bldc_dev.step_counter = 0;
                bldc_dev.step_counter_prev = 0;	// 20211115 hjl
                bldc_dev.stalling_count = 0;
                bldc_dev.stalling_number = 0;	// 20211115 hjl
                IncPIDInit();
                
                // for test, hjl, 20211027
#if (HJL_TEST == 1)
                tmp_I = 0;
                memset(tmp_Set_Speed, 0, sizeof(tmp_Set_Speed));
                memset(tmp_Curr_Speed, 0, sizeof(tmp_Curr_Speed));
                memset(tmp_Curr_Duty, 0, sizeof(tmp_Curr_Duty));
#endif                
                
#if (HJL_TEST2 == 1)		// 20211113 hjl
				speed_duty = (int)(((int)10)*gs_Motor_Param.sMotorParam_Local.byRunSpeed);
				if(speed_duty > 500)
				{	speed_duty = 500;}
#else
                if((bldc_dev.motor_speed*10/16) > MOTOR_MIN_DUTY_SPEED)   // 120 20211024
                {
                    speed_duty = bldc_dev.motor_speed*10/16;// *10/25为转速和占空比一个转换，转速（0~2500），占空比（0~1000） 转速只有2500吗
                }
                else
                {
                    speed_duty = MOTOR_MIN_DUTY_SPEED;      // 120， 20211024 占空比至少会到20%的 确保启动
                }
#endif                
#if (HJL_TEST3 == 1)
				tmp_SetSpeed = gs_Motor_Param.dnMotorSpeed * 10 / 16;
				tmp_DeltaSpeed = gs_Motor_Param.dnMotorSpeed / 16;
#endif
                NVIC_EnableIRQ(HALL_TIM_IRQn);  
                HALL_TIMx_Callback();              ///马达启动的时候，倒是要调一下这个函数，以便于给这个霍尔定个初值吧GXD
                //设定目标转速
                bldc_dev.motor_setspeed = gs_Motor_Param.dnMotorSpeed;//(int)(((int)4)*gs_Motor_Param.sMotorParam_Local.byRunSpeed);6/1.5设定转速/1.5倍
                Motor_Run_State = MOTOR_RUN_ACC;
            }  
        }
        break;
        
        case MOTOR_RUN_ACC:  // 加速
        {
          // modified by hjl, 20211124
            if((0 == gs_Motor_Param.dwMotorRunTimer) || (g_Motor_Hall_Count >= g_Motor_Hall_Count_Dec) )//运行时间结束
            {
                Set_Motor_Dec();//减速
                gs_Motor_Param.dwMotorRunTimer = 1500;               ////这里是100ms
            } 
            else // 20211026
            {
	            if((bldc_dev.motor_speed+MOTOR_ACC_DELTA_SPEED) < bldc_dev.motor_setspeed)
	            {
	                bldc_dev.motor_speed += MOTOR_ACC_DELTA_SPEED;                       ////每毫秒增加5         这个加的挺快的  1秒钟10转的话，1毫秒就是0.01转
	            }
	            else
	            {
	                bldc_dev.motor_speed = bldc_dev.motor_setspeed;
	               // Motor_Run_State = MOTOR_RUN;
	            }
	            
	            if(bldc_dev.motor_speed > MOTOR_MAX_SPEED)
	            {
	                bldc_dev.motor_speed = MOTOR_MAX_SPEED;
	            }
            
	            bldc_pid.SetPoint = bldc_dev.motor_speed;       //motor_speed是自增的，相当于目标转速一直在自增，当增到最大速度motor_setspeed时不变
	            //HALL_TIMx_Callback();
	             //BLDC_PHASE_CHANGE(Test_pinstate);//驱动换相 
             }
        }
        break;
        
        case MOTOR_RUN_DEC: // 减速
        {
            bldc_pid.Proportion = P_DATA_DEC;              //比例常数 Proportional Const   0.7
            bldc_pid.Integral = I_DATA_DEC;					// 0.01
            if((gs_Motor_Param.dnMotor_NowSpeed <= MOTOR_MIN_SPEED) || (0 == gs_Motor_Param.dwMotorRunTimer) || (g_Motor_Hall_Count >= g_Motor_Hall_Count_Stop))
            {
                bldc_dev.motor_speed = MOTOR_MIN_SPEED;
                Set_Motor_Stop();//减速完停止
            }
            else 
            {
	            //bldc_dev.motor_speed = 100;  
	            
	            if((bldc_dev.motor_speed - MOTOR_DEC_DELTA_SPEED) >= MOTOR_MIN_SPEED)
	            {
	                bldc_dev.motor_speed -= MOTOR_DEC_DELTA_SPEED;             ////20210906  原先为50
	            }
	            else
	            {
	                bldc_dev.motor_speed = MOTOR_MIN_SPEED;
	            }
	            
	            bldc_pid.SetPoint = bldc_dev.motor_speed;     
	        }
        }
        break;
        
        case MOTOR_RUN: // 电机运行  ---没有运行到这里来啊  GXD  而且这里来回正反转好象也不合适吧
        {
            if(bldc_dev.motor_direction == CW)
            {
                bldc_dev.motor_direction = CCW;
            }
            else
            {
                bldc_dev.motor_direction = CW;
            }
            // modified by hjl 20211024
//            if(0 == gs_Motor_Param.dwMotorRunTimer)//运行时间结束
//            if((0 == gs_Motor_Param.dwMotorRunTimer) || (g_Motor_Hall_Count >= g_Motor_Hall_Count_Dec) )//运行时间结束
            if(g_Motor_Hall_Count >= g_Motor_Hall_Count_Dec)//运行时间结束	// 20211025
            {
 //               g_Motor_Hall_Count_Dec = g_Motor_Hall_Count;  // for test
                Set_Motor_Dec();//减速
            }
        }
        break;
        
        case MOTOR_STOP:        // 停机 
        {
 //           g_Motor_Hall_Count_Stop = g_Motor_Hall_Count;  // for test
            bldc_dev.motor_state = STOP;
            TIM_ClearITPendingBit (HALL_TIMx,TIM_IT_Trigger); 
            NVIC_DisableIRQ(HALL_TIM_IRQn);
            
#if(FORCE_MODE_ENABLE == 1)//采用三下管到通刹车模式
            Motor_PWM_IDLE();
#else
            TIM_CCxCmd(BLDC_TIMx,TIM_Channel_1,TIM_CCx_Disable);
            TIM_CCxNCmd(BLDC_TIMx,TIM_Channel_1,TIM_CCxN_Disable);
            TIM_CCxCmd(BLDC_TIMx,TIM_Channel_2,TIM_CCx_Disable);
            TIM_CCxNCmd(BLDC_TIMx,TIM_Channel_2,TIM_CCxN_Disable);
            TIM_CCxCmd(BLDC_TIMx,TIM_Channel_3,TIM_CCx_Disable);
            TIM_CCxNCmd(BLDC_TIMx,TIM_Channel_3,TIM_CCxN_Disable);
#endif
            //Motor_PWM_IDLE();
            Set_Motor_Stop();
            Set_Motor_Stop_Delay(5);//设置停机延时1ms
            Motor_Run_State = MOTOR_STOP_NEXT;
        }
        break;
        
        case MOTOR_STOP_NEXT:        // 停机续
        {                  
            if(0 == g_Motor_Stop_Delay_Timer)//停机延时完成            
            {
#if(FORCE_MODE_ENABLE == 1)//采用三下管到通刹车模式
                Motor_PWM_IDLE();
#else
                TIM_CCxNCmd(BLDC_TIMx,TIM_Channel_1,TIM_CCxN_Enable);          ////在这个时候，这个下管是导通呢，还是不导通？感觉好象是有时候会导通（PWM）
                TIM_CCxNCmd(BLDC_TIMx,TIM_Channel_2,TIM_CCxN_Enable);
                TIM_CCxNCmd(BLDC_TIMx,TIM_Channel_3,TIM_CCxN_Enable);
#endif
               
                Motor_Run_State = MOTOR_STOP_2NEXT;
                Set_Motor_Stop_Delay(100);//设置停机延时1ms 20210717 GXD  原先为1000 现改为100
            }
        }
        break;
        
        case MOTOR_STOP_2NEXT:        // 停机续
        {                  
            if(0 == g_Motor_Stop_Delay_Timer)//停机延时完成   //停机完成            
            {
#if(FORCE_MODE_ENABLE == 1)//采用三下管到通刹车模式
                Motor_PWM_IDLE();
#else
                TIM_CCxNCmd(BLDC_TIMx,TIM_Channel_1,TIM_CCxN_Disable);        /////此时下管关闭，为无效电平即高电平状态
                TIM_CCxNCmd(BLDC_TIMx,TIM_Channel_2,TIM_CCxN_Disable);
                TIM_CCxNCmd(BLDC_TIMx,TIM_Channel_3,TIM_CCxN_Disable);
#endif
                //Motor_PWM_IDLE();
                bldc_dev.motor_state = STOP;
                Motor_Run_State = MOTOR_IDLE;
                
                gs_Motor_Param.byRunTrig = TRIG_ILDE;//    空闲            这个标志起什么作用呢？  没什么作用，多余 GXD
                ////20210717  GXD 在这里将状态标志清为空闲。
            }
        }
        break;
    }
}

/**************设置停机********************************/
void Set_Motor_Stop(void)
{
     Motor_Run_State = MOTOR_STOP;
     Hall_Check_Flag = HALL_CHECK_IDLE;
     Hall_Err_Cnt = 0;
     Hall_OK_Cnt = 0;
}
/**************设置启动********************************/
void Set_Motor_Start(void)
{
    // Motor_Run_State = MOTOR_START;
     bldc_dev.motor_state = START;
     Hall_Check_Flag = HALL_CHECK_IDLE;
     Hall_Err_Cnt = 0;
     Hall_OK_Cnt = 0;
}

/**************设置减速********************************/
void Set_Motor_Dec(void)
{
     Motor_Run_State = MOTOR_RUN_DEC;
}

//测试代码
void Key_Test(void)
{
    if(Bit_RESET == GPIO_ReadInputDataBit(DEVICE_ADDR1_PORT,DEVICE_ADDR1_PIN))//低电平拨码1
    {
        if(MOTOR_IDLE == Motor_Run_State)//停机空闲
        {
            if(Bit_RESET == GPIO_ReadInputDataBit(DEVICE_ADDR64_PORT,DEVICE_ADDR64_PIN))//低电平拨码6
            {
                //bldc_dev.motor_direction = CW;//反转
                gs_Motor_Param.sMotorParam_Set.byRunDir = CW;//反转
            }
            else
            {
                //bldc_dev.motor_direction = CCW;//正转
                gs_Motor_Param.sMotorParam_Set.byRunDir = CCW;//正转
            }
            Set_Motor_Start();
        }
    }
    else
    {
        if((MOTOR_RUN_ACC == Motor_Run_State) || (MOTOR_RUN == Motor_Run_State))//加速或者运行
        {
            Set_Motor_Dec();//设置减速停止
        }
    }
}

//电机空闲或停机  此时6个引脚的电平是多少呢？特别是N引脚？
void Motor_PWM_IDLE(void)
{
    TIM_CCxCmd(BLDC_TIMx,TIM_Channel_1,TIM_CCx_Disable);
    TIM_CCxNCmd(BLDC_TIMx,TIM_Channel_1,TIM_CCxN_Disable);
    TIM_CCxCmd(BLDC_TIMx,TIM_Channel_2,TIM_CCx_Disable);
    TIM_CCxNCmd(BLDC_TIMx,TIM_Channel_2,TIM_CCxN_Disable);
    TIM_CCxCmd(BLDC_TIMx,TIM_Channel_3,TIM_CCx_Disable);
    TIM_CCxNCmd(BLDC_TIMx,TIM_Channel_3,TIM_CCxN_Disable);
    TIM_ForcedOC1Config(BLDC_TIMx,TIM_ForcedAction_InActive);        //        强制低
    TIM_ForcedOC2Config(BLDC_TIMx,TIM_ForcedAction_InActive);
    TIM_ForcedOC3Config(BLDC_TIMx,TIM_ForcedAction_InActive);
    TIM_OC1NPolarityConfig(BLDC_TIMx,TIM_OCNPolarity_High);          ////停机时，这个极性配置是高电平有效  也就是 CC1NP = 0;    TIM_OCNPolarity_High=0x0
    TIM_OC2NPolarityConfig(BLDC_TIMx,TIM_OCNPolarity_High);         //// 按照定义：OCxN=CCxNP，OCxN_EN=0  此时，这个N的输出为低电平，而低电平，正好是下管导通
    TIM_OC3NPolarityConfig(BLDC_TIMx,TIM_OCNPolarity_High);

}

///准备运行时，到底这个6个引脚的电平是多少呢？ N设为低电平有效，则意味着CCNP=1  也就是说在CNT<CC1时，它是REF是有效电平  而OCxREF反相则为无效电平，那么CNT<CC1时 它的输出为高电平
////                                                                           注意到N的输出 ==== OCxREF反相 + 极性 + 死区

//准备运行     开始初始化时有个定时器是对中计数模式  现在又是PWM模式1，好象矛盾，检查一下怎么回事
void Motor_PWM_READY(void)
{
    TIM_OC1NPolarityConfig(BLDC_TIMx,TIM_OCNPolarity_Low);     ///定时器N通道的三个脚都是低电平有效， 非N通道在那里设置呢   缺省就是高电平有效。
    TIM_OC2NPolarityConfig(BLDC_TIMx,TIM_OCNPolarity_Low);
    TIM_OC3NPolarityConfig(BLDC_TIMx,TIM_OCNPolarity_Low);
    TIM_SelectOCxM(BLDC_TIMx,TIM_Channel_1,TIM_OCMode_PWM1);    ///：PWM模式1－ 在向上计数时，一旦TIMx_CNT<TIMx_CCR1时通道1为有效电平，否则为无效电平；
                                                                ////         在向下计数时，一旦TIMx_CNT>TIMx_CCR1时通道1为无效电平(OC1REF=0)，否则为有效电平(OC1REF=1)。
    TIM_SelectOCxM(BLDC_TIMx,TIM_Channel_2,TIM_OCMode_PWM1);
    TIM_SelectOCxM(BLDC_TIMx,TIM_Channel_3,TIM_OCMode_PWM1);
    TIM_CCxCmd(BLDC_TIMx,TIM_Channel_1,TIM_CCx_Disable);
    TIM_CCxNCmd(BLDC_TIMx,TIM_Channel_1,TIM_CCxN_Disable);
    TIM_CCxCmd(BLDC_TIMx,TIM_Channel_2,TIM_CCx_Disable);
    TIM_CCxNCmd(BLDC_TIMx,TIM_Channel_2,TIM_CCxN_Disable);
    TIM_CCxCmd(BLDC_TIMx,TIM_Channel_3,TIM_CCx_Disable);
    TIM_CCxNCmd(BLDC_TIMx,TIM_Channel_3,TIM_CCxN_Disable);
    TIM_OC1PreloadConfig(BLDC_TIMx,TIM_OCPreload_Enable);     ////预装载是允许的        禁止为0：可随时写入TIMx_CCR1寄存器，并且新写入的数值立即起作用。
    TIM_OC2PreloadConfig(BLDC_TIMx,TIM_OCPreload_Enable);  ///  开启为1：开启TIMx_CCR1寄存器的预装载功能，读写操作仅对预装载寄存器操作，TIMx_CCR1的预装载值在更新事件到来时被加载至当前寄存器中
    TIM_OC3PreloadConfig(BLDC_TIMx,TIM_OCPreload_Enable);

#if(MOTOR_BRAKE_ENABLE == 0)//关闭刹车功能    
    TIM_ClearITPendingBit(BLDC_TIMx,TIM_IT_Break);
    TIM_ITConfig(BLDC_TIMx,TIM_IT_Break,DISABLE);
#else//开启刹车功能
    TIM_ClearITPendingBit(BLDC_TIMx,TIM_IT_Break);
    TIM_ITConfig(BLDC_TIMx,TIM_IT_Break,ENABLE);
#endif
    /* TIM主输出使能 */
    TIM_CtrlPWMOutputs(BLDC_TIMx, ENABLE);       ////  这个是MOE位=1    这个早就设过了，没有再设回过0 一直就是1了
}

//电机过流监测
void MotorOverLoad_Check(void)
{
    u8 i;
    static u32 dwCurrent = 0;
    if(((MOTOR_RUN_ACC == Motor_Run_State) || (MOTOR_RUN == Motor_Run_State)\
      ||(MOTOR_RUN_DEC == Motor_Run_State)) && (RUN == bldc_dev.motor_state))
    {
        dwCurrent = 0;
        for(i = 0; i < 4; i++)
        {
             dwCurrent += g_dwAdc_CurrentValue[i];
        }
        
        gs_Motor_Param.dwMotor_Current = (dwCurrent >> 2);   ////得到马达的电流值
        
        if(gs_Motor_Param.dwMotor_Current  > LIMIT_CURRENT_L)
        {
            gs_Motor_Param.dwOverLoadCnt_L++;                    ///   过流L计次值++
            gs_Motor_Param.byCurrentDownL_Flag = 0;              ///   下溢L标志清0
            
            if(gs_Motor_Param.dwMotor_Current  > LIMIT_CURRENT_H)
            {
                gs_Motor_Param.dwOverLoadCnt_H++;                 ///过流H计次值++ 
                gs_Motor_Param.byCurrentDownH_Flag = 0;            ///下溢H标志清0
            }
            else
            {
                 gs_Motor_Param.byCurrentDownH_Flag++;
                if(gs_Motor_Param.byCurrentDownH_Flag >= 3)//连续3次低于高电流，则将这个高电流计次计数器清0
                {
                    gs_Motor_Param.dwOverLoadCnt_H = 0;
                }           
            }
        }
        else        ////电流小于低电流
        {
            gs_Motor_Param.byCurrentDownH_Flag++;
            gs_Motor_Param.byCurrentDownL_Flag++;
            if(gs_Motor_Param.byCurrentDownL_Flag >= 3)//连续3次低
            {
                gs_Motor_Param.dwOverLoadCnt_L = 0;
            }
            if(gs_Motor_Param.byCurrentDownH_Flag >= 3)//连续3次低
            {
                gs_Motor_Param.dwOverLoadCnt_H = 0;
            }
            //gs_Motor_Param.dwOverLoadCnt_H = 0;
            //gs_Motor_Param.dwOverLoadCnt_L = 0;
        }
        
        if((gs_Motor_Param.dwOverLoadCnt_L >= OVERLOAD_TIMER_L)\
           ||(gs_Motor_Param.dwOverLoadCnt_H >= OVERLOAD_TIMER_H))//过流超限
        {
            bldc_dev.motor_state = STOP;
            Set_MotorErr(M_RUN_FAULT);//设置电机运作失败故障
            Set_MotorErr(M_OVERLOAD);//设置电机过流故障
            Set_Motor_Stop();//设置停机                     Motor_Run_State = MOTOR_STOP;就可以停车吗
            gs_Motor_Param.dwOverLoadCnt_L = 0;
            gs_Motor_Param.dwOverLoadCnt_H = 0;
#if(FORCE_MODE_ENABLE == 1)//采用三下管到通刹车模式
            Motor_PWM_IDLE();//停机
#else
            TIM_CCxCmd(BLDC_TIMx,TIM_Channel_1,TIM_CCx_Disable);      ///不刹车的话，就直接将6个定时器的输出不允许了。  在MOE=1的情况下，此时OCx=CCxP=0  OCxN=CCxNP=1 
            TIM_CCxNCmd(BLDC_TIMx,TIM_Channel_1,TIM_CCxN_Disable);
            TIM_CCxCmd(BLDC_TIMx,TIM_Channel_2,TIM_CCx_Disable);
            TIM_CCxNCmd(BLDC_TIMx,TIM_Channel_2,TIM_CCxN_Disable);
            TIM_CCxCmd(BLDC_TIMx,TIM_Channel_3,TIM_CCx_Disable);
            TIM_CCxNCmd(BLDC_TIMx,TIM_Channel_3,TIM_CCxN_Disable);
#endif
        }
    }
    
}