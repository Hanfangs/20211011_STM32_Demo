/*
********************** (C) COPYRIGHT 2020 SOLAR  *************************
* File Name          : Motor_Control_BLDC.c
* Author             : dingyongjian 
* Date First Issued  : 
* Description        : bldc�������
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
/* ˽�б��� ------------------------------------------------------------------*/
MOTOR_DEVICE bldc_dev = {200,1000,STOP,CCW,0,0,0,0};        ///ת��Ϊ200  ����ת��Ϊ1000; 20211115 hjl
static PID bldc_pid;
__IO uint16_t speed_duty = MOTOR_MIN_DUTY_SPEED; // �ٶ�ռ�ձȣ�0~1000  Ϊ1000��ռ�ձ�Ϊ100%
                              // ��ʼ��ֵ���벻С��70�������������
__IO uint16_t Limit_speed_duty = 500; //������������ռ�ձ�
MOTOR_RUN_STATE Motor_Run_State = MOTOR_IDLE;                        ////ȱʡʱ�����״̬�ǿ���

uint32_t g_Motor_Stop_Delay_Timer = 0;
uint16_t Hall_Err_Cnt = 0;//�����쳣����
uint16_t Hall_OK_Cnt = 0;//������������
uint8_t  Hall_Check_Seq[6] = {1,3,2,6,4,5};//�����ж�����
uint8_t  Hall_Check_Flag = 0;//�������������ʶ
uint8_t  Hall_Check_SeqNum = 0;//���к�

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
/**************����ͣ����ʱ********************************/
void Set_Motor_Stop_Delay(uint32_t delay_time) 
{
    g_Motor_Stop_Delay_Timer = delay_time;
}
/**************���ͣ����ʱ********************************/
void Motor_Stop_Delay(void)
{
    if(g_Motor_Stop_Delay_Timer > 0)     ///��1ms��ʱ���е�����
    {
        g_Motor_Stop_Delay_Timer--;
    }
}
/**************PID������ʼ��********************************/
void IncPIDInit(void) 
{
    bldc_dev.motor_speed = MOTOR_MIN_SPEED;//���ת������       100   �ϵ�ʱ������ٶ������ת��
    bldc_pid.LastError  = 0;                    //Error[-1]
    bldc_pid.PrevError  = 0;                    //Error[-2]
    bldc_pid.Proportion = P_DATA_ACC;              //�������� Proportional Const
    bldc_pid.Integral   = I_DATA_ACC;                //���ֳ���  Integral Const
    bldc_pid.Derivative = D_DATA_ACC;              //΢�ֳ��� Derivative Const
    bldc_pid.SetPoint   = bldc_dev.motor_speed;  //�趨Ŀ��Desired Value           �ղ��Ѿ���Ϊ100��
    Hall_Err_Cnt = 0;//�����쳣��������
    Hall_Check_Flag = HALL_CHECK_START;//                    ���������쳣�����ж�
    memset(&g_dwAdc_CurrentValue,0,8);
    gs_Motor_Param.dwOverLoadCnt_H = 0;
    gs_Motor_Param.dwOverLoadCnt_L = 0;
    gs_Motor_Param.dwMotor_Current = 0;
    gs_Motor_Param.dwLimitCurrent_H = LIMIT_CURRENT_H;
    gs_Motor_Param.dwLimitCurrent_L = LIMIT_CURRENT_L;
}
/********************����ʽPID�������************************************/
int IncPIDCalc(int NextPoint) 
{
    int iError,iIncpid;                                       //��ǰ���
    
    iError = bldc_pid.SetPoint - NextPoint;                     //��������
    iIncpid = (int)((bldc_pid.Proportion * iError)             /*E[k]��*/\
                -(bldc_pid.Integral * bldc_pid.LastError)     /*E[k-1]��*/\
                +(bldc_pid.Derivative * bldc_pid.PrevError));  /*E[k-2]*/
                
    bldc_pid.PrevError = bldc_pid.LastError;                    //�洢�������´μ���
    bldc_pid.LastError = iError;
    
    return(iIncpid);                                    //��������ֵ
}
/**
  * ��������: ϵͳ�δ�ʱ���ص�����     ���ʵ���Ͼ���ת�ٿ���  ���ת�ٴ��ˣ�������PIDȥ��ռ�ձȼ�С�����ת�ٵ��ˣ��ͽ����ռ�ձ����ӡ�
  * �������: ��
  * �� �� ֵ: ��
  * ˵    ��: 1ms���иú���һ��
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
            bldc_dev.motor_state = STOP;         ///       ���ֵ������Ժ����Ҳ���ᵽ����ط����ˣ����ÿ������õĺ����ͻ�����ֱ���˳���---��ǰ������5�С�����
            //Motor_Run_State = MOTOR_STOP;
            //���ö�ת����
            Set_MotorErr(M_RUN_FAULT);  //���õ������ʧ�ܹ���
            Set_MotorErr(M_OVERLOAD);   //���õ����������
            Set_Motor_Stop();//���ͣ��                                             ---���ͣ��ֻ��������һ����־����---��������    Motor_Run_State = MOTOR_STOP;
		}
		else
		{
			HALL_Abnormal_Callback();
		}
	}   
	else
  {
    if(time_count >= 50) //   original is > 50  ����ʱ����� 50ms  ÿ��50�����ֵ�ᱻ��0     �����������״̬����������Զ�������
    {
        static int temp;
        int pid_result;
        // bldc_dev.step_counter��¼������������50msʱ���ڲ���������������������תһȦ�����ܹ�
        // (MOTOR_POLE_PAIR_NUM*6)������,  24��       ʹ�� n=bldc_dev.step_counter/(MOTOR_POLE_PAIR_NUM*6) Ϊ
        // 50ms�ڵ��ת��Ȧ����Ϊ����Ϊ��ת/����(rpm)��
        //  n/50 = x/(60*1000) --> x=bldc_dev.step_counter*50
//        temp = bldc_dev.step_counter * 50;             ////ÿ��HALL��Ӧ��һ�Σ����ֵ�ͻ�++  ----�ⲿ����HALL�ж�����      ���ǿ�50�������棬������ӵ������ˡ�����ӵ�10  ���ʱ����500
        temp = (bldc_dev.step_counter + bldc_dev.step_counter_prev) * 25;     // 20211115 hjl ȡ��ֵ
        
        gs_Motor_Param.dnMotor_NowSpeed = temp;      ////����㲻����ʲô��˼�� �����ٶȾ���50ms�ڵ�������� * 50   �ռ����������ȷʵ����ÿ���ӵ�ת�١�
        
        pid_result = IncPIDCalc(temp);        //   ��������
        pid_result = pid_result * 10 / 16;    // *10/25Ϊת�ٺ�ռ�ձ�һ��ת����ת�٣�0~2500����ռ�ձȣ�0~1000��
        
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
            if(temp <= 50)                 ///��ת�ٱȽϵ͵�����£���������ռ�ձ��ֱȽϴ�Ļ�����ɵ���ܲ��˵�
            {
                if(speed_duty >= Limit_speed_duty)         ///  ���ֵ�� 500
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
//        HALL_TIMx_Callback();       ////��֪��Ϊʲô����Ҫ��������жϺ���   ������ ��һ�ο���ôҲ�㲻��ΪʲôҪ����������أ���֪��ȥ������᲻���������� 20211115 hjl
        
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
  * ��������: BLDC�������
  * �������: ��
  * �� �� ֵ: ��
  * ˵    ��: ��
  ��ӿ� 5V | HB         5V | HA
         -------  ----> ---------
         HC | HA         HC | HB
  step˳��451326  ---->  462315 ֻ��һ��

 */
void BLDC_PHASE_CHANGE(uint8_t step)
{
    switch(step)         ////��������Ϲ�����Ҫ��ͨ����һ�㣬���¹���ͨ�Ķ�һ��
    {
        case 4: //B+ C-            C- ==BLDC_TIM_PERIOD       B+ ==speed_duty*BLDC_TIM_PERIOD
        {
            /* Next step: Step 2 Configuration -------------------------------------- */ 
            TIM_CCxCmd(BLDC_TIMx,TIM_Channel_1,TIM_CCx_Disable);
            TIM_CCxNCmd(BLDC_TIMx,TIM_Channel_1,TIM_CCxN_Disable);        ////��ʱ��1��ͨ��1������
        
            /*  Channel1 configuration */
            /*  Channel2 configuration */  
            TIM_CCxNCmd(BLDC_TIMx,TIM_Channel_2,TIM_CCxN_Disable);           ///�Ȱ�ͨ��2�ص�

            TIM_SetCompare2(BLDC_TIMx,BLDC_TIM_PERIOD*speed_duty/1000);     ///�ٰ�����Ƚ�������һ��   �������ֵ������ô������   ����Ƚ������ֵ�� ǧ��֮speed_dutyռ�ձ�
            TIM_CCxCmd(BLDC_TIMx,TIM_Channel_2,TIM_CCx_Enable);           ///  �ٰ����ͨ��2 ����
            /*  Channel3 configuration */
            TIM_CCxCmd(BLDC_TIMx,TIM_Channel_3,TIM_CCx_Disable);          
            TIM_SetCompare3(BLDC_TIMx,BLDC_TIM_PERIOD);                  ////�������ͨ��3N �ıȽ�ֵ����Ǿ�������  20K�Ǹ���50us
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
            TIM_SetCompare1(BLDC_TIMx,BLDC_TIM_PERIOD);                 ///��������B+�ĵط�����ľ���ռ�ձ�*���ڣ���A-�ĵط�����ľ�������
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
    
    TIM_SetCompare4(BLDC_TIMx,BLDC_TIM_PERIOD*speed_duty/1000 - 1);         ////�Ƚ���ͨ��4��CC��Ϊ ռ�ձȵ�����
}

/**
  * ��������: �����������ӿ��жϻص�����
  * �������: ��
  * �� �� ֵ: ��
  * ˵    ��: ��
��ӿ�   5V | HB         5V | HA
         -------  ----> ---------
         HC | HA         HC | HB

1��2��4->2��1��4 ֻ��һ��     ע�⵽����ж���ָ���Ǵ����ж�  ��HALL�б仯ʱ�ж�һ��  ��Ȼ�����Ļ����ǵ������ô�����������أ�
  */

u8 tttt = 1;
void HALL_TIMx_Callback(void)
{
    uint8_t pinstate = 0;
    uint8_t i = 0;
    static __IO int8_t pinstate0 = 0; // �����ϴλ����������ź�
    static uint8_t ii = 0;
    
#if (HJL_TEST1 == 1)		// 20211113 hjl
    if((HALL_TIM_CH1_GPIO->IDR & HALL_TIM_CH1_PIN) != (uint32_t)Bit_RESET)  //����������״̬��ȡ U1 ->W4// V2
    {
        pinstate |= 0x04;//0x01;
    }
    
    if((HALL_TIM_CH2_GPIO->IDR & HALL_TIM_CH2_PIN) != (uint32_t)Bit_RESET)  //����������״̬��ȡ V2 -> V2//U1
    {
        pinstate |= 0x02;
    }
    
    if((HALL_TIM_CH3_GPIO->IDR & HALL_TIM_CH3_PIN) != (uint32_t)Bit_RESET)  //����������״̬��ȡ W4 -> U1//W4
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
        return;  // �������ֹͣ����״̬���˳�����
    }
    //���ϻ�ȡ����λ��  shf
    if((HALL_TIM_CH1_GPIO->IDR & HALL_TIM_CH1_PIN) != (uint32_t)Bit_RESET)  //����������״̬��ȡ U1 ->W4// V2
    {
        pinstate |= 0x04;//0x01;
    }
    
    if((HALL_TIM_CH2_GPIO->IDR & HALL_TIM_CH2_PIN) != (uint32_t)Bit_RESET)  //����������״̬��ȡ V2 -> V2//U1
    {
        pinstate |= 0x02;
    }
    
    if((HALL_TIM_CH3_GPIO->IDR & HALL_TIM_CH3_PIN) != (uint32_t)Bit_RESET)  //����������״̬��ȡ W4 -> U1//W4
    {
        pinstate |= 0x01;// 0x04;
    }
    
    //˳ʱ��תCCW 1 3 2 6 4 5����ʱ��תCW 6 4 5 1 3 2
    //�����쳣�ж�
    if(HALL_CHECK_START == Hall_Check_Flag)//���������쳣���  ��һ���������е���������
    {
        Hall_Check_SeqNum = 0xFF;
        for(i = 0;i < 6;i++)
        {
            //��ʼλ�ü��
            if(Hall_Check_Seq[i] == pinstate)      ////   Hall_Check_Seq[6] = {1,3,2,6,4,5};    �������жϳ�ʼʱ��HALL��λ�õ���������--���ʼǱ����һҳ����ͼ
            {
                Hall_Check_SeqNum = i;
                Hall_Check_Flag = HALL_CHECK;           ///����е����Ǹ�һ���Եı�־���������ֵ֮�󣬾���Ҳû�����ù��ˡ�
                Hall_OK_Cnt = 0;
                Hall_Err_Cnt = 0;
                break;
            }
        }
        
        if(0xFF == Hall_Check_SeqNum)//�����쳣
        {
            Hall_Err_Cnt++;
            if(Hall_Err_Cnt >= HALL_ERR_CNT_LIMIT )          ////��γ���Ż�ͣ��������һֱ�ڼ��
            {
                //���ϱ�����ͣ��
                Hall_Err_Cnt = 0;
                Set_MotorErr(M_RUN_FAULT);//���õ������ʧ�ܹ���
                Set_MotorErr(M_HALL_ERR);//���û����쳣����
                Set_Motor_Stop();//���ͣ��
            }
        }
    }
    else if(HALL_CHECK == Hall_Check_Flag)//          ��ʼ���    �Ժ��Ǵδ����е���������
    {    
         //�˴������û���л�
         if(Hall_Check_Seq[(Hall_Check_SeqNum + 1) % 6] == pinstate)//��һ����
         {
             Hall_OK_Cnt++;
             Hall_Check_SeqNum = ((Hall_Check_SeqNum + 1) % 6);
         }
         else if(Hall_Check_Seq[(Hall_Check_SeqNum + 5) % 6] == pinstate)//ǰһ����
         {
             Hall_OK_Cnt++;
             Hall_Check_SeqNum = ((Hall_Check_SeqNum + 5) % 6);
         }
         else                                               ///�Ȳ���ǰһ���У�Ҳ���Ǻ�һ���У�������鷳�� ���Ƴ������˰� 
         {
             if(Hall_Check_Seq[Hall_Check_SeqNum] != pinstate)    ////�Ȳ���ǰ��Ҳ���Ǻ�Ҳ���ǵ�ǰ�ģ�˵�����г�����   ����ǵ�ǰ�ģ���ʲô�¶���������
             {
                 Hall_Err_Cnt++;
                 Hall_OK_Cnt = 0;
                 if(Hall_Err_Cnt >= HALL_ERR_CNT_LIMIT)  ////�������������ģ��Ż������ĳ������ֻ��ż�������Σ�Ҳ������������Ϊ���������п�����ż���Եĸ��ų��֡�Ŀ�����˵���Щ����
                 {
                      //���ϱ�����ͣ��
                      Set_MotorErr(M_RUN_FAULT);//���õ������ʧ�ܹ���
                      Set_MotorErr(M_HALL_ERR);//���û����쳣����
                      Set_Motor_Stop();//���ͣ��  ----------------------------���ͣ����ֻ������һ����־���ɣ��Ե������״̬���л��Զ�ͣ����        
                      Hall_Err_Cnt = 0;
                 }
             }
         }
         
         if(Hall_OK_Cnt >= 24)//����24�Σ�һȦ������     ����һȦ����ʾ���к��ˡ� 4*6
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
    //-----------�����쳣�ж�-------------------//
    
    if(CW == bldc_dev.motor_direction) // �����жϣ���ʱ�룩  Ϊʲô��7��  ˳ʱ�ӾͲ�Ҫ����
    {
        pinstate = 7 - pinstate;
    }
    //pinstate = 7 - pinstate;
    
    //pinstate = tttt;
    BLDC_PHASE_CHANGE(pinstate);//��������   
    
    if(pinstate0 != pinstate) // ���Է�����ʱ����������������ͬ�����ݣ������˵��ظ���
    {    
        bldc_dev.step_counter++;    // ��������
        bldc_dev.stalling_count = 0;  // ������������
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
    
    //˳ʱ��תCCW 1 3 2 6 4 5����ʱ��תCW 6 4 5 1 3 2
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
	
    BLDC_PHASE_CHANGE(tmp_pinstate);//��������   
}
/**
  * ��������: ������п��ƺ���     �������Ϊһ�����������еġ�����HALL���ã��������ж��е��õġ����Ǹ������ȼ���Ȼ�������
  * �������: ��    
  * �� �� ֵ: ��          ������1ms����һ��
  * ˵    ��: ��
  */
uint8_t Test_pinstate = 0;
void Motor_Run_Control(void)
{
    int32_t startTime;
    Test_pinstate = 0;
    if((HALL_TIM_CH1_GPIO->IDR & HALL_TIM_CH1_PIN) != (uint32_t)Bit_RESET)  //����������״̬��ȡ U1->W4
    {
        Test_pinstate |= 0x04;
    }
    
    if((HALL_TIM_CH2_GPIO->IDR & HALL_TIM_CH2_PIN) != (uint32_t)Bit_RESET)  //����������״̬��ȡ V2->V2
    {
        Test_pinstate |= 0x02;
    }
    
    if((HALL_TIM_CH3_GPIO->IDR & HALL_TIM_CH3_PIN) != (uint32_t)Bit_RESET)  //����������״̬��ȡ W4->U1
    {
        Test_pinstate |= 0x01;
    }
#if (KEY_TEST == 1)
    Key_Test();//�������Դ���
#endif
    
    switch(Motor_Run_State)
    {
        case MOTOR_IDLE://����
        {
#if(FORCE_MODE_ENABLE == 1)//�������¹ܵ�ͨɲ��ģʽ
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
            if(TRIG_MOTOR_RUN == gs_Motor_Param.byRunTrig)//������������  ���ֻ�ڽӵ������Ż������  GXD���û�ӵ�����������ֹͣ�ġ�
            {
                bldc_dev.motor_state = START;         ///����������״̬������е����ӻ�����  һ����ȫ�ֱ��� Motor_Run_State  һ��������״̬
            }
            
            if(START == bldc_dev.motor_state)// �������
            {
                Motor_Run_State = MOTOR_INIT;//��ʼ��
                gs_Motor_Param.byRunOrDelayFlag = DECREASE_IDLE;//�ӳٿ���
#if (KEY_TEST == 1)
                gs_Motor_Param.byRunTrig = TRIG_MOTOR_RUN;                   ///�����ָ�һ��״̬ �������Ҫ������״̬����ʶ��  ��������һ�����д���״̬
                gs_Motor_Param.sMotorParam_Set.byRunSpeed = (696/6);
                gs_Motor_Param.sMotorParam_Set.byDelayTimer = 100;//1S
                gs_Motor_Param.sMotorParam_Set.byRunTimer = 100;//1S
#endif
            }
        }
        break;
        
        case MOTOR_INIT://���г�ʼ��
        {
            Clear_MotorErr(M_HALL_ERR);//������쳣����
            Clear_MotorErr(M_OVERLOAD);//������������
            Clear_MotorErr(M_RUN_FAULT);//��������ʧ�ܹ���
            
            if(TRIG_MOTOR_RUN == gs_Motor_Param.byRunTrig)//��������  ������������ط������һ�������ز���ʱ��һ����KEY_TEST=1ʱ
            {
                memcpy(&gs_Motor_Param.sMotorParam_Local,&gs_Motor_Param.sMotorParam_Set,sizeof(S_RUN_PARAM));  ////������������������������������
 //20210717  GXD           gs_Motor_Param.byRunTrig = TRIG_ILDE;//    ����            �����־��ʲô�����أ�  ûʲô���ã����� GXD
            }
            //gs_Motor_Param.dwMotorRunTimer = (u32)(M_TIMER_BASE * gs_Motor_Param.sMotorParam_Local.byRunTimer);//ת��Ϊ100us
            
            
            gs_Motor_Param.dwMotorDelayTimer = (u32)(M_TIMER_BASE * gs_Motor_Param.sMotorParam_Local.byDelayTimer);//   ת��Ϊ100us   ������ô��⣿  ������·������� ���������100usΪ��λ��ֵ
                                                                                                                   ///��ô������ʲô�����أ�100us ��1��
            gs_Motor_Param.byRunOrDelayFlag = DECREASE_DELAY;//�ӳ�����  ����һ����־  =2
            Motor_Run_State = MOTOR_DELAY_RUN;                
#if(FORCE_MODE_ENABLE == 1)//�������¹ܵ�ͨɲ��ģʽ   ����Ƕ����=1���������������ִ�е�
            Motor_PWM_READY();
#endif
        }
        break;
        
        case MOTOR_DELAY_RUN://�ӳ�����
        {
            if(0 == gs_Motor_Param.dwMotorDelayTimer)//�ӳ�ʱ�����
            {
                bldc_dev.motor_state = START;//��������
                gs_Motor_Param.dnMotorSpeed = (int)(((int)6)*gs_Motor_Param.sMotorParam_Local.byRunSpeed);//6/1.5 �趨ת��/1.5�� ����Ҫ�ĳ�6 20210902 ԭΪ4
                startTime = gs_Motor_Param.dnMotorSpeed*5;
                gs_Motor_Param.dwMotorRunTimer = (u32)(M_TIMER_BASE * gs_Motor_Param.sMotorParam_Local.byRunTimer + startTime);//ת��Ϊ100us*1.5��       100usΪ��λ������ʱ��
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
                gs_Motor_Param.byRunOrDelayFlag = DECREASE_RUN;//����
                Motor_Run_State = MOTOR_START;//�������
            }
        }
        break;
        
        case MOTOR_START://��ʼ
        {
            if(START == bldc_dev.motor_state) // �������
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
                    speed_duty = bldc_dev.motor_speed*10/16;// *10/25Ϊת�ٺ�ռ�ձ�һ��ת����ת�٣�0~2500����ռ�ձȣ�0~1000�� ת��ֻ��2500��
                }
                else
                {
                    speed_duty = MOTOR_MIN_DUTY_SPEED;      // 120�� 20211024 ռ�ձ����ٻᵽ20%�� ȷ������
                }
#endif                
#if (HJL_TEST3 == 1)
				tmp_SetSpeed = gs_Motor_Param.dnMotorSpeed * 10 / 16;
				tmp_DeltaSpeed = gs_Motor_Param.dnMotorSpeed / 16;
#endif
                NVIC_EnableIRQ(HALL_TIM_IRQn);  
                HALL_TIMx_Callback();              ///���������ʱ�򣬵���Ҫ��һ������������Ա��ڸ��������������ֵ��GXD
                //�趨Ŀ��ת��
                bldc_dev.motor_setspeed = gs_Motor_Param.dnMotorSpeed;//(int)(((int)4)*gs_Motor_Param.sMotorParam_Local.byRunSpeed);6/1.5�趨ת��/1.5��
                Motor_Run_State = MOTOR_RUN_ACC;
            }  
        }
        break;
        
        case MOTOR_RUN_ACC:  // ����
        {
          // modified by hjl, 20211124
            if((0 == gs_Motor_Param.dwMotorRunTimer) || (g_Motor_Hall_Count >= g_Motor_Hall_Count_Dec) )//����ʱ�����
            {
                Set_Motor_Dec();//����
                gs_Motor_Param.dwMotorRunTimer = 1500;               ////������100ms
            } 
            else // 20211026
            {
	            if((bldc_dev.motor_speed+MOTOR_ACC_DELTA_SPEED) < bldc_dev.motor_setspeed)
	            {
	                bldc_dev.motor_speed += MOTOR_ACC_DELTA_SPEED;                       ////ÿ��������5         ����ӵ�ͦ���  1����10ת�Ļ���1�������0.01ת
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
            
	            bldc_pid.SetPoint = bldc_dev.motor_speed;       //motor_speed�������ģ��൱��Ŀ��ת��һֱ������������������ٶ�motor_setspeedʱ����
	            //HALL_TIMx_Callback();
	             //BLDC_PHASE_CHANGE(Test_pinstate);//�������� 
             }
        }
        break;
        
        case MOTOR_RUN_DEC: // ����
        {
            bldc_pid.Proportion = P_DATA_DEC;              //�������� Proportional Const   0.7
            bldc_pid.Integral = I_DATA_DEC;					// 0.01
            if((gs_Motor_Param.dnMotor_NowSpeed <= MOTOR_MIN_SPEED) || (0 == gs_Motor_Param.dwMotorRunTimer) || (g_Motor_Hall_Count >= g_Motor_Hall_Count_Stop))
            {
                bldc_dev.motor_speed = MOTOR_MIN_SPEED;
                Set_Motor_Stop();//������ֹͣ
            }
            else 
            {
	            //bldc_dev.motor_speed = 100;  
	            
	            if((bldc_dev.motor_speed - MOTOR_DEC_DELTA_SPEED) >= MOTOR_MIN_SPEED)
	            {
	                bldc_dev.motor_speed -= MOTOR_DEC_DELTA_SPEED;             ////20210906  ԭ��Ϊ50
	            }
	            else
	            {
	                bldc_dev.motor_speed = MOTOR_MIN_SPEED;
	            }
	            
	            bldc_pid.SetPoint = bldc_dev.motor_speed;     
	        }
        }
        break;
        
        case MOTOR_RUN: // �������  ---û�����е���������  GXD  ����������������ת����Ҳ�����ʰ�
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
//            if(0 == gs_Motor_Param.dwMotorRunTimer)//����ʱ�����
//            if((0 == gs_Motor_Param.dwMotorRunTimer) || (g_Motor_Hall_Count >= g_Motor_Hall_Count_Dec) )//����ʱ�����
            if(g_Motor_Hall_Count >= g_Motor_Hall_Count_Dec)//����ʱ�����	// 20211025
            {
 //               g_Motor_Hall_Count_Dec = g_Motor_Hall_Count;  // for test
                Set_Motor_Dec();//����
            }
        }
        break;
        
        case MOTOR_STOP:        // ͣ�� 
        {
 //           g_Motor_Hall_Count_Stop = g_Motor_Hall_Count;  // for test
            bldc_dev.motor_state = STOP;
            TIM_ClearITPendingBit (HALL_TIMx,TIM_IT_Trigger); 
            NVIC_DisableIRQ(HALL_TIM_IRQn);
            
#if(FORCE_MODE_ENABLE == 1)//�������¹ܵ�ͨɲ��ģʽ
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
            Set_Motor_Stop_Delay(5);//����ͣ����ʱ1ms
            Motor_Run_State = MOTOR_STOP_NEXT;
        }
        break;
        
        case MOTOR_STOP_NEXT:        // ͣ����
        {                  
            if(0 == g_Motor_Stop_Delay_Timer)//ͣ����ʱ���            
            {
#if(FORCE_MODE_ENABLE == 1)//�������¹ܵ�ͨɲ��ģʽ
                Motor_PWM_IDLE();
#else
                TIM_CCxNCmd(BLDC_TIMx,TIM_Channel_1,TIM_CCxN_Enable);          ////�����ʱ������¹��ǵ�ͨ�أ����ǲ���ͨ���о���������ʱ��ᵼͨ��PWM��
                TIM_CCxNCmd(BLDC_TIMx,TIM_Channel_2,TIM_CCxN_Enable);
                TIM_CCxNCmd(BLDC_TIMx,TIM_Channel_3,TIM_CCxN_Enable);
#endif
               
                Motor_Run_State = MOTOR_STOP_2NEXT;
                Set_Motor_Stop_Delay(100);//����ͣ����ʱ1ms 20210717 GXD  ԭ��Ϊ1000 �ָ�Ϊ100
            }
        }
        break;
        
        case MOTOR_STOP_2NEXT:        // ͣ����
        {                  
            if(0 == g_Motor_Stop_Delay_Timer)//ͣ����ʱ���   //ͣ�����            
            {
#if(FORCE_MODE_ENABLE == 1)//�������¹ܵ�ͨɲ��ģʽ
                Motor_PWM_IDLE();
#else
                TIM_CCxNCmd(BLDC_TIMx,TIM_Channel_1,TIM_CCxN_Disable);        /////��ʱ�¹ܹرգ�Ϊ��Ч��ƽ���ߵ�ƽ״̬
                TIM_CCxNCmd(BLDC_TIMx,TIM_Channel_2,TIM_CCxN_Disable);
                TIM_CCxNCmd(BLDC_TIMx,TIM_Channel_3,TIM_CCxN_Disable);
#endif
                //Motor_PWM_IDLE();
                bldc_dev.motor_state = STOP;
                Motor_Run_State = MOTOR_IDLE;
                
                gs_Motor_Param.byRunTrig = TRIG_ILDE;//    ����            �����־��ʲô�����أ�  ûʲô���ã����� GXD
                ////20210717  GXD �����ｫ״̬��־��Ϊ���С�
            }
        }
        break;
    }
}

/**************����ͣ��********************************/
void Set_Motor_Stop(void)
{
     Motor_Run_State = MOTOR_STOP;
     Hall_Check_Flag = HALL_CHECK_IDLE;
     Hall_Err_Cnt = 0;
     Hall_OK_Cnt = 0;
}
/**************��������********************************/
void Set_Motor_Start(void)
{
    // Motor_Run_State = MOTOR_START;
     bldc_dev.motor_state = START;
     Hall_Check_Flag = HALL_CHECK_IDLE;
     Hall_Err_Cnt = 0;
     Hall_OK_Cnt = 0;
}

/**************���ü���********************************/
void Set_Motor_Dec(void)
{
     Motor_Run_State = MOTOR_RUN_DEC;
}

//���Դ���
void Key_Test(void)
{
    if(Bit_RESET == GPIO_ReadInputDataBit(DEVICE_ADDR1_PORT,DEVICE_ADDR1_PIN))//�͵�ƽ����1
    {
        if(MOTOR_IDLE == Motor_Run_State)//ͣ������
        {
            if(Bit_RESET == GPIO_ReadInputDataBit(DEVICE_ADDR64_PORT,DEVICE_ADDR64_PIN))//�͵�ƽ����6
            {
                //bldc_dev.motor_direction = CW;//��ת
                gs_Motor_Param.sMotorParam_Set.byRunDir = CW;//��ת
            }
            else
            {
                //bldc_dev.motor_direction = CCW;//��ת
                gs_Motor_Param.sMotorParam_Set.byRunDir = CCW;//��ת
            }
            Set_Motor_Start();
        }
    }
    else
    {
        if((MOTOR_RUN_ACC == Motor_Run_State) || (MOTOR_RUN == Motor_Run_State))//���ٻ�������
        {
            Set_Motor_Dec();//���ü���ֹͣ
        }
    }
}

//������л�ͣ��  ��ʱ6�����ŵĵ�ƽ�Ƕ����أ��ر���N���ţ�
void Motor_PWM_IDLE(void)
{
    TIM_CCxCmd(BLDC_TIMx,TIM_Channel_1,TIM_CCx_Disable);
    TIM_CCxNCmd(BLDC_TIMx,TIM_Channel_1,TIM_CCxN_Disable);
    TIM_CCxCmd(BLDC_TIMx,TIM_Channel_2,TIM_CCx_Disable);
    TIM_CCxNCmd(BLDC_TIMx,TIM_Channel_2,TIM_CCxN_Disable);
    TIM_CCxCmd(BLDC_TIMx,TIM_Channel_3,TIM_CCx_Disable);
    TIM_CCxNCmd(BLDC_TIMx,TIM_Channel_3,TIM_CCxN_Disable);
    TIM_ForcedOC1Config(BLDC_TIMx,TIM_ForcedAction_InActive);        //        ǿ�Ƶ�
    TIM_ForcedOC2Config(BLDC_TIMx,TIM_ForcedAction_InActive);
    TIM_ForcedOC3Config(BLDC_TIMx,TIM_ForcedAction_InActive);
    TIM_OC1NPolarityConfig(BLDC_TIMx,TIM_OCNPolarity_High);          ////ͣ��ʱ��������������Ǹߵ�ƽ��Ч  Ҳ���� CC1NP = 0;    TIM_OCNPolarity_High=0x0
    TIM_OC2NPolarityConfig(BLDC_TIMx,TIM_OCNPolarity_High);         //// ���ն��壺OCxN=CCxNP��OCxN_EN=0  ��ʱ�����N�����Ϊ�͵�ƽ�����͵�ƽ���������¹ܵ�ͨ
    TIM_OC3NPolarityConfig(BLDC_TIMx,TIM_OCNPolarity_High);

}

///׼������ʱ���������6�����ŵĵ�ƽ�Ƕ����أ� N��Ϊ�͵�ƽ��Ч������ζ��CCNP=1  Ҳ����˵��CNT<CC1ʱ������REF����Ч��ƽ  ��OCxREF������Ϊ��Ч��ƽ����ôCNT<CC1ʱ �������Ϊ�ߵ�ƽ
////                                                                           ע�⵽N����� ==== OCxREF���� + ���� + ����

//׼������     ��ʼ��ʼ��ʱ�и���ʱ���Ƕ��м���ģʽ  ��������PWMģʽ1������ì�ܣ����һ����ô����
void Motor_PWM_READY(void)
{
    TIM_OC1NPolarityConfig(BLDC_TIMx,TIM_OCNPolarity_Low);     ///��ʱ��Nͨ���������Ŷ��ǵ͵�ƽ��Ч�� ��Nͨ��������������   ȱʡ���Ǹߵ�ƽ��Ч��
    TIM_OC2NPolarityConfig(BLDC_TIMx,TIM_OCNPolarity_Low);
    TIM_OC3NPolarityConfig(BLDC_TIMx,TIM_OCNPolarity_Low);
    TIM_SelectOCxM(BLDC_TIMx,TIM_Channel_1,TIM_OCMode_PWM1);    ///��PWMģʽ1�� �����ϼ���ʱ��һ��TIMx_CNT<TIMx_CCR1ʱͨ��1Ϊ��Ч��ƽ������Ϊ��Ч��ƽ��
                                                                ////         �����¼���ʱ��һ��TIMx_CNT>TIMx_CCR1ʱͨ��1Ϊ��Ч��ƽ(OC1REF=0)������Ϊ��Ч��ƽ(OC1REF=1)��
    TIM_SelectOCxM(BLDC_TIMx,TIM_Channel_2,TIM_OCMode_PWM1);
    TIM_SelectOCxM(BLDC_TIMx,TIM_Channel_3,TIM_OCMode_PWM1);
    TIM_CCxCmd(BLDC_TIMx,TIM_Channel_1,TIM_CCx_Disable);
    TIM_CCxNCmd(BLDC_TIMx,TIM_Channel_1,TIM_CCxN_Disable);
    TIM_CCxCmd(BLDC_TIMx,TIM_Channel_2,TIM_CCx_Disable);
    TIM_CCxNCmd(BLDC_TIMx,TIM_Channel_2,TIM_CCxN_Disable);
    TIM_CCxCmd(BLDC_TIMx,TIM_Channel_3,TIM_CCx_Disable);
    TIM_CCxNCmd(BLDC_TIMx,TIM_Channel_3,TIM_CCxN_Disable);
    TIM_OC1PreloadConfig(BLDC_TIMx,TIM_OCPreload_Enable);     ////Ԥװ���������        ��ֹΪ0������ʱд��TIMx_CCR1�Ĵ�����������д�����ֵ���������á�
    TIM_OC2PreloadConfig(BLDC_TIMx,TIM_OCPreload_Enable);  ///  ����Ϊ1������TIMx_CCR1�Ĵ�����Ԥװ�ع��ܣ���д��������Ԥװ�ؼĴ���������TIMx_CCR1��Ԥװ��ֵ�ڸ����¼�����ʱ����������ǰ�Ĵ�����
    TIM_OC3PreloadConfig(BLDC_TIMx,TIM_OCPreload_Enable);

#if(MOTOR_BRAKE_ENABLE == 0)//�ر�ɲ������    
    TIM_ClearITPendingBit(BLDC_TIMx,TIM_IT_Break);
    TIM_ITConfig(BLDC_TIMx,TIM_IT_Break,DISABLE);
#else//����ɲ������
    TIM_ClearITPendingBit(BLDC_TIMx,TIM_IT_Break);
    TIM_ITConfig(BLDC_TIMx,TIM_IT_Break,ENABLE);
#endif
    /* TIM�����ʹ�� */
    TIM_CtrlPWMOutputs(BLDC_TIMx, ENABLE);       ////  �����MOEλ=1    ����������ˣ�û������ع�0 һֱ����1��
}

//����������
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
        
        gs_Motor_Param.dwMotor_Current = (dwCurrent >> 2);   ////�õ����ĵ���ֵ
        
        if(gs_Motor_Param.dwMotor_Current  > LIMIT_CURRENT_L)
        {
            gs_Motor_Param.dwOverLoadCnt_L++;                    ///   ����L�ƴ�ֵ++
            gs_Motor_Param.byCurrentDownL_Flag = 0;              ///   ����L��־��0
            
            if(gs_Motor_Param.dwMotor_Current  > LIMIT_CURRENT_H)
            {
                gs_Motor_Param.dwOverLoadCnt_H++;                 ///����H�ƴ�ֵ++ 
                gs_Motor_Param.byCurrentDownH_Flag = 0;            ///����H��־��0
            }
            else
            {
                 gs_Motor_Param.byCurrentDownH_Flag++;
                if(gs_Motor_Param.byCurrentDownH_Flag >= 3)//����3�ε��ڸߵ�����������ߵ����ƴμ�������0
                {
                    gs_Motor_Param.dwOverLoadCnt_H = 0;
                }           
            }
        }
        else        ////����С�ڵ͵���
        {
            gs_Motor_Param.byCurrentDownH_Flag++;
            gs_Motor_Param.byCurrentDownL_Flag++;
            if(gs_Motor_Param.byCurrentDownL_Flag >= 3)//����3�ε�
            {
                gs_Motor_Param.dwOverLoadCnt_L = 0;
            }
            if(gs_Motor_Param.byCurrentDownH_Flag >= 3)//����3�ε�
            {
                gs_Motor_Param.dwOverLoadCnt_H = 0;
            }
            //gs_Motor_Param.dwOverLoadCnt_H = 0;
            //gs_Motor_Param.dwOverLoadCnt_L = 0;
        }
        
        if((gs_Motor_Param.dwOverLoadCnt_L >= OVERLOAD_TIMER_L)\
           ||(gs_Motor_Param.dwOverLoadCnt_H >= OVERLOAD_TIMER_H))//��������
        {
            bldc_dev.motor_state = STOP;
            Set_MotorErr(M_RUN_FAULT);//���õ������ʧ�ܹ���
            Set_MotorErr(M_OVERLOAD);//���õ����������
            Set_Motor_Stop();//����ͣ��                     Motor_Run_State = MOTOR_STOP;�Ϳ���ͣ����
            gs_Motor_Param.dwOverLoadCnt_L = 0;
            gs_Motor_Param.dwOverLoadCnt_H = 0;
#if(FORCE_MODE_ENABLE == 1)//�������¹ܵ�ͨɲ��ģʽ
            Motor_PWM_IDLE();//ͣ��
#else
            TIM_CCxCmd(BLDC_TIMx,TIM_Channel_1,TIM_CCx_Disable);      ///��ɲ���Ļ�����ֱ�ӽ�6����ʱ��������������ˡ�  ��MOE=1������£���ʱOCx=CCxP=0  OCxN=CCxNP=1 
            TIM_CCxNCmd(BLDC_TIMx,TIM_Channel_1,TIM_CCxN_Disable);
            TIM_CCxCmd(BLDC_TIMx,TIM_Channel_2,TIM_CCx_Disable);
            TIM_CCxNCmd(BLDC_TIMx,TIM_Channel_2,TIM_CCxN_Disable);
            TIM_CCxCmd(BLDC_TIMx,TIM_Channel_3,TIM_CCx_Disable);
            TIM_CCxNCmd(BLDC_TIMx,TIM_Channel_3,TIM_CCxN_Disable);
#endif
        }
    }
    
}