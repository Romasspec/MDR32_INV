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

void SOGI_UpdateCoefficients (SOGI_Q15_t *s) {
//	float x;
//	float den;
//	
//	x = s->omega * s->Ts;
//	
//	den = 4.0f + 2.0f * s->k * x * x * x;
//	
//	/*
//	 * D(s)
//	 */
//	
//	s->bD0 = (2.0f * s->k * x) / den;
//	s->bD1 = 0.0f;
//	s->bD2 = -s->bD0;
//	
//	/*
//	 * Q(s)
//	 */
//	
//	s->bQ0 = (s->k * x * x) / den;
//	s->bQ1 = 2.0f * s->bQ0;
//	s->bQ2 = s->bQ0;
//	
//	/*
//	 * Denominator
//	 */
//	
//	s->a1 = (-8.0f + 2.0f * x * x) / den;
//	s->a2 = (4.0f - 2.0f * s->k * x * x * x) / den;
}
static inline int32_t sat_q15(int64_t x)
{
	if(x > 32768)
		return 32768;
	
	if(x < -32768)
		return -32768;
	
	return (int32_t)x;
}

void SOGI_Init(SOGI_Q15_t *s) {
	s->alpha 	= 0;
	s->beta 	= 0;
	
	s->u_z1		= 0;
	s->u_z2		= 0;
	
	s->alpha_z1 = 0;
	s->alpha_z2 = 0;
	
	s->beta_z1 	= 0;
	s->beta_z2 	= 0;
	
//	s->omega	= WG_NOM;
//	s->k		= SOGI_K;
//	s->Ts		= TS;
	// коэффициенты Q30
	s->bD0 = 11794541;
	s->bD1 = 0;
	s->bD2 = -11794541;
	
	s->bQ0 = 92634;
	s->bQ1 = 185268;
	s->bQ2 = 92634;
	
	s->a1	 = 2123632557;
	s->a2	 = -1050152742;
	
//	SOGI_UpdateCoefficients (s);
}

void SOGI_Run(SOGI_Q15_t *s, int16_t u) {
	int64_t alpha;
	int64_t beta;
	int32_t u_q30;
	
	u_q30 = u << 15;
	
	/*
	 * Alpha channel
	 */
	alpha = 
				  (int64_t)s->bD0 * u_q30
				+ (int64_t)s->bD1 * s->u_z1
				+ (int64_t)s->bD2 * s->u_z2
				+ (int64_t)s->a1  * s->alpha_z1
				+ (int64_t)s->a2  * s->alpha_z2;
	
	alpha >>= 30;
	
	/*
	 * Beta channel
	 */
	beta = 
				  (int64_t)s->bQ0 * u_q30
				+ (int64_t)s->bQ1 * s->u_z1
				+ (int64_t)s->bQ2 * s->u_z2
				+ (int64_t)s->a1  * s->beta_z1
				+ (int64_t)s->a2  * s->beta_z2;
				
			beta >>= 30;
					
	 /*
	 *  Saturation and Outputs
	 */	
		s->alpha 	= sat_q15(alpha>>15);
		s->beta 	= sat_q15(beta>>15);
				
	/*
	 * Update state
	 */
	 s->u_z2 = s->u_z1;
	 s->u_z1 = u_q30;
	 
	 s->alpha_z2 = s->alpha_z1;
	 s->alpha_z1 = alpha;
	 
	 s->beta_z2 = s->beta_z1;
	 s->beta_z1 = beta;	 
}
