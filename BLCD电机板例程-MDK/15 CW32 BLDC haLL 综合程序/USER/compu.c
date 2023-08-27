#include "compu.h"
#include "global.h"

void SampleSpeed(void)
{
	unsigned int tem=0;
	unsigned int st=0;
	
			 st=SampleData[3];
					
			 if(st<NMINVD-10) SetSpeed=0;
			 else if(st<=NMINVD);
			 else if(st<=NMAXVD)
			 {				  
							tem=MINSPEED+KKN*(st-NMINVD);
							SetSpeed=tem;					
       }	
}

void SampleVI(void)
{
	float t;
	static unsigned char IErCount=0,VErCount=0;
	
	  //5 ����  4��ѹ
	  if(SampleData[2]<=DIin)CanshuI=0;
		else 
			{
				t=(SampleData[2]-DIin);			  
			  t=t*1.61;// /t=t/4096*3300/4.3/0.1; //0.1ŷ 
				CanshuI=t;				
      }
		if(CanshuI>=ISH*1000&&ErrorCode==0) //��������
			{
				IErCount++;
				if(IErCount>=NumErr)
					{ErrorCode=4;}
			}
		else 	IErCount=0;

  	t =SampleData[0];
		t=t/4096*3.3/RV1*(RV1+RV2); 
		 CanshuV=t*10;	//�ɼ�ĸ�ߵ�ѹ�Ŵ�10��

     if(CanshuV>=VSH*10&&ErrorCode==0)  //��ѹ�ж�
			{
				VErCount++;
				if(VErCount>=NumErr)
					{ErrorCode=6;}
			}
		else 	VErCount=0;	
		canshu=	TempSampleComu();
			
	
}

//NTC�¶Ȳ�����	
unsigned int TempSampleComu(void)
{	
	float t;
	unsigned int dat,max,min,mid,da,j;

	      t=SampleData[1];
				t=t/4096;
				t=t*3300;
				t=t/(5-t/1000); 
				dat=t*10;
	      da=dat;
		    max=97;	min=0;	
		
				while(1)	
				{
				mid=(max+min)/2;
				if(Table[mid]<da)		max=mid;
				else 					min=mid;
				if((max-min)<=1)		break;
				}	
      
				if(max==min)da=min*10;
				else 
				{
					j=(Table[min]-Table[max])/10;
					j=(Table[min]-da)/j;
					da=j;
					da=10*min+da; //�ɼ����¶ȷŴ���10��
				 }
				
return da;		 
}
