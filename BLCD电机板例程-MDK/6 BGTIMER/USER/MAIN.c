//ϵͳʱ������Ϊ64M,BTIMER+GTIMER 
//��KEY1ʱ��LED3�Ʒ�ת��ͬʱLED1,LED2ÿ0.5S��˸һ�Ρ���
//LED1ʹ��GTIM1��ʱ����ɶ�ʱ����
//LED2ʹ��BTIM1��ʱ����ɶ�ʱ����
#include "main.h"

void GPIO_Configuration(void);
void RCC_Configuration(void);
void GTIM_init(void);
void BTIM_init(void);
	
int main()
{
	unsigned int key_1=0,i=0;
	RCC_Configuration();
	GPIO_Configuration();
	GTIM_init();
	BTIM_init();

	while(1)
	{
		 if(GPIO_ReadPin(CW_GPIOB,GPIO_PIN_4)==GPIO_Pin_SET)key_1=0;
		 else if(key_1==0)
		 {
			 for(i=0;i<2000;i++);
			 if(GPIO_ReadPin(CW_GPIOB,GPIO_PIN_4)==GPIO_Pin_RESET)
			 { key_1=1; PB02_TOG();}
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
		
	__RCC_GPIOA_CLK_ENABLE();
	__RCC_GPIOB_CLK_ENABLE();
	__RCC_GPIOC_CLK_ENABLE();


  GPIO_InitStruct.IT = GPIO_IT_NONE;  //KEY1
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT_PULLUP;
  GPIO_InitStruct.Pins = GPIO_PIN_4;
  GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;
  GPIO_Init(CW_GPIOB, &GPIO_InitStruct);

  GPIO_InitStruct.IT = GPIO_IT_NONE; //LED1 
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pins = GPIO_PIN_13;
  GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;
  GPIO_Init(CW_GPIOC, &GPIO_InitStruct);
	
	GPIO_InitStruct.Pins = GPIO_PIN_11;//LED2
	GPIO_Init(CW_GPIOA, &GPIO_InitStruct);
	PA11_SETHIGH();
	PC13_SETHIGH();

}


void GTIM_init(void)
{	
  GTIM_InitTypeDef GTIM_InitStruct;
 
	__RCC_GTIM1_CLK_ENABLE();
	
	__disable_irq(); 
  NVIC_EnableIRQ(GTIM1_IRQn); 
  __enable_irq();
	
	GTIM_InitStruct.Mode = GTIM_MODE_TIME;
  GTIM_InitStruct.OneShotMode = GTIM_COUNT_CONTINUE;
  GTIM_InitStruct.Prescaler = GTIM_PRESCALER_DIV8;
  GTIM_InitStruct.ReloadValue = 8000;
  GTIM_InitStruct.ToggleOutState = DISABLE;
  GTIM_TimeBaseInit(CW_GTIM1, &GTIM_InitStruct);

  GTIM_ITConfig(CW_GTIM1, GTIM_IT_OV, ENABLE);
  GTIM_Cmd(CW_GTIM1, ENABLE);
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

/**
 * @brief This funcation handles GTIM1
 */
void GTIM1_IRQHandler(void)
{
static unsigned int count1=0;
  if(GTIM_GetITStatus(CW_GTIM1,GTIM_IT_OV))
  {
    GTIM_ClearITPendingBit(CW_GTIM1,GTIM_IT_OV);
		count1++;
		if(count1>=500)//0.5S
		{
				count1=0; 
				PC13_TOG();
		}
	}
}

/**
 * @brief This funcation handles BTIM1
 */
void BTIM1_IRQHandler(void)
{
 static unsigned int count2=0;
 if(BTIM_GetITStatus(CW_BTIM1, BTIM_IT_OV))
  {
    BTIM_ClearITPendingBit(CW_BTIM1, BTIM_IT_OV);
		count2++;
		if(count2>=500)//0.5S
		{
				count2=0; 
				PA11_TOG();
		}
	}
}
