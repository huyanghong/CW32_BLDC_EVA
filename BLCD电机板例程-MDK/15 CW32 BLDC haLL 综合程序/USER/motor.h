#include "main.h"

#include "globalv.h"
#include "init.h"

void PWMtimer_init(void);
void Halltimer_init(void);

void Commutation(unsigned int step,unsigned int PWM_ON_flag);
void UPPWM(void);
unsigned char  HALL_Check(void);
//void Halltimer_ioinit(void);
void HALL_MOTOR_START(void);
void MOTOR_STOP0(void);
