#include "motor.h"
#include "globalv.h"

void PWMtimer_init(void)
{
	ATIM_InitTypeDef ATIM_InitStruct;
  ATIM_OCInitTypeDef ATIM_OCInitStruct;
	GPIO_InitTypeDef GPIO_InitStruct;
	
	__RCC_ATIM_CLK_ENABLE();
	__RCC_GPIOA_CLK_ENABLE();
	__RCC_GPIOB_CLK_ENABLE();
	
  PA08_AFx_ATIMCH1A();
  PA09_AFx_ATIMCH2A();
  PA10_AFx_ATIMCH3A();	
		
  GPIO_InitStruct.IT = GPIO_IT_NONE;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pins =  GPIO_PIN_13|GPIO_PIN_14|GPIO_PIN_15;
  GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;
  GPIO_Init(CW_GPIOB, &GPIO_InitStruct); 
	
	PWM_AH_OFF;
	PWM_BH_OFF;
	PWM_CH_OFF;
	
  GPIO_InitStruct.IT = GPIO_IT_NONE;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pins =  GPIO_PIN_8|GPIO_PIN_9|GPIO_PIN_10;
  GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;
  GPIO_Init(CW_GPIOA, &GPIO_InitStruct);   
	
	PWM_AL_OFF;
	PWM_BL_OFF;
	PWM_CL_OFF;
	
	ATIM_InitStruct.BufferState = ENABLE;//;//;
  ATIM_InitStruct.ClockSelect = ATIM_CLOCK_PCLK;
  ATIM_InitStruct.CounterAlignedMode = ATIM_COUNT_MODE_EDGE_ALIGN;
  ATIM_InitStruct.CounterDirection = ATIM_COUNTING_UP;
  ATIM_InitStruct.CounterOPMode = ATIM_OP_MODE_REPETITIVE;
  ATIM_InitStruct.OverFlowMask = ENABLE;
  ATIM_InitStruct.Prescaler = ATIM_Prescaler_DIV1;    // 计算时钟1MHz
  ATIM_InitStruct.ReloadValue = PWM_PERIOD;    // 溢出周期100us
  ATIM_InitStruct.RepetitionCounter = 0;
  ATIM_InitStruct.UnderFlowMask = DISABLE;
  ATIM_Init(&ATIM_InitStruct);

  ATIM_OCInitStruct.BufferState = DISABLE;
  ATIM_OCInitStruct.OCDMAState = DISABLE;
  ATIM_OCInitStruct.OCInterruptSelect = ATIM_OC_IT_NONE;
  ATIM_OCInitStruct.OCInterruptState = DISABLE;
  ATIM_OCInitStruct.OCMode = ATIM_OCMODE_PWM1;
  ATIM_OCInitStruct.OCPolarity = ATIM_OCPOLARITY_NONINVERT;
  ATIM_OC1AInit(&ATIM_OCInitStruct);
	ATIM_OC2AInit(&ATIM_OCInitStruct);
	ATIM_OC3AInit(&ATIM_OCInitStruct);

  ATIM_SetCompare1A(0);
  ATIM_SetCompare2A(0);
  ATIM_SetCompare3A(0);
  ATIM_PWMOutputConfig(OCREFA_TYPE_SINGLE, OUTPUT_TYPE_ALONE, -1);
  ATIM_CtrlPWMOutputs(ENABLE);
  ATIM_Cmd(ENABLE);
	
	CW_ATIM->MSCR_f.CCDS = 1;    // DMA 总开关
  ATIM_OC4Init(ENABLE, ATIM_OC_IT_UP_COUNTER, ENABLE, DISABLE, DISABLE);
  ATIM_SetCompare4(PWM_PERIOD/2);    // 触发时刻
  ATIM_Cmd(ENABLE); 
}

void ATIM_IRQHandler(void)
{
  if (ATIM_GetITStatus(ATIM_IT_OVF))
  {
    ATIM_ClearITPendingBit(ATIM_IT_OVF);		
	}
}

//steP,为当前步，PWM_ON_flag=1时打开PWM输出
void Commutation(unsigned int step,unsigned int PWM_ON_flag)
{	 	
	if(PWM_ON_flag==0||MOTORSTATE==STATEERROR||MOTORSTATE==STATEERROROVER)
               { 
								 	CW_ATIM->CH1CCRA=0;CW_ATIM->CH2CCRA=0;CW_ATIM->CH3CCRA=0;	
									PWM_AL_OFF;	PWM_BL_OFF;	PWM_CL_OFF;							 
									return;
							 }	 
	if(step==0||step==5){	PWM_AL_OFF;	PWM_CL_OFF;}//AB
  else if(step==1||step==2){	PWM_AL_OFF;	PWM_BL_OFF;	}//AC
	else if(step==3||step==4){	PWM_BL_OFF;	PWM_CL_OFF;	}//BA
	
	if(step==0||step==1){	 CW_ATIM->CH2CCRA=0;CW_ATIM->CH3CCRA=0;CW_ATIM->CH1CCRA=OutPwmValue;	}
	if(step==2||step==3){	 CW_ATIM->CH1CCRA=0;CW_ATIM->CH3CCRA=0;CW_ATIM->CH2CCRA=OutPwmValue;	}
	if(step==4||step==5){	 CW_ATIM->CH1CCRA=0;CW_ATIM->CH2CCRA=0;CW_ATIM->CH3CCRA=OutPwmValue;	}
	
	
	if(step==0||step==5){PWM_BL_ON;}//AB
  else if(step==1||step==2){PWM_CL_ON;}//AC
	else if(step==3||step==4){PWM_AL_ON;}//BA	
	STEP_last=step;
}

//更新占空比
void UPPWM(void)
{	
	if(STEP_last==0||STEP_last==1){	 CW_ATIM->CH2CCRA=0;CW_ATIM->CH3CCRA=0; CW_ATIM->CH1CCRA=OutPwmValue;	}
	if(STEP_last==2||STEP_last==3){	 CW_ATIM->CH1CCRA=0;CW_ATIM->CH3CCRA=0;CW_ATIM->CH2CCRA=OutPwmValue;	}
	if(STEP_last==4||STEP_last==5){	 CW_ATIM->CH1CCRA=0;CW_ATIM->CH2CCRA=0;CW_ATIM->CH3CCRA=OutPwmValue;	}
}

//霍尔定时器初始化
void Halltimer_init(void) 
{
	GTIM_InitTypeDef GTIM_InitStruct;
  GTIM_ICInitTypeDef GTIM_ICInitStruct;
	 GPIO_InitTypeDef GPIO_InitStruct;

  __RCC_GTIM2_CLK_ENABLE();	
	__RCC_GPIOA_CLK_ENABLE();
	__RCC_GPIOB_CLK_ENABLE();
	
  GPIO_InitStruct.IT = GPIO_IT_NONE;
  GPIO_InitStruct.Mode =GPIO_MODE_INPUT_PULLUP;// GPIO_MODE_INPUT;
  GPIO_InitStruct.Pins = GPIO_PIN_15 |GPIO_PIN_2;
  GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;
  GPIO_Init(CW_GPIOA, &GPIO_InitStruct);
	
	GPIO_InitStruct.IT = GPIO_IT_NONE;
  GPIO_InitStruct.Mode =GPIO_MODE_INPUT_PULLUP;// GPIO_MODE_INPUT;
  GPIO_InitStruct.Pins = GPIO_PIN_3;
  GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;
  GPIO_Init(CW_GPIOB, &GPIO_InitStruct);

  PA15_AFx_GTIM2CH1();
  PB03_AFx_GTIM2CH2();
  PA02_AFx_GTIM2CH3();	
	
  __disable_irq(); 
  NVIC_EnableIRQ(GTIM2_IRQn); 
  __enable_irq();
	
	GTIM_InitStruct.Mode = GTIM_MODE_TIME;
  GTIM_InitStruct.OneShotMode = GTIM_COUNT_CONTINUE;
  GTIM_InitStruct.Prescaler = GTIM_PRESCALER_DIV1;
  GTIM_InitStruct.ReloadValue = 0xFFFF;
  GTIM_InitStruct.ToggleOutState = DISABLE;
  GTIM_TimeBaseInit(CW_GTIM2, &GTIM_InitStruct);

  GTIM_ICInitStruct.CHx = GTIM_CHANNEL1;
  GTIM_ICInitStruct.ICFilter = GTIM_CHx_FILTER_PCLK_N2;
  GTIM_ICInitStruct.ICInvert = GTIM_CHx_INVERT_OFF;
  GTIM_ICInitStruct.ICPolarity = GTIM_ICPolarity_BothEdge;
  GTIM_ICInit(CW_GTIM2, &GTIM_ICInitStruct);

  GTIM_ICInitStruct.CHx = GTIM_CHANNEL2;
  GTIM_ICInit(CW_GTIM2, &GTIM_ICInitStruct);
	
  GTIM_ICInitStruct.CHx = GTIM_CHANNEL3;
  GTIM_ICInit(CW_GTIM2, &GTIM_ICInitStruct);
	
  GTIM_ITConfig(CW_GTIM2, GTIM_IT_CC1 | GTIM_IT_CC2 | GTIM_IT_CC3, ENABLE);
  GTIM_Cmd(CW_GTIM2, ENABLE);
}

//霍尔信号读取
unsigned char  HALL_Check(void)
{
	 static unsigned char hallerrnum=0;
	 unsigned char x=0;
	 
	 if(GPIO_ReadPin(CW_GPIOA,GPIO_PIN_15)!=0)x=1;
	 if(GPIO_ReadPin(CW_GPIOB,GPIO_PIN_3)!=0)x|=0x2;
	 if(GPIO_ReadPin(CW_GPIOA,GPIO_PIN_2)!=0)x|=0x4;
	 if(x==0||x==7)
	   {hallerrnum++;if(hallerrnum>=10){hallerrnum=10;ErrorCode=2;}}
   else hallerrnum=0;
	 return x;
}

//霍尔信号变换，换相
void GTIM2_IRQHandler(void)
{
  unsigned char x = 0;	
	
  /* USER CODE BEGIN */
   if (GTIM_GetITStatus(CW_GTIM2, GTIM_IT_CC1))
  {
    GTIM_ClearITPendingBit(CW_GTIM2, GTIM_IT_CC1);
  }

  if (GTIM_GetITStatus(CW_GTIM2, GTIM_IT_CC2))
  {    
    GTIM_ClearITPendingBit(CW_GTIM2, GTIM_IT_CC2);
  }
  
	if (GTIM_GetITStatus(CW_GTIM2, GTIM_IT_CC3))
  {    
    GTIM_ClearITPendingBit(CW_GTIM2, GTIM_IT_CC3);
  }
	
	  x=HALL_Check();	
    if(x==0||x==7){return;}
				
		HALLcount++;HALLcount1++;	
		Cur_step=STEP_TAB[Dir][x-1];	
		if(Motor_Start_F==1&&ErrorCode==0)
 		Commutation(Cur_step,Motor_Start_F); 
}

//电机启动
void HALL_MOTOR_START(void) 
{ 	
	unsigned char x=0;
				
		 x=HALL_Check();	 
		 if(x==0||x==7){return;}	 
		 Cur_step=STEP_TAB[Dir][x-1];
		 RealS1=0; 
		 Motor_Start_F=1;
		 Commutation(Cur_step,Motor_Start_F);   
}

//电机停止
void MOTOR_STOP0(void)
{
	 Motor_Start_F=0;
	 Commutation(0,0);
}


