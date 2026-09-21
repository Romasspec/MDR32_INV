#ifndef PLL_H
#define PLL_H

typedef struct
{
	float theta;
	float omega;
	
	float integrator;
	
	float kp;
	float ki;
	
	float omega_nom;
	
	float omega_min;
	float omega_max;
	float theta_2PI;
	
	float Ts;	
} PLL_t;

void PLL_Init(PLL_t *pll);
void PLL_Run(PLL_t *pll, float alpha, float beta);
void ParkTransform(float alpha, float beta, float sin_theta, float cos_theta, float *d, float *q);

#endif /*PLL_H*/
