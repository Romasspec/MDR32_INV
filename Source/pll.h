#ifndef PLL_H
#define PLL_H
#include <stdint.h>

typedef struct
{
	int32_t theta;
	int32_t omega;
	
	int32_t integrator;
	
	int32_t kp;
	int32_t ki;
	
	int32_t omega_nom;
	
	int32_t omega_min;
	int32_t omega_max;
	int32_t theta_2PI;
	
	int32_t Ts;
	
	uint32_t phase_base;
	uint32_t phase_inc;
	uint32_t phase_inc_max;
	uint32_t phase_inc_min;
	uint32_t phase;
} PLL_Q15_t;

void PLL_Init(PLL_Q15_t *pll);
void PLL_Run(PLL_Q15_t *pll, int16_t alpha, int16_t beta);
void ParkTransform(int16_t alpha, int16_t beta, int16_t sin_theta, int16_t cos_theta, int16_t *d, int16_t *q);

#endif /*PLL_H*/
