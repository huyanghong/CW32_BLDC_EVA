#include "motorcontrol.h"

//const unsigned int PWM_EN_TAB[6]={0x001,0x001,0x010,0x010,0x100,0x100};//	6步PWM输出使能控制	AB,AC,BC,BA,CA,CB
//const unsigned char STEP_TAB[6]={1,3,2,5,0,4};//{4,0,5,2,3,1};//
//                               //0 1 2 3 4 5
//															 //1 2 3 4 5 6
unsigned int ErrorCode=0; //电机运行错误代码
unsigned int Cur_step=0; //当前HALL状态
unsigned int Motor_Start_F=0; //电机启动运行标志
unsigned int OutPwm=0;  


unsigned char  HALL_Check(void);

//steP,为当前换相序号，OutPwmValue 输出PWM值，PWM_ON_flag=1时启动PWM输出
void Commutation(unsigned int step,unsigned int OutPwmValue,unsigned int PWM_ON_flag)
{
		if(PWM_ON_flag==0) //不启动则关闭输出
    {
			CW_ATIM->CH1CCRA=0;		CW_ATIM->CH2CCRA=0;		CW_ATIM->CH3CCRA=0;	
			ATIM_CtrlPWMOutputs(DISABLE);
		  PWM_AL_OFF;						PWM_BL_OFF;						PWM_CL_OFF;
		  return;
		}
		PWM_AL_OFF;	PWM_BL_OFF;	PWM_CL_OFF;
		
//		switch(step){
//			case 5:
//				CW_ATIM->CH1CCRA=OutPwmValue;	CW_ATIM->CH2CCRA=0;							CW_ATIM->CH3CCRA=0;
//				PWM_AL_OFF;										PWM_BL_ON;											PWM_CL_OFF;	
//				break;
//			case 1:
//				CW_ATIM->CH1CCRA=OutPwmValue;	CW_ATIM->CH2CCRA=0;							CW_ATIM->CH3CCRA=0;
//				PWM_AL_OFF;										PWM_BL_OFF;											PWM_CL_ON;
//				break;
//			case 3:
//				CW_ATIM->CH1CCRA=0;						CW_ATIM->CH2CCRA=OutPwmValue;		CW_ATIM->CH3CCRA=0;
//				PWM_AL_OFF;										PWM_BL_OFF;											PWM_CL_ON;
//				break;
//			case 2:
//				CW_ATIM->CH1CCRA=0;						CW_ATIM->CH2CCRA=OutPwmValue;		CW_ATIM->CH3CCRA=0;
//				PWM_AL_ON;										PWM_BL_OFF;											PWM_CL_OFF;	
//				break;
//			case 6:
//				CW_ATIM->CH1CCRA=0;						CW_ATIM->CH2CCRA=0;							CW_ATIM->CH3CCRA=OutPwmValue;
//				PWM_AL_ON;										PWM_BL_OFF;											PWM_CL_OFF;	
//				break;
//			case 4:
//				CW_ATIM->CH1CCRA=0;						CW_ATIM->CH2CCRA=0;							CW_ATIM->CH3CCRA=OutPwmValue;
//				PWM_AL_OFF;										PWM_BL_ON;											PWM_CL_OFF;	
//				break;
//		}
		switch(step){
			case 2:
				CW_ATIM->CH1CCRA=OutPwmValue;	CW_ATIM->CH2CCRA=0;							CW_ATIM->CH3CCRA=0;
				PWM_AL_OFF;										PWM_BL_ON;											PWM_CL_OFF;	
				break;
			case 6:
				CW_ATIM->CH1CCRA=OutPwmValue;	CW_ATIM->CH2CCRA=0;							CW_ATIM->CH3CCRA=0;
				PWM_AL_OFF;										PWM_BL_OFF;											PWM_CL_ON;
				break;
			case 4:
				CW_ATIM->CH1CCRA=0;						CW_ATIM->CH2CCRA=OutPwmValue;		CW_ATIM->CH3CCRA=0;
				PWM_AL_OFF;										PWM_BL_OFF;											PWM_CL_ON;
				break;
			case 5:
				CW_ATIM->CH1CCRA=0;						CW_ATIM->CH2CCRA=OutPwmValue;		CW_ATIM->CH3CCRA=0;
				PWM_AL_ON;										PWM_BL_OFF;											PWM_CL_OFF;	
				break;
			case 1:
				CW_ATIM->CH1CCRA=0;						CW_ATIM->CH2CCRA=0;							CW_ATIM->CH3CCRA=OutPwmValue;
				PWM_AL_ON;										PWM_BL_OFF;											PWM_CL_OFF;	
				break;
			case 3:
				CW_ATIM->CH1CCRA=0;						CW_ATIM->CH2CCRA=0;							CW_ATIM->CH3CCRA=OutPwmValue;
				PWM_AL_OFF;										PWM_BL_ON;											PWM_CL_OFF;	
				break;
		}
		
		//输出上桥
//		if(step==0||step==1){	 CW_ATIM->CH1CCRA=OutPwmValue;CW_ATIM->CH2CCRA=0;CW_ATIM->CH3CCRA=0;	} //0:AB; 1:AC
//		if(step==2||step==3){	 CW_ATIM->CH1CCRA=0;CW_ATIM->CH2CCRA=OutPwmValue;CW_ATIM->CH3CCRA=0;	} //2:BC; 3:BA
//		if(step==4||step==5){	 CW_ATIM->CH1CCRA=0;CW_ATIM->CH2CCRA=0;CW_ATIM->CH3CCRA=OutPwmValue;	} //4:CA; 5:CB
//	
//		//输出下桥
//		if(step==0||step==5){PWM_AL_OFF;	PWM_CL_OFF;PWM_BL_ON;} //AB CB ; B下桥导通
// 		else if(step==1||step==2){	PWM_AL_OFF;	PWM_BL_OFF;	PWM_CL_ON;}//AC BC; C下桥导通
//		else if(step==3||step==4){	PWM_BL_OFF;	PWM_CL_OFF;	PWM_AL_ON;}//BA CA; A下桥导通
		
		ATIM_CtrlPWMOutputs(ENABLE);	 //输出有效
}

void HALL_MOTOR_START(void)
{ 	
	 unsigned int x;  
	
	 x=HALL_Check();
	 if(x==0||x==7) {x=1;} //霍尔异常时，输出一项，使电机先转起来
	 Commutation(x,OutPwm,Motor_Start_F);
}

unsigned char  HALL_Check(void)
{
	//读取霍尔状态
	
  static unsigned char hallerrnum=0; 
  unsigned char x=0;
  
  if(PA15_GETVALUE()!=0)x=1;
  if(PB03_GETVALUE()!=0)x|=0x2;
  if(PA02_GETVALUE()!=0)x|=0x4;
  if(x==0||x==7)
    {
				hallerrnum++;
		if(hallerrnum>=10)
		{hallerrnum=10;ErrorCode=2;}
		}
   else hallerrnum=0;
  return x;
}



