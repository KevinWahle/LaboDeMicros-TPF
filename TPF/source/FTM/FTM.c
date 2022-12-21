/***************************************************************************//**
  @file		FTM.c
  @brief	Funciones para manejo de timer FTM
  @author	Grupo 5
  @date		17 oct. 2022
 ******************************************************************************/

/*******************************************************************************
 * INCLUDE HEADER FILES
 ******************************************************************************/

#include "FTM.h"
#include "MK64F12.h"
#include "hardware.h"
#include "MCAL/gpio.h"
#include <stdlib.h>

#include "MCAL/gpio.h"
#include "hardware.h"

/*******************************************************************************
 * CONSTANT AND MACRO DEFINITIONS USING #DEFINE
 ******************************************************************************/

#define TEST_PIN	PORTNUM2PIN(PC, 16)


#define FTM_CLK	50000000UL	// 50MHz Bus Clock

#define FTM_MAX_VAL	0xFFFFUL

#define FTM_CLK_PTRS {&(SIM->SCGC6), &(SIM->SCGC6), &(SIM->SCGC6), &(SIM->SCGC3)}
#define FTM_CLK_MASKS {SIM_SCGC6_FTM0_MASK, SIM_SCGC6_FTM1_MASK, SIM_SCGC6_FTM2_MASK, SIM_SCGC3_FTM3_MASK}

#define FTM_CnSC_EDGE(x)	(((uint32_t)(((uint32_t)(x)) << FTM_CnSC_ELSA_SHIFT)) & (FTM_CnSC_ELSB_MASK | FTM_CnSC_ELSA_MASK))

#define U12_MAX_VAL		0xFFF

/*******************************************************************************
 * ENUMERATIONS AND STRUCTURES AND TYPEDEFS
 ******************************************************************************/


/*******************************************************************************
 * VARIABLES WITH GLOBAL SCOPE
 ******************************************************************************/

// +ej: unsigned int anio_actual;+


/*******************************************************************************
 * FUNCTION PROTOTYPES FOR PRIVATE FUNCTIONS WITH FILE LEVEL SCOPE
 ******************************************************************************/

void FTM_IRQHandler(FTM_MODULE ftm);

/*******************************************************************************
 * ROM CONST VARIABLES WITH FILE LEVEL SCOPE
 ******************************************************************************/

static FTM_Type* const FTMPtrs[] = FTM_BASE_PTRS;
static const IRQn_Type FTMIRQs[] = FTM_IRQS;



static __IO uint32_t* const FTMClkSimPtr[] = FTM_CLK_PTRS;
static const uint32_t FTMClkSimMask[] = FTM_CLK_MASKS;

static PORT_Type* const portPtr[] = PORT_BASE_PTRS;

// Pin of each channel of each FTM
// Todos los pines disponibles en package 100LQFP
//															CH0	CH1	CH2	CH3	CH4	CH5	CH6	CH7
static const uint8_t FTMPinPort[][FTM_CH_COUNT] =	{	{	PC,	PC,	PC,	PC,	PD,	PD,	PD,	PD,	},	// FTM0
														{	PB,	PB	},							// FTM1
														{	PB,	PB	},							// FTM2
														{	PD,	PD,	PD,	PD,	PC,	PC,	PC,	PC,	}	// FTM3
													};

static const uint8_t FTMPinNum[][FTM_CH_COUNT] =	{	{	1,	2,	3,	4,	4,	5,	6,	7,	},	// FTM0
														{	0,	1	},							// FTM1
														{	18,	19	},							// FTM2
														{	0,	1,	2,	3,	8,	9,	10,	11,	}	// FTM3
													};

static const uint8_t FTMPinMuxAlt[][FTM_CH_COUNT] =	{	{	4,	4,	4,	4,	4,	4,	4,	4,	},	// FTM0
														{	3,	3,	},							// FTM1
														{	3,	3	},							// FTM2
														{	4,	4,	4,	4,	3,	3,	3,	3,	}	// FTM3
													};


/*******************************************************************************
 * STATIC VARIABLES AND CONST VARIABLES WITH FILE LEVEL SCOPE
 ******************************************************************************/

static uint16_t** PWMPtrs[FTM_COUNT][FTM_CH_COUNT];


static callbackICEdge* ICCbPtrs[FTM_COUNT][FTM_CH_COUNT];
static uint32_t ICTOFCont[FTM_COUNT][FTM_CH_COUNT];

static uint16_t ICChannelValue[FTM_COUNT][FTM_CH_COUNT];
static FTM_tick_t ICLastTicks[FTM_COUNT][FTM_CH_COUNT];
static bool ICnewEdge[FTM_COUNT][FTM_CH_COUNT];

static bool FTMMode[FTM_COUNT];		// 0: PWM, 1: Input Capture

static uint8_t FTMPWMPtrList[FTM_COUNT][FTM_CH_COUNT];
static uint8_t FTMICList[FTM_COUNT][FTM_CH_COUNT];

static uint8_t FTMPWMPtrLength[FTM_COUNT];
static uint8_t FTMICLength[FTM_COUNT];

/*******************************************************************************
 *******************************************************************************
                        GLOBAL FUNCTION DEFINITIONS
 *******************************************************************************
 ******************************************************************************/

/**
 * @brief Reinicia el contador
 * @param ftm: módulo FTM
*/
void FTMReset(FTM_MODULE ftm) {
	FTMPtrs[ftm]->CNT = 0x00;	// Reset counter to CNTIN
}

////// PWM //////

/**
 * @brief Inicializa un canal de PWM
 * @param ftm: módulo FTM
 * @param channel: Canal del modulo FTM
 * @param freq: frecuencua del PWM (0 - MAX_FREQ)
*/
void PWMInit(FTM_MODULE ftm, FTM_CHANNEL channel, uint32_t freq) {

	gpioMode(TEST_PIN, OUTPUT);
	gpioWrite(TEST_PIN, LOW);

	FTMMode[ftm] = 0;

	// Enable FTMx clock
	*(FTMClkSimPtr[ftm]) |= FTMClkSimMask[ftm];

	FTM_Type* const pFTM = FTMPtrs[ftm];

	// Disable write protection (if enabled)
	if (pFTM->FMS & FTM_FMS_WPEN_MASK)	pFTM->MODE = FTM_MODE_WPDIS_MASK;

	// Disblae FTM for PWM mode
	pFTM->MODE &= ~FTM_MODE_FTMEN_MASK;		// FTMEN = 0

	// Set PWM Mode to edge aligned with high-true polarity
	pFTM->CONTROLS[channel].CnSC = FTM_CnSC_MSB_MASK | FTM_CnSC_ELSB_MASK;		// MSB = ELSB = 1
	// Start with duty = 0
	pFTM->CONTROLS[channel].CnV = 0;

	// frequency configuration
	pFTM->MOD =	FTM_CLK/freq - 1;

	// Pin configuration
	portPtr[FTMPinPort[ftm][channel]]->PCR[FTMPinNum[ftm][channel]] = PORT_PCR_MUX(FTMPinMuxAlt[ftm][channel]);

	// Set clock source and disable interrupts
	// Obs: prescales set to 1
	FTMPtrs[ftm]->SC = FTM_SC_CLKS(0x01) | FTM_SC_PS(0x00);		// Start clock

}

/**
 * @brief Activa la salida PWM de un canal con el duty deseado
 * @param ftm: módulo FTM
 * @param channel: Canal del modulo FTM
 * @param duty: duty cycle del PWM (0.0 - 1.0)
*/
void PWMStart(FTM_MODULE ftm, FTM_CHANNEL channel, double duty) {
	FTM_Type* const pFTM = FTMPtrs[ftm];
	// Update channel value
	pFTM->CONTROLS[channel].CnV = duty*(pFTM->MOD & FTM_MOD_MOD_MASK);
}

/**
 * @brief Recibe el duty a traves de un doble puntero
 * @param ftm: módulo FTM
 * @param channel: Canal del modulo FTM
 * @param ptr: direccion del puntero que apuntaa al duty actual. (12 bits). NULL para desactivar
*/
void PWMFromPtr(FTM_MODULE ftm, FTM_CHANNEL channel, uint16_t** ptr) {

	PWMPtrs[ftm][channel] = ptr;
	FTMPWMPtrList[ftm][FTMPWMPtrLength[ftm]++] = channel;


//	FTMPtrs[ftm]->CONTROLS.CnSC |= FTM_CnSC_CHIE_MASK;

	FTMPtrs[ftm]->SC |= FTM_SC_TOIE_MASK;

	NVIC_EnableIRQ(FTMIRQs[ftm]);
}


////// Input Capture //////

/**
 * @brief Inicializa un canal FTM en Input Capture
 * @param ftm: módulo FTM
 * @param channel: Canal del modulo FTM
 * @param edge: capture edge
 * @param edgeCb: callback que se llama en cada flanco activo con la cantidad de ticks desde el ultimo.
*/
void ICInit(FTM_MODULE ftm, FTM_CHANNEL channel, IC_CAPTURE_EDGE edge, callbackICEdge edgeCb) {

	gpioMode(TEST_PIN, OUTPUT);
	gpioWrite(TEST_PIN, LOW);

		FTMMode[ftm] = 1;



	// Enable FTMx clock
		*(FTMClkSimPtr[ftm]) |= FTMClkSimMask[ftm];

		FTM_Type* const pFTM = FTMPtrs[ftm];

		// Disable write protection (if enabled)
		if (pFTM->FMS & FTM_FMS_WPEN_MASK)	pFTM->MODE = FTM_MODE_WPDIS_MASK;

		// Enable FTM
		pFTM->MODE = FTM_MODE_FTMEN_MASK;

		pFTM->CNTIN = 0x00;
		pFTM->MOD = FTM_MOD_MOD_MASK;	// Free running clock
		pFTM->CONF = 0x00;

		// Filter enable
//		pFTM->FILTER = FTM_FILTER_CH0FVAL_MASK;

		// Set channel to input capture and enable channel interrupts
		pFTM->QDCTRL = 0x00;
		pFTM->COMBINE = 0x00;		// TODO: Chequear canal
		pFTM->CONTROLS[channel].CnSC = FTM_CnSC_EDGE(edge) | FTM_CnSC_CHIE_MASK;

		if (ftm == FTM_1 && channel == 0) {
			SIM->SOPT4 = SIM_SOPT4_FTM1CH0SRC(0x01);	// Input from CMP_0
		}
		else if (ftm == FTM_2 && channel == 0) {
			SIM->SOPT4 = SIM_SOPT4_FTM2CH0SRC(0x01);	// Input from CMP_0
		}
		else {
			// Pin configuration
			portPtr[FTMPinPort[ftm][channel]]->PCR[FTMPinNum[ftm][channel]] = PORT_PCR_MUX(FTMPinMuxAlt[ftm][channel]);
		}



		// Set callback
		ICCbPtrs[ftm][channel] = edgeCb;

		FTMICList[ftm][FTMICLength[ftm]++] = channel;

		// Reset counters
		ICTOFCont[ftm][channel] = 0;
		ICChannelValue[ftm][channel] = 0;
		ICLastTicks[ftm][channel] = 0;

		// Set clock source and enable TOF interrupt
		FTMPtrs[ftm]->SC = FTM_SC_CLKS(0x01) | FTM_SC_PS(0x00);// | FTM_SC_TOIE_MASK;		// Start clock

		NVIC_EnableIRQ(FTMIRQs[ftm]);

}


/**
 * @brief Devuelve el valor del contador en el ultimo evento
 * @param ftm: módulo FTM
 * @param channel: Canal del modulo FTM
 * @return valore del contador del canal
*/
FTM_tick_t ICGetCont(FTM_MODULE ftm, FTM_CHANNEL channel) {

	hw_DisableInterrupts();
	// TODO: Disable interrupts
	ICnewEdge[ftm][channel] = 0;
	// TODO: Enable interrupts
	FTM_tick_t ticks = ICLastTicks[ftm][channel];
	hw_EnableInterrupts();

	return ticks;

}

/**
 * @brief Devuelve si hay un nuevo flanco detectado
 * @param ftm: módulo FTM
 * @param channel: Canal del modulo FTM
 * @return true si hay un nuevo flanco
*/
bool ICisEdge(FTM_MODULE ftm, FTM_CHANNEL channel) {

	hw_DisableInterrupts();
	// TODO: Disable interrupts
	bool new = ICnewEdge[ftm][channel];
	if (new) ICnewEdge[ftm][channel] = 0;
	// TODO: Enable interrupts
	hw_EnableInterrupts();

	return new;
}

/**
 * @brief Reinicia el contador y el valor del canal
 * @param ftm: módulo FTM
 * @param channel: Canal del modulo FTM
*/
void ICReset(FTM_MODULE ftm, FTM_CHANNEL channel) {

	// TODO: Disable interrupts while modifying values
	FTMReset(ftm);
	ICLastTicks[ftm][channel] = 0;
	ICTOFCont[ftm][channel] = 0;
	ICChannelValue[ftm][channel] = 0;

}

/*******************************************************************************
 *******************************************************************************
                        LOCAL FUNCTION DEFINITIONS
 *******************************************************************************
 ******************************************************************************/

void FTM_IRQHandler(FTM_MODULE ftm) {
	gpioWrite(TEST_PIN, HIGH);
	FTM_Type* const pFTM = FTMPtrs[ftm];
	uint8_t status = pFTM->STATUS;

	if (pFTM->SC & FTM_SC_TOF_MASK) {	// Overflow
		pFTM->SC &= ~FTM_SC_TOF_MASK;
//		uint32_t* pTOF = ICTOFCont[ftm];
		for (uint8_t i = 0; i < FTM_CH_COUNT; i++) {
//			(*pTOF++)++;	// Increment all channel counters
			if (PWMPtrs[ftm][i] != NULL) {
				pFTM->CONTROLS[i].CnV = (double)(**PWMPtrs[ftm][i])/U12_MAX_VAL*(pFTM->MOD & FTM_MOD_MOD_MASK);
			}
		}
	}

	if (status && FTMMode[ftm] == 1) {		// Channel Event
		for (uint8_t j = 0; j < FTMICLength[ftm]; j++) {
			uint8_t i = FTMICList[ftm][j];
			if (status & (1UL << i)) {
				// Calculate ticks diff
				ICLastTicks[ftm][i] = (pFTM->CONTROLS[i].CnV - ICChannelValue[ftm][i] + (FTM_MAX_VAL+1) ) % (FTM_MAX_VAL+1);
//				ICLastTicks[ftm][i] = pFTM->CONTROLS[i].CnV - ICChannelValue[ftm][i] + ICTOFCont[ftm][i]*(FTM_MAX_VAL+1);
				// Save new value
				ICChannelValue[ftm][i] = pFTM->CONTROLS[i].CnV;

				ICTOFCont[ftm][i] = 0;	// Reset TOF counter

				// Exec callback
				if (ICCbPtrs[ftm][i] != NULL) ICCbPtrs[ftm][i](ICLastTicks[ftm][i]);
				else ICnewEdge[ftm][i] = true;
			}
		}
		pFTM->STATUS = 0x00;	// Clear all flags




//		for (uint8_t i = 0; i < FTM_CH_COUNT; i++) {
//			if (status & (1UL << i)) {
////				pFTM->STATUS &= ~(1UL << i);	// Write 0 to clear flag
//				// Calculate ticks diff
//				ICLastTicks[ftm][i] = pFTM->CONTROLS[i].CnV - ICChannelValue[ftm][i] + ICTOFCont[ftm][i]*(FTM_MAX_VAL+1);
//				// Save new value
//				ICChannelValue[ftm][i] = pFTM->CONTROLS[i].CnV;
//
//				ICTOFCont[ftm][i] = 0;	// Reset TOF counter
//
//				// Exec callback
//				if (ICCbPtrs[ftm][i] != NULL) ICCbPtrs[ftm][i](ICLastTicks[ftm][i]);
//				else ICnewEdge[ftm][i] = true;
//			}
//		}
//		pFTM->STATUS = 0x00;	// Clear all flags
	}
	gpioWrite(TEST_PIN, LOW);

}


__ISR__ FTM0_IRQHandler () {
//	FTM_IRQHandler(FTM_0);


	gpioWrite(TEST_PIN, HIGH);

	if (FTM0->SC & FTM_SC_TOF_MASK) {	// Overflow
		FTM0->SC &= ~FTM_SC_TOF_MASK;
		FTM0->CONTROLS[2].CnV = (uintmax_t)(**PWMPtrs[0][2])*(FTM0->MOD & FTM_MOD_MOD_MASK)/U12_MAX_VAL;
	}

	gpioWrite(TEST_PIN, LOW);

}

__ISR__ FTM1_IRQHandler () {

	gpioWrite(TEST_PIN, HIGH);

	uint8_t status = FTM1->STATUS;
	if (status & FTM_STATUS_CH0F_MASK) {		// Channel Event

		uint32_t cnv = FTM1->CONTROLS[0].CnV;

		ICLastTicks[1][0] = ((FTM_MAX_VAL+1) + cnv - ICChannelValue[1][0] ) % (FTM_MAX_VAL+1);
//				ICLastTicks[ftm][i] = pFTM->CONTROLS[i].CnV - ICChannelValue[ftm][i] + ICTOFCont[ftm][i]*(FTM_MAX_VAL+1);
		// Save new value
		ICChannelValue[1][0] = cnv;

//		ICTOFCont[1][0] = 0;	// Reset TOF counter

		// Exec callback
		if (ICCbPtrs[1][0] != NULL) ICCbPtrs[1][0](ICLastTicks[1][0]);
		else ICnewEdge[1][0] = true;
//		}
		FTM1->STATUS = 0x00;	// Clear all flags
	}

	gpioWrite(TEST_PIN, LOW);

}

//__ISR__ FTM2_IRQHandler () {
//	FTM_IRQHandler(FTM_2);
//}
//
//__ISR__ FTM3_IRQHandler () {
//	FTM_IRQHandler(FTM_3);
//}
