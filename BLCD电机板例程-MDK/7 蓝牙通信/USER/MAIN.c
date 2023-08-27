/*ϵͳʱ������Ϊ64M,���ڷ�������
  �����յ� {0X00 0X01 0X02 0X03 0X04}������LED3��ת
  ������9600
  ����1 PB8-TX PB9-RX
	��������������
  LED1 0.5�뷴תһ��״̬
*/

#include "main.h"

#define PUTCHAR_PROTOTYPE int fputc(int ch, FILE *f)              //printf
void GPIO_Configuration(void);
void RCC_Configuration(void);
void UART_init(void);	
void BTIM_init(void);

unsigned int counttime=0;
unsigned char dat[5]={0x00,0x01,0x02,0x03,0x04};  //���͵�����
char rec[10]={0};                                 //��������
unsigned char re_count=0;
unsigned char check_flag=0,rev_start=0;

int main()
{	
	uint16_t t=0;
	RCC_Configuration();
	GPIO_Configuration();
	UART_init();
	BTIM_init(); 
	while(1)
	{		
			if(check_flag==1)
			{
				 check_flag=0;
				 PB11_TOG();   //�յ�����LED3--PB11��״̬��ת
			}
		
		 if(counttime>1000)
		 {	
			  counttime=0;
				for(t=0;t<5;t++)
			  {
						USART_SendData(CW_UART1,dat[t]);										
						while(USART_GetFlagStatus(CW_UART1,USART_FLAG_TXE)==RESET);
						USART_ClearFlag(CW_UART1,USART_FLAG_TXE);			
			  }		
     }
	}
}

void UART_init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	
  RCC_APBPeriphClk_Enable2(RCC_APB2_PERIPH_UART1, ENABLE);
  RCC_AHBPeriphClk_Enable( RCC_AHB_PERIPH_GPIOB, ENABLE);  

	PB08_AFx_UART1TXD();
	PB09_AFx_UART1RXD();
	 
  GPIO_InitStructure.Pins = GPIO_PIN_8; 
  GPIO_InitStructure.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStructure.Speed = GPIO_SPEED_HIGH;
  GPIO_Init(CW_GPIOB, &GPIO_InitStructure);
    
  GPIO_InitStructure.Pins = GPIO_PIN_9; 
  GPIO_InitStructure.Mode = GPIO_MODE_INPUT_PULLUP;
  GPIO_Init(CW_GPIOB, &GPIO_InitStructure);
		

  USART_InitStructure.USART_BaudRate = 9600;
  USART_InitStructure.USART_Over = USART_Over_16;
  USART_InitStructure.USART_Source = USART_Source_PCLK;
  USART_InitStructure.USART_UclkFreq = 64000000;
  USART_InitStructure.USART_StartBit = USART_StartBit_FE;
  USART_InitStructure.USART_StopBits = USART_StopBits_1;
  USART_InitStructure.USART_Parity = USART_Parity_No ;
  USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
  USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
  USART_Init(CW_UART1, &USART_InitStructure); 
	
	
  //ʹ��UARTx RC�ж�
  USART_ITConfig(CW_UART1, USART_IT_RC, ENABLE);
  //���ȼ��������ȼ�����
  NVIC_SetPriority(UART1_IRQn, 0);
  //UARTx�ж�ʹ��
  NVIC_EnableIRQ(UART1_IRQn);

}

 
void UART1_IRQHandler(void)
{
  unsigned char TxRxBuffer;
  if(USART_GetITStatus(CW_UART1, USART_IT_RC) != RESET)
  { 
		USART_ClearITPendingBit(CW_UART1, USART_IT_RC); 
    TxRxBuffer = USART_ReceiveData_8bit(CW_UART1);
   
			rec[re_count]=TxRxBuffer;
		 if (re_count == 0&&rec[0]==0x00)   //�ж�֡ͷ
			{          
				rev_start=1;
				re_count++;
			}
			else if(rev_start==1)
			{		
				if(re_count==1&&rec[1]!=0x01)
				{
					re_count=0;rev_start=0;
				}
				else if(re_count==4)            //�����ܸ���
			      {check_flag=1;re_count=0;rev_start=0;}	  //����ԣ���ô����־λ��1������������ֵ���½���
				else		
				   re_count++;		
			}
  }
}



void RCC_Configuration(void)
{
	
  // 0. HSIʹ�ܲ�У׼ 
  RCC_HSI_Enable(RCC_HSIOSC_DIV6);

  // 1. ����HCLK��PCLK�ķ�Ƶϵ����
  RCC_HCLKPRS_Config(RCC_HCLK_DIV1);
  RCC_PCLKPRS_Config(RCC_PCLK_DIV1);
  
  // 2. ʹ��PLL��ͨ��PLL��Ƶ��72MHz 
  RCC_PLL_Enable(RCC_PLLSOURCE_HSI, 8000000, 8);     // HSI Ĭ�����Ƶ��8MHz
 // RCC_PLL_OUT();  //PC13�����PLLʱ��
  
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
	
	__RCC_GPIOC_CLK_ENABLE();
	__RCC_GPIOB_CLK_ENABLE();

  GPIO_InitStruct.IT = GPIO_IT_NONE; //LED1  
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pins = GPIO_PIN_13;
  GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;
  GPIO_Init(CW_GPIOC, &GPIO_InitStruct);
	
	GPIO_InitStruct.Pins = GPIO_PIN_11;//LED3
	GPIO_Init(CW_GPIOB, &GPIO_InitStruct);

	PB11_SETHIGH();
	PC13_SETHIGH();
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
		counttime++;
		if(count2>=500)//0.5S
		{
				count2=0; 
				PC13_TOG();
		}
	}
  /* USER CODE END */
}


PUTCHAR_PROTOTYPE    //��C���printf�������¶�λ��USART
{
  USART_SendData_8bit(CW_UART1, (uint8_t)ch);

  while (USART_GetFlagStatus(CW_UART1, USART_FLAG_TXE) == RESET);

  return ch;
}

