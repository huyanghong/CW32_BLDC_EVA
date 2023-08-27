#include "sensorlessP.h"

#define RAMP_VALUE0		12 
#define RAMP_VALUE1		11 
#define RAMP_VALUE2		10 
#define RAMP_VALUE3		10 
#define RAMP_VALUE4		9 
#define RAMP_VALUE5		9 
#define RAMP_VALUE6		9 
#define RAMP_VALUE7		8 
#define RAMP_VALUE8		8 
#define RAMP_VALUE9		8 
#define RAMP_VALUE10		7 
#define RAMP_VALUE11		7 
#define RAMP_VALUE12		7 
#define RAMP_VALUE13		7 
#define RAMP_VALUE14		6 
#define RAMP_VALUE15		6 
#define RAMP_VALUE16		6 
#define RAMP_VALUE17		6 
#define RAMP_VALUE18		6 
#define RAMP_VALUE19		5 
#define RAMP_VALUE20		5 
#define RAMP_VALUE21		5 
#define RAMP_VALUE22		5 
#define RAMP_VALUE23		5 
#define RAMP_VALUE24		5 
#define RAMP_VALUE25		5 
#define RAMP_VALUE26		5 
#define RAMP_VALUE27		4 
#define RAMP_VALUE28		4 
#define RAMP_VALUE29		4 
#define RAMP_VALUE30		4 
#define RAMP_VALUE31		4 
#define RAMP_VALUE32		4 
#define RAMP_VALUE33		4 
#define RAMP_VALUE34		4 
#define RAMP_VALUE35		4 
#define RAMP_VALUE36		4 
#define RAMP_VALUE37		4 
#define RAMP_VALUE38		4 
#define RAMP_VALUE39		3 
#define RAMP_VALUE40		3 
#define RAMP_VALUE41		3 
#define RAMP_VALUE42		3 
#define RAMP_VALUE43		3 
#define RAMP_VALUE44		3 
#define RAMP_VALUE45		3 
#define RAMP_VALUE46		3 
#define RAMP_VALUE47		3 
#define RAMP_VALUE48		3 
#define RAMP_VALUE49		3 
#define RAMP_VALUE50		3 
#define RAMP_VALUE51		3 
#define RAMP_VALUE52		3 
#define RAMP_VALUE53		3 
#define RAMP_VALUE54		3 
#define RAMP_VALUE55		3 
#define RAMP_VALUE56		3 
#define RAMP_VALUE57		3 
#define RAMP_VALUE58		3 
#define RAMP_VALUE59		3 
#define RAMP_VALUE60		3 
#define RAMP_VALUE61		2 
#define RAMP_VALUE62		2 
#define RAMP_VALUE63		2 
		
const unsigned char RAMP_TABLE[ STEP_RAMP_SIZE ] = 
{
	RAMP_VALUE0,
	RAMP_VALUE1,
	RAMP_VALUE2,
	RAMP_VALUE3,
	RAMP_VALUE4,
	RAMP_VALUE5,
	RAMP_VALUE6,
	RAMP_VALUE7,
	RAMP_VALUE8,
	RAMP_VALUE9,
	RAMP_VALUE10,
	RAMP_VALUE11,
	RAMP_VALUE12,
	RAMP_VALUE13,
	RAMP_VALUE14,
	RAMP_VALUE15,
	RAMP_VALUE16,
	RAMP_VALUE17,
	RAMP_VALUE18,
	RAMP_VALUE19,
	RAMP_VALUE20,
	RAMP_VALUE21,
	RAMP_VALUE22,
	RAMP_VALUE23,
	RAMP_VALUE24,
	RAMP_VALUE25,
	RAMP_VALUE26,
	RAMP_VALUE27,
	RAMP_VALUE28,
	RAMP_VALUE29,
	RAMP_VALUE30,
	RAMP_VALUE31,
	RAMP_VALUE32,
	RAMP_VALUE33,
	RAMP_VALUE34,
	RAMP_VALUE35,
	RAMP_VALUE36,
	RAMP_VALUE37,
	RAMP_VALUE38,
	RAMP_VALUE39,
	RAMP_VALUE40,
	RAMP_VALUE41,
	RAMP_VALUE42,
	RAMP_VALUE43,
	RAMP_VALUE44,
	RAMP_VALUE45,
	RAMP_VALUE46,
	RAMP_VALUE47,
	RAMP_VALUE48,
	RAMP_VALUE49,
	RAMP_VALUE50,
	RAMP_VALUE51,
	RAMP_VALUE52,
	RAMP_VALUE53,
	RAMP_VALUE54,
	RAMP_VALUE55,
	RAMP_VALUE56,
	RAMP_VALUE57,
	RAMP_VALUE58,
	RAMP_VALUE59,
	RAMP_VALUE60,
	RAMP_VALUE61,
	RAMP_VALUE62,
	RAMP_VALUE63
};
