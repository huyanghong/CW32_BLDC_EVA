#include "globalv.h"

unsigned int TimeCountCompuSpeed=0,TimeCountTemp=0,TimeCountkey=0;
unsigned int TimeCountRealSpd=0,TimeCountPID=0,TimeCountAvgSpd=0,TimeCountVI=0;
float canshu=0;//�¶�
unsigned char MOTORSTATE=STATESTARTCHECK;

const unsigned int PWM_EN_TAB[6]={0x001,0x001,0x010,0x010,0x100,0x100};//	6?PWM??????	AB,AC,BC,BA,CA,CB
const unsigned char STEP_TAB[2][6]={{4,0,5,2,3,1},{1,3,2,5,0,4}};

uint16_t SampleData[4];//�ڲ����� U���綯�� V���綯�� ĸ�ߵ�ѹ W���綯�� ��ѹ ���� �ڲ����� �ⲿ����

unsigned char ErrorCode=0; //����״̬
unsigned int  Motor_Start_F=0; //�����ͣ״̬
unsigned int  Cur_step=0; //��ǰ��������
unsigned char Dir=0; //�������

unsigned int HALLcount=0,HALLcount1=0,HALLcountTemp; // �����������ֵ�����ڼ���ƽ���ٶȺ�˲ʱ�ٶ�
unsigned int TargS1=150;//PIDĿ���ٶ�
unsigned int RealS1=0;//ʵʱ˲ʱ�ٶ�
unsigned int RealS=0;//ƽ���ٶ�
unsigned int SetSpeed=0;
float KKN=0,KPWMN=0;//�����λ���ٶ�ʱ��б��

unsigned int OutPwmValue=0;//���ռ�ձ�ֵ


unsigned int DIin=0; //����ƫ��
unsigned int CanshuV=0;//ĸ�ߵ�ѹ100mVΪ��λ
unsigned int CanshuI=0;//ĸ�ߵ���mA

//�������������������������������������²���Ϊ���籣�����

unsigned int  MPolePairs=2;  //������
unsigned int  MAXSPEED=4000; //�ת��

unsigned int  ISH=8;//��������ֵ A
unsigned int  VSH=40;//��ѹ����ֵ V
unsigned int  LSH=9;//Ƿѹ����ֵ V

//�ٶȱջ�����  ʵ����С1000��
unsigned int  MINSPEED=150;  //���ת��
unsigned int SP=50,SI=50,SD=50;
////������������������������������������������������������Ϊ�������籣�����
unsigned int STEP_last;

unsigned int startflag=0,dirflag=0;
