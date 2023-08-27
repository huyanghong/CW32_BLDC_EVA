#include "globalv.h"

unsigned int TimeCountCompuSpeed=0,TimeCountTemp=0,TimeCountkey=0;
unsigned int TimeCountRealSpd=0,TimeCountPID=0,TimeCountAvgSpd=0,TimeCountVI=0;
float canshu=0;//温度
unsigned char MOTORSTATE=STATESTARTCHECK;

const unsigned int PWM_EN_TAB[6]={0x001,0x001,0x010,0x010,0x100,0x100};//	6?PWM??????	AB,AC,BC,BA,CA,CB
const unsigned char STEP_TAB[2][6]={{4,0,5,2,3,1},{1,3,2,5,0,4}};

uint16_t SampleData[4];//内部参数 U反电动势 V反电动势 母线电压 W反电动势 电压 电流 内部调速 外部调速

unsigned char ErrorCode=0; //故障状态
unsigned int  Motor_Start_F=0; //电机启停状态
unsigned int  Cur_step=0; //当前换相相序
unsigned char Dir=0; //方向控制

unsigned int HALLcount=0,HALLcount1=0,HALLcountTemp; // 霍尔脉冲计数值，用于计算平均速度和瞬时速度
unsigned int TargS1=150;//PID目标速度
unsigned int RealS1=0;//实时瞬时速度
unsigned int RealS=0;//平均速度
unsigned int SetSpeed=0;
float KKN=0,KPWMN=0;//计算电位器速度时的斜率

unsigned int OutPwmValue=0;//输出占空比值


unsigned int DIin=0; //电流偏置
unsigned int CanshuV=0;//母线电压100mV为单位
unsigned int CanshuI=0;//母线电流mA

//、、、、、、、、、、、、、、、、、以下参数为掉电保存参数

unsigned int  MPolePairs=2;  //极对数
unsigned int  MAXSPEED=4000; //额定转速

unsigned int  ISH=8;//过流报警值 A
unsigned int  VSH=40;//过压报警值 V
unsigned int  LSH=9;//欠压报警值 V

//速度闭环参数  实际缩小1000倍
unsigned int  MINSPEED=150;  //最低转速
unsigned int SP=50,SI=50,SD=50;
////、、、、、、、、、、、、、、、、、、、、、、、、、以上为参数掉电保存参数
unsigned int STEP_last;

unsigned int startflag=0,dirflag=0;
