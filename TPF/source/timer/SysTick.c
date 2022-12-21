/***************************************************************************//**
  @file     SysTick.c
  @brief    SysTick driver
  @author   Grupo 5
 ******************************************************************************/

/*******************************************************************************
 * INCLUDE HEADER FILES
 ******************************************************************************/

#include "SysTick.h"
#include "hardware.h"
#include "MK64F12.h"

/*******************************************************************************
 * CONSTANT AND MACRO DEFINITIONS USING #DEFINE
 ******************************************************************************/

//#define ENABLE_TP		// Enable Testpoint to check Interruption time

#ifdef ENABLE_TP
#include "MCAL/gpio.h"
#endif

#define TP_PIN	PORTNUM2PIN(PC, 10)

/*******************************************************************************
 * GLOBAL VARIABLES
 ******************************************************************************/

static void (*SysTickCallback)(void);

/*******************************************************************************
 *******************************************************************************
                        GLOBAL FUNCTION DEFINITIONS
 *******************************************************************************
 ******************************************************************************/

/**
 * @brief Initialize SysTic driver
 * @param funcallback Function to be call every SysTick
 * @return Initialization and registration succeed
 */
bool SysTick_Init (void (*funcallback)(void)) {

	SysTick->CTRL = 0x00;	// Disable SysTick

	SysTick->VAL = 0;		// SysTick value reset

	SysTick->LOAD = 1000 - 1;	// 10us @ 100MHz

	NVIC_EnableIRQ(SysTick_IRQn);

	SysTickCallback = funcallback;

	SysTick->CTRL |= SysTick_CTRL_ENABLE_Msk | SysTick_CTRL_CLKSOURCE_Msk | SysTick_CTRL_TICKINT_Msk;	// Enable SysTick

#ifdef ENABLE_TP
	gpioMode(TP_PIN, OUTPUT);
#endif

	return true;
}


/*******************************************************************************
 *******************************************************************************
                        LOCAL FUNCTION DEFINITIONS
 *******************************************************************************
 ******************************************************************************/

__ISR__ SysTick_Handler (void){

#ifdef ENABLE_TP
	gpioWrite(TP_PIN, HIGH);
#endif

	// No es necesario borrar el flag

	SysTickCallback();

#ifdef ENABLE_TP
	gpioWrite(TP_PIN, LOW);
#endif

}
