/***************************************************************************//**
  @file		DeepSleep.c
  @brief	+Descripcion del archivo+
  @author	KevinWahle
  @date		3 ene. 2023
 ******************************************************************************/

/*******************************************************************************
 * INCLUDE HEADER FILES
 ******************************************************************************/

#include "DeepSleep.h"

#include "../MCAL/board.h"
#include "../MCAL/gpio.h"

#include "MK64F12.h"
#include "hardware.h"
#include "core_cm4.h"
#include "../event_queue/event_queue.h"
#include "const.h"
#include <stdint.h>

/*******************************************************************************
 * CONSTANT AND MACRO DEFINITIONS USING #DEFINE
 ******************************************************************************/
#define DEBUG_sleep
#define WAKE_UP_PIN PORTNUM2PIN(PC,3)	//P7


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

static void deepSleep(void);
static void LLS_btn_cb (void);


/*******************************************************************************
 * ROM CONST VARIABLES WITH FILE LEVEL SCOPE
 ******************************************************************************/

// +ej: static const int temperaturas_medias[4] = {23, 26, 24, 29};+


/*******************************************************************************
 * STATIC VARIABLES AND CONST VARIABLES WITH FILE LEVEL SCOPE
 ******************************************************************************/

static bool btn_pressed;

/*******************************************************************************
 *******************************************************************************
                        GLOBAL FUNCTION DEFINITIONS
 *******************************************************************************
 ******************************************************************************/
static SIM_Type* sim_ptr = SIM;				// For clock enable
static LLWU_Type* LLWUPorts = LLWU_BASE_PTRS;
static PORT_Type* const portPtr[] = PORT_BASE_PTRS;
static SMC_Type* SMCPort = SMC_BASE_PTRS;

// Initial configuration of sleep mode (LLS)
void LLS_config (void){
	volatile unsigned int dummyread;
	 /* Enable Port C to be a digital pin. */
	 sim_ptr->SCGC5 |= SIM_SCGC5_PORTC_MASK;

	 portPtr[PIN2PORT(WAKE_UP_PIN)]->PCR[PIN2NUM(WAKE_UP_PIN)] = (PORT_PCR_ISF_MASK | // Clear flag if there
			 PORT_PCR_MUX(01) | 	// Set pin functionality -GPIO
			 PORT_PCR_IRQC(0x0A)| 	// Falling edge interrupt enable
			 PORT_PCR_PE_MASK|		// Pull enable
			 PORT_PCR_PS_MASK);  	// Pull up
	 LLWUPorts->PE2 |= LLWU_PE2_WUPE7(2); // Falling edge detection


	  // Clear the wake-up flag in the LLWU-write one to clear the flag
	  if (LLWUPorts->F1 & LLWU_F1_WUF7_MASK) {
		  LLWUPorts->F1 |= LLWU_F1_WUF7_MASK;
	  }

	 LLWUPorts->RST &=~0x03;	// RESET pin not enabled, digital filter for the RESET pin not enable

	 gpioIRQ(WAKE_UP_PIN, GPIO_IRQ_MODE_FALLING_EDGE, LLS_btn_cb);	// Set btn falling edge interruption


	 btn_pressed=false;

	#ifdef DEBUG_sleep
		gpioMode(PIN_LED_GREEN, OUTPUT);
		gpioWrite(PIN_LED_GREEN,!LED_ACTIVE);
		gpioMode(PIN_LED_BLUE, OUTPUT);
		gpioWrite(PIN_LED_BLUE,!LED_ACTIVE);
		gpioMode(PIN_LED_RED, OUTPUT);
		gpioWrite(PIN_LED_RED,!LED_ACTIVE);
	#endif

	 NVIC_EnableIRQ(LLWU_IRQn);
}

// Check if btn is pressed
bool is_onoff_btn(void){
	bool is_pressed=btn_pressed;
	btn_pressed=false;
	return is_pressed;
}

// LLS mode entry routine. Puts the processor into LLS mode from normal Run mode
void LLS_start(void)
{
	volatile unsigned int dummyread;

	#ifdef DEBUG_sleep
		gpioWrite(PIN_LED_GREEN,LED_ACTIVE);
	#endif

	/* Write to PMPROT to allow LLS power modes this write-once
	bit allows the MCU to enter the LLS low power mode*/
	SMCPort->PMPROT = SMC_PMPROT_ALLS_MASK;

	/* Set the (for MC1) LPLLSM or (for MC2)STOPM field to 0b011 for LLS mode
	Retains LPWUI and RUNM values */
	SMCPort->PMCTRL &= ~SMC_PMCTRL_STOPM_MASK ;
	SMCPort->PMCTRL |= SMC_PMCTRL_STOPM(0x3) ;

	// Wait for write to complete to SMC before stopping core
	dummyread = SMCPort->PMCTRL;

	// Now execute the stop instruction to go into LLS
	deepSleep();

	#ifdef DEBUG_sleep
		gpioWrite(PIN_LED_GREEN,!LED_ACTIVE);
	#endif
}





/*******************************************************************************
 *******************************************************************************
                        LOCAL FUNCTION DEFINITIONS
 *******************************************************************************
 ******************************************************************************/
//Enters sleep mode
void deepSleep(void)
{
	volatile unsigned int dummyread;

    //Set the SLEEPDEEP bit to enable deep sleep mode (STOP)
    SCB->SCR |= SCB_SCR_SLEEPDEEP_Msk;

    __DSB();

    //Enters sleep mode
    __WFI();

    //wait for write to complete to SMC before stopping core
    dummyread = SMCPort->PMCTRL;

    SCB->SCR &= ~SCB_SCR_SLEEPDEEP_Msk;

    //Interrupt can take place here
    __ISB();
}



void LLS_btn_cb (void){
	btn_pressed=true;
	add_event(BTN_SLEEP);
}


void LLW_IRQHandler(void){
	  if (LLWUPorts->F1 & LLWU_F1_WUF7_MASK) {
		  LLWUPorts->F1 |= LLWU_F1_WUF7_MASK;
	  }
	  hw_Init();
	  //MCG->C1 &= ~MCG_C1_CLKS_MASK;
}


 
