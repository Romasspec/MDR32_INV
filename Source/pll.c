#include "pll.h"
#include "sogi.h"
#include <math.h>

void ParkTransform(float alpha, float beta, float sin_theta, float cos_theta, float *d, float *q)
{
	*d = alpha * cos_theta + beta * sin_theta;
	*q = -alpha * sin_theta + beta * cos_theta;
	//uq = alpha * cos_theta - beta * sin_theta;
}

void PLL_Init(PLL_t *pll)
{
	pll->theta = 0.0f;
	
	pll->omega_nom = 2.0f * PI * 50.0f;
	
	pll->omega = pll->omega_nom;
	
	pll->integrator = 0.0f;
	
	pll->Ts = TS;
	
	/*
	 * init value
	 */
	
	pll->kp = 50.0f;
	pll->ki = 1000.0f;
	
	pll->omega_min = 45.0f;
	pll->omega_max = 55.0f;
	
	pll->theta_2PI = 2.0 * PI;	
}

void PLL_Run(PLL_t *pll, float alpha, float beta)
{
	float sin_theta;
	float cos_theta;
	float uq;
	float ud;
	float error;
	
	sin_theta = sinf(pll->theta);
	cos_theta = cosf(pll->theta);
	
	//uq = alpha * cos_theta - beta * sin_theta;
	
	ParkTransform(alpha, beta, sin_theta, cos_theta, &ud, &uq);
	
	/*
	 * нормировка
	 * 230 Vrms -> 325 Vpeak
	 * на вход подаю уже нормированную величину поэтому делить не надо
	 */
	
	error = uq;
	
	/*
	 * PI
	 */
	pll->integrator += pll->ki * error * pll->Ts;
	
	pll->omega = pll->omega_nom + pll->kp * error + pll->integrator;
	
	
	/*
	 * Ограничение частоты
	 */
	 
	 if(pll->omega > pll->omega_max)
		  pll->omega = pll->omega_max;
	 
	 if(pll->omega < pll->omega_min)
		  pll->omega = pll->omega_min;
	 
	 /*
		* Фаза
	  */
	 
	 pll->theta += pll->omega * pll->Ts;
	 
	 if(pll->theta >= pll->theta_2PI)
		  pll->theta -= pll->theta_2PI;
	 
	 if(pll->theta < 0.0f)
		  pll->theta += pll->theta_2PI;
}
