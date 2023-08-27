#include "control.h"

extern  unsigned char save_flag;
extern  void Flash_Save(void);

void MotorStartCheck(void)
{		
	if(MOTORSTATE==STATEERROR)return;
		
	if(SetSpeed>0&&startflag==1)
	{		
	    MOTORSTATE=STATESTARTPID;		
			Dir=dirflag;			
	}
}
	
void MotorStartPID(void)
{		
	if(MOTORSTATE==STATEERROR)return;
 		
			PID_init();
			PIDcompute(MINSPEED,0);
			HALL_MOTOR_START();

    	TargS1=SetSpeed;MOTORSTATE=STATERUNPID;	
}

void MotorRunPID(void)
{	
	
	if(MOTORSTATE==STATEERROR)return;
	if(SetSpeed==0||startflag==0) //给出停止信号
	{
	 	MOTORSTATE=STATESTOP;	
	}
	
	TargS1=SetSpeed;	
	if(TimeCountPID>=20)//计算实时速度并时行PID运算
			{
			  TimeCountPID=0;
				RealS1=HALLcountTemp*500/MPolePairs;//HALLcount*100*60/6/POLEPAIRS;	
				PIDcompute(TargS1,RealS1);		 
			}
}

void MotorStop(void)
{
	if(MOTORSTATE==STATEERROR)return;
	Motor_Start_F=0;	
	MOTOR_STOP0();

  if(RealS==0) 
				{
					 MOTORSTATE=STATESTARTCHECK;	   //停下来后才切换流程 
				}			
}

void MotorError(void)
{
	MOTOR_STOP0();
	Motor_Start_F=0;
	MOTORSTATE=STATEERROROVER;
}


void MotorErrorOver(void)
{
	unsigned char times=0;	
	
	times=0;
	
	LEDOFF();TimeCountTemp=0;
	while(TimeCountTemp<200);
	while(1) //发生故障，则故障指示灯闪烁。需重启
	{
		
		if(times<ErrorCode)
		{
			if(TimeCountTemp<200){LEDON();}
		  else if(TimeCountTemp<=400)//LED flashing
			{
				LEDOFF();
			}
		  else if(TimeCountTemp>400)
		  {
			  if(times<ErrorCode)
			  {
				 times++;TimeCountTemp=0;
				 if(times>=ErrorCode);
				 else LEDOFF();
			  }
	     }
	  }
		else if(TimeCountTemp<500);
		else {times=0;TimeCountTemp=0;}		
	}
}

void WaitStart(void)
{
	if(SetSpeed==0)MOTORSTATE=STATESTARTCHECK;	
}

void EnDirCheck(void)
{
	unsigned int dd;
	static unsigned char key_dir=0,key_en=0;
	
	if(MOTORSTATE==STATEERROR)return;
	
  if(GPIO_ReadPin(EN_GPIO_PORT,EN_GPIO_PIN)==GPIO_Pin_SET)key_en=0; //使能判断
	else if(key_en==0)
				 { 
						 for(dd=0;dd<500;dd++); //消抖
					   if(GPIO_ReadPin(EN_GPIO_PORT,EN_GPIO_PIN)==GPIO_Pin_RESET)
		         {
							 startflag=1-startflag;
							  if(startflag==0)
									GPIO_WritePin(LEDSTSTO_GPIO_PORT,LEDSTSTO_GPIO_PIN,GPIO_Pin_SET); //启停指示灯灭
								else 
									GPIO_WritePin(LEDSTSTO_GPIO_PORT,LEDSTSTO_GPIO_PIN,GPIO_Pin_RESET); //启停指示灯灭
							 key_en=1;
					    }
					 }	

  if(GPIO_ReadPin(DR_GPIO_PORT,DR_GPIO_PIN)==GPIO_Pin_SET)key_dir=0; //使能判断
	else if(key_dir==0)
				 { 
						 for(dd=0;dd<500;dd++); //消抖
					   if(GPIO_ReadPin(DR_GPIO_PORT,DR_GPIO_PIN)==GPIO_Pin_RESET)
		         {
							 dirflag=1-dirflag;				
               if(dirflag==0)
									GPIO_WritePin(LEDDIR_GPIO_PORT,LEDDIR_GPIO_PIN,GPIO_Pin_RESET); //方向指示灯亮
							else 
									GPIO_WritePin(LEDDIR_GPIO_PORT,LEDDIR_GPIO_PIN,GPIO_Pin_SET); //方向指示灯亮							 
							 if(dirflag!=Dir)
						     MOTORSTATE=STATESTOP;
							 key_dir=1;
					    }
					}					 	 
	
}
