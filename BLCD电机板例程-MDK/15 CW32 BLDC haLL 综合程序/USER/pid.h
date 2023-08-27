#include "motor.h"
#include "globalv.h"

void PID_init(void);
void PIDcompute(unsigned int Target,unsigned int Real);
void PIDIcompute(unsigned int Target,unsigned int Real);
