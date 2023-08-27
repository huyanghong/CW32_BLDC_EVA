//系统时钟配置为64M,BTIMER+GTIMER 

/*
六路PWM: ATIM
U+ PA8
V+:PA9
W+:PA10

U-:PB13
V-:PB14
W-:PB15

霍尔：GTIM2
HA:PA15
HB:PB3
HC:PA2

电位器AD：
PB0
可通过电位器调速

六路PWM,上管调制，下管IO，上管高电平，下管低电平驱动。
下载运行：旋转电位器，对电机进行调速。
上电，PC13指示灯常亮。旋转电们器 电机运转后PC13灯闪烁
*/

#include "main.h"
#include "motorcontrol.h"
#include "OLED_I2C.h"

#define MotorPoles 2 //电机极对数


void GPIO_Configuration(void);
void RCC_Configuration(void);
void ATIIM_init(void);
void GTIM_init(void);
void BTIM_init(void);
void ADC_Configuration(void);

unsigned int RealS=0;
unsigned int HALLcount=0;
uint16_t ADC_Result_Array;
unsigned int timecount=0,timecount1=0;
char temp_buff[100],temp_buff1[50];
char PWMFLA=0;
	
	unsigned int Menu=0; //0: speed 1:V  Press KEY1 to change	
	
	
int main()
{	
  unsigned int test;
	float t;
	
	RCC_Configuration(); //64M时钟配置
	GPIO_Configuration(); //LED&KEYS
	I2C_init();			//OLED init
	I2C_OLED_Init();
	ADC_Configuration(); //AD PONITER
	BTIM_init();  //10MS USRER TIMER
	ATIIM_init(); //PWM TIMER
	GTIM_init();  //HALL TIMER
  I2C_OLED_Clear(1);
		
	sprintf(temp_buff1,"PWM:%d %%  STOP...",PWMFLA);  //输出显示占空比
	sprintf(temp_buff,"Real:%d rpm   ",RealS);	//显示电机转速
	I2C_OLED_ShowString(0,15,temp_buff1);
	I2C_OLED_ShowString(0,0,temp_buff);			
	I2C_OLED_UPdata(); 
	
	
	GPIO_WritePin(CW_GPIOC,GPIO_PIN_13,GPIO_Pin_RESET);  //LED ON
	
	while(1)
	{
		if (CW_DMA->ISR_f.TC1)
		{ //AD DMA 启动
			CW_DMA->ICR_f.TC1 = 0;	
      CW_DMACHANNEL1->CNT=bv16|60000;	 //MUST RET AGAIN BEFORE CW_DMACHANNEL1->CNT=0
			CW_DMACHANNEL1->CSR_f.EN = 1; 		
		}
		
		if(timecount>=10)
		{
			timecount=0;//100ms
				
			//电位器采集的AD代码值在3-4000范围内有效 3对应最大输出，4000对应最小输出			
				if(ADC_Result_Array>=4000)	test=0; 
				else if(ADC_Result_Array<3)	test=4000; 
				else 	test=4000-ADC_Result_Array;
			
				t=test*0.8;///40*32;
				test=t;	//计算当前 应当输出的值
			
			
			   if(OutPwm<test)
					{
						OutPwm+=TS*0.05; //缓启动
						if(OutPwm>=test)OutPwm=test;
					}
				 else OutPwm=test;
				
				if(Motor_Start_F==1)//电机启动后，及时更新PWM
				 Commutation(HALL_Check(),OutPwm,Motor_Start_F);//此调用主要及时更新输出占空比
				 						
			
				if(test<OUTMINPWM) //小于最小输出时，电机停止运行
				{	  
					 if(Motor_Start_F==1)
					 {
						Motor_Start_F=0;		//电机停止状态
						HALL_MOTOR_START(); //停止电机
						GPIO_WritePin(CW_GPIOC,GPIO_PIN_13,GPIO_Pin_RESET);  //LED ON 上电指示灯常亮
					 }					 
				}
				else if(test>OUTMINPWM+50&&Motor_Start_F==0) //大于最小输出时，开始转
				{			 			
						Motor_Start_F=1;				//改变启停状态
					  OutPwm=OUTMINPWM;
						HALL_MOTOR_START();		 //启动电机
				}		
		}

		
			 if(timecount1>=50)    //1秒计算一次速度   MotorPoles--极对数  HALLcount--脉冲个数  RealS：实测转速
			 {
			    timecount1=0; //500MS
				 //1秒计算速度值并显示
					RealS=HALLcount*20/MotorPoles;	//计算实测电机RPM		
					HALLcount=0;			
					sprintf(temp_buff,"Real:%d rpm   ",RealS);	//显示电机转速
					I2C_OLED_ShowString(0,0,temp_buff);		

					PWMFLA=OutPwm/32; //显示输出占空比
					if(Motor_Start_F==1)
					{	
						sprintf(temp_buff1,"PWM:%d %%  START  ",PWMFLA);  //输出显示占空比
					}
					else
					{	
						sprintf(temp_buff1,"PWM:%d %%  STOP...",PWMFLA);  //输出显示占空比
					}
						
					I2C_OLED_ShowString(0,15,temp_buff1);
					I2C_OLED_UPdata();
			 }	
	} // while(1)			 
}

void GTIM2_IRQHandler(void)
{	
	unsigned int x;  
	
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
	
   HALLcount++;//霍尔脉冲计数	
	 x=HALL_Check(); //读取霍尔状态	
	 if(Motor_Start_F==1&&ErrorCode==0) //根据启停状态 换相
			Commutation(x,OutPwm,Motor_Start_F);   	
  /* USER CODE END */
}

void GTIM_init(void)
{
	GTIM_InitTypeDef GTIM_InitStruct;
  GTIM_ICInitTypeDef GTIM_ICInitStruct;
	 GPIO_InitTypeDef GPIO_InitStruct;

  __RCC_GTIM2_CLK_ENABLE();	
	__RCC_GPIOA_CLK_ENABLE();
	__RCC_GPIOB_CLK_ENABLE();
	
  GPIO_InitStruct.IT = GPIO_IT_NONE;
  GPIO_InitStruct.Mode =GPIO_MODE_INPUT_PULLUP;// 霍尔输入配置;
  GPIO_InitStruct.Pins = GPIO_PIN_15|GPIO_PIN_2;
  GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;
  GPIO_Init(CW_GPIOA, &GPIO_InitStruct);

	GPIO_InitStruct.IT = GPIO_IT_NONE;
  GPIO_InitStruct.Mode =GPIO_MODE_INPUT_PULLUP;// 霍尔输入配置
  GPIO_InitStruct.Pins = GPIO_PIN_3;
  GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;
  GPIO_Init(CW_GPIOB, &GPIO_InitStruct);

	PA15_AFx_GTIM2CH1();//  GTIM2CH1();
	PB03_AFx_GTIM2CH2();//  GTIM2CH2();
  PA02_AFx_GTIM2CH3();//	GTIM2CH3();	
	
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

void ADC_Configuration(void)
{
	 DMA_InitTypeDef   DMA_InitStruct;
	 ADC_InitTypeDef   ADC_InitStruct;	 
	
    RCC_AHBPeriphClk_Enable(RCC_AHB_PERIPH_DMA | RCC_AHB_PERIPH_GPIOB, ENABLE);  //Open DMA and GPIOA Clk
    RCC_APBPeriphClk_Enable2(RCC_APB2_PERIPH_ADC, ENABLE);    //Open ADC Clk

    //配置ADC测试IO口  电位器接口
	  PB00_ANALOG_ENABLE();    //PB00 (AIN8)
	
    //ADC初始化
    ADC_InitStruct.ADC_OpMode = ADC_SingleChContinuousMode; 
    ADC_InitStruct.ADC_ClkDiv = ADC_Clk_Div8; //PCLK
    ADC_InitStruct.ADC_SampleTime = ADC_SampTime10Clk; //5个ADC时钟周期
    ADC_InitStruct.ADC_VrefSel = ADC_Vref_VDDA; //VDDA参考电压
    ADC_InitStruct.ADC_InBufEn = ADC_BufDisable; //开启跟随器
    ADC_InitStruct.ADC_TsEn = ADC_TsDisable; //内置温度传感器禁用
    ADC_InitStruct.ADC_DMAEn = ADC_DmaEnable; //ADC转换完成触发DMA传输
    ADC_InitStruct.ADC_Align = ADC_AlignRight; //ADC转换结果右对齐
    ADC_InitStruct.ADC_AccEn = ADC_AccDisable; //转换结果累加不使能
    ADC_Init(&ADC_InitStruct);                 //初始化ADC配置
    CW_ADC->CR1_f.DISCARD = FALSE;             //配置数据更新策略，不包含在ADC结构体中               
    CW_ADC->CR1_f.CHMUX = ADC_ExInputCH8;      //配置ADC输入通道，不包含在ADC结构体中
    
    DMA_StructInit( &DMA_InitStruct );
    DMA_InitStruct.DMA_Mode = DMA_MODE_BLOCK;
    DMA_InitStruct.DMA_TransferWidth = DMA_TRANSFER_WIDTH_32BIT;
    DMA_InitStruct.DMA_SrcInc = DMA_SrcAddress_Fix;
    DMA_InitStruct.DMA_DstInc = DMA_DstAddress_Fix;
    DMA_InitStruct.DMA_TransferCnt =60000;
    DMA_InitStruct.DMA_SrcAddress = (uint32_t) &(CW_ADC->RESULT0);
    DMA_InitStruct.DMA_DstAddress = (uint32_t)&ADC_Result_Array;
    DMA_InitStruct.TrigMode = DMA_HardTrig;
    DMA_InitStruct.HardTrigSource = DMA_HardTrig_ADC_TRANSCOMPLETE;
    DMA_Init(CW_DMACHANNEL1,&DMA_InitStruct);
    DMA_ClearITPendingBit(DMA_IT_ALL);
    DMA_ITConfig(CW_DMACHANNEL1, DMA_IT_TC|DMA_IT_TE , ENABLE);  //使能DMA_CHANNEL1中断
    DMA_Cmd(CW_DMACHANNEL1, ENABLE);  //使能DMA

    //ADC使能
    ADC_Enable();
    ADC_SoftwareStartConvCmd(ENABLE);
}

void RCC_Configuration(void)
{
  //...........................64M
  // 0. HSI使能并校准 
  RCC_HSI_Enable(RCC_HSIOSC_DIV6);

  // 1. 设置HCLK和PCLK的分频系数　
  RCC_HCLKPRS_Config(RCC_HCLK_DIV1);
  RCC_PCLKPRS_Config(RCC_PCLK_DIV1);
  
  // 2. 使能PLL，通过PLL倍频到64MHz 
  RCC_PLL_Enable(RCC_PLLSOURCE_HSI, 8000000, 8);     // HSI 默认输出频率8MHz

  ///< 当使用的时钟源HCLK大于24M,小于等于48MHz：设置FLASH 读等待周期为2 cycle
  ///< 当使用的时钟源HCLK大于48MHz：设置FLASH 读等待周期为3 cycle
  __RCC_FLASH_CLK_ENABLE();
  FLASH_SetLatency(FLASH_Latency_3);       
   
  // 3. 时钟切换到PLL
  RCC_SysClk_Switch(RCC_SYSCLKSRC_PLL);
  RCC_SystemCoreClockUpdate(64000000);	
	//..............................
}


void GPIO_Configuration(void)
{
  GPIO_InitTypeDef GPIO_InitStruct;
	
	__RCC_GPIOA_CLK_ENABLE();
	__RCC_GPIOB_CLK_ENABLE();
	__RCC_GPIOC_CLK_ENABLE();

  GPIO_InitStruct.IT = GPIO_IT_NONE;  //KEY1 KEY2
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT_PULLUP;
  GPIO_InitStruct.Pins = GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_10;
  GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;
  GPIO_Init(CW_GPIOB, &GPIO_InitStruct);

  GPIO_InitStruct.IT = GPIO_IT_NONE; //LED1  LED2
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pins = GPIO_PIN_11;
  GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;
  GPIO_Init(CW_GPIOB, &GPIO_InitStruct);
	
	GPIO_InitStruct.Pins = GPIO_PIN_13;
  GPIO_Init(CW_GPIOC, &GPIO_InitStruct);
	
	GPIO_InitStruct.Pins = GPIO_PIN_11;
  GPIO_Init(CW_GPIOA, &GPIO_InitStruct);

	GPIO_WritePin(CW_GPIOA,GPIO_PIN_11,GPIO_Pin_SET);  //LED OFF
	GPIO_WritePin(CW_GPIOC,GPIO_PIN_13,GPIO_Pin_SET);  //LED OFF
}

void ATIIM_init(void)
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
	
  GPIO_InitStruct.IT = GPIO_IT_NONE;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pins =  GPIO_PIN_8|GPIO_PIN_9|GPIO_PIN_10;
  GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;
  GPIO_Init(CW_GPIOA, &GPIO_InitStruct);   
	
	PWM_AL_OFF;
	PWM_BL_OFF;
	PWM_CL_OFF;
	
	ATIM_InitStruct.BufferState = DISABLE;
  ATIM_InitStruct.ClockSelect = ATIM_CLOCK_PCLK;
  ATIM_InitStruct.CounterAlignedMode = ATIM_COUNT_MODE_EDGE_ALIGN;
  ATIM_InitStruct.CounterDirection = ATIM_COUNTING_UP;
  ATIM_InitStruct.CounterOPMode = ATIM_OP_MODE_REPETITIVE;
  ATIM_InitStruct.OverFlowMask = DISABLE;
  ATIM_InitStruct.Prescaler = ATIM_Prescaler_DIV1;    // 计算时钟1MHz
  ATIM_InitStruct.ReloadValue = TS;    // 20K 
  ATIM_InitStruct.RepetitionCounter = 0;
  ATIM_InitStruct.UnderFlowMask = DISABLE;
  ATIM_Init(&ATIM_InitStruct);

  ATIM_OCInitStruct.BufferState = DISABLE;
  ATIM_OCInitStruct.OCDMAState = DISABLE;
  ATIM_OCInitStruct.OCInterruptSelect = ATIM_OC_IT_UP_COUNTER;
  ATIM_OCInitStruct.OCInterruptState = ENABLE;
  ATIM_OCInitStruct.OCMode = ATIM_OCMODE_PWM1;
  ATIM_OCInitStruct.OCPolarity = ATIM_OCPOLARITY_NONINVERT;
  ATIM_OC1AInit(&ATIM_OCInitStruct);
	ATIM_OC2AInit(&ATIM_OCInitStruct);
	ATIM_OC3AInit(&ATIM_OCInitStruct);

  ATIM_SetCompare1A(0);
  ATIM_SetCompare2A(0);
  ATIM_SetCompare3A(0);
  ATIM_PWMOutputConfig(OCREFA_TYPE_SINGLE, OUTPUT_TYPE_COMP, 0);
  ATIM_CtrlPWMOutputs(ENABLE);
  ATIM_Cmd(ENABLE);
}

void ATIM_IRQHandler(void)
{
  if (ATIM_GetITStatus(ATIM_IT_OVF))
  {
    ATIM_ClearITPendingBit(ATIM_IT_OVF);		
	}
}


void BTIM_init(void)
{//10MS TIMER
	BTIM_TimeBaseInitTypeDef BTIM_InitStruct;
	
	__RCC_BTIM_CLK_ENABLE();
	
	__disable_irq(); 
  NVIC_EnableIRQ(BTIM1_IRQn); 
  __enable_irq();
	
	BTIM_InitStruct.BTIM_Mode = BTIM_Mode_TIMER;
  BTIM_InitStruct.BTIM_OPMode = BTIM_OPMode_Repetitive;
  BTIM_InitStruct.BTIM_Period = 40000;
  BTIM_InitStruct.BTIM_Prescaler = BTIM_PRS_DIV16;
  BTIM_TimeBaseInit(CW_BTIM1, &BTIM_InitStruct);
	
  BTIM_ITConfig(CW_BTIM1, BTIM_IT_OV, ENABLE);
  BTIM_Cmd(CW_BTIM1, ENABLE);
}

/**
 * @brief This funcation handles BTIM1
 */
void BTIM1_IRQHandler(void)
{
  /* USER CODE BEGIN */
 static unsigned int count2=0;
 if(BTIM_GetITStatus(CW_BTIM1, BTIM_IT_OV))
  {  //10MS
    BTIM_ClearITPendingBit(CW_BTIM1, BTIM_IT_OV);
		timecount++;
		timecount1++;
		
		if(Motor_Start_F==1) // 电机启动状态时，灯闪
		{count2++;
			if(count2>=50)
			{
					count2=0; 
					PC13_TOG();//PC13
			} 
	 }
 }
  /* USER CODE END */
}




