#include "globalv.h"
#include "compu.h"
#include "motor.h"
#include "pid.h"

void MotorStartCheck(void);
void MotorStartDealy(void);
void MotorStartOPEN(void);
void MotorRunOPEN(void);
void MotorStartPID(void);
void MotorRunPID(void);
void MotorStop(void);
void MotorError(void);
void MotorErrorOver(void);
void MotorBreak(void);
void WaitStart(void);
void EnDirCheck(void);

extern void Send_Value(void);

