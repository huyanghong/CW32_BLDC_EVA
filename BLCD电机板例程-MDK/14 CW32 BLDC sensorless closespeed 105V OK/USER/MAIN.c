//系统时钟配置为64M,BTIMER+GTIMER 

/*
六路PWM: ATIM
U+ PA8
V+:PA9
W+:PA10

U-:PB13
V-:PB14
W-:PB15

无霍尔：
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
#include "pid.h"

unsigned int MotorPoles=2; //电机极对数

void GPIO_Configuration(void);
void RCC_Configuration(void);
void ATIIM_init(void);
void BTIM_init(void);
void ADC_Configuration(void);

char temp_buff[100],temp_buff1[50];
char PWMFLA=0;	
	
int main()
{	
  unsigned int test;
	
	RCC_Configuration(); //64M时钟配置
	GPIO_Configuration(); //LED&KEYS初始化
	
	I2C_init();			//OLED init
	I2C_OLED_Init();
	
	ADC_Configuration(); //AD配置
	BTIM_init();  //1MS定时器初始化
	ATIIM_init(); //六路PWM定时器初始化	
	SENSORLESS_TIM_Config();	 //无感用定时器初始化
	
  I2C_OLED_Clear(1);		
	sprintf(temp_buff1,"Targ:%d rpm ",TargetS);  //输出显示占空比
	sprintf(temp_buff,"Real:%d rpm   ",RealS);	//显示电机转速
	I2C_OLED_ShowString(0,15,temp_buff1);
	I2C_OLED_ShowString(0,0,temp_buff);			
	I2C_OLED_UPdata(); 
		
	GPIO_WritePin(CW_GPIOC,GPIO_PIN_13,GPIO_Pin_RESET);  //LED ON
	
	while(1)
	{
		if(timecount>=50)
		{
			timecount=0;//100ms
				
			//电位器采集的AD代码值在3-4000范围内有效 3对应最大输出，4000对应最小输出			
				if(SampleData[5]>=4000)	test=0; 
				else if(SampleData[5]<3)	test=4000; 
				else 	test=4000-SampleData[5];
						
			  TargetS=test; //速度范围为700-4000，不用运算，直接赋值。
			
				if(TargetS<MinSpeed) //小于最小输出速度时，电机停止运行
				{	  
					 if(Motor_Start_F==1)
					 {
						Motor_Start_F=0;		//电机停止状态
						MOTOR_STOP(); //停止电机
						GPIO_WritePin(CW_GPIOC,GPIO_PIN_13,GPIO_Pin_RESET);  //LED ON 上电指示灯常亮
					 }					 
				}
				else if(TargetS>MinSpeed+50&&Motor_Start_F==0&&ErrorCode==0) //大于最小输出时，开始转
				{			 			
						Motor_Start_F=1;				//改变启停状态
						Sensorless_MOTOR_START();		 //启动电机
					 //TargetS=MinSpeed;
					  if(ErrorCode==0)
							PID_init(); //启动成功后
						HALLcount=0;timecountpid=0;
				}		
		}
				//因为屏幕操作运行的影响，为了及时进行PID进行，将PID运算执行时间为设10MS，放在BTIM定时器中断中完成。

		if(timecount1>=500)    //1秒计算一次速度   MotorPoles--极对数  HALLcount--脉冲个数  RealS：实测转速
			 {
			    timecount1=0;
				 //1秒计算速度值并显示
					RealS=HALLcountS*20/MotorPoles;	//计算实测电机RPM		
					HALLcountS=0;			
					sprintf(temp_buff,"Real:%d rpm       ",RealS);	//显示电机转速 RealS
					I2C_OLED_ShowString(0,0,temp_buff);							  	 	
					sprintf(temp_buff1,"Targ:%d rpm      ",TargetS);   			
				  I2C_OLED_ShowString(0,15,temp_buff1);//显示输出占空比	
		  	  I2C_OLED_UPdata();
			 }	
	} // while(1)			 
}

void ADC_Configuration(void)
{
	   ADC_InitTypeDef   ADC_InitStruct;	 
		 DMA_InitTypeDef DMA_InitStruct = {0};
	
    __RCC_GPIOA_CLK_ENABLE();
    __RCC_GPIOB_CLK_ENABLE();
    __RCC_ADC_CLK_ENABLE();
		

    //配置ADC测试IO口 
  
    PA00_ANALOG_ENABLE() ;      //PB00 (AIN0)
    PA01_ANALOG_ENABLE() ;      //PB00 (AIN1)
    PA03_ANALOG_ENABLE() ;      //PB00 (AIN3)
    PA05_ANALOG_ENABLE() ;      //PB00 (AIN5)
    PA06_ANALOG_ENABLE() ;      //PB00 (AIN6)  
		PB00_ANALOG_ENABLE() ;      //PB00 (AIN8)
   	
	
    ADC_InitStruct.ADC_AccEn = ADC_AccDisable;
    ADC_InitStruct.ADC_Align = ADC_AlignRight;
    ADC_InitStruct.ADC_ClkDiv = ADC_Clk_Div8; // ADCCLK=16MHz
    ADC_InitStruct.ADC_DMAEn = ADC_DmaEnable;
    ADC_InitStruct.ADC_InBufEn = ADC_BufDisable;
    ADC_InitStruct.ADC_OpMode = ADC_SingleChOneMode;
    ADC_InitStruct.ADC_SampleTime = ADC_SampTime10Clk;
    ADC_InitStruct.ADC_TsEn = ADC_TsDisable;
    ADC_InitStruct.ADC_VrefSel = ADC_Vref_VDDA;

    ADC_Init(&ADC_InitStruct);

    CW_ADC->CR1_f.CHMUX = 0; // AN0
    ADC_Enable();

 // 使用4路DMA通道：CH1、CH2、CH3、 CH4
    // CH1 将ADC单次单通道的采样结果传入RAM（ADC_ResultBuff[6]）
    // CH2 将ADC的CR1寄存器的配置值从RAM（ADC_CR1Array）传入寄存器
    // CH3 将ADC的START寄存器的配置值从RAM（ADC_Start)传入寄存器
    // CH1、CH2、CH3由ADC硬件触发
    // CH4由ATIM硬件触发,启动ADC
		

    __RCC_DMA_CLK_ENABLE();

    DMA_InitStruct.DMA_DstAddress = (uint32_t)&SampleData[0];       // 目标地址
    DMA_InitStruct.DMA_DstInc = DMA_DstAddress_Increase;            // 目标地址递增
    DMA_InitStruct.DMA_Mode = DMA_MODE_BLOCK;                       // BLOCK传输模式
    DMA_InitStruct.DMA_SrcAddress = (uint32_t)&CW_ADC->RESULT0;     // 源地址： ADC的结果寄存器
    DMA_InitStruct.DMA_SrcInc = DMA_SrcAddress_Fix;                 // 源地址固定
    DMA_InitStruct.DMA_TransferCnt = 0x6;                           // DMA传输次数
    DMA_InitStruct.DMA_TransferWidth = DMA_TRANSFER_WIDTH_16BIT;    // 数据位宽16bit
    DMA_InitStruct.HardTrigSource = DMA_HardTrig_ADC_TRANSCOMPLETE; // ADC转换完成硬触发
    DMA_InitStruct.TrigMode = DMA_HardTrig;                         //  硬触发模式
    DMA_Init(CW_DMACHANNEL1, &DMA_InitStruct);
    DMA_Cmd(CW_DMACHANNEL1, ENABLE);

    DMA_InitStruct.DMA_DstAddress = (uint32_t)&CW_ADC->CR1;         // 目标地址
    DMA_InitStruct.DMA_DstInc = DMA_DstAddress_Fix;                 // 目标地址固定
    DMA_InitStruct.DMA_Mode = DMA_MODE_BLOCK;                       // BLOCK传输模式
    DMA_InitStruct.DMA_SrcAddress = (uint32_t)&ADC_CR1Array[0];     // 源地址
    DMA_InitStruct.DMA_SrcInc = DMA_SrcAddress_Increase;            // 源地址递增
    DMA_InitStruct.DMA_TransferCnt = 0x5;                           // DMA传输次数
    DMA_InitStruct.DMA_TransferWidth = DMA_TRANSFER_WIDTH_8BIT;     // 数据位宽8bit
    DMA_InitStruct.HardTrigSource = DMA_HardTrig_ADC_TRANSCOMPLETE; // ADC转换完成硬触发
    DMA_InitStruct.TrigMode = DMA_HardTrig;                         //  硬触发模式
    DMA_Init(CW_DMACHANNEL2, &DMA_InitStruct);
    DMA_Cmd(CW_DMACHANNEL2, ENABLE);

    DMA_InitStruct.DMA_DstAddress = (uint32_t)&CW_ADC->START;       // 目标地址
    DMA_InitStruct.DMA_DstInc = DMA_DstAddress_Fix;                 // 目标地址固定
    DMA_InitStruct.DMA_Mode = DMA_MODE_BLOCK;                       // BLOCK传输模式
    DMA_InitStruct.DMA_SrcAddress = (uint32_t)&ADC_Start;           // 源地址
    DMA_InitStruct.DMA_SrcInc = DMA_SrcAddress_Fix;                 // 源地址固定
    DMA_InitStruct.DMA_TransferCnt = 0x5;                           // DMA传输次数
    DMA_InitStruct.DMA_TransferWidth = DMA_TRANSFER_WIDTH_8BIT;     // 数据位宽8bit
    DMA_InitStruct.HardTrigSource = DMA_HardTrig_ADC_TRANSCOMPLETE; // ADC转换完成硬触发
    DMA_InitStruct.TrigMode = DMA_HardTrig;                         //  硬触发模式
    DMA_Init(CW_DMACHANNEL3, &DMA_InitStruct);
    DMA_Cmd(CW_DMACHANNEL3, ENABLE);
    
    DMA_InitStruct.DMA_DstAddress = (uint32_t)&CW_ADC->START;       // 目标地址
    DMA_InitStruct.DMA_DstInc = DMA_DstAddress_Fix;                 // 目标地址固定
    DMA_InitStruct.DMA_Mode = DMA_MODE_BLOCK;                       // BLOCK传输模式
    DMA_InitStruct.DMA_SrcAddress = (uint32_t)&ADC_Start;           // 源地址
    DMA_InitStruct.DMA_SrcInc = DMA_SrcAddress_Fix;                 // 源地址固定
    DMA_InitStruct.DMA_TransferCnt = 0x1;                           // DMA传输次数
    DMA_InitStruct.DMA_TransferWidth = DMA_TRANSFER_WIDTH_8BIT;     // 数据位宽8bit
    DMA_InitStruct.HardTrigSource = DMA_HardTrig_ATIM_CH1A2A3A4;    // ATIM硬件触发
    DMA_InitStruct.TrigMode = DMA_HardTrig;                         //  硬触发模式
    DMA_Init(CW_DMACHANNEL4, &DMA_InitStruct);
    DMA_Cmd(CW_DMACHANNEL4, ENABLE);

    DMA_ITConfig(CW_DMACHANNEL1, DMA_IT_TC, ENABLE);
				
    __disable_irq();
    NVIC_EnableIRQ(DMACH1_IRQn);
    __enable_irq();
}

void RCC_Configuration(void)
{
		//.......................... 48M
	RCC_HSI_Enable(RCC_HSIOSC_DIV6);
	__RCC_FLASH_CLK_ENABLE();
  FLASH_SetLatency(FLASH_Latency_3);  
	RCC_HSI_Enable(RCC_HSIOSC_DIV1); 
//	//..............................
//	
//	RCC_HSI_Enable(RCC_HSIOSC_DIV6);

//  // 1. 设置HCLK和PCLK的分频系数　
//  RCC_HCLKPRS_Config(RCC_HCLK_DIV1);
//  RCC_PCLKPRS_Config(RCC_PCLK_DIV1);
//  
//  // 2. 使能PLL，通过PLL倍频到72MHz 
//  RCC_PLL_Enable(RCC_PLLSOURCE_HSI, 8000000, 6);     // HSI 默认输出频率8MHz
// // RCC_PLL_OUT();  //PC13脚输出PLL时钟
//  
//  ///< 当使用的时钟源HCLK大于24M,小于等于48MHz：设置FLASH 读等待周期为2 cycle
//  ///< 当使用的时钟源HCLK大于48MHz：设置FLASH 读等待周期为3 cycle
//  __RCC_FLASH_CLK_ENABLE();
//  FLASH_SetLatency(FLASH_Latency_3);   
//    
//   
//  // 3. 时钟切换到PLL
//  RCC_SysClk_Switch(RCC_SYSCLKSRC_PLL);
//  RCC_SystemCoreClockUpdate(48000000);	
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
	
	ATIM_InitStruct.BufferState = ENABLE;
  ATIM_InitStruct.ClockSelect = ATIM_CLOCK_PCLK;
  ATIM_InitStruct.CounterAlignedMode = ATIM_COUNT_MODE_EDGE_ALIGN;
  ATIM_InitStruct.CounterDirection = ATIM_COUNTING_UP;
  ATIM_InitStruct.CounterOPMode = ATIM_OP_MODE_REPETITIVE;
  ATIM_InitStruct.OverFlowMask = DISABLE;
  ATIM_InitStruct.Prescaler = ATIM_Prescaler_DIV1;    // 计算时钟1MHz
  ATIM_InitStruct.ReloadValue = TSPeriod;    // 15K 
  ATIM_InitStruct.RepetitionCounter = 0;
  ATIM_InitStruct.UnderFlowMask = DISABLE;
  ATIM_Init(&ATIM_InitStruct);

  ATIM_OCInitStruct.BufferState = ENABLE;
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
    ATIM_OC4Init(ENABLE, ATIM_OC_IT_UP_COUNTER, ENABLE, DISABLE, ENABLE);
    ATIM_SetCompare4(TSPeriod/2);    // 触发时刻
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
{//1MS TIMER
	BTIM_TimeBaseInitTypeDef BTIM_InitStruct;
	
	__RCC_BTIM_CLK_ENABLE();
	
	__disable_irq(); 
  NVIC_EnableIRQ(BTIM1_IRQn); 
  __enable_irq();
	
	BTIM_InitStruct.BTIM_Mode = BTIM_Mode_TIMER;
  BTIM_InitStruct.BTIM_OPMode = BTIM_OPMode_Repetitive;
  BTIM_InitStruct.BTIM_Period = 3000;
  BTIM_InitStruct.BTIM_Prescaler = BTIM_PRS_DIV16;
  BTIM_TimeBaseInit(CW_BTIM1, &BTIM_InitStruct);
	
  BTIM_ITConfig(CW_BTIM1, BTIM_IT_OV, ENABLE);
  BTIM_Cmd(CW_BTIM1, ENABLE);
}

extern unsigned char StOk;
/**
 * @brief This funcation handles BTIM1
 */
void BTIM1_IRQHandler(void)
{
  /* USER CODE BEGIN */
 static unsigned int count2=0;
	unsigned int test;
	
 if(BTIM_GetITStatus(CW_BTIM1, BTIM_IT_OV))
  {  //1MS
    BTIM_ClearITPendingBit(CW_BTIM1, BTIM_IT_OV);
		timecount++;
		timecount1++;
		TimeCountTemp++;
		
		if(Motor_Start_F==1) // 电机启动状态时，灯闪
		{
			count2++;
			if(count2>=500)
			{
					count2=0; 
					PC13_TOG();//PC13
			} 
			
		//因为屏幕操作运行的影响，为了及时进行PID进行，将PID运算执行时间为设10MS，放在BTIM定时器中断中完成。
			
		timecountpid++;
		if(timecountpid>=10&&StOk==1) //10MS进行一次PID运算,而且启动成功
		{
			timecountpid=0;
			
			test=HALLcount;HALLcount=0;
			RealSpid=test*1000/MotorPoles;	//计算实测电机RPM		*100*60/6
			PIDcompute(TargetS,RealSpid);		//PID运算		
		}
	 }
 }
  /* USER CODE END */
}




