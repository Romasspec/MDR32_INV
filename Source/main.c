#include <MDR32F9Qx_rst_clk.h>
#include <MDR32F9Qx_port.h>
#include <MDR32F9Qx_can.h>
#include <MDR32F9Qx_timer.h>
#include <MDR32F9Qx_adc.h>
#include "define.h"
#include "CNTL_3P3Z.h"
#include "PID.h"
#include "string.h"
#include <math.h>
#define OPEN_LOOP 1

//CNTL_3P3Z_CoefStruct1.b3 = _IQ26(-0.0002191);
//CNTL_3P3Z_CoefStruct1.b2 = _IQ26(0.0054683);
//CNTL_3P3Z_CoefStruct1.b1 = _IQ26(-0.0454941);
//CNTL_3P3Z_CoefStruct1.b0 = _IQ26(0.1261641);

//CNTL_3P3Z_CoefStruct1.a3 = _IQ26(0.4974124);
//CNTL_3P3Z_CoefStruct1.a2 = _IQ26(-1.9087017);
//CNTL_3P3Z_CoefStruct1.a1 = _IQ26(2.4112892);

//#define B0 (0.1261641)
//#define B1 (-0.0454941)
//#define B2 (0.0054683)
//#define B3 (-0.0002191)
//#define A1 (2.4112892)
//#define A2 (-1.9087017)
//#define A3 (0.4974124)
//#define K (1)
//#define REF (410)
//#define DUTY_TICKS_MIN (0)
//#define DUTY_TICKS_MAX (3600)

//#define B0 (+0.886478314902)
//#define B1 (-0.828670344869)
//#define B2 (-0.886151012126)
//#define B3 (+0.828997647645)
//#define A1 (+0.754914346688)
//#define A2 (+0.239966793784)
//#define A3 (+0.005118859528)
//#define K (+95.238095238095)
//#define REF (819)
//#define DUTY_TICKS_MIN (0)
//#define DUTY_TICKS_MAX (3510)

#define B0 (0.3416446)
#define B1 (0.09607146)
#define B2 (-0.20144382)
#define B3 (0.04412931)
#define A1 (-0.46257508)
#define A2 (0.94999731)
#define A3 (0.51257776)
#define K (3.1)
#define REF (410)
#define DUTY_TICKS_MIN (0)
#define DUTY_TICKS_MAX (3600)

//#define B0 0.2798459
//#define B1 0.7771431
//#define B2 0.7189250
//#define B3 0.2215419
//#define A1 -0.9974551
//#define A2 0.9999984
//#define A3 0.9974568
//#define K (1)

#define STEP_SINUS		400
#define PI 				3.141592653589


void RST_clk_Init (void);
void myPort_Init (void);
void CAN2_Init (void);
void myTimer1_Init (void);
void myTimer2_Init (void);
void myTimer3_Init (void);
void (*task_m)(void);
void task_m1 (void);
void task_m2 (void);
void set_Fnom (void);
void myADC_Init(void);

CAN_RxMsgTypeDef RxMsg;
CAN_TxMsgTypeDef TxMsg;
j1939msg_t CANmsg;
uint16_t Tim_CCR1, Tim_ARR, temp_ARR, timer_1ms[2], ADC_Result;
uint8_t	faza_shag, flags, VCNTTimer;
int32_t Uout;
uint32_t UoutRef;
int32_t ErrorU, SumErrorU;
int32_t PIDout;
int32_t kp;
int32_t ki;
int32_t kd;
int64_t temp;
float PIDout_gui;
volatile float Uout_ADC;
volatile float Uout_ref;
volatile float Uout_ref_sin;
volatile float CNTL_out;
int32_t Vref_165;
volatile float Uerr;
float sinus_ref[STEP_SINUS];
float Kdc;

CNTL_3P3Z_CoefStruct	CNTL_3P3Z_CoefStruct1;
CNTL_PID_CoefStruct 	CNTL_PID_CoefStruct1;

int main (void)
{
//	CAN_RxMsgTypeDef RxMsg;
////	CAN_TxMsgTypeDef TxMsg;
//	j1939msg_t CANmsg;
//	uint32_t cnt_shag;
//	uint8_t	faza_shag, timer_1ms[2], flags;
	
	uint16_t i;
	
	RST_clk_Init();
	myPort_Init();
	CAN2_Init ();
	myADC_Init ();
	myTimer1_Init ();	
	myTimer3_Init ();
	
	faza_shag = 0;
	VCNTTimer = 0;
	timer_1ms[0] = 0;
	timer_1ms[1] = 0;
		
	UoutRef = _IQ24(0.415482);
	kp = _IQ24(0.1);
	ki = _IQ24(0.00390625);
	SumErrorU = 0;
	Kdc = 1.0;
	
	
	for(i = 0; i < STEP_SINUS; i++) {
		sinus_ref[i] = sinf (2*PI*i/STEP_SINUS);
	}

	CNTL_PID_CoefStruct1.kp = 1.0;
	CNTL_PID_CoefStruct1.ki = 0.001;
	CNTL_PID_CoefStruct1.kd = 0.001;
	CNTL_PID_CoefStruct1.Ti = 1/20000;
	CNTL_PID_CoefStruct1.i_max = 1.0;
	CNTL_PID_CoefStruct1.i_min = -1.0;
	CNTL_PID_CoefStruct1.max = 0.8;
	CNTL_PID_CoefStruct1.min = -0.8;
	

	CNTL_3P3Z_CoefStruct1.b3 = (float)K * (float)B3;
	CNTL_3P3Z_CoefStruct1.b2 = (float)K * (float)B2;
	CNTL_3P3Z_CoefStruct1.b1 = (float)K * (float)B1;
	CNTL_3P3Z_CoefStruct1.b0 = (float)K * (float)B0;
	CNTL_3P3Z_CoefStruct1.a3 = (float)A3;
	CNTL_3P3Z_CoefStruct1.a2 = (float)A2;
	CNTL_3P3Z_CoefStruct1.a1 = (float)A1;
	CNTL_3P3Z_CoefStruct1.max = 0.8;
	CNTL_3P3Z_CoefStruct1.min = -0.8;
	
	Uout_ref = 0.2;	
	Vref_165 = 2000;
	
	task_m = &task_m1;
//	set_Fnom ();
	
	while (1)
	{
//			if ((TIMER_GetFlagStatus (MDR_TIMER1, TIMER_STATUS_CNT_ARR)) == SET)
//			{
//				TIMER_ClearFlag (MDR_TIMER1, TIMER_STATUS_CNT_ARR);
//				timer_1ms[0] ++;
//				timer_1ms[1] ++;
//			}
		
		if (VCNTTimer > 10)
		{
				VCNTTimer = 0;
				timer_1ms[0] ++;
				timer_1ms[1] ++;
				MDR_PORTD->RXTX ^= (1<<PD7);
				MDR_PORTC->RXTX ^= (1<<1);
		}
		
		(*task_m)();
	}
}

void task_m1 ()
{
	if ((MDR_CAN2->BUF_CON[1] & CAN_STATUS_RX_FULL) ==  CAN_STATUS_RX_FULL)
			{
				MDR_CAN2->BUF_CON[1] &=~CAN_STATUS_RX_FULL;
								
				CAN_GetRawReceivedData (MDR_CAN2, 1, &RxMsg);
				CANmsg.idt = RxMsg.Rx_Header.ID;
				CANmsg.data_u32[1] = RxMsg.Data[1];
				CANmsg.data_u32[0] = RxMsg.Data[0];
				
				if (CANmsg.pf == PGN_DEV_RESET)
				{					
//					RST_CLK_PCLKcmd (RST_CLK_PCLK_WWDG, ENABLE);
					MDR_WWDG->CFR	= 0x00000180;
					MDR_WWDG->CR	 = 0x7F;
					MDR_WWDG->CR	|= 0x80;					
				}
				
				if (CANmsg.pf == PGN_DEV_RUN_FD)
				{
					Tim_CCR1 = (CANmsg.data[1] << 8 ) | CANmsg.data[0];					
					flags |= (1<<CCR_UPDATE);						
				}
				
				else if (CANmsg.pf == PGN_DEV_RUN_BC)
				{						
					Tim_ARR = (CANmsg.data[1] << 8 ) | CANmsg.data[0];
					flags |= (1<<ARR_UPDATE);
				}
				
				if (CANmsg.pf == PGN_DEV_Uout)							// 0x03
				{
					//UoutRef = CANmsg.data_u32[0];
					memcpy((float*)&Uout_ref, &CANmsg.data_u32[0], 4);
					memcpy((float*)&Kdc, &CANmsg.data_u32[1], 4);
				}
				
				if (CANmsg.pf == PGN_DEV_Vref)							// 0x04
				{
					//kp = CANmsg.data_u32[0];
					memcpy(&Vref_165, &CANmsg.data_u32[0], 4);
				}
				
				
				if (CANmsg.pf == PGN_DEV_kp)							// 0x05
				{
					//kp = CANmsg.data_u32[0];
					memcpy(&CNTL_PID_CoefStruct1.kp, &CANmsg.data_u32[0], 4);
				}
				
				if (CANmsg.pf == PGN_DEV_ki)							// 0x06
				{
					//ki = CANmsg.data_u32[0];
					memcpy(&CNTL_PID_CoefStruct1.ki, &CANmsg.data_u32[0], 4);					
				}
			}
			
			if (timer_1ms[0] > 1000)
			{
				timer_1ms[0]	=	0;
								
//				ADC1_Start ();
//				MDR_PORTD->RXTX |= (1<<PD6);				
//				while (!ADC1_GetFlagStatus (ADC1_FLAG_END_OF_CONVERSION)) {MDR_PORTD->RXTX ^= (1<<PD7);}
//				MDR_PORTD->RXTX &=~(1<<PD6);
//				ADC_Result = (uint16_t) ADC1_GetResult ();				
				
				MDR_ADC->ADC1_CFG |= ADC1_CFG_REG_GO;				
				while ((MDR_ADC->ADC1_STATUS & ADC1_FLAG_END_OF_CONVERSION) == 0) {}							
				ADC_Result = (uint16_t) MDR_ADC->ADC1_RESULT;				
					
				CANmsg.p		= 7;
				CANmsg.r		= 0;
				CANmsg.dp		= 0;
				CANmsg.pf 		= 0x01;
				CANmsg.ps	 	= 0x00;
				CANmsg.sa 		= 0x07;
				
				CANmsg.len		=	8;
				
//				CANmsg.data[0]	= ADC_Result & 0xFF;
//				CANmsg.data[1]	= (ADC_Result >> 8) & 0xFF;
				
				TxMsg.ID 		= CANmsg.idt;
				TxMsg.DLC		= CANmsg.len;
				TxMsg.IDE		= CAN_ID_EXT;
				
				memcpy(&TxMsg.Data[0], (float*)&Uout_ADC, 4);
				//TxMsg.Data[0]	= Uout; //CANmsg.data_u32[0];
				memcpy(&TxMsg.Data[1], &Uout, 4);
				//TxMsg.Data[1]	= PIDout_gui;
				
				CAN_Transmit (MDR_CAN2, 0, &TxMsg);
			}
	
	task_m = &task_m2;
}

void task_m2 ()
{
	if (flags & (1<<CCR_UPDATE))
	{
		flags &=~(1<<CCR_UPDATE);
		TIMER_SetChnCompare (MDR_TIMER3, TIMER_CHANNEL1, Tim_CCR1);
	}
	
	if (flags & (1<<ARR_UPDATE))
	{
		flags &=~(1<<ARR_UPDATE);
		temp_ARR = (Tim_ARR / 2) * 2;
		TIMER_SetCntAutoreload	(MDR_TIMER3, temp_ARR);
		TIMER_SetChnCompare 		(MDR_TIMER3, TIMER_CHANNEL1, Tim_ARR / 2);
	}
	
	task_m = &task_m1;
}
void RST_clk_Init()
{
	RST_CLK_DeInit();
	RST_CLK_HSEconfig (RST_CLK_HS_CONTROL_HSE_ON);								// eternal oscillator on
	while (!(RST_CLK_GetFlagStatus(RST_CLK_FLAG_HSERDY)));				// waiting
	RST_CLK_CPU_PLLcmd (ENABLE);																	// PLL on
	// configures the CPU_PLL clock source and multiplication factor
	RST_CLK_CPU_PLLconfig (RST_CLK_CPU_PLLsrcHSEdiv1, RST_CLK_CPU_PLLmul10);//RST_CLK_CPU_PLLsrcHSEdiv1
	while (!(RST_CLK_GetFlagStatus(RST_CLK_FLAG_PLLCPURDY)));
	// select the CPU_PLL output as input for CPU_C2_SEL
	RST_CLK_CPU_PLLuse (ENABLE);
	// configures the CPU_C3_SEL division factor
	RST_CLK_CPUclkPrescaler (RST_CLK_CPUclkDIV1);
	// select the HCLK clock source
	RST_CLK_CPUclkSelection (RST_CLK_CPUclkCPU_C3);
	// Enable peripheral clocks  
	RST_CLK_PCLKcmd ( RST_CLK_PCLK_RST_CLK, ENABLE);
	RST_CLK_PCLKcmd ( RST_CLK_PCLK_PORTA, 	ENABLE);
	RST_CLK_PCLKcmd ( RST_CLK_PCLK_PORTB, 	ENABLE);
	RST_CLK_PCLKcmd ( RST_CLK_PCLK_PORTC, 	ENABLE);
	RST_CLK_PCLKcmd ( RST_CLK_PCLK_PORTD, 	ENABLE);
	RST_CLK_PCLKcmd ( RST_CLK_PCLK_PORTE, 	ENABLE);
	RST_CLK_PCLKcmd ( RST_CLK_PCLK_PORTF, 	ENABLE);
	RST_CLK_PCLKcmd ( RST_CLK_PCLK_EEPROM,	ENABLE);
	RST_CLK_PCLKcmd ( RST_CLK_PCLK_TIMER1,	ENABLE);
	RST_CLK_PCLKcmd ( RST_CLK_PCLK_TIMER2,	ENABLE);
	RST_CLK_PCLKcmd ( RST_CLK_PCLK_TIMER3,	ENABLE);
	RST_CLK_PCLKcmd	( RST_CLK_PCLK_UART2, 	ENABLE);
	RST_CLK_PCLKcmd	( RST_CLK_PCLK_ADC,	  	ENABLE);
	RST_CLK_PCLKcmd (RST_CLK_PCLK_WWDG, 	ENABLE);	
}

void myPort_Init ()
{
	PORT_InitTypeDef Port_Initstructure;
	
	PORT_DeInit (MDR_PORTA);
	PORT_DeInit (MDR_PORTB);
	PORT_DeInit (MDR_PORTC);
	PORT_DeInit (MDR_PORTD);
	PORT_DeInit (MDR_PORTE);
	PORT_DeInit (MDR_PORTF);
	
	
	PORT_StructInit (&Port_Initstructure);
	// Configure PORTB pins  for output
	Port_Initstructure.PORT_Pin		= PORT_Pin_0 | PORT_Pin_2;
	Port_Initstructure.PORT_OE		= PORT_OE_OUT;
	Port_Initstructure.PORT_FUNC	= PORT_FUNC_ALTER;
	Port_Initstructure.PORT_MODE	= PORT_MODE_DIGITAL;
	Port_Initstructure.PORT_SPEED	= PORT_SPEED_MAXFAST;
	PORT_Init (MDR_PORTB, &Port_Initstructure);
	
	// Configure PORTC pins  for output
	PORT_StructInit (&Port_Initstructure);
	Port_Initstructure.PORT_Pin		= PORT_Pin_1;
	Port_Initstructure.PORT_OE		= PORT_OE_OUT;
	Port_Initstructure.PORT_FUNC	= PORT_FUNC_PORT;
	Port_Initstructure.PORT_MODE	= PORT_MODE_DIGITAL;
	Port_Initstructure.PORT_SPEED	= PORT_SPEED_SLOW;
	PORT_Init (MDR_PORTC, &Port_Initstructure);
	
	// Configure PORTD pins  for output
	PORT_StructInit (&Port_Initstructure);
	Port_Initstructure.PORT_Pin		= PORT_Pin_6 | PORT_Pin_7;
	Port_Initstructure.PORT_OE		= PORT_OE_OUT;
	Port_Initstructure.PORT_FUNC	= PORT_FUNC_PORT;
	Port_Initstructure.PORT_MODE	= PORT_MODE_DIGITAL;
	Port_Initstructure.PORT_SPEED	= PORT_SPEED_SLOW;
	PORT_Init (MDR_PORTD, &Port_Initstructure);
	
	 
	
	// Configure PORTE pins  for input (CAN2RX)
	PORT_StructInit (&Port_Initstructure);
	Port_Initstructure.PORT_Pin		= PORT_Pin_6;
//	Port_Initstructure.PORT_OE		= PORT_OE_IN;
	Port_Initstructure.PORT_FUNC	= PORT_FUNC_ALTER;
	Port_Initstructure.PORT_MODE	= PORT_MODE_DIGITAL;
	PORT_Init (MDR_PORTE, &Port_Initstructure);
	
	// Configure PORTE pins  for output (CAN2TX)
//	Port_Initstructure.PORT_PD		= PORT_PD_DRIVER;
	Port_Initstructure.PORT_SPEED	= PORT_SPEED_MAXFAST;
//	Port_Initstructure.PORT_OE		= PORT_OE_OUT;
	Port_Initstructure.PORT_Pin		= PORT_Pin_7;
	PORT_Init (MDR_PORTE, &Port_Initstructure);
}

void CAN2_Init ()
{
	
	CAN_InitTypeDef CAN_InitStructure;
	CAN_FilterInitTypeDef CAN_FilterInitStructure;
	
	RST_CLK_PCLKcmd (RST_CLK_PCLK_CAN2, ENABLE);
	CAN_BRGInit (MDR_CAN2, CAN_HCLKdiv1);
	CAN_DeInit (MDR_CAN2);
	
	CAN_StructInit (&CAN_InitStructure);
	
	CAN_InitStructure.CAN_ROP				= DISABLE;
	CAN_InitStructure.CAN_SAP				= DISABLE;
	CAN_InitStructure.CAN_STM			= DISABLE;
	CAN_InitStructure.CAN_ROM				= DISABLE;
	CAN_InitStructure.CAN_OVER_ERROR_MAX = 255;
	CAN_InitStructure.CAN_SB			= CAN_SB_1_SAMPLE;
	CAN_InitStructure.CAN_PSEG		= CAN_PSEG_Mul_7TQ;
	CAN_InitStructure.CAN_SEG1		= CAN_SEG1_Mul_5TQ;
	CAN_InitStructure.CAN_SEG2		= CAN_SEG2_Mul_3TQ;
	CAN_InitStructure.CAN_SJW			= CAN_SJW_Mul_1TQ;
	CAN_InitStructure.CAN_BRP			= 19;
	CAN_Init (MDR_CAN2, &CAN_InitStructure);
	
	CAN_Cmd (MDR_CAN2, ENABLE);
	
	CAN_RxITConfig (MDR_CAN2, 1, ENABLE);
	CAN_TxITConfig (MDR_CAN2, 0, ENABLE);
	
	CAN_FilterInitStructure.Mask_ID 		=	 0x1F00FF00;
	CAN_FilterInitStructure.Filter_ID		=	 0x0C000700;
	CAN_FilterInit (MDR_CAN2, 1, &CAN_FilterInitStructure);
	CAN_Receive (MDR_CAN2, 1, ENABLE);

}

void myTimer1_Init ()
{
	TIMER_CntInitTypeDef sTim_CntInit;	
	TIMER_CntStructInit (&sTim_CntInit);
	TIMER_DeInit (MDR_TIMER1);
	sTim_CntInit.TIMER_IniCounter 					= 0;
	sTim_CntInit.TIMER_Prescaler					= 0;
	sTim_CntInit.TIMER_Period						= 4000;
	sTim_CntInit.TIMER_CounterMode					= TIMER_CntMode_ClkFixedDir;
	sTim_CntInit.TIMER_CounterDirection				= TIMER_CntDir_Up;
	sTim_CntInit.TIMER_EventSource					= TIMER_EvSrc_None;
	sTim_CntInit.TIMER_FilterSampling				= TIMER_FDTS_TIMER_CLK_div_1;
	sTim_CntInit.TIMER_ARR_UpdateMode				= TIMER_ARR_Update_On_CNT_Overflow;
	
	TIMER_CntInit (MDR_TIMER1, &sTim_CntInit);
	TIMER_BRGInit (MDR_TIMER1, TIMER_HCLKdiv1);
	
//	TIMER_ITConfig (MDR_TIMER1, TIMER_STATUS_CNT_ARR, ENABLE);
//	NVIC_EnableIRQ (Timer1_IRQn);
	
	TIMER_Cmd (MDR_TIMER1, ENABLE);
}
void myTimer2_Init ()
{
	TIMER_CntInitTypeDef sTim_CntInit;	
	TIMER_CntStructInit (&sTim_CntInit);
	TIMER_DeInit (MDR_TIMER2);
	sTim_CntInit.TIMER_IniCounter 					= 0;
	sTim_CntInit.TIMER_Prescaler					= 0;
	sTim_CntInit.TIMER_Period						= 8000;
	sTim_CntInit.TIMER_CounterMode					= TIMER_CntMode_ClkFixedDir;
	sTim_CntInit.TIMER_CounterDirection				= TIMER_CntDir_Up;
	sTim_CntInit.TIMER_EventSource					= TIMER_EvSrc_None;
	sTim_CntInit.TIMER_FilterSampling				= TIMER_FDTS_TIMER_CLK_div_1;
	sTim_CntInit.TIMER_ARR_UpdateMode				= TIMER_ARR_Update_On_CNT_Overflow;
	
	
	
	TIMER_CntInit (MDR_TIMER2, &sTim_CntInit);
	TIMER_BRGInit (MDR_TIMER2, TIMER_HCLKdiv1);
	TIMER_Cmd (MDR_TIMER2, ENABLE);
}
void myTimer3_Init ()
{
	TIMER_CntInitTypeDef 			sTim_CntInit;
	TIMER_ChnInitTypeDef			sTim_ChnInit;
	TIMER_ChnOutInitTypeDef			sTim_ChnOutInit;
	
	TIMER_CntStructInit (&sTim_CntInit);
	TIMER_DeInit (MDR_TIMER3);
	sTim_CntInit.TIMER_IniCounter 							= 0;
	sTim_CntInit.TIMER_Prescaler							= 0;
	sTim_CntInit.TIMER_Period								= 4000;
	sTim_CntInit.TIMER_CounterMode							= TIMER_CntMode_ClkFixedDir;
	sTim_CntInit.TIMER_CounterDirection						= TIMER_CntDir_Up;
	sTim_CntInit.TIMER_EventSource							= TIMER_EvSrc_None;
	sTim_CntInit.TIMER_FilterSampling						= TIMER_FDTS_TIMER_CLK_div_1;
//	sTim_CntInit.TIMER_ARR_UpdateMode						= TIMER_ARR_Update_On_CNT_Overflow;
	sTim_CntInit.TIMER_ARR_UpdateMode						= TIMER_ARR_Update_Immediately;
	TIMER_CntInit (MDR_TIMER3, &sTim_CntInit);
	
	
	TIMER_ChnStructInit (&sTim_ChnInit);
	sTim_ChnInit.TIMER_CH_Number 							= TIMER_CHANNEL1;
	sTim_ChnInit.TIMER_CH_Mode								= TIMER_CH_MODE_PWM;
	sTim_ChnInit.TIMER_CH_ETR_Ena							= DISABLE;
	sTim_ChnInit.TIMER_CH_ETR_Reset							= TIMER_CH_ETR_RESET_Disable;
	sTim_ChnInit.TIMER_CH_BRK_Reset							= TIMER_CH_BRK_RESET_Disable;
	sTim_ChnInit.TIMER_CH_REF_Format						= TIMER_CH_REF_Format6;
	sTim_ChnInit.TIMER_CH_Prescaler							= TIMER_CH_Prescaler_None;
	sTim_ChnInit.TIMER_CH_EventSource						= TIMER_CH_EvSrc_PE;
	sTim_ChnInit.TIMER_CH_FilterConf						= TIMER_Filter_1FF_at_TIMER_CLK;
	sTim_ChnInit.TIMER_CH_CCR_UpdateMode					= TIMER_CH_CCR_Update_On_CNT_eq_0;
	sTim_ChnInit.TIMER_CH_CCR1_Ena							= DISABLE;
	sTim_ChnInit.TIMER_CH_CCR1_EventSource					= TIMER_CH_CCR1EvSrc_PE;
	TIMER_ChnInit (MDR_TIMER3, &sTim_ChnInit);
	
	
	TIMER_ChnOutStructInit (&sTim_ChnOutInit);
	sTim_ChnOutInit.TIMER_CH_Number							= TIMER_CHANNEL1;
	sTim_ChnOutInit.TIMER_CH_DirOut_Polarity				= TIMER_CHOPolarity_NonInverted;
	//sTim_ChnOutInit.TIMER_CH_DirOut_Source					= TIMER_CH_OutSrc_DTG;
	sTim_ChnOutInit.TIMER_CH_DirOut_Source					= TIMER_CH_OutSrc_REF;
	sTim_ChnOutInit.TIMER_CH_DirOut_Mode					= TIMER_CH_OutMode_Output;
	sTim_ChnOutInit.TIMER_CH_NegOut_Polarity				= TIMER_CHOPolarity_NonInverted;
	//sTim_ChnOutInit.TIMER_CH_NegOut_Source					= TIMER_CH_OutSrc_DTG;
	sTim_ChnOutInit.TIMER_CH_NegOut_Source					= TIMER_CH_OutSrc_REF;
	sTim_ChnOutInit.TIMER_CH_NegOut_Mode					= TIMER_CH_OutMode_Output;
	sTim_ChnOutInit.TIMER_CH_DTG_MainPrescaler				= 40;
	sTim_ChnOutInit.TIMER_CH_DTG_AuxPrescaler				= 0;
	sTim_ChnOutInit.TIMER_CH_DTG_ClockSource				= TIMER_CH_DTG_ClkSrc_TIMER_CLK;
	TIMER_ChnOutInit (MDR_TIMER3, &sTim_ChnOutInit);
	
	
	sTim_ChnInit.TIMER_CH_Number 							= TIMER_CHANNEL2;
	TIMER_ChnInit (MDR_TIMER3, &sTim_ChnInit);
	
	
	sTim_ChnOutInit.TIMER_CH_Number							= TIMER_CHANNEL2;
	TIMER_ChnOutInit (MDR_TIMER3, &sTim_ChnOutInit);
	
	MDR_TIMER3->CCR1 = 0;
	MDR_TIMER3->CCR2 = 0;
	
	TIMER_ITConfig (MDR_TIMER3, TIMER_STATUS_CNT_ZERO, ENABLE);
	NVIC_EnableIRQ (Timer3_IRQn);
	
	TIMER_BRGInit (MDR_TIMER3, TIMER_HCLKdiv1);
	TIMER_Cmd (MDR_TIMER3, ENABLE);
}
void myADC_Init()
{
	ADC_InitTypeDef		ADC_InitStructure;
	ADCx_InitTypeDef 	ADCx_InitStructure;
	
	ADC_StructInit (&ADC_InitStructure);
	ADC_DeInit ();
	ADC_InitStructure.ADC_SynchronousMode 			= ADC_SyncMode_Independent;
	ADC_InitStructure.ADC_StartDelay				= 0;
	ADC_InitStructure.ADC_TempSensor				= ADC_TEMP_SENSOR_Disable;
	ADC_InitStructure.ADC_TempSensorAmplifier		= ADC_TEMP_SENSOR_AMPLIFIER_Disable;
	ADC_InitStructure.ADC_TempSensorConversion		= ADC_TEMP_SENSOR_CONVERSION_Disable;
	ADC_InitStructure.ADC_IntVRefConversion			= ADC_VREF_CONVERSION_Disable;
	ADC_InitStructure.ADC_IntVRefTrimming			= 0;
	ADC_Init (&ADC_InitStructure);
	
	ADCx_StructInit (&ADCx_InitStructure);
	ADCx_InitStructure.ADC_ClockSource				= ADC_CLOCK_SOURCE_CPU;
	ADCx_InitStructure.ADC_SamplingMode				= ADC_SAMPLING_MODE_SINGLE_CONV;
	ADCx_InitStructure.ADC_ChannelSwitching			= ADC_CH_SWITCHING_Disable;
	ADCx_InitStructure.ADC_ChannelNumber			= ADC_CH_ADC4;
	ADCx_InitStructure.ADC_Channels					= 0;
	ADCx_InitStructure.ADC_LevelControl				= ADC_LEVEL_CONTROL_Disable;
	ADCx_InitStructure.ADC_LowLevel					= 0;
	ADCx_InitStructure.ADC_HighLevel				= 0;
	ADCx_InitStructure.ADC_VRefSource				= ADC_VREF_SOURCE_INTERNAL;
	ADCx_InitStructure.ADC_IntVRefSource			= ADC_INT_VREF_SOURCE_INEXACT;
	ADCx_InitStructure.ADC_Prescaler				= ADC_CLK_div_8;
	ADCx_InitStructure.ADC_DelayGo					= 0;
	ADC1_Init (&ADCx_InitStructure);
	
	ADC1_Cmd (ENABLE);	
}
//void set_Fnom ()
//{
//	Tim_ARR = F_NOM;
//	temp_ARR = (Tim_ARR / 2) * 2;
//	TIMER_SetCntAutoreload	(MDR_TIMER3, temp_ARR);
//	TIMER_SetChnCompare 		(MDR_TIMER3, TIMER_CHANNEL1, Tim_ARR / 2);
//	
//	TIMER_Cmd (MDR_TIMER3, ENABLE);
//}
void Timer3_IRQHandler ()
{
	static volatile uint16_t i = 0;
	if (MDR_TIMER3->STATUS & TIMER_STATUS_CNT_ZERO)
	{
		MDR_TIMER3->STATUS &=~TIMER_STATUS_CNT_ZERO;
		VCNTTimer++;
//		MDR_PORTD->RXTX |= (1<<PD6);
		MDR_ADC->ADC1_CFG |= ADC1_CFG_REG_GO;				
		while ((MDR_ADC->ADC1_STATUS & ADC1_FLAG_END_OF_CONVERSION) == 0) {}
//		MDR_PORTD->RXTX &=~(1<<PD6);			
		
		Uout = (MDR_ADC->ADC1_RESULT & 0xFFF) - Vref_165;
			
		Uout_ADC = (float) ((int32_t)(MDR_ADC->ADC1_RESULT & 0xFFF) - Vref_165)/(float)2047.0;
		//Uout_ADC = 0.3;
			
		
		Uout_ref_sin = Uout_ref * sinus_ref[i];
			i++;
		if (i == STEP_SINUS) {
			i=0;
			MDR_PORTD->RXTX ^= (1<<PD6);
		}
		
		CNTL_3P3Z (&CNTL_out, &Uout_ref_sin, &Uout_ADC, &CNTL_3P3Z_CoefStruct1);
		//CNTL_PID  (&CNTL_out, &Uout_ref_sin, &Uout_ADC, &CNTL_PID_CoefStruct1);
		
		PIDout_gui = CNTL_out;	
		PIDout = (int32_t)(CNTL_out * (float) 3900.0);
		//PIDout = (int32_t)(Uout_ref_sin * (float) 3900.0);
			
		if (PIDout > 3900) {
			PIDout = 3900;
		}		
		if (PIDout < -3900) {
			PIDout = -3900;
		}	
		
		if (PIDout >= 0 ) {
			MDR_TIMER3->CCR1 = PIDout;
			MDR_TIMER3->CCR2 = 0;
		} else {
			MDR_TIMER3->CCR1 = 0;
			MDR_TIMER3->CCR2 = PIDout * (-1);
		}
		
	}
}
