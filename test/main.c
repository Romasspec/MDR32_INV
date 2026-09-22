#include <stdio.h>
#include <stdint.h>

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

SOGI_Q15_t sogi;

void SOGI_Init(SOGI_Q15_t *s);
void SOGI_Run(SOGI_Q15_t *s, int16_t u);
void ParkTransform_Q15(int16_t alpha, int16_t beta, int16_t sin_theta, int16_t cos_theta, int16_t *d, int16_t *q);
int16_t get_sin_1024(uint32_t phase);
int16_t get_cos_1024(uint32_t phase);

int main() {
	
	SOGI_Init(&sogi);
	
	uint16_t phase;
	int32_t uin = 1000 << 4;
	
	int16_t sinus;
	int16_t cosinus;
	int16_t test_alpha;
	int16_t test_beta;
	
	int16_t ud = 0;
	int16_t uq = 0;
	
	for(uint32_t iter = 0; iter < 1; iter++)
	{
		
		//проверка преобразования парка
		printf("Введи фазу 0...360\r\n");
		scanf("%d", &phase);				
		
		sinus 	= get_sin_1024 (phase * 1024/360);
		cosinus = get_cos_1024 (phase * 1024/360);
		printf("sin = %4.2f, cos = %.2f\r\n", ((float)sinus/32768), ((float)cosinus/32768));
		printf("\r\n");
		
		printf("Введи alpha -32768...32767\r\n");
		scanf("%d", &test_alpha);				
		
		printf("Введи beta -32768...32767\r\n");
		scanf("%d", &test_beta);
//		test_beta = test_alpha - 32767;
		printf("alpha = %d, beta = %d\r\n", test_alpha, test_beta);
			
		for(uint32_t i = 0; i < 10; i++)
		{						
			ParkTransform_Q15(test_alpha, test_beta, sinus, cosinus, &ud, &uq);
		}
		 printf("park: ud = %d, uq = %d\r\n", ud, uq);
		 printf("\r\n");
		 printf("******************************************\r\n");
		 
		 
		/*  //проверка алгоритма SOGI
		uin = 3276;
		
		 for(uint32_t i = 0; i < 2000; i++)
		{
			SOGI_Run(&sogi, uin);			
		}
		
		printf("sogi %d, %d\r\n", sogi.alpha, sogi.beta);
		printf("\r\n"); */
		 
	}
	
	
	
//	void ParkTransform_Q15(int16_t alpha, int16_t beta, int16_t sin_theta, int16_t cos_theta, int16_t *d, int16_t *q);
   
    return 0;
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

static inline int32_t sat_q15(int64_t x)
{
	if(x > 32767)
		return 32767;
	
	if(x < -32768)
		return -32768;
	
	return (int32_t)x;
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

const int16_t sin_lut_1024[257] = {
			 0, 201, 402, 603, 804, 1005, 1206, 1407, 1608, 1809, 2009, 2210, 2410, 2611, 2811, 3012,
    3212, 3412, 3612, 3811, 4011, 4210, 4410, 4609, 4808, 5007, 5205, 5404, 5602, 5800, 5998, 6195,
    6393, 6590, 6786, 6983, 7179, 7375, 7571, 7767, 7962, 8157, 8351, 8545, 8739, 8933, 9126, 9319,
    9512, 9704, 9896, 10087, 10278, 10469, 10659, 10849, 11039, 11228, 11417, 11605, 11793, 11980, 12167, 12353,
    12539, 12725, 12910, 13094, 13279, 13462, 13645, 13828, 14010, 14191, 14372, 14553, 14732, 14912, 15090, 15269,
    15446, 15623, 15800, 15976, 16151, 16325, 16499, 16673, 16846, 17018, 17189, 17360, 17530, 17700, 17869, 18037,
    18204, 18371, 18537, 18703, 18868, 19032, 19195, 19357, 19519, 19680, 19841, 20000, 20159, 20317, 20475, 20631,
    20787, 20942, 21096, 21250, 21403, 21554, 21705, 21856, 22005, 22154, 22301, 22448, 22594, 22739, 22884, 23027,
    23170, 23311, 23452, 23592, 23731, 23870, 24007, 24143, 24279, 24413, 24547, 24680, 24811, 24942, 25072, 25201,
    25329, 25456, 25582, 25708, 25832, 25955, 26077, 26198, 26319, 26438, 26556, 26674, 26790, 26905, 27019, 27133,
    27245, 27356, 27466, 27575, 27683, 27790, 27896, 28001, 28105, 28208, 28310, 28411, 28510, 28609, 28706, 28803,
    28898, 28992, 29085, 29177, 29268, 29358, 29447, 29534, 29621, 29706, 29791, 29874, 29956, 30037, 30117, 30195,
    30273, 30349, 30424, 30498, 30571, 30643, 30714, 30783, 30852, 30919, 30985, 31050, 31113, 31176, 31237, 31297,
    31356, 31414, 31470, 31526, 31580, 31633, 31685, 31736, 31785, 31833, 31880, 31926, 31971, 32014, 32057, 32098,
    32137, 32176, 32213, 32250, 32285, 32318, 32351, 32382, 32412, 32441, 32469, 32495, 32521, 32545, 32567, 32589,
    32609, 32628, 32646, 32663, 32678, 32692, 32705, 32717, 32728, 32737, 32745, 32752, 32757, 32761, 32765, 32766,
    32767
};


// Быстрый синус. Принимает фазу 0..1023. Возвращает int16_t (-32767..32767)
int16_t get_sin_1024(uint32_t phase) {
    phase &= 0x3FF; // Зацикливаем фазу в диапазоне 0..1023
    
    uint32_t angle = phase & 0x1FF; // Сводим к полупериоду 0..511
    if (angle > 256) {
        angle = 512 - angle; // Зеркалим вторую четверть (257..511 -> 255..1)
    }
    
    int16_t val = sin_lut_1024[angle];
    
    if (phase > 511) {
        return -val; // Отрицательная полуволна для фаз 512..1023
    }
    return val;
}

// Быстрый косинус. Использует сдвиг фазы на 90 градусов (256 шагов)
int16_t get_cos_1024(uint32_t phase) {
    return get_sin_1024(phase + 256);
}

void ParkTransform_Q15(int16_t alpha, int16_t beta, int16_t sin_theta, int16_t cos_theta, int16_t *d, int16_t *q)
{		
	int32_t ud;
	int32_t uq;
	
//	ud = ( (int32_t)alpha * cos_theta + (int32_t)beta * sin_theta);
//	uq = (-(int32_t)alpha * sin_theta + (int32_t)beta * cos_theta);
	
	ud = ( (int32_t)alpha * sin_theta - (int32_t)beta * cos_theta);
	uq = ( -(int32_t)alpha * cos_theta + (int32_t)beta * sin_theta);
	
	*d = (int16_t)(ud >> 15);
	*q = (int16_t)(uq >> 15);
	
	//uq = alpha * cos_theta - beta * sin_theta;
}