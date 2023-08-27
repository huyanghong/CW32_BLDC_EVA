/*
ϵͳʱ������Ϊ64M
����IO�ڣ�PB0
LED1 0.5S��תһ��
OLED��Ļ��ʾADֵ���ѹֵ
*/

#include "main.h"
#include "OLED_I2C.h"

void GPIO_Configuration(void);
void RCC_Configuration(void);
void ADC_Configuration(void);
void BTIM_init(void);

unsigned int timecount=0;
uint16_t adcvalue;


int main()
{	
	float temp;
	char temp_buff[10];
  char temp_buff1[10];
	RCC_Configuration(); 
	GPIO_Configuration();
	ADC_Configuration();
	BTIM_init();
	I2C_init();
	I2C_OLED_Init();
	
	while(1)
	{ 
			if(timecount>200)//ÿ200ms����ת��һ�Σ���ͨ��OLED��ʾ
			{
				  timecount=0;		
			    ADC_SoftwareStartConvCmd(ENABLE);
					while(ADC_GetITStatus(ADC_IT_EOC))
					{
						ADC_ClearITPendingBit(ADC_IT_EOC);      
						adcvalue=ADC_GetConversionValue();	
						sprintf(temp_buff,"AD/VAL:%d  ",adcvalue);
					  I2C_OLED_ShowString(0,15,temp_buff);
	          I2C_OLED_UPdata();
						temp=(float)adcvalue*(3.3/4096);
					  sprintf(temp_buff1,"AD/VOL:%0.1f V  ",temp);
					  I2C_OLED_ShowString(0,0,temp_buff1);
	          I2C_OLED_UPdata();
					} 					 	
			}				
	}
}

void RCC_Configuration(void)
{
  /* 0. HSIʹ�ܲ�У׼ */
  RCC_HSI_Enable(RCC_HSIOSC_DIV6);

  /* 1. ����HCLK��PCLK�ķ�Ƶϵ����*/
  RCC_HCLKPRS_Config(RCC_HCLK_DIV1);
  RCC_PCLKPRS_Config(RCC_PCLK_DIV1);
  
  /* 2. ʹ��PLL��ͨ��PLL��Ƶ��64MHz */
  RCC_PLL_Enable(RCC_PLLSOURCE_HSI, 8000000, 8);     // HSI Ĭ�����Ƶ��8MHz
 // RCC_PLL_OUT();  //PC13�����PLLʱ��
  
  ///< ��ʹ�õ�ʱ��ԴHCLK����24M,С�ڵ���48MHz������FLASH ���ȴ�����Ϊ2 cycle
  ///< ��ʹ�õ�ʱ��ԴHCLK����48MHz������FLASH ���ȴ�����Ϊ3 cycle
  __RCC_FLASH_CLK_ENABLE();
  FLASH_SetLatency(FLASH_Latency_3);   
    
  /* 3. ʱ���л���PLL */
  RCC_SysClk_Switch(RCC_SYSCLKSRC_PLL);
  RCC_SystemCoreClockUpdate(64000000);	
}



void GPIO_Configuration(void)
{
  GPIO_InitTypeDef GPIO_InitStruct;
	
	__RCC_GPIOC_CLK_ENABLE();

  GPIO_InitStruct.IT = GPIO_IT_NONE; //LED1 
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pins = GPIO_PIN_13;
  GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;
  GPIO_Init(CW_GPIOC, &GPIO_InitStruct);
	GPIO_WritePin(CW_GPIOC,GPIO_PIN_13,GPIO_Pin_SET);
}


void ADC_Configuration(void)
{
   ADC_SingleChTypeDef ADC_SingleInitStruct;

  __RCC_ADC_CLK_ENABLE();    // ADCʱ��ʹ��
	__RCC_GPIOB_CLK_ENABLE();

  PB00_ANALOG_ENABLE();
  ADC_SingleInitStruct.ADC_Chmux = ADC_ExInputCH8;         //ͨ��8����PB00
	
  ADC_SingleInitStruct.ADC_DiscardEn = ADC_DiscardNull;
  ADC_SingleInitStruct.ADC_InitStruct.ADC_AccEn = ADC_AccDisable;//ת������ۼӲ�ʹ��
  ADC_SingleInitStruct.ADC_InitStruct.ADC_Align = ADC_AlignRight; //ADCת������Ҷ���
  ADC_SingleInitStruct.ADC_InitStruct.ADC_ClkDiv = ADC_Clk_Div16;  //PCLK  
  ADC_SingleInitStruct.ADC_InitStruct.ADC_DMAEn = ADC_DmaDisable;  //�ر�DMA����
  ADC_SingleInitStruct.ADC_InitStruct.ADC_InBufEn = ADC_BufEnable;   //����������
  ADC_SingleInitStruct.ADC_InitStruct.ADC_OpMode = ADC_SingleChOneMode;   
  ADC_SingleInitStruct.ADC_InitStruct.ADC_SampleTime = ADC_SampTime5Clk; //5��ADCʱ������
  ADC_SingleInitStruct.ADC_InitStruct.ADC_TsEn = ADC_TsDisable;    //�����¶ȴ���������
  ADC_SingleInitStruct.ADC_InitStruct.ADC_VrefSel = ADC_Vref_VDDA;//VDDA�ο���ѹ
  ADC_SingleInitStruct.ADC_WdtStruct.ADC_WdtAll = ADC_WdtDisable;    
	
  ADC_SingleChOneModeCfg(&ADC_SingleInitStruct);
  ADC_Enable();    // ʹ��ADC
	
	ADC_SoftwareStartConvCmd(ENABLE);
}

void BTIM_init(void)
{
	BTIM_TimeBaseInitTypeDef BTIM_InitStruct;
	
	__RCC_BTIM_CLK_ENABLE();
	
	__disable_irq(); 
  NVIC_EnableIRQ(BTIM1_IRQn); 
  __enable_irq();
	
	BTIM_InitStruct.BTIM_Mode = BTIM_Mode_TIMER;
  BTIM_InitStruct.BTIM_OPMode = BTIM_OPMode_Repetitive;
  BTIM_InitStruct.BTIM_Period = 8000;
  BTIM_InitStruct.BTIM_Prescaler = BTIM_PRS_DIV8;
  BTIM_TimeBaseInit(CW_BTIM1, &BTIM_InitStruct);
	
  BTIM_ITConfig(CW_BTIM1, BTIM_IT_OV, ENABLE);
  BTIM_Cmd(CW_BTIM1, ENABLE);
}

void BTIM1_IRQHandler(void)
{
  /* USER CODE BEGIN */
 static unsigned int count2=0;
 if(BTIM_GetITStatus(CW_BTIM1, BTIM_IT_OV))
  {
    BTIM_ClearITPendingBit(CW_BTIM1, BTIM_IT_OV);
		count2++;
		timecount++;
		if(count2>500)
		{
			count2=0;
		  PC13_TOG();
		}
	}
  /* USER CODE END */
}









