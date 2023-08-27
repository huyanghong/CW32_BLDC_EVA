#include "motorcontrol.h"
#include "sensorlessP.h"

#define RISING 1
#define FALLING 2

const unsigned char TAB_RFling[2][6]={
{RISING,FALLING,RISING,FALLING,RISING,FALLING},
{FALLING,RISING,FALLING,RISING,FALLING,RISING}};

const unsigned char TAB_BEMFChannel[2][6]={
{3,1,0,3,1,0},
{3,1,0,3,1,0} 
};

unsigned int TimeCountCompuSpeed=0,TimeCountTemp=0,TimeCountSTDelay=0,TimeCount6=0;

//AD用
uint16_t SampleData[6];//U反电动势 V反电动势 母线电压 W反电动势 电流 内部调速
uint8_t ADC_CR1Array[5] = {0x81, 0x83, 0x85, 0x86, 0x88}; // 通过更改ADC的CR1寄存器实现ADC通道自动切换
uint8_t ADC_Start = 0x01;  

//无感启动用
#define STCount 15//过零检测连续有效次数

unsigned char StOk=0;//启动成功标志
unsigned int StCountComm=0;//过零检测计数
unsigned int NumCoun=0;
unsigned char FFlag=0;
unsigned char QDPwm=10;
unsigned int TimeAli=100;

unsigned int StepTime=0;
unsigned char Sta=0;

unsigned char Tonoroff=0;

unsigned int  bHallStartStep1;
unsigned char RisingFalling;

unsigned int BEMFConvertedValue;

unsigned int ErrorCode=0; //电机运行错误代码
unsigned int Cur_step=0; //当前HALL状态
unsigned int Motor_Start_F=0; //电机启动运行标志
unsigned int OutPwm=0;  

unsigned int STEP_last=0;

unsigned int RealS=0;
unsigned int HALLcount=0; 

unsigned int timecount=0,timecount1=0;

unsigned int Dir=1;

void Commutation(unsigned int step,unsigned int PWM_ON_flag);
void SENSORLESS_TIM_Config(void);
unsigned char Sensorless_START(void);
void ADCS_chuli(void);
void MOTOR_STOP(void);

//steP,为当前换相序号，OutPwmValue 输出PWM值，PWM_ON_flag=1时启动PWM输出
void Commutation(unsigned int step,unsigned int PWM_ON_flag)
{
			if(PWM_ON_flag==0) //不启动则关闭输出
			{
				CW_ATIM->CH1CCRA=0;CW_ATIM->CH2CCRA=0;CW_ATIM->CH3CCRA=0;	
				PWM_AL_OFF;	PWM_BL_OFF;	PWM_CL_OFF;
				CW_ATIM->CH4CCR=TSPeriod-800;					
				return;
			}	 		
		
			if(step==0||step==5){	PWM_AL_OFF;	PWM_CL_OFF;}//AB
			else if(step==1||step==2){	PWM_AL_OFF;	PWM_BL_OFF;	}//AC
			else if(step==3||step==4){	PWM_BL_OFF;	PWM_CL_OFF;	}//BA
			
			if(step==0||step==1){	 CW_ATIM->CH2CCRA=0;CW_ATIM->CH3CCRA=0;CW_ATIM->CH1CCRA=OutPwm;	}
			if(step==2||step==3){	 CW_ATIM->CH1CCRA=0;CW_ATIM->CH3CCRA=0;CW_ATIM->CH2CCRA=OutPwm;	}
			if(step==4||step==5){	 CW_ATIM->CH1CCRA=0;CW_ATIM->CH2CCRA=0;CW_ATIM->CH3CCRA=OutPwm;	}
			
			
			if(step==0||step==5){PWM_BL_ON;}//AB
			else if(step==1||step==2){PWM_CL_ON;}//AC
			else if(step==3||step==4){PWM_AL_ON;}//BA
		
		  STEP_last=step;
		
		  if(OutPwm>=1200&&Tonoroff==0)	{Tonoroff=1;CW_ATIM->CH4CCR=300;}	
			else	if(OutPwm<1200&&Tonoroff==1)	{Tonoroff=0;CW_ATIM->CH4CCR=TSPeriod-400;	}

			
		  StepTime=BTIM_GetCounter(CW_BTIM2);
		  BTIM_SetCounter(CW_BTIM2,0);
			
			if(StOk==0)
			BTIM_SetAutoreload(CW_BTIM3,StepTime/5);	
			else 
			BTIM_SetAutoreload(CW_BTIM3,StepTime/3);	
			
			BTIM_SetCounter(CW_BTIM3,0);
			BTIM_Cmd(CW_BTIM3, ENABLE);
			//启动退磁 
			Sta=1;//退磁状态 
	
			HALLcount++;	  	
}

void UPPWM(void)
{	 
	if(STEP_last==0||STEP_last==1){	 CW_ATIM->CH2CCRA=0;CW_ATIM->CH3CCRA=0; CW_ATIM->CH1CCRA=OutPwm;	}
	if(STEP_last==2||STEP_last==3){	 CW_ATIM->CH1CCRA=0;CW_ATIM->CH3CCRA=0;CW_ATIM->CH2CCRA=OutPwm;	}
	if(STEP_last==4||STEP_last==5){	 CW_ATIM->CH1CCRA=0;CW_ATIM->CH2CCRA=0;CW_ATIM->CH3CCRA=OutPwm;	}
 	
		  if(OutPwm>=1600&&Tonoroff==0)	{Tonoroff=1;	CW_ATIM->CH4CCR=300;}	 
			else	if(OutPwm<1600&&Tonoroff==1)	{Tonoroff=0;CW_ATIM->CH4CCR=TSPeriod-400;		} 
	
}

void Sensorless_MOTOR_START(void)
{ 	 
	 unsigned char coun=0;	  
	 
			HALLcount=0;  
			QDPwm=10;//启动占空比
			do{
				if(Sensorless_START()==0){coun++;QDPwm+=5;}		
				else break;
			}while(coun<5&&ErrorCode==0);
			if(coun>=5&&ErrorCode==0) 
					{ErrorCode=3;}	//启动失败	
}


void MOTOR_STOP(void)
{
	 Motor_Start_F=0;StOk=0;
	 Commutation(0,0);Sta=0;
}


void SENSORLESS_TIM_Config(void)
{
	BTIM_TimeBaseInitTypeDef BTIM_InitStruct;
	
	__RCC_BTIM_CLK_ENABLE();
	
	__disable_irq(); 
  NVIC_EnableIRQ(BTIM3_IRQn); 
  __enable_irq();
	
	BTIM_InitStruct.BTIM_Mode = BTIM_Mode_TIMER;
  BTIM_InitStruct.BTIM_OPMode = BTIM_OPMode_Repetitive;
  BTIM_InitStruct.BTIM_Period = 65530;
  BTIM_InitStruct.BTIM_Prescaler = BTIM_PRS_DIV8;
  BTIM_TimeBaseInit(CW_BTIM2, &BTIM_InitStruct);
  BTIM_TimeBaseInit(CW_BTIM3, &BTIM_InitStruct);
	
  BTIM_Cmd(CW_BTIM2, ENABLE);
	BTIM_Cmd(CW_BTIM3, DISABLE);
  BTIM_ITConfig(CW_BTIM3, BTIM_IT_OV, ENABLE);	
}


void BTIM3_IRQHandler(void)
{
	if(BTIM_GetITStatus(CW_BTIM3, BTIM_IT_OV))
  {  
    BTIM_ClearITPendingBit(CW_BTIM3, BTIM_IT_OV);
		
		if(Sta==1)//退磁结束
		{
      Sta=2;
			BTIM_Cmd(CW_BTIM3, DISABLE);
		}
		else if(Sta==3&&StOk==1)
		{//换相时刻。			
	    BTIM_Cmd(CW_BTIM3, DISABLE);
			
			if(Dir==1)
			 {
					bHallStartStep1++;
					if(bHallStartStep1>=6)bHallStartStep1=0;
			 }
			 else 
				{
					if(bHallStartStep1==0)bHallStartStep1=5;
					else bHallStartStep1--;
				}
				
	      Commutation(bHallStartStep1,Motor_Start_F); 
			}
	}
}

void DMACH1_IRQHandler(void)
{
  /* USER CODE BEGIN */
  if( DMA_GetITStatus(DMA_IT_TC1) )
  {
    DMA_ClearITPendingBit(DMA_IT_TC1);
  
    CW_DMA->ICR_f.TC1 = 0x00;    // 清中断标志
    CW_ATIM->ICR  = 0x00;        // 清ATIM的中断标志
    CW_ADC->ICR = 0x00;          // 请ADC的中断标志
    CW_ADC->CR1 = 0x80;     // AdcIn = ch0
    //------------------------------------------------
    // DMA.CH1 : ADC_Result -> RAM
    CW_DMACHANNEL1->CNT = 0x10006; //传6个
    CW_DMACHANNEL1->SRCADDR = (uint32_t)(&CW_ADC->RESULT0);
    CW_DMACHANNEL1->DSTADDR = (uint32_t)(&SampleData[0]);
    CW_DMACHANNEL1->CSR_f.EN = 1;


    //------------------------------------------------
    // DMA.CH2 : RAM -> ADC_CR1
    CW_DMACHANNEL2->CNT = 0x10005; //传5个
    CW_DMACHANNEL2->SRCADDR = (uint32_t)(&ADC_CR1Array[0]);
    CW_DMACHANNEL2->DSTADDR = (uint32_t)(&CW_ADC->CR1);
    CW_DMACHANNEL2->CSR_f.EN = 1;

    //------------------------------------------------
    // DMA.CH3 : RAM -> ADC_START
    CW_DMACHANNEL3->CNT = 0x10005; //传5个
    CW_DMACHANNEL3->SRCADDR = (uint32_t)(&ADC_Start);
    CW_DMACHANNEL3->DSTADDR = (uint32_t)(&CW_ADC->START);
    CW_DMACHANNEL3->CSR_f.EN = 1;
    
    //------------------------------------------------
    // DMA.CH4 : RAM -> ADC_START
    CW_DMACHANNEL4->CNT = 0x10001; //传1个
    CW_DMACHANNEL4->SRCADDR = (uint32_t)(&ADC_Start);
    CW_DMACHANNEL4->DSTADDR = (uint32_t)(&CW_ADC->START);
    CW_DMACHANNEL4->CSR_f.EN = 1;
			
		ADCS_chuli();	
	}
}

void ADCS_chuli(void)
{	
	static unsigned char cou=0;
	unsigned char hx=0;
	unsigned int thre=0;

		if(Sta!=2)return;
				
		BEMFConvertedValue =SampleData[TAB_BEMFChannel[Dir][bHallStartStep1]];
		RisingFalling=TAB_RFling[Dir][bHallStartStep1];
			
		if(Tonoroff==0) 	thre=50;//30;//248;  //OFF时刻采集。0.2V阀值248
		else 		thre=SampleData[2];			
		
		if(RisingFalling==FALLING)//下降沿
		{			
			if(BEMFConvertedValue<thre)
			{
				cou++;
				if(cou>=2)
				{
					cou=0;	
				  Sta=3;					
					
					StCountComm++;//正确检测到第三相反电动势		
					FFlag=1;
					hx=1;
				}
			}
			else { cou=0;}
	  }
		else if(RisingFalling==RISING)
		{
			if(BEMFConvertedValue>thre)
			{
				cou++;
				if(cou>=2)
				{
					cou=0;	
					Sta=3;			
					
					StCountComm++;//正确检测到第三相反电动势		
					FFlag=1;
					hx=1;
				}
			}
			else {cou=0;}
	  }	
		
		if(StCountComm>=STCount&&StOk==0)//连续检测到固定数量的过零时，认为启动成功
		{
				StOk=1;
		}	
 
    if(StOk==1&&hx==1)
    {
			    hx=0;
										
						BTIM_SetAutoreload(CW_BTIM3,StepTime/5);								
					  BTIM_SetCounter(CW_BTIM3,0);
						BTIM_Cmd(CW_BTIM3, ENABLE);
		}  
}


unsigned char Sensorless_START(void)
{
	unsigned int Com_time=0;			
		
	 	BTIM_SetCounter(CW_BTIM2,0);
		
		Tonoroff=0;		
		StOk=2;
		Sta=0;
			
		OutPwm=TSPeriod*QDPwm/100;
	  Motor_Start_F=1;
		UPPWM();
	  Commutation(bHallStartStep1,Motor_Start_F); 
				
	  TimeCountTemp=0;
	  while(TimeCountTemp<40);
		
		Com_time=0;
		StCountComm=0;
	  FFlag=0;
		
	  StepTime=10;
	  StOk=0;	
		do{						
			if(Dir==1) 
			{
					bHallStartStep1++;
					if(bHallStartStep1>=6)bHallStartStep1=0;
			}
			else 
				{
					if(bHallStartStep1==0)bHallStartStep1=5;
					else bHallStartStep1--;
				}
				
			FFlag=0;
			if(StOk==0)	
			{
	       Commutation(bHallStartStep1,Motor_Start_F); 
			}				   
			
			TimeCountTemp=0;
			while(TimeCountTemp<RAMP_TABLE[Com_time])
			{
				if(FFlag==1||StOk==1)break;
			}

			Com_time++;
			OutPwm+=10;
	}while(StOk==0&&Com_time<60&&ErrorCode==0); 
		
			
		if(StOk==0)
	  {
			Motor_Start_F=0;
			Commutation(0,0); 
			return 0;			
		}
		
		if(ErrorCode!=0) 
			return ErrorCode;
		else	 
			return 1;
}


