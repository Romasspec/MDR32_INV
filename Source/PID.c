#include "pid.h"

void CNTL_PID (volatile float* Out, volatile float* Ref, volatile float* Fdbk, CNTL_PID_CoefStruct* Coef)
{
	float En;
	static float Un[2];
	static float EnSum = 0;
	
	En = *Ref - *Fdbk;
	Un[0] = Coef->kp * En;
	EnSum += Coef->ki * En;
	
	if (EnSum > Coef->i_max) {
		EnSum = Coef->i_max;
	}
	if (EnSum < Coef->i_min) {
		EnSum = Coef->i_min;
	}
	
	Un[0] += EnSum;
	
	if(Un[0] > Coef->max) {
		Un[0] = Coef->max;
	}
	if(Un[0] < Coef->min){
		Un[0] = Coef->min;
	}
	
	*Out = Un[0];
	
//		Uerr = (float)kp * (UoutRef - Uout_ADC);
//		PIDout = (int32_t) Uerr;		
		
//		ErrorU = (UoutRef - (Uout << 12)) ;																	// q24-(q12<<12)=q24		
//		SumErrorU = (int32_t) (SumErrorU + (((int64_t) ki * ErrorU) >> 24));						// q24+((q24*q24)>>24) = q24
//		if (SumErrorU > _IQ24 (1))
//		{
//			SumErrorU = _IQ24 (1);
//		}
//		if (SumErrorU < _IQ24 (-1))
//		{
//			SumErrorU = _IQ24 (-1);
//		}
//		temp = (((int64_t) kp * ErrorU) >> 24) + SumErrorU;						// ((q24*q24)>>24)+q24 = q24			
//		
//		if (temp < 0) temp = 0;
//		PIDout =(int16_t)(((int64_t) T_MAX * temp) >> 24);												// q16*q24 = q40 >> 24 -> q16
//		
//		if (PIDout > T_MAX)
//		{
//			PIDout = T_MAX;
//		}
//		
//		if (PIDout < T_MIN)
//		{
//			PIDout = T_MIN;
//		}
//			
//		temp_ARR = (PIDout >> 1) << 1;		
//		MDR_TIMER3->CCR1 		= PIDout >> 1;
}

