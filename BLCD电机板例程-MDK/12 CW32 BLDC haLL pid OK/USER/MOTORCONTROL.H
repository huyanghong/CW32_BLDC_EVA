
#include "MAIN.h" 

///***********************		PWM	definition   *************************/

//#define PWM_AN_PORT									(CW_GPIOB)
//#define PWM_AN_PIN									(GPIO_PIN_13)

//#define PWM_BN_PORT									(CW_GPIOB)
//#define PWM_BN_PIN									(GPIO_PIN_14)

//#define PWM_CN_PORT									(CW_GPIOB)
//#define PWM_CN_PIN									(GPIO_PIN_15)

////上管调制，下管开关控制, 上下管高电平开关管导通
//#define PWM_AL_OFF GPIO_WritePin(PWM_AN_PORT,PWM_AN_PIN,GPIO_Pin_SET)
//#define PWM_BL_OFF GPIO_WritePin(PWM_BN_PORT,PWM_BN_PIN,GPIO_Pin_SET)
//#define PWM_CL_OFF GPIO_WritePin(PWM_CN_PORT,PWM_CN_PIN,GPIO_Pin_SET)

//#define PWM_AL_ON GPIO_WritePin(PWM_AN_PORT,PWM_AN_PIN,GPIO_Pin_RESET)
//#define PWM_BL_ON GPIO_WritePin(PWM_BN_PORT,PWM_BN_PIN,GPIO_Pin_RESET)
//#define PWM_CL_ON GPIO_WritePin(PWM_CN_PORT,PWM_CN_PIN,GPIO_Pin_RESET)

//#define PWM_FRQ					   (20000)	    	//PWM频率(HZ)
#define TS  		7200
////(unsigned short)(72*1000*1000/PWM_FRQ)	 //PWM_TIM的周期
//	
//#define OUTMAXPWM  TS*1.05 
//#define OUTMINPWM  TS*0.15 

//extern const unsigned char STEP_TAB[6];

//extern unsigned int ErrorCode; //电机运行错误代码
//extern unsigned int Cur_step; //当前HALL状态
//extern unsigned int SpeedTime; //HALL切换间时间
//extern unsigned int Motor_Start_F; //电机启动运行标志
//extern unsigned int OutPwm; //输出PWM值


//extern void Commutation(unsigned int step,unsigned int OutPwmValue,unsigned int PWM_ON_flag);

//extern void HALL_MOTOR_START(void);
/////////////////////////
