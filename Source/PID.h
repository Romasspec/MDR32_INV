#ifndef PID_H
#define PID_H

#include <stdint.h>

typedef struct
{
	float kp;
	float ki;
	float kd;
	float Ti;
	float i_max;
	float i_min;
	float max;
	float min;
}CNTL_PID_CoefStruct;

void CNTL_PID (volatile float* Out, volatile float* Ref, volatile float* Fdbk, CNTL_PID_CoefStruct* Coef);

#endif
