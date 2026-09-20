#ifndef SOGI_H
#define SOGI_H

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
	float alpha;
	float beta;
	
	float u_z1;
	float u_z2;
	
	float alpha_z1;
	float alpha_z2;
	
	float beta_z1;
	float beta_z2;
	
	float omega;
	float k;
	float Ts;
	
	float bD0;
	float bD1;
	float bD2;
	
	float bQ0;
	float bQ1;
	float bQ2;
	
	float a1;
	float a2;	
} SOGI_t;

void SOGI_Init(SOGI_t *s);
void SOGI_Run(SOGI_t *s, float u);
void SOGI_UpdateCoefficients (SOGI_t *s);

#endif /*SOGI_H*/
