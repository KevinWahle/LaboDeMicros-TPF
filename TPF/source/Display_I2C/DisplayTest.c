/***************************************************************************//**
  @file     DisplayTest.c
  @brief    Archivo de prueba del Display con I2C
  @author   Grupo 5
 ******************************************************************************/

/*******************************************************************************
 * INCLUDE HEADER FILES
 ******************************************************************************/
#include "Display.h"
#include "../timer/timer.h"
/*******************************************************************************
 * CONSTANT AND MACRO DEFINITIONS USING #DEFINE
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
/* Función que se llama 1 vez, al comienzo del programa */
void App_Init (void){
	timerInit();
}

/* Función que se llama constantemente en un ciclo infinito */
void App_Run (void){
    initDisplay();

    while(1)
    {
    	clearDisplay();
    	displayLine(0, "Primero Arg");
		displayLine(1, "Segundo Fra");

		timerDelay(TIMER_MS2TICKS(2000));
		clearDisplay();
		displayText(0, 2, "WORLD CHAMPs");
		displayText(1, 2, "D10s >> ");
		displayText(1, 10, "PELE");

		timerDelay(TIMER_MS2TICKS(2000));
    }


    /*while(1){
    	backlightOFF();
    	timerDelay(TIMER_MS2TICKS(250));
    	backlightON();
    	timerDelay(TIMER_MS2TICKS(250));
    }*/
}


/*******************************************************************************
 *******************************************************************************
                        LOCAL FUNCTION DEFINITIONS
 *******************************************************************************
 ******************************************************************************/


/*******************************************************************************
 ******************************************************************************/
