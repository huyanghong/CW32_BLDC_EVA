/*
2页菜单：1：设定速度、实时速度； 2：母线电压、电流显示。
 KEY1：按键切换速度及电压电流菜单
 KEY2：方向切换按键。电机运行时也可以切换。LED2亮灭表示不同的方向。
 KEY3：电机启停控制，LED3灯亮时，电机处于启动状态； LED3灯来时，电机处于停止状态； 
 电位器调速，切换到速度菜单，可以显示当前设定转速。
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
	RCC_Configuration(); //时钟配置
	GPIOInit(); //GPIO配置
	LEDON();		
	ADC_Configuration(); //AD配置
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
	
	
	
	DIin=0x500;	//偏置电流设定 0.9V-1.1V中间，认为正确
	if(SampleData[2]>1117&&SampleData[2]<=1500)DIin=SampleData[2];	
	SampleVI();	//采集电压电流
			
	while(1)
	{		
			if(TimeCountCompuSpeed>20) //20MS采集一次速度设定值,并检查HALL是否有误
	       {
					 TimeCountCompuSpeed=0;
					 SampleSpeed();
					 HALL_Check();
	       }
				 
			if(TimeCountVI>=200)//100MS计算一次电压电流,并作一次实时速度计算
		 {
			 TimeCountVI=0;
			 SampleVI();
			 			  
			 RealS=hte*50/MPolePairs;//HALLcount1*10*60/6/MPolePairs; 100MS存储一次霍尔边沿数							
			 if(MOTORSTATE==STATESTARTPID||MOTORSTATE==STATERUNPID)
				 {
						if(RealS<120)
						{
							 DZCount++;
							 if(DZCount>=10)//2S保护
							 { 								
									DZCount=0;ErrorCode=7;//堵转
							 }
						}
						else DZCount=0;
					}			
		 }		
		
		  if(ErrorCode!=0&&MOTORSTATE!=STATEERROR&&MOTORSTATE!=STATEERROROVER)//发生故障
		   {  
				  MOTORSTATE=STATEERROR;
    	}		 				
	 
	
			if(GPIO_ReadPin(CW_GPIOB,GPIO_PIN_4)==GPIO_Pin_SET)key_1=0; //MENU 切换
			else if(key_1==0)
				 { 		
					     Menu=1-Menu;
							 key_1=1;		
							       if(Menu==0)
											{
												sprintf(temp_buff1,"Set :%4d RPM  ",SetSpeed);  //输出显示占空比
												sprintf(temp_buff,"Real:%4d RPM  ",RealS);	//显示电机转速
											}
											else
											{
												sprintf(temp_buff1,"Vbus :%d.%d V  ",CanshuV/10,CanshuV%10);  //输出显示占空比
												sprintf(temp_buff,"I(MA):%d      ",CanshuI);	//显示电机转速
											}
											I2C_OLED_ShowString(0,0,temp_buff1);
											I2C_OLED_ShowString(0,15,temp_buff);	
											I2C_OLED_UPdata();				 
					}	
			
			if(TimeCountkey>=500) //500ms刷新显示
			{
				TimeCountkey=0;
									if(Menu==0)
											{
												sprintf(temp_buff1,"Set :%4d RPM  ",SetSpeed);  //输出显示占空比
												sprintf(temp_buff,"Real:%4d RPM  ",RealS);	//显示电机转速
											}
											else
											{
												sprintf(temp_buff1,"Vbus :%d.%d V  ",CanshuV/10,CanshuV%10);  //输出显示占空比
												sprintf(temp_buff,"I(MA):%d      ",CanshuI);	//显示电机转速
											}
				I2C_OLED_ShowString(0,0,temp_buff1);
				I2C_OLED_ShowString(0,15,temp_buff);	
				I2C_OLED_UPdata();
			}
			
		//状态机	
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


//1ms 定时器
void BTIM1_IRQHandler(void)
{	
  /* USER CODE BEGIN */
 if(BTIM_GetITStatus(CW_BTIM1, BTIM_IT_OV))
  {  //1MS timer
    BTIM_ClearITPendingBit(CW_BTIM1, BTIM_IT_OV);		
		
		TimeCountTemp++;			
		TimeCountCompuSpeed++; //计算速度
		TimeCountRealSpd++; //瞬时速度
		if(TimeCountAvgSpd>=200){hte=HALLcount1;HALLcount1=0;TimeCountAvgSpd=0;}//用于显示的速度值
		if(TimeCountRealSpd>=20){HALLcountTemp=HALLcount;HALLcount=0;TimeCountRealSpd=0;}
	  TimeCountPID++; //PID
		TimeCountAvgSpd++;	//平均速度
		TimeCountVI++;//计算电压电流
		
		TimeCountkey++;
	}
  /* USER CODE END */
}


