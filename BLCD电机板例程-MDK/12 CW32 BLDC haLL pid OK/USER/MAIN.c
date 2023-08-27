/*
2ҳ�˵���1���趨�ٶȡ�ʵʱ�ٶȣ� 2��ĸ�ߵ�ѹ��������ʾ��
 KEY1�������л��ٶȼ���ѹ�����˵�
 KEY2�������л��������������ʱҲ�����л���LED2�����ʾ��ͬ�ķ���
 KEY3�������ͣ���ƣ�LED3����ʱ�������������״̬�� LED3����ʱ���������ֹͣ״̬�� 
 ��λ�����٣��л����ٶȲ˵���������ʾ��ǰ�趨ת�١�
 Designed by Cache.lee from UET company
 Tel:18129915976
*/
#include "main.h"
#include "motor.h"
#include "globalv.h"
#include "init.h"
#include "pid.h"
#include "compu.h"
#include "control.h"
#include "OLED_I2C.h"

unsigned int Menu=0;


void PowerOnDelay(void)
{
	unsigned int i;
		
	for(i=0;i<60000;i++);
	for(i=0;i<60000;i++);
	for(i=0;i<60000;i++);
	for(i=0;i<60000;i++);
	for(i=0;i<60000;i++);
	for(i=0;i<60000;i++);
	
		KKN=(MAXSPEED-MINSPEED);
	  KKN=KKN/(NMAXVD-NMINVD);
}

unsigned int hte=0;
int main()
{	
	unsigned char DZCount=0;	
  char temp_buff[100],temp_buff1[50];
	
	unsigned char key_1=0;
	
	PowerOnDelay();
	RCC_Configuration(); //ʱ������
	GPIOInit(); //GPIO����
	LEDON();		
	ADC_Configuration(); //AD����
	PWMtimer_init();
	BTIM_init();		
	Halltimer_init();
		
	PWM_AL_ON;
	PWM_BL_ON;
	PWM_CL_ON;
	TimeCountTemp=0;
	while(TimeCountTemp<50);
	TimeCountTemp=0;
	PWM_AL_OFF;
	PWM_BL_OFF;
	PWM_CL_OFF;	 		
	
	I2C_init();			//OLED init
	I2C_OLED_Init();	
	I2C_OLED_Clear(1);
	I2C_OLED_UPdata(); 
	
	
	
	DIin=0x500;	//ƫ�õ����趨 0.9V-1.1V�м䣬��Ϊ��ȷ
	if(SampleData[2]>1117&&SampleData[2]<=1500)DIin=SampleData[2];	
	SampleVI();	//�ɼ���ѹ����
			
	while(1)
	{		
			if(TimeCountCompuSpeed>20) //20MS�ɼ�һ���ٶ��趨ֵ,�����HALL�Ƿ�����
	       {
					 TimeCountCompuSpeed=0;
					 SampleSpeed();
					 HALL_Check();
	       }
				 
			if(TimeCountVI>=200)//100MS����һ�ε�ѹ����,����һ��ʵʱ�ٶȼ���
		 {
			 TimeCountVI=0;
			 SampleVI();
			 			  
			 RealS=hte*50/MPolePairs;//HALLcount1*10*60/6/MPolePairs; 100MS�洢һ�λ���������							
			 if(MOTORSTATE==STATESTARTPID||MOTORSTATE==STATERUNPID)
				 {
						if(RealS<120)
						{
							 DZCount++;
							 if(DZCount>=10)//2S����
							 { 								
									DZCount=0;ErrorCode=7;//��ת
							 }
						}
						else DZCount=0;
					}			
		 }		
		
		  if(ErrorCode!=0&&MOTORSTATE!=STATEERROR&&MOTORSTATE!=STATEERROROVER)//��������
		   {  
				  MOTORSTATE=STATEERROR;
    	}		 				
	 
	
			if(GPIO_ReadPin(CW_GPIOB,GPIO_PIN_4)==GPIO_Pin_SET)key_1=0; //MENU �л�
			else if(key_1==0)
				 { 		
					     Menu=1-Menu;
							 key_1=1;		
							       if(Menu==0)
											{
												sprintf(temp_buff1,"Set :%4d RPM  ",SetSpeed);  //�����ʾռ�ձ�
												sprintf(temp_buff,"Real:%4d RPM  ",RealS);	//��ʾ���ת��
											}
											else
											{
												sprintf(temp_buff1,"Vbus :%d.%d V  ",CanshuV/10,CanshuV%10);  //�����ʾռ�ձ�
												sprintf(temp_buff,"I(MA):%d      ",CanshuI);	//��ʾ���ת��
											}
											I2C_OLED_ShowString(0,0,temp_buff1);
											I2C_OLED_ShowString(0,15,temp_buff);	
											I2C_OLED_UPdata();				 
					}	
			
			if(TimeCountkey>=500) //500msˢ����ʾ
			{
				TimeCountkey=0;
									if(Menu==0)
											{
												sprintf(temp_buff1,"Set :%4d RPM  ",SetSpeed);  //�����ʾռ�ձ�
												sprintf(temp_buff,"Real:%4d RPM  ",RealS);	//��ʾ���ת��
											}
											else
											{
												sprintf(temp_buff1,"Vbus :%d.%d V  ",CanshuV/10,CanshuV%10);  //�����ʾռ�ձ�
												sprintf(temp_buff,"I(MA):%d      ",CanshuI);	//��ʾ���ת��
											}
				I2C_OLED_ShowString(0,0,temp_buff1);
				I2C_OLED_ShowString(0,15,temp_buff);	
				I2C_OLED_UPdata();
			}
			
		//״̬��	
		 switch(MOTORSTATE)
		 {
			 case STATESTARTCHECK:  	
						EnDirCheck();				 
				    MotorStartCheck();			      
						break;
			 case STATESTARTPID:
						MotorStartPID();	
			      break;			
			 case STATERUNPID: 
						MotorRunPID();
						EnDirCheck();
						break;
			 case STATESTOP: MotorStop();break;
			 case STATEERROR: MotorError();break;
			 case STATEERROROVER: MotorErrorOver();break;
	   }			   			
	}
}


//1ms ��ʱ��
void BTIM1_IRQHandler(void)
{	
  /* USER CODE BEGIN */
 if(BTIM_GetITStatus(CW_BTIM1, BTIM_IT_OV))
  {  //1MS timer
    BTIM_ClearITPendingBit(CW_BTIM1, BTIM_IT_OV);		
		
		TimeCountTemp++;			
		TimeCountCompuSpeed++; //�����ٶ�
		TimeCountRealSpd++; //˲ʱ�ٶ�
		if(TimeCountAvgSpd>=200){hte=HALLcount1;HALLcount1=0;TimeCountAvgSpd=0;}//������ʾ���ٶ�ֵ
		if(TimeCountRealSpd>=20){HALLcountTemp=HALLcount;HALLcount=0;TimeCountRealSpd=0;}
	  TimeCountPID++; //PID
		TimeCountAvgSpd++;	//ƽ���ٶ�
		TimeCountVI++;//�����ѹ����
		
		TimeCountkey++;
	}
  /* USER CODE END */
}


