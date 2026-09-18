#ifndef CNTL_3P3Z_H
#define CNTL_3P3Z_H
#include <stdint.h>

typedef struct
{
	float b3;
	float b2;
	float b1;
	float b0;
	float a3;
	float a2;
	float a1;
	float max;
	float min;
}CNTL_3P3Z_CoefStruct;

void CNTL_3P3Z (volatile float* Out, volatile float* Ref, volatile float* Fdbk, CNTL_3P3Z_CoefStruct* Coef);

#endif
