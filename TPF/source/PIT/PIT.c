/***************************************************************************//**
  @file		PIT.c
  @brief	Funciones del timer PIT
  @author	Grupo 5
  @date		24 oct. 2022
 ******************************************************************************/

/*******************************************************************************
 * INCLUDE HEADER FILES
 ******************************************************************************/

#include "PIT.h"
#include "MK64F12.h"
#include "hardware.h"
#include <stddef.h>

#include "../MCAL/gpio.h"
/*******************************************************************************
 * CONSTANT AND MACRO DEFINITIONS USING #DEFINE
 ******************************************************************************/

#define PIT_IRQS_2	{ PIT0_IRQn, PIT1_IRQn, PIT2_IRQn, PIT3_IRQn }


/*******************************************************************************
 * ROM CONST VARIABLES WITH FILE LEVEL SCOPE
 ******************************************************************************/

static const IRQn_Type PITIRQs[] = PIT_IRQS_2;

/*******************************************************************************
 * STATIC VARIABLES AND CONST VARIABLES WITH FILE LEVEL SCOPE
 ******************************************************************************/

static PITCallback_t PITCbs[PIT_COUNT];

//#define ENABLE_TP

#ifdef ENABLE_TP
#define TP_PIN	PORTNUM2PIN(PC, 8)
#endif


/*******************************************************************************
 *******************************************************************************
                        GLOBAL FUNCTION DEFINITIONS
 *******************************************************************************
 ******************************************************************************/


/**
 * @brief Inicializa un timer periodico. NO se inicia hasta llamar a PITStart
 * @param pit: modulol PIT a utilizar
 * @param ticks: cantidad de tiempo a contar, en ticks
 * @param cb: NULL o callback a llamar en cada periodo
*/
void PITInit(PIT_MOD pit, PITTick_t ticks, PITCallback_t cb) {
	#ifdef ENABLE_TP
		gpioMode(TP_PIN, OUTPUT);
		gpioWrite(TP_PIN, LOW);
	#endif
	// Clock gating
	SIM->SCGC6 |= SIM_SCGC6_PIT_MASK;

	// PIT Enable
	PIT->MCR = 0x00;

	// Set interrupts and stop timer
	if (cb != NULL) {
		PITCbs[pit] = cb;
		PIT->CHANNEL[pit].TCTRL = PIT_TCTRL_TIE_MASK;
		NVIC_EnableIRQ(PITIRQs[pit]);
	}
	else {
		PIT->CHANNEL[pit].TCTRL = 0x00;
	}

	// Set Load Value
	PIT->CHANNEL[pit].LDVAL = ticks;

}

/**
 * @brief Inicia la cuenta del timer
 * @param pit: modulol PIT a iniciar
*/
void PITStart(PIT_MOD pit) {
	PIT->CHANNEL[pit].TCTRL |= PIT_TCTRL_TEN_MASK;		// Start the timer
}

/**
 * @brief Detiene un timer en curso
 * @param pit: modulol PIT a deteenr
*/
void PITStop(PIT_MOD pit) {
	PIT->CHANNEL[pit].TCTRL &= ~PIT_TCTRL_TEN_MASK;		// Stop the timer
}


/*******************************************************************************
 *******************************************************************************
                        LOCAL FUNCTION DEFINITIONS
 *******************************************************************************
 ******************************************************************************/

__ISR__ PIT0_IRQHandler() {
	#ifdef ENABLE_TP
		gpioWrite(TP_PIN, HIGH);
	#endif

	PIT->CHANNEL[0].TFLG = PIT_TFLG_TIF_MASK;		// Clear flag
	PITCbs[0]();									// Callback()

	#ifdef ENABLE_TP
		gpioWrite(TP_PIN, LOW);
	#endif
}

__ISR__ PIT1_IRQHandler() {
	PIT->CHANNEL[1].TFLG = PIT_TFLG_TIF_MASK;		// Clear flag
	PITCbs[1]();									// Callback()
}

__ISR__ PIT2_IRQHandler() {
	PIT->CHANNEL[2].TFLG = PIT_TFLG_TIF_MASK;		// Clear flag
	PITCbs[2]();									// Callback()
}

__ISR__ PIT3_IRQHandler() {
	PIT->CHANNEL[3].TFLG = PIT_TFLG_TIF_MASK;		// Clear flag
	PITCbs[3]();									// Callback()
}

 
