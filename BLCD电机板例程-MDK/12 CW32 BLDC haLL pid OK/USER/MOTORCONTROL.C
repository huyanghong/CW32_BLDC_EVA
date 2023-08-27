#include "motorcontrol.h"

//const unsigned int PWM_EN_TAB[6]={0x001,0x001,0x010,0x010,0x100,0x100};//	6步PWM输出使能控制	AB,AC,BC,BA,CA,CB
//const unsigned char STEP_TAB[6]={1,3,2,5,0,4};//{4,0,5,2,3,1};//

//unsigned int Cur_step=0; //当前HALL状态
//unsigned int Motor_Start_F=0; //电机启动运行标志
//unsigned int OutPwm=0;  


////steP,为当前步，OutPwmValue 输出PWM值，PWM_ON_flag=1时打开PWM输出
//void Commutation(unsigned int step,unsigned int OutPwmValue,unsigned int PWM_ON_flag)
//{
//	if(PWM_ON_flag==0)
//    {
//			CW_ATIM->CH1CCRA=0;CW_ATIM->CH2CCRA=0;CW_ATIM->CH3CCRA=0;	
//			ATIM_CtrlPWMOutputs(DISABLE);
//		  PWM_AL_OFF;	PWM_BL_OFF;	PWM_CL_OFF;
//		  return;
//		}
//	 
//  PWM_AL_OFF;	PWM_BL_OFF;	PWM_CL_OFF;
//		
//	if(step==0||step==1){	 CW_ATIM->CH1CCRA=OutPwmValue;CW_ATIM->CH2CCRA=0;CW_ATIM->CH3CCRA=0;	}
//	if(step==2||step==3){	 CW_ATIM->CH1CCRA=0;CW_ATIM->CH2CCRA=OutPwmValue;CW_ATIM->CH3CCRA=0;	}
//	if(step==4||step==5){	 CW_ATIM->CH1CCRA=0;CW_ATIM->CH2CCRA=0;CW_ATIM->CH3CCRA=OutPwmValue;	}
//	
//	if(step==0||step==5){PWM_AL_OFF;	PWM_CL_OFF;PWM_BL_ON;}//AB
//  else if(step==1||step==2){	PWM_AL_OFF;	PWM_BL_OFF;	PWM_CL_ON;}//AC
//	else if(step==3||step==4){	PWM_BL_OFF;	PWM_CL_OFF;	PWM_AL_ON;}//BA
//		
//	ATIM_CtrlPWMOutputs(ENABLE);	 
//}

//void HALL_MOTOR_START(void)
//{ 	
//	unsigned int x;  
//	Cur_step=STEP_TAB[(CW_GPIOA->IDR&0x0007)-1];	  
//	Commutation(Cur_step,OutPwm,Motor_Start_F);
//}
