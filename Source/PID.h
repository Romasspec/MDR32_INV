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

typedef struct
{
	int16_t kp;
	int16_t ki;
	int16_t kd;
	int32_t integral_max;
	int32_t integral_min;
	int16_t out_max;
	int16_t out_min;
} CNTL_PID_Q15_Coef_t;


typedef struct
{
	int16_t ref;
	int16_t fdbk;
	int32_t integral;	
	int16_t out;
	int32_t debug;
} CNTL_PID_Q15_t;


void CNTL_PID (volatile float* Out, volatile float* Ref, volatile float* Fdbk, CNTL_PID_CoefStruct* Coef);
void CNTL_PID_Q15 (CNTL_PID_Q15_t* pid, CNTL_PID_Q15_Coef_t* coef);

#endif
