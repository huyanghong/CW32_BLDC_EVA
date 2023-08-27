//ϵͳʱ������Ϊ64M,BTIMER+GTIMER 

/*
��·PWM: ATIM
U+ PA8
V+:PA9
W+:PA10

U-:PB13
V-:PB14
W-:PB15

������GTIM2
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

#define MotorPoles 2 //���������


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
	
	RCC_Configuration(); //64Mʱ������
	GPIO_Configuration(); //LED&KEYS
	I2C_init();			//OLED init
	I2C_OLED_Init();
	ADC_Configuration(); //AD PONITER
	BTIM_init();  //10MS USRER TIMER
	ATIIM_init(); //PWM TIMER
	GTIM_init();  //HALL TIMER
  I2C_OLED_Clear(1);
		
	sprintf(temp_buff1,"PWM:%d %%  STOP...",PWMFLA);  //�����ʾռ�ձ�
	sprintf(temp_buff,"Real:%d rpm   ",RealS);	//��ʾ���ת��
	I2C_OLED_ShowString(0,15,temp_buff1);
	I2C_OLED_ShowString(0,0,temp_buff);			
	I2C_OLED_UPdata(); 
	
	
	GPIO_WritePin(CW_GPIOC,GPIO_PIN_13,GPIO_Pin_RESET);  //LED ON
	
	while(1)
	{
		if (CW_DMA->ISR_f.TC1)
		{ //AD DMA ����
			CW_DMA->ICR_f.TC1 = 0;	
      CW_DMACHANNEL1->CNT=bv16|60000;	 //MUST RET AGAIN BEFORE CW_DMACHANNEL1->CNT=0
			CW_DMACHANNEL1->CSR_f.EN = 1; 		
		}
		
		if(timecount>=10)
		{
			timecount=0;//100ms
				
			//��λ���ɼ���AD����ֵ��3-4000��Χ����Ч 3��Ӧ��������4000��Ӧ��С���			
				if(ADC_Result_Array>=4000)	test=0; 
				else if(ADC_Result_Array<3)	test=4000; 
				else 	test=4000-ADC_Result_Array;
			
				t=test*0.8;///40*32;
				test=t;	//���㵱ǰ Ӧ�������ֵ
			
			
			   if(OutPwm<test)
					{
						OutPwm+=TS*0.05; //������
						if(OutPwm>=test)OutPwm=test;
					}
				 else OutPwm=test;
				
				if(Motor_Start_F==1)//��������󣬼�ʱ����PWM
				 Commutation(HALL_Check(),OutPwm,Motor_Start_F);//�˵�����Ҫ��ʱ�������ռ�ձ�
				 						
			
				if(test<OUTMINPWM) //С����С���ʱ�����ֹͣ����
				{	  
					 if(Motor_Start_F==1)
					 {
						Motor_Start_F=0;		//���ֹͣ״̬
						HALL_MOTOR_START(); //ֹͣ���
						GPIO_WritePin(CW_GPIOC,GPIO_PIN_13,GPIO_Pin_RESET);  //LED ON �ϵ�ָʾ�Ƴ���
					 }					 
				}
				else if(test>OUTMINPWM+50&&Motor_Start_F==0) //������С���ʱ����ʼת
				{			 			
						Motor_Start_F=1;				//�ı���ͣ״̬
					  OutPwm=OUTMINPWM;
						HALL_MOTOR_START();		 //�������
				}		
		}

		
			 if(timecount1>=50)    //1�����һ���ٶ�   MotorPoles--������  HALLcount--�������  RealS��ʵ��ת��
			 {
			    timecount1=0; //500MS
				 //1������ٶ�ֵ����ʾ
					RealS=HALLcount*20/MotorPoles;	//����ʵ����RPM		
					HALLcount=0;			
					sprintf(temp_buff,"Real:%d rpm   ",RealS);	//��ʾ���ת��
					I2C_OLED_ShowString(0,0,temp_buff);		

					PWMFLA=OutPwm/32; //��ʾ���ռ�ձ�
					if(Motor_Start_F==1)
					{	
						sprintf(temp_buff1,"PWM:%d %%  START  ",PWMFLA);  //�����ʾռ�ձ�
					}
					else
					{	
						sprintf(temp_buff1,"PWM:%d %%  STOP...",PWMFLA);  //�����ʾռ�ձ�
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
	
   HALLcount++;//�����������	
	 x=HALL_Check(); //��ȡ����״̬	
	 if(Motor_Start_F==1&&ErrorCode==0) //������ͣ״̬ ����
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
  GPIO_InitStruct.Mode =GPIO_MODE_INPUT_PULLUP;// ������������;
  GPIO_InitStruct.Pins = GPIO_PIN_15|GPIO_PIN_2;
  GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;
  GPIO_Init(CW_GPIOA, &GPIO_InitStruct);

	GPIO_InitStruct.IT = GPIO_IT_NONE;
  GPIO_InitStruct.Mode =GPIO_MODE_INPUT_PULLUP;// ������������
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

    //����ADC����IO��  ��λ���ӿ�
	  PB00_ANALOG_ENABLE();    //PB00 (AIN8)
	
    //ADC��ʼ��
    ADC_InitStruct.ADC_OpMode = ADC_SingleChContinuousMode; 
    ADC_InitStruct.ADC_ClkDiv = ADC_Clk_Div8; //PCLK
    ADC_InitStruct.ADC_SampleTime = ADC_SampTime10Clk; //5��ADCʱ������
    ADC_InitStruct.ADC_VrefSel = ADC_Vref_VDDA; //VDDA�ο���ѹ
    ADC_InitStruct.ADC_InBufEn = ADC_BufDisable; //����������
    ADC_InitStruct.ADC_TsEn = ADC_TsDisable; //�����¶ȴ���������
    ADC_InitStruct.ADC_DMAEn = ADC_DmaEnable; //ADCת����ɴ���DMA����
    ADC_InitStruct.ADC_Align = ADC_AlignRight; //ADCת������Ҷ���
    ADC_InitStruct.ADC_AccEn = ADC_AccDisable; //ת������ۼӲ�ʹ��
    ADC_Init(&ADC_InitStruct);                 //��ʼ��ADC����
    CW_ADC->CR1_f.DISCARD = FALSE;             //�������ݸ��²��ԣ���������ADC�ṹ����               
    CW_ADC->CR1_f.CHMUX = ADC_ExInputCH8;      //����ADC����ͨ������������ADC�ṹ����
    
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
    DMA_ITConfig(CW_DMACHANNEL1, DMA_IT_TC|DMA_IT_TE , ENABLE);  //ʹ��DMA_CHANNEL1�ж�
    DMA_Cmd(CW_DMACHANNEL1, ENABLE);  //ʹ��DMA

    //ADCʹ��
    ADC_Enable();
    ADC_SoftwareStartConvCmd(ENABLE);
}

void RCC_Configuration(void)
{
  //...........................64M
  // 0. HSIʹ�ܲ�У׼ 
  RCC_HSI_Enable(RCC_HSIOSC_DIV6);

  // 1. ����HCLK��PCLK�ķ�Ƶϵ����
  RCC_HCLKPRS_Config(RCC_HCLK_DIV1);
  RCC_PCLKPRS_Config(RCC_PCLK_DIV1);
  
  // 2. ʹ��PLL��ͨ��PLL��Ƶ��64MHz 
  RCC_PLL_Enable(RCC_PLLSOURCE_HSI, 8000000, 8);     // HSI Ĭ�����Ƶ��8MHz

  ///< ��ʹ�õ�ʱ��ԴHCLK����24M,С�ڵ���48MHz������FLASH ���ȴ�����Ϊ2 cycle
  ///< ��ʹ�õ�ʱ��ԴHCLK����48MHz������FLASH ���ȴ�����Ϊ3 cycle
  __RCC_FLASH_CLK_ENABLE();
  FLASH_SetLatency(FLASH_Latency_3);       
   
  // 3. ʱ���л���PLL
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
  ATIM_InitStruct.Prescaler = ATIM_Prescaler_DIV1;    // ����ʱ��1MHz
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
		
		if(Motor_Start_F==1) // �������״̬ʱ������
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




