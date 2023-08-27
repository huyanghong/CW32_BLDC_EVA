#include "globalv.h"
#include "init.h"

uint8_t ADC_CR1Array[4] = {0x83, 0x84, 0x87, 0x88}; // 通过更改ADC的CR1寄存器实现ADC通道自动切换
uint8_t ADC_Start = 0x01;    

void RCC_Configuration(void)
{//48M,
	
	//.......................... 48M
	RCC_HSI_Enable(RCC_HSIOSC_DIV6);
	__RCC_FLASH_CLK_ENABLE();
  FLASH_SetLatency(FLASH_Latency_3);  
	RCC_HSI_Enable(RCC_HSIOSC_DIV1); 
  //.............................  
}

void GPIOInit(void)
{
	 GPIO_InitTypeDef GPIO_InitStruct;
	
	__RCC_GPIOA_CLK_ENABLE();
	__RCC_GPIOB_CLK_ENABLE();
	__RCC_GPIOC_CLK_ENABLE();

  GPIO_InitStruct.IT = GPIO_IT_NONE; //LED1 
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;
	
  GPIO_InitStruct.Pins = LED_GPIO_PIN;
  GPIO_Init(LED_GPIO_PORT, &GPIO_InitStruct);
  GPIO_InitStruct.Pins = LEDDIR_GPIO_PIN;
  GPIO_Init(LEDDIR_GPIO_PORT, &GPIO_InitStruct);
  GPIO_InitStruct.Pins = LEDSTSTO_GPIO_PIN;
  GPIO_Init(LEDSTSTO_GPIO_PORT, &GPIO_InitStruct);
	 GPIO_InitStruct.Pins = BUZZER_GPIO_PIN;//蜂鸣器
  GPIO_Init(BUZZER_GPIO_PORT, &GPIO_InitStruct);
	
	GPIO_WritePin(LEDDIR_GPIO_PORT,LEDDIR_GPIO_PIN,GPIO_Pin_RESET); //方向指示灯亮
	GPIO_WritePin(LEDSTSTO_GPIO_PORT,LEDSTSTO_GPIO_PIN,GPIO_Pin_SET); //启停指示灯灭


  GPIO_InitStruct.IT = GPIO_IT_NONE; 
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT_PULLUP;
  GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;
	
  GPIO_InitStruct.Pins = EN_GPIO_PIN;
  GPIO_Init(EN_GPIO_PORT, &GPIO_InitStruct);
  GPIO_InitStruct.Pins = DR_GPIO_PIN;
  GPIO_Init(DR_GPIO_PORT, &GPIO_InitStruct);
  GPIO_InitStruct.Pins = GPIO_PIN_4;
  GPIO_Init(CW_GPIOB, &GPIO_InitStruct);
}



void ADC_Configuration(void)
{
	   ADC_InitTypeDef   ADC_InitStruct;	 
		 DMA_InitTypeDef DMA_InitStruct = {0};
	
    __RCC_GPIOA_CLK_ENABLE();
    __RCC_GPIOB_CLK_ENABLE();
    __RCC_ADC_CLK_ENABLE();
		

    //配置ADC测试IO口 
    PA03_ANALOG_ENABLE() ;      //母线电压
		 PA04_ANALOG_ENABLE() ;      //NTC温度
    PA07_ANALOG_ENABLE() ;      //电流
    PB00_ANALOG_ENABLE() ;      //电位器 
	
	
    ADC_InitStruct.ADC_AccEn = ADC_AccDisable;
    ADC_InitStruct.ADC_Align = ADC_AlignRight;
    ADC_InitStruct.ADC_ClkDiv = ADC_Clk_Div2; // ADCCLK=16MHz
    ADC_InitStruct.ADC_DMAEn = ADC_DmaEnable;
    ADC_InitStruct.ADC_InBufEn = ADC_BufDisable;
    ADC_InitStruct.ADC_OpMode = ADC_SingleChOneMode;
    ADC_InitStruct.ADC_SampleTime = ADC_SampTime10Clk;
    ADC_InitStruct.ADC_TsEn = ADC_TsDisable;
    ADC_InitStruct.ADC_VrefSel = ADC_Vref_VDDA;

    ADC_Init(&ADC_InitStruct);

    CW_ADC->CR1_f.CHMUX = 3; // AN0
    ADC_Enable();

 // 使用4路DMA通道：CH1、CH2、CH3、 CH4
    // CH1 将ADC单次单通道的采样结果传入RAM（ADC_ResultBuff[6]）
    // CH2 将ADC的CR1寄存器的配置值从RAM（ADC_CR1Array）传入寄存器
    // CH3 将ADC的START寄存器的配置值从RAM（ADC_Start)传入寄存器
    // CH1、CH2、CH3由ADC硬件触发
    // CH4由ATIM硬件触发,启动ADC
		

    __RCC_DMA_CLK_ENABLE();

    DMA_InitStruct.DMA_DstAddress = (uint32_t)&SampleData[0];        // 目标地址
    DMA_InitStruct.DMA_DstInc = DMA_DstAddress_Increase;                 // 目标地址递增
    DMA_InitStruct.DMA_Mode = DMA_MODE_BLOCK;                       // BLOCK传输模式
    DMA_InitStruct.DMA_SrcAddress = (uint32_t)&CW_ADC->RESULT0;     // 源地址： ADC的结果寄存器
    DMA_InitStruct.DMA_SrcInc = DMA_SrcAddress_Fix;                 // 源地址固定
    DMA_InitStruct.DMA_TransferCnt = 0x4;                           // DMA传输次数
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
    DMA_InitStruct.DMA_TransferCnt = 0x3;                           // DMA传输次数
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
    DMA_InitStruct.DMA_TransferCnt = 0x3;                           // DMA传输次数
    DMA_InitStruct.DMA_TransferWidth = DMA_TRANSFER_WIDTH_8BIT;     // 数据位宽8bit
    DMA_InitStruct.HardTrigSource = DMA_HardTrig_ADC_TRANSCOMPLETE;    // ADC转换完成硬触发
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

void DMACH1_IRQHandlerCallBack(void)
{	
    CW_DMA->ICR_f.TC1 = 0x00;    // 清中断标志
    CW_ATIM->ICR  = 0x00;        // 清ATIM的中断标志
    CW_ADC->ICR = 0x00;          // 请ADC的中断标志
    CW_ADC->CR1 = 0x83;     // AdcIn = ch0
    //------------------------------------------------
    // DMA.CH1 : ADC_Result -> RAM
    CW_DMACHANNEL1->CNT = 0x10004; //传4个
    CW_DMACHANNEL1->SRCADDR = (uint32_t)(&CW_ADC->RESULT0);
    CW_DMACHANNEL1->DSTADDR = (uint32_t)(&SampleData[0]);
    CW_DMACHANNEL1->CSR_f.EN = 1;


    //------------------------------------------------
    // DMA.CH2 : RAM -> ADC_CR1
    CW_DMACHANNEL2->CNT = 0x10003; //传3个
    CW_DMACHANNEL2->SRCADDR = (uint32_t)(&ADC_CR1Array[1]);
    CW_DMACHANNEL2->DSTADDR = (uint32_t)(&CW_ADC->CR1);
    CW_DMACHANNEL2->CSR_f.EN = 1;

    //------------------------------------------------
    // DMA.CH3 : RAM -> ADC_START
    CW_DMACHANNEL3->CNT = 0x10003; //传3个
    CW_DMACHANNEL3->SRCADDR = (uint32_t)(&ADC_Start);
    CW_DMACHANNEL3->DSTADDR = (uint32_t)(&CW_ADC->START);
    CW_DMACHANNEL3->CSR_f.EN = 1;
    
    //------------------------------------------------
    // DMA.CH4 : RAM -> ADC_START
    CW_DMACHANNEL4->CNT = 0x10001; //传1个
    CW_DMACHANNEL4->SRCADDR = (uint32_t)(&ADC_Start);
    CW_DMACHANNEL4->DSTADDR = (uint32_t)(&CW_ADC->START);
    CW_DMACHANNEL4->CSR_f.EN = 1;		
}


void BTIM_init(void)
{
	//1MS 48m/16/3000
	BTIM_TimeBaseInitTypeDef BTIM_InitStruct;
	
	__RCC_BTIM_CLK_ENABLE();
	
	__disable_irq(); 
  NVIC_EnableIRQ(BTIM1_IRQn); 
  __enable_irq();
	
	BTIM_InitStruct.BTIM_Mode = BTIM_Mode_TIMER; //1ms定时器
  BTIM_InitStruct.BTIM_OPMode = BTIM_OPMode_Repetitive;
  BTIM_InitStruct.BTIM_Period = 3000;
  BTIM_InitStruct.BTIM_Prescaler = BTIM_PRS_DIV16;
  BTIM_TimeBaseInit(CW_BTIM1, &BTIM_InitStruct);
	
  BTIM_ITConfig(CW_BTIM1, BTIM_IT_OV, ENABLE);
  BTIM_Cmd(CW_BTIM1, ENABLE);
}

void UART3_IRQHandler()
{ //未用
  // unsigned char tx;
	
  if(USART_GetITStatus(CW_UART3, USART_IT_RC) != RESET)
  {
			USART_ClearITPendingBit(CW_UART3, USART_IT_RC); 
		//	tx = USART_ReceiveData_8bit(CW_UART3);  	
	 }
}
