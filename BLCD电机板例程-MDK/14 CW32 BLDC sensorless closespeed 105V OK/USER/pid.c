#include "PID.h"

//速度闭环参数  实际缩小1000倍
unsigned int SP=100,SI=100,SD=10;

float SPEED_P,SPEED_I,SPEED_D=0;

double SSumError=0,SPrevError=0,SLastError=0;
int SdError=0,SError=0;

extern unsigned int OutPwm;
extern unsigned int MotorPoles; //电机极对数

void PID_init(void)
{	
	SError=0;SSumError=0;SLastError=0;
	
	
	SPEED_P=(float)SP/1000;
	SPEED_I=(float)SI/1000;
	SPEED_D=(float)SD/1000;
		
		SSumError=OutPwm;
		SSumError=SSumError/SPEED_I;	
}

void PIDcompute(unsigned int Target,unsigned int Real)
{
   float j=0.0,i;
    
		SError =(Target-Real);   //本次偏差
		
			if(SSumError<500)SSumError=500;
			else if(SSumError*SPEED_I>TSPeriod)
					{
							SSumError=TSPeriod;
							SSumError=SSumError/SPEED_I;
		       } // 输出到最大值后，不再累计偏差，抗积分饱和
			else SSumError +=SError;
	

    SdError=SError-SLastError; //微分项，偏差的变化 
    SLastError=SError; //记录本次偏差，用于下次计算
	    
		i=SPEED_P;
		j=SError*i;
		i=SPEED_I;
		j=j+SSumError*i;
		i=SPEED_D;
		j=j+SdError*i;

  		if(j>TSPeriod)OutPwm=TSPeriod;
			else if(j<1)OutPwm=1;
			else OutPwm=j;	 
		
		
	UPPWM();		
}
