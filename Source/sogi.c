#include "sogi.h"

//void SOGI_Init(SOGI_t *sogi, float Ts, float omega_nom) {
//	sogi->alpha = 0.0f;
//	sogi->alpha	= 0.0f;
//	
//	sogi->omega = omega_nom;
//	
//	sogi->k = 1.414213562f;
//	
//	sogi->Ts = Ts;
//}

//void SOGI_Run(SOGI_t *sogi, float u) {
//	float error;
//	float d_alpha;
//	float d_beta;
//	
//	error = u - sogi->alpha;
//	
//	d_alpha = sogi->k * sogi->omega * error - sogi->omega * sogi->beta;
//	
//	d_beta = sogi->omega * sogi->alpha;
//	
//	sogi->alpha += d_alpha * sogi->Ts;
//	
//	sogi->beta += d_beta * sogi->Ts;
//}

void SOGI_UpdateCoefficients (SOGI_t *s) {
	float x;
	float den;
	
	x = s->omega * s->Ts;
	
	den = 4.0f + 2.0f * s->k * x * x * x;
	
	/*
	 * D(s)
	 */
	
	s->bD0 = (2.0f * s->k * x) / den;
	s->bD1 = 0.0f;
	s->bD2 = -s->bD0;
	
	/*
	 * Q(s)
	 */
	
	s->bQ0 = (s->k * x * x) / den;
	s->bQ1 = 2.0f * s->bQ0;
	s->bQ2 = s->bQ0;
	
	/*
	 * Denominator
	 */
	
	s->a1 = (-8.0f + 2.0f * x * x) / den;
	s->a2 = (4.0f - 2.0f * s->k * x * x * x) / den;
}

void SOGI_Init(SOGI_t *s) {
	s->alpha 	= 0.0f;
	s->beta 	= 0.0f;
	
	s->u_z1		= 0.0f;
	s->u_z2		= 0.0f;
	
	s->alpha_z1 = 0.0f;
	s->alpha_z2 = 0.0f;
	
	s->omega	= WG_NOM;
	s->k		= SOGI_K;
	s->Ts		= TS;
	
	SOGI_UpdateCoefficients (s);
}

void SOGI_Run(SOGI_t *s, float u) {
	float alpha;
	float beta;
	
	/*
	 * Alpha channel
	 */
	alpha = 
				  s->bD0 * u
				+ s->bD1 * s->u_z1
				+ s->bD2 * s->u_z2
				- s->a1 * s->alpha_z1
				- s->a2 * s->alpha_z2;
	
	/*
	 * Beta channel
	 */
	beta = 
				  s->bQ0 * u
				+ s->bQ1 * s->u_z1
				+ s->bQ2 * s->u_z2
				- s->a1 * s->beta_z1
				- s->a2 * s->beta_z2;
				
	/*
	 * Update state
	 */
	 s->u_z2 = s->u_z1;
	 s->u_z1 = u;
	 
	 s->alpha_z2 = s->alpha_z1;
	 s->alpha_z1 = alpha;
	 
	 s->beta_z2 = s->beta_z1;
	 s->beta_z1 = beta;
	 
	 /*
	 * Outputs
	 */
	 
	 s->alpha = alpha;
	 s->beta = beta;
}
