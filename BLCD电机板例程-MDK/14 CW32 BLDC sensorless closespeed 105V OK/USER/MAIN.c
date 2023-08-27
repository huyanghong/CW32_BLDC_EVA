//ϵͳʱ������Ϊ64M,BTIMER+GTIMER 

/*
��·PWM: ATIM
U+ PA8
V+:PA9
W+:PA10

U-:PB13
V-:PB14
W-:PB15

�޻�����
HA:PA15
HB:PB3
HC:PA2

��λ��AD��
PB0
��ͨ����λ������

��·PWM,�Ϲܵ��ƣ��¹�IO���Ϲܸߵ�ƽ���¹ܵ͵�ƽ������
�������У���ת��λ�����Ե�����е��١�
�ϵ磬PC13ָʾ�Ƴ�������ת������ �����ת��PC13����˸
*/

#include "main.h"
#include "motorcontrol.h"
#include "OLED_I2C.h"
#include "pid.h"

unsigned int MotorPoles=2; //���������

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
	
	RCC_Configuration(); //64Mʱ������
	GPIO_Configuration(); //LED&KEYS��ʼ��
	
	I2C_init();			//OLED init
	I2C_OLED_Init();
	
	ADC_Configuration(); //AD����
	BTIM_init();  //1MS��ʱ����ʼ��
	ATIIM_init(); //��·PWM��ʱ����ʼ��	
	SENSORLESS_TIM_Config();	 //�޸��ö�ʱ����ʼ��
	
  I2C_OLED_Clear(1);		
	sprintf(temp_buff1,"Targ:%d rpm ",TargetS);  //�����ʾռ�ձ�
	sprintf(temp_buff,"Real:%d rpm   ",RealS);	//��ʾ���ת��
	I2C_OLED_ShowString(0,15,temp_buff1);
	I2C_OLED_ShowString(0,0,temp_buff);			
	I2C_OLED_UPdata(); 
		
	GPIO_WritePin(CW_GPIOC,GPIO_PIN_13,GPIO_Pin_RESET);  //LED ON
	
	while(1)
	{
		if(timecount>=50)
		{
			timecount=0;//100ms
				
			//��λ���ɼ���AD����ֵ��3-4000��Χ����Ч 3��Ӧ��������4000��Ӧ��С���			
				if(SampleData[5]>=4000)	test=0; 
				else if(SampleData[5]<3)	test=4000; 
				else 	test=4000-SampleData[5];
						
			  TargetS=test; //�ٶȷ�ΧΪ700-4000���������㣬ֱ�Ӹ�ֵ��
			
				if(TargetS<MinSpeed) //С����С����ٶ�ʱ�����ֹͣ����
				{	  
					 if(Motor_Start_F==1)
					 {
						Motor_Start_F=0;		//���ֹͣ״̬
						MOTOR_STOP(); //ֹͣ���
						GPIO_WritePin(CW_GPIOC,GPIO_PIN_13,GPIO_Pin_RESET);  //LED ON �ϵ�ָʾ�Ƴ���
					 }					 
				}
				else if(TargetS>MinSpeed+50&&Motor_Start_F==0&&ErrorCode==0) //������С���ʱ����ʼת
				{			 			
						Motor_Start_F=1;				//�ı���ͣ״̬
						Sensorless_MOTOR_START();		 //�������
					 //TargetS=MinSpeed;
					  if(ErrorCode==0)
							PID_init(); //�����ɹ���
						HALLcount=0;timecountpid=0;
				}		
		}
				//��Ϊ��Ļ�������е�Ӱ�죬Ϊ�˼�ʱ����PID���У���PID����ִ��ʱ��Ϊ��10MS������BTIM��ʱ���ж�����ɡ�

		if(timecount1>=500)    //1�����һ���ٶ�   MotorPoles--������  HALLcount--�������  RealS��ʵ��ת��
			 {
			    timecount1=0;
				 //1������ٶ�ֵ����ʾ
					RealS=HALLcountS*20/MotorPoles;	//����ʵ����RPM		
					HALLcountS=0;			
					sprintf(temp_buff,"Real:%d rpm       ",RealS);	//��ʾ���ת�� RealS
					I2C_OLED_ShowString(0,0,temp_buff);							  	 	
					sprintf(temp_buff1,"Targ:%d rpm      ",TargetS);   			
				  I2C_OLED_ShowString(0,15,temp_buff1);//��ʾ���ռ�ձ�	
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
		

    //����ADC����IO�� 
  
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

 // ʹ��4·DMAͨ����CH1��CH2��CH3�� CH4
    // CH1 ��ADC���ε�ͨ���Ĳ����������RAM��ADC_ResultBuff[6]��
    // CH2 ��ADC��CR1�Ĵ���������ֵ��RAM��ADC_CR1Array������Ĵ���
    // CH3 ��ADC��START�Ĵ���������ֵ��RAM��ADC_Start)����Ĵ���
    // CH1��CH2��CH3��ADCӲ������
    // CH4��ATIMӲ������,����ADC
		

    __RCC_DMA_CLK_ENABLE();

    DMA_InitStruct.DMA_DstAddress = (uint32_t)&SampleData[0];       // Ŀ���ַ
    DMA_InitStruct.DMA_DstInc = DMA_DstAddress_Increase;            // Ŀ���ַ����
    DMA_InitStruct.DMA_Mode = DMA_MODE_BLOCK;                       // BLOCK����ģʽ
    DMA_InitStruct.DMA_SrcAddress = (uint32_t)&CW_ADC->RESULT0;     // Դ��ַ�� ADC�Ľ���Ĵ���
    DMA_InitStruct.DMA_SrcInc = DMA_SrcAddress_Fix;                 // Դ��ַ�̶�
    DMA_InitStruct.DMA_TransferCnt = 0x6;                           // DMA�������
    DMA_InitStruct.DMA_TransferWidth = DMA_TRANSFER_WIDTH_16BIT;    // ����λ��16bit
    DMA_InitStruct.HardTrigSource = DMA_HardTrig_ADC_TRANSCOMPLETE; // ADCת�����Ӳ����
    DMA_InitStruct.TrigMode = DMA_HardTrig;                         //  Ӳ����ģʽ
    DMA_Init(CW_DMACHANNEL1, &DMA_InitStruct);
    DMA_Cmd(CW_DMACHANNEL1, ENABLE);

    DMA_InitStruct.DMA_DstAddress = (uint32_t)&CW_ADC->CR1;         // Ŀ���ַ
    DMA_InitStruct.DMA_DstInc = DMA_DstAddress_Fix;                 // Ŀ���ַ�̶�
    DMA_InitStruct.DMA_Mode = DMA_MODE_BLOCK;                       // BLOCK����ģʽ
    DMA_InitStruct.DMA_SrcAddress = (uint32_t)&ADC_CR1Array[0];     // Դ��ַ
    DMA_InitStruct.DMA_SrcInc = DMA_SrcAddress_Increase;            // Դ��ַ����
    DMA_InitStruct.DMA_TransferCnt = 0x5;                           // DMA�������
    DMA_InitStruct.DMA_TransferWidth = DMA_TRANSFER_WIDTH_8BIT;     // ����λ��8bit
    DMA_InitStruct.HardTrigSource = DMA_HardTrig_ADC_TRANSCOMPLETE; // ADCת�����Ӳ����
    DMA_InitStruct.TrigMode = DMA_HardTrig;                         //  Ӳ����ģʽ
    DMA_Init(CW_DMACHANNEL2, &DMA_InitStruct);
    DMA_Cmd(CW_DMACHANNEL2, ENABLE);

    DMA_InitStruct.DMA_DstAddress = (uint32_t)&CW_ADC->START;       // Ŀ���ַ
    DMA_InitStruct.DMA_DstInc = DMA_DstAddress_Fix;                 // Ŀ���ַ�̶�
    DMA_InitStruct.DMA_Mode = DMA_MODE_BLOCK;                       // BLOCK����ģʽ
    DMA_InitStruct.DMA_SrcAddress = (uint32_t)&ADC_Start;           // Դ��ַ
    DMA_InitStruct.DMA_SrcInc = DMA_SrcAddress_Fix;                 // Դ��ַ�̶�
    DMA_InitStruct.DMA_TransferCnt = 0x5;                           // DMA�������
    DMA_InitStruct.DMA_TransferWidth = DMA_TRANSFER_WIDTH_8BIT;     // ����λ��8bit
    DMA_InitStruct.HardTrigSource = DMA_HardTrig_ADC_TRANSCOMPLETE; // ADCת�����Ӳ����
    DMA_InitStruct.TrigMode = DMA_HardTrig;                         //  Ӳ����ģʽ
    DMA_Init(CW_DMACHANNEL3, &DMA_InitStruct);
    DMA_Cmd(CW_DMACHANNEL3, ENABLE);
    
    DMA_InitStruct.DMA_DstAddress = (uint32_t)&CW_ADC->START;       // Ŀ���ַ
    DMA_InitStruct.DMA_DstInc = DMA_DstAddress_Fix;                 // Ŀ���ַ�̶�
    DMA_InitStruct.DMA_Mode = DMA_MODE_BLOCK;                       // BLOCK����ģʽ
    DMA_InitStruct.DMA_SrcAddress = (uint32_t)&ADC_Start;           // Դ��ַ
    DMA_InitStruct.DMA_SrcInc = DMA_SrcAddress_Fix;                 // Դ��ַ�̶�
    DMA_InitStruct.DMA_TransferCnt = 0x1;                           // DMA�������
    DMA_InitStruct.DMA_TransferWidth = DMA_TRANSFER_WIDTH_8BIT;     // ����λ��8bit
    DMA_InitStruct.HardTrigSource = DMA_HardTrig_ATIM_CH1A2A3A4;    // ATIMӲ������
    DMA_InitStruct.TrigMode = DMA_HardTrig;                         //  Ӳ����ģʽ
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

//  // 1. ����HCLK��PCLK�ķ�Ƶϵ����
//  RCC_HCLKPRS_Config(RCC_HCLK_DIV1);
//  RCC_PCLKPRS_Config(RCC_PCLK_DIV1);
//  
//  // 2. ʹ��PLL��ͨ��PLL��Ƶ��72MHz 
//  RCC_PLL_Enable(RCC_PLLSOURCE_HSI, 8000000, 6);     // HSI Ĭ�����Ƶ��8MHz
// // RCC_PLL_OUT();  //PC13�����PLLʱ��
//  
//  ///< ��ʹ�õ�ʱ��ԴHCLK����24M,С�ڵ���48MHz������FLASH ���ȴ�����Ϊ2 cycle
//  ///< ��ʹ�õ�ʱ��ԴHCLK����48MHz������FLASH ���ȴ�����Ϊ3 cycle
//  __RCC_FLASH_CLK_ENABLE();
//  FLASH_SetLatency(FLASH_Latency_3);   
//    
//   
//  // 3. ʱ���л���PLL
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
  ATIM_InitStruct.Prescaler = ATIM_Prescaler_DIV1;    // ����ʱ��1MHz
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
	
	  CW_ATIM->MSCR_f.CCDS = 1;    // DMA �ܿ���
    ATIM_OC4Init(ENABLE, ATIM_OC_IT_UP_COUNTER, ENABLE, DISABLE, ENABLE);
    ATIM_SetCompare4(TSPeriod/2);    // ����ʱ��
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
		
		if(Motor_Start_F==1) // �������״̬ʱ������
		{
			count2++;
			if(count2>=500)
			{
					count2=0; 
					PC13_TOG();//PC13
			} 
			
		//��Ϊ��Ļ�������е�Ӱ�죬Ϊ�˼�ʱ����PID���У���PID����ִ��ʱ��Ϊ��10MS������BTIM��ʱ���ж�����ɡ�
			
		timecountpid++;
		if(timecountpid>=10&&StOk==1) //10MS����һ��PID����,���������ɹ�
		{
			timecountpid=0;
			
			test=HALLcount;HALLcount=0;
			RealSpid=test*1000/MotorPoles;	//����ʵ����RPM		*100*60/6
			PIDcompute(TargetS,RealSpid);		//PID����		
		}
	 }
 }
  /* USER CODE END */
}




