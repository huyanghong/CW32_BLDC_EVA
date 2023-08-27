#include "PID.h"

float SPEED_P,SPEED_I,SPEED_D=0;

double SSumError=0,SPrevError=0,SLastError=0;
int SdError=0,SError=0;

void PID_init(void)
{	
	SError=0;SSumError=0;SLastError=0;
	
	
	SPEED_P=(float)SP/1000;
	SPEED_I=(float)SI/1000;
	SPEED_D=(float)SD/1000;
		
		SSumError=OutPwmValue;
		SSumError=SSumError/SPEED_I;	
}

void PIDcompute(unsigned int Target,unsigned int Real)
{
   float j=0.0,i;
    

	if(Target*MPolePairs<600)Target=600/MPolePairs; //���٣�4�Լ���150RPM
	
		SError =(Target-Real);   //����ƫ��
		
				if(SSumError<500)SSumError=500;
			else if(SSumError*SPEED_I>PWM_PERIOD)
					{
							SSumError=PWM_PERIOD;
							SSumError=SSumError/SPEED_I;
		       } // ��������ֵ�󣬲����ۼ�ƫ������ֱ���
			else SSumError +=SError;
	

    SdError=SError-SLastError; //΢���ƫ��ı仯 
    SLastError=SError; //��¼����ƫ������´μ���
	    
		i=SPEED_P;
		j=SError*i;
		i=SPEED_I;
		j=j+SSumError*i;
		i=SPEED_D;
		j=j+SdError*i;

  			if(j>PWM_PERIOD)OutPwmValue=PWM_PERIOD;
			else if(j<1)OutPwmValue=1;
			else OutPwmValue=j;	 
		
		
	UPPWM();		
}
