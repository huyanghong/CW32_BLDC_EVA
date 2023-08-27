
#include "MAIN.h" 

/***********************		PWM	definition   *************************/

#define PWM_AN_PORT									(CW_GPIOB)
#define PWM_AN_PIN									(GPIO_PIN_13)


#define PWM_BN_PORT									(CW_GPIOB)
#define PWM_BN_PIN									(GPIO_PIN_14)

#define PWM_CN_PORT									(CW_GPIOB)
#define PWM_CN_PIN									(GPIO_PIN_15)

//上管调制，下管开关控制, 上管高电平开关管导通
#define PWM_AL_OFF GPIO_WritePin(PWM_AN_PORT,PWM_AN_PIN,GPIO_Pin_SET)
#define PWM_BL_OFF GPIO_WritePin(PWM_BN_PORT,PWM_BN_PIN,GPIO_Pin_SET)
#define PWM_CL_OFF GPIO_WritePin(PWM_CN_PORT,PWM_CN_PIN,GPIO_Pin_SET)

#define PWM_AL_ON GPIO_WritePin(PWM_AN_PORT,PWM_AN_PIN,GPIO_Pin_RESET)
#define PWM_BL_ON GPIO_WritePin(PWM_BN_PORT,PWM_BN_PIN,GPIO_Pin_RESET)
#define PWM_CL_ON GPIO_WritePin(PWM_CN_PORT,PWM_CN_PIN,GPIO_Pin_RESET)

#define PWM_FRQ					   (15000)	    	//PWM频率(HZ)
#define TSPeriod  		3200
//15K
	
#define Maxspeed  4000
#define MinSpeed  800

extern unsigned int TimeCountTemp;

extern unsigned int ErrorCode; //电机运行错误代码
extern unsigned int Cur_step; //当前HALL状态
extern unsigned int SpeedTime; //HALL切换间时间
extern unsigned int Motor_Start_F; //电机启动运行标志
extern unsigned int OutPwm; //输出PWM值

extern unsigned int RealS,RealSpid,TargetS;
extern unsigned int HALLcount,HALLcountS; 

extern unsigned int timecount,timecount1,timecountpid;

extern uint8_t ADC_CR1Array[5]; // 通过更改ADC的CR1寄存器实现ADC通道自动切换
extern uint8_t ADC_Start ;  
extern uint16_t SampleData[6];//内部参数 U反电动势 V反电动势 母线电压 W反电动势 电压 电流 内部调速 外部调速

extern void Commutation(unsigned int step,unsigned int PWM_ON_flag);
extern void Sensorless_MOTOR_START(void); 
extern void SENSORLESS_TIM_Config(void);
extern unsigned char Sensorless_START(void);
extern void MOTOR_STOP(void);

void UPPWM(void);
/////////////////////////
