#include "CNTL_3P3Z.h"
extern float PIDout_gui;
extern float Kdc;
void CNTL_3P3Z (volatile float* Out, volatile float* Ref, volatile float* Fdbk, CNTL_3P3Z_CoefStruct* Coef)
{
	static float En[3] = {0,0,0};
	static float Un[3] = {0,0,0};
	float acc;
	uint8_t i;
	// ACC =  e(n-3)B3 + e(n-2)*B2 + e(n-1)*B1 + e(n)*B0 + u(n-3)*A3 + u(n-2)*A2 + u(n-1)*A1
//	En[0] = *Ref - *Fdbk;	
//	Un[0] = Coef->b0 * En[0];
//	Un[0] += (Coef->b1 * En[1]) + (Coef->a1 * Un[1]);
//	Un[0] += (Coef->b2 * En[2]) + (Coef->a2 * Un[2]);
//	Un[0] += (Coef->b3 * En[3]) + (Coef->a3 * Un[3]);
	acc  = Coef->b3 * En[2]; En[2] = En[1];
	acc += Coef->b2 * En[1]; En[1] = En[0];
	acc += Coef->b1 * En[0]; En[0] = *Ref - *Fdbk;
	acc += Coef->b0 * En[0];

	acc += Coef->a3 * Un[2]; Un[2] = Un[1];
	acc += Coef->a2 * Un[1]; Un[1] = Un[0];
	acc += Coef->a1 * Un[0];
	
	if (acc > Coef->max) {
		acc = Coef->max;
	} else if (acc < Coef->min) {
		acc = Coef->min;
	}
	Un[0] = acc;
	*Out = Un[0];
	
//	for (i = 3; i != 0; i--) {
//		En[i] = En[i-1];
//		Un[i] = Un[i-1];
//	}
}
