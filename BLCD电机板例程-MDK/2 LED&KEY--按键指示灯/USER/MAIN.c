//ϵͳʱ������Ϊ64M,��KEY1ʱ��LED1�Ʒ�ת;��KEY2ʱ��LED2�Ʒ�ת:��KEY3ʱ��LED3�Ʒ�ת

#include "main.h"

void GPIO_Configuration(void);  
void RCC_Configuration(void);
	
int main()
{
	unsigned int key_1,key_2,key_3;
	
	RCC_Configuration();  //64Mʱ������
	GPIO_Configuration(); //LED&KEYS
	PA11_SETLOW();
	PB11_SETLOW();
	PC13_SETLOW();
	
	while(1)
	{
		 if(GPIO_ReadPin(CW_GPIOB,GPIO_PIN_4)==GPIO_Pin_SET)key_1=0;  //KEY1
		 else if(key_1==0)//��ⰴ������
		 {  
			 delay1ms(10);  //����
			 if(GPIO_ReadPin(CW_GPIOB,GPIO_PIN_4)==GPIO_Pin_RESET)
			 { key_1=1; PC13_TOG();}
		 }
		 
		 if(GPIO_ReadPin(CW_GPIOB,GPIO_PIN_5)==GPIO_Pin_SET)key_2=0;  //KEY2
		 else if(key_2==0)
		 { 
				delay1ms(10); 
			  if(GPIO_ReadPin(CW_GPIOB,GPIO_PIN_5)==GPIO_Pin_RESET)
			 { 
			  key_2=1;PA11_TOG();
			 }
		 }
		 	
		 if(GPIO_ReadPin(CW_GPIOB,GPIO_PIN_10)==GPIO_Pin_SET)key_3=0;  //KEY3
		 else if(key_3==0)
		 {
			  delay1ms(10);
			 if(GPIO_ReadPin(CW_GPIOB,GPIO_PIN_10)==GPIO_Pin_RESET)
			 { 
			  key_3=1;PB11_TOG();
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

	__RCC_GPIOB_CLK_ENABLE();
	__RCC_GPIOC_CLK_ENABLE();
	__RCC_GPIOA_CLK_ENABLE();

  GPIO_InitStruct.IT = GPIO_IT_NONE;  //KEY1 KEY2 KEY3
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT_PULLUP;
  GPIO_InitStruct.Pins = GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_10;
  GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;
  GPIO_Init(CW_GPIOB, &GPIO_InitStruct);
	

	 GPIO_InitStruct.IT = GPIO_IT_NONE;  //LED3
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pins = GPIO_PIN_11;
  GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;
  GPIO_Init(CW_GPIOB, &GPIO_InitStruct);

  GPIO_InitStruct.Pins = GPIO_PIN_13; //LED2
  GPIO_Init(CW_GPIOC, &GPIO_InitStruct);
	
  GPIO_InitStruct.Pins = GPIO_PIN_11; //LED1
  GPIO_Init(CW_GPIOA, &GPIO_InitStruct);
}

