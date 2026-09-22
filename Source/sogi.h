#ifndef SOGI_Q125_H
#define SOGI_Q125_H
#include <stdint.h>
#define PI				3.14159265359f

#define F_PWM			20000.0f
#define TS				(1.0f/F_PWM)

#define FG_NOM		50.0f
#define WG_NOM		(2.0f*PI*FG_NOM)

#define SOGI_K		1.41421356237f

//typedef struct
//{
//	float alpha;
//	float beta;
//	
//	float omega;
//	float k;
//	
//	float Ts;
//} SOGI_t;

//void SOGI_Init(SOGI_t *sogi, float Ts, float omega_nom);


typedef struct
{
	int32_t alpha;
	int32_t beta;
	
	int32_t u_z1;
	int32_t u_z2;
	
	int32_t alpha_z1;
	int32_t alpha_z2;
	
	int32_t beta_z1;
	int32_t beta_z2;
	
	int32_t omega;
	int32_t k;
	int32_t Ts;
	
	int32_t bD0;
	int32_t bD1;
	int32_t bD2;
	
	int32_t bQ0;
	int32_t bQ1;
	int32_t bQ2;
	
	int32_t a1;
	int32_t a2;	
} SOGI_Q15_t;

void SOGI_Init(SOGI_Q15_t *s);
void SOGI_Run(SOGI_Q15_t *s, int16_t u);
void SOGI_UpdateCoefficients (SOGI_Q15_t *s);

#endif /*SOGI_Q125_H*/
