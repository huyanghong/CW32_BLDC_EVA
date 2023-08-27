#include "main.h"

//LED灯
#define LED_GPIO_PIN        GPIO_PIN_13                  
#define LED_GPIO_PORT      	CW_GPIOC            
#define LEDON()	  {GPIO_WritePin(LED_GPIO_PORT,LED_GPIO_PIN,GPIO_Pin_RESET);}
#define LEDOFF()  {GPIO_WritePin(LED_GPIO_PORT,LED_GPIO_PIN,GPIO_Pin_SET);}

//方向控制按键IO定义
#define DR_GPIO_PIN        	GPIO_PIN_5                  
#define DR_GPIO_PORT      	CW_GPIOB               

//使能控制端按键IO定义
#define EN_GPIO_PIN        	GPIO_PIN_10                  
#define EN_GPIO_PORT      	CW_GPIOB    

//方向指示灯IO定义
#define LEDDIR_GPIO_PIN        	GPIO_PIN_11                  
#define LEDDIR_GPIO_PORT      	CW_GPIOA               

//启停指示灯IO定义
#define LEDSTSTO_GPIO_PIN        	GPIO_PIN_11                  
#define LEDSTSTO_GPIO_PORT      	CW_GPIOB  


//六路PWM IO定义
#define PWM_AP_PORT									(CW_GPIOA)
#define PWM_AP_PIN									(GPIO_PIN_8)
#define PWM_AN_PORT									(CW_GPIOB)
#define PWM_AN_PIN									(GPIO_PIN_13)

#define PWM_BP_PORT									(CW_GPIOA)
#define PWM_BP_PIN									(GPIO_PIN_9)
#define PWM_BN_PORT									(CW_GPIOB)
#define PWM_BN_PIN									(GPIO_PIN_14)

#define PWM_CP_PORT									(CW_GPIOA)
#define PWM_CP_PIN									(GPIO_PIN_10)
#define PWM_CN_PORT									(CW_GPIOB)
#define PWM_CN_PIN									(GPIO_PIN_15)


#define PWM_AH_OFF GPIO_WritePin(PWM_AP_PORT,PWM_AP_PIN,GPIO_Pin_RESET) 
#define PWM_BH_OFF GPIO_WritePin(PWM_BP_PORT,PWM_BP_PIN,GPIO_Pin_RESET)
#define PWM_CH_OFF GPIO_WritePin(PWM_CP_PORT,PWM_CP_PIN,GPIO_Pin_RESET)

#define PWM_AH_ON GPIO_WritePin(PWM_AP_PORT,PWM_AP_PIN,GPIO_Pin_SET) 
#define PWM_BH_ON GPIO_WritePin(PWM_BP_PORT,PWM_BP_PIN,GPIO_Pin_SET) 
#define PWM_CH_ON GPIO_WritePin(PWM_CP_PORT,PWM_CP_PIN,GPIO_Pin_SET) 

//上管调制，下管开关控制, 上高电平下低电平
#define PWM_AL_OFF GPIO_WritePin(PWM_AN_PORT,PWM_AN_PIN,GPIO_Pin_SET) 
#define PWM_BL_OFF GPIO_WritePin(PWM_BN_PORT,PWM_BN_PIN,GPIO_Pin_SET) 
#define PWM_CL_OFF GPIO_WritePin(PWM_CN_PORT,PWM_CN_PIN,GPIO_Pin_SET) 

#define PWM_AL_ON GPIO_WritePin(PWM_AN_PORT,PWM_AN_PIN,GPIO_Pin_RESET)
#define PWM_BL_ON GPIO_WritePin(PWM_BN_PORT,PWM_BN_PIN,GPIO_Pin_RESET)
#define PWM_CL_ON GPIO_WritePin(PWM_CN_PORT,PWM_CN_PIN,GPIO_Pin_RESET)


//BKIN端口定义
#define BKIN_GPIO_PIN        	GPIO_Pin_12                  
#define BKIN_GPIO_PORT      	GPIOB   


//载波频率20k
#define PWM_PERIOD 2399 
// (48000000 / 20000 ) - 1; 
#define OnepercentPWM (PWM_PERIOD+1)/100

#define STATESTARTCHECK 0
#define STATESTARTPID 1
#define STATERUNPID 2
#define STATESTOP 3
#define STATEERROR 5
#define STATEERROROVER 6

//电位器
#define NMAXVD 3971
#define NMINVD 500
//0.4V~3.2V进行调节

#define RS (float)0.1
//取样电阻
#define AP 5
//放大倍数
#define Ikvalue (float)1200/(float)AP/RS
//电流计算常数 内部参考1.2V

#define NumErr 15
//过流过压错误认定次数

//#define VSh 60
//过压值设定
#define RV1 5.1
#define RV2	200
//输入电压与总线电压之比：为RV1:(RV1+RV2)

#define Kvvalue (float)1.2/RV1*(RV1+RV2)


extern uint8_t ADC_CR1Array[4] ; // 通过更改ADC的CR1寄存器实现ADC通道自动切换
extern uint8_t ADC_Start ;    

extern unsigned int TimeCountCompuSpeed,TimeCountTemp,TimeCountkey;
extern unsigned int TimeCountRealSpd,TimeCountPID,TimeCountAvgSpd,TimeCountVI;

extern unsigned char MOTORSTATE;

extern const unsigned int PWM_EN_TAB[6];
extern const unsigned char STEP_TAB[2][6];

extern uint16_t SampleData[4];

extern unsigned char ErrorCode; //故障状态
extern unsigned int  Motor_Start_F; //电机启停状态
extern unsigned int  Cur_step; //当前换相相序
extern unsigned char Dir; //方向控制

extern unsigned int HALLcount,HALLcount1,HALLcountTemp; // 霍尔脉冲计数值，用于计算平均速度和瞬时速度
extern unsigned int TargS1;//PID目标速度
extern unsigned int RealS1;//实时瞬时速度
extern unsigned int RealS;//平均速度
extern unsigned int SetSpeed;
extern float KKN,KPWMN;//计算电位器速度时的斜率

extern unsigned int OutPwmValue;//输出占空比值

extern unsigned char ErrCleFlag;

extern unsigned int SpeedD;//

extern unsigned int DIin; //电流偏置
extern unsigned int CanshuV;//母线电压100mV为单位
extern unsigned int CanshuI;//母线电流mA


//、、、、、、、、、、、、、、、、、以下参数为掉电保存参数
extern unsigned int MPolePairs;  //极对数
extern unsigned int  MAXSPEED; //额定转速
extern unsigned int  CanshuTimeDelay;//启动延时间MS

extern unsigned int  ISH;//过流报警值 A
extern unsigned int  VSH;//过压报警值 V
extern unsigned int  LSH;//欠压报警值 V
//开环参数
extern unsigned int PWMMin;//最低占空比
extern unsigned int PWMMax;//最大占空比输出
extern unsigned int TImeUP;//从最小占空比到最大占空比的时间
//速度闭环参数  实际缩小1000倍
extern unsigned int  MINSPEED;  //最低转速
extern unsigned int  CanshuSTT;//启动时间MS   时间参数
extern unsigned int SP,SI,SD;

extern unsigned int STEP_last;
extern unsigned int startflag,dirflag;
