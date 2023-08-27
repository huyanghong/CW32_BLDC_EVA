#include "main.h"

//LED��
#define LED_GPIO_PIN        GPIO_PIN_13                  
#define LED_GPIO_PORT      	CW_GPIOC            
#define LEDON()	  {GPIO_WritePin(LED_GPIO_PORT,LED_GPIO_PIN,GPIO_Pin_RESET);}
#define LEDOFF()  {GPIO_WritePin(LED_GPIO_PORT,LED_GPIO_PIN,GPIO_Pin_SET);}

//������ư���IO����
#define DR_GPIO_PIN        	GPIO_PIN_5                  
#define DR_GPIO_PORT      	CW_GPIOB               

//ʹ�ܿ��ƶ˰���IO����
#define EN_GPIO_PIN        	GPIO_PIN_10                  
#define EN_GPIO_PORT      	CW_GPIOB    

//����ָʾ��IO����
#define LEDDIR_GPIO_PIN        	GPIO_PIN_11                  
#define LEDDIR_GPIO_PORT      	CW_GPIOA               

//��ָͣʾ��IO����
#define LEDSTSTO_GPIO_PIN        	GPIO_PIN_11                  
#define LEDSTSTO_GPIO_PORT      	CW_GPIOB  


//��·PWM IO����
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

//�Ϲܵ��ƣ��¹ܿ��ؿ���, �ϸߵ�ƽ�µ͵�ƽ
#define PWM_AL_OFF GPIO_WritePin(PWM_AN_PORT,PWM_AN_PIN,GPIO_Pin_SET) 
#define PWM_BL_OFF GPIO_WritePin(PWM_BN_PORT,PWM_BN_PIN,GPIO_Pin_SET) 
#define PWM_CL_OFF GPIO_WritePin(PWM_CN_PORT,PWM_CN_PIN,GPIO_Pin_SET) 

#define PWM_AL_ON GPIO_WritePin(PWM_AN_PORT,PWM_AN_PIN,GPIO_Pin_RESET)
#define PWM_BL_ON GPIO_WritePin(PWM_BN_PORT,PWM_BN_PIN,GPIO_Pin_RESET)
#define PWM_CL_ON GPIO_WritePin(PWM_CN_PORT,PWM_CN_PIN,GPIO_Pin_RESET)


//BKIN�˿ڶ���
#define BKIN_GPIO_PIN        	GPIO_Pin_12                  
#define BKIN_GPIO_PORT      	GPIOB   


//�ز�Ƶ��20k
#define PWM_PERIOD 2399 
// (48000000 / 20000 ) - 1; 
#define OnepercentPWM (PWM_PERIOD+1)/100

#define STATESTARTCHECK 0
#define STATESTARTPID 1
#define STATERUNPID 2
#define STATESTOP 3
#define STATEERROR 5
#define STATEERROROVER 6

//��λ��
#define NMAXVD 3971
#define NMINVD 500
//0.4V~3.2V���е���

#define RS (float)0.1
//ȡ������
#define AP 5
//�Ŵ���
#define Ikvalue (float)1200/(float)AP/RS
//�������㳣�� �ڲ��ο�1.2V

#define NumErr 15
//������ѹ�����϶�����

//#define VSh 60
//��ѹֵ�趨
#define RV1 5.1
#define RV2	200
//�����ѹ�����ߵ�ѹ֮�ȣ�ΪRV1:(RV1+RV2)

#define Kvvalue (float)1.2/RV1*(RV1+RV2)


extern uint8_t ADC_CR1Array[4] ; // ͨ������ADC��CR1�Ĵ���ʵ��ADCͨ���Զ��л�
extern uint8_t ADC_Start ;    

extern unsigned int TimeCountCompuSpeed,TimeCountTemp,TimeCountkey;
extern unsigned int TimeCountRealSpd,TimeCountPID,TimeCountAvgSpd,TimeCountVI;

extern unsigned char MOTORSTATE;

extern const unsigned int PWM_EN_TAB[6];
extern const unsigned char STEP_TAB[2][6];

extern uint16_t SampleData[4];

extern unsigned char ErrorCode; //����״̬
extern unsigned int  Motor_Start_F; //�����ͣ״̬
extern unsigned int  Cur_step; //��ǰ��������
extern unsigned char Dir; //�������

extern unsigned int HALLcount,HALLcount1,HALLcountTemp; // �����������ֵ�����ڼ���ƽ���ٶȺ�˲ʱ�ٶ�
extern unsigned int TargS1;//PIDĿ���ٶ�
extern unsigned int RealS1;//ʵʱ˲ʱ�ٶ�
extern unsigned int RealS;//ƽ���ٶ�
extern unsigned int SetSpeed;
extern float KKN,KPWMN;//�����λ���ٶ�ʱ��б��

extern unsigned int OutPwmValue;//���ռ�ձ�ֵ

extern unsigned char ErrCleFlag;

extern unsigned int SpeedD;//

extern unsigned int DIin; //����ƫ��
extern unsigned int CanshuV;//ĸ�ߵ�ѹ100mVΪ��λ
extern unsigned int CanshuI;//ĸ�ߵ���mA


//�������������������������������������²���Ϊ���籣�����
extern unsigned int MPolePairs;  //������
extern unsigned int  MAXSPEED; //�ת��
extern unsigned int  CanshuTimeDelay;//������ʱ��MS

extern unsigned int  ISH;//��������ֵ A
extern unsigned int  VSH;//��ѹ����ֵ V
extern unsigned int  LSH;//Ƿѹ����ֵ V
//��������
extern unsigned int PWMMin;//���ռ�ձ�
extern unsigned int PWMMax;//���ռ�ձ����
extern unsigned int TImeUP;//����Сռ�ձȵ����ռ�ձȵ�ʱ��
//�ٶȱջ�����  ʵ����С1000��
extern unsigned int  MINSPEED;  //���ת��
extern unsigned int  CanshuSTT;//����ʱ��MS   ʱ�����
extern unsigned int SP,SI,SD;

extern unsigned int STEP_last;
extern unsigned int startflag,dirflag;
