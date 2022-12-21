/***************************************************************************//**
  @file     App.c
  @brief    TP2: Comunicacion Serie
  @author   Grupo 5
 ******************************************************************************/

/*******************************************************************************
 * INCLUDE HEADER FILES
 ******************************************************************************/

#include "I2Cm/I2Cm.h"
#include "timer/timer.h"
#include "MCAL/gpio.h"
#include "MCAL/board.h"
/*******************************************************************************
 * CONSTANT AND MACRO DEFINITIONS USING #DEFINE
 ******************************************************************************/

#define I2C_ID		I2C_0

#define I2C_ADDR	0x1D
//#define I2C_ADDR	0xFF


/*******************************************************************************
 * FUNCTION PROTOTYPES FOR PRIVATE FUNCTIONS WITH FILE LEVEL SCOPE
 ******************************************************************************/

/*******************************************************************************
 *******************************************************************************
                        GLOBAL VARIABLES
 *******************************************************************************
 ******************************************************************************/


/*******************************************************************************
 *******************************************************************************
                        GLOBAL FUNCTION DEFINITIONS
 *******************************************************************************
 ******************************************************************************/
uint8_t readBuffer [10];
uint8_t writeBuffer = 0x0D;  // WHO AM I -> DEVICE ID
/* Función que se llama 1 vez, al comienzo del programa */
void App_Init (void)
{
	gpioMode(PIN_LED_RED, OUTPUT);
	gpioWrite(PIN_LED_RED, LOW);

	I2CmInit(I2C_ID);
	timerInit();
	I2CmStartTransaction(I2C_ID, I2C_ADDR, &writeBuffer, 1, readBuffer, 1);
}

/* Función que se llama constantemente en un ciclo infinito */
void App_Run (void)
{
	timerDelay(TIMER_MS2TICKS(1000));
	gpioToggle(PIN_LED_RED);
}


/*******************************************************************************
 *******************************************************************************
                        LOCAL FUNCTION DEFINITIONS
 *******************************************************************************
 ******************************************************************************/


/*******************************************************************************
 ******************************************************************************/
