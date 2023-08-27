/*
ϵͳʱ������Ϊ64M,�ϵ磬LED1������������
BEEP_IO:PA12
*/

#include "main.h"

void GPIO_Configuration(void);
void RCC_Configuration(void);
	
int main()
{
	uint32_t i;
	RCC_Configuration();//64Mʱ������
	GPIO_Configuration();//LED&BEEP
	
	while(1)
	{
	  PA12_SETHIGH();
    for(i=0;i<1000000;i++);
		PA12_SETLOW();
		for(i=0;i<1000000;i++);
	}
}


void RCC_Configuration(void)
{
  /* 0. HSIʹ�ܲ�У׼ */
  RCC_HSI_Enable(RCC_HSIOSC_DIV6);

  /* 1. ����HCLK��PCLK�ķ�Ƶϵ����*/
  RCC_HCLKPRS_Config(RCC_HCLK_DIV1);
  RCC_PCLKPRS_Config(RCC_PCLK_DIV1);
  
  /* 2. ʹ��PLL��ͨ��PLL��Ƶ��72MHz */
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
	__RCC_GPIOC_CLK_ENABLE();
	
	GPIO_InitStruct.IT = GPIO_IT_NONE;   //BEEP
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pins = GPIO_PIN_12;
  GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;
  GPIO_Init(CW_GPIOA, &GPIO_InitStruct);
	
  GPIO_InitStruct.Pins = GPIO_PIN_13; //LED1
  GPIO_Init(CW_GPIOC, &GPIO_InitStruct);
	GPIO_WritePin(CW_GPIOC,GPIO_PIN_13,GPIO_Pin_RESET);
}

