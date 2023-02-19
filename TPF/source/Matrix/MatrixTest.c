/***************************************************************************//**
  @file     MatrixTest.c
  @brief    Archivo de prueba de la Matriz
  @author   Grupo 5
 ******************************************************************************/

/*******************************************************************************
 * INCLUDE HEADER FILES
 ******************************************************************************/
#include "Matrix.h"
#include "../timer/timer.h"
#include "../MCAL/gpio.h"

/*******************************************************************************
 * CONSTANT AND MACRO DEFINITIONS USING #DEFINE
 ******************************************************************************/
#define DEBUG_DELAY 200

/*******************************************************************************
 *******************************************************************************
                        GLOBAL VARIABLES
 *******************************************************************************
 ******************************************************************************/

uint8_t colsTest[9][8] = {{0,1,2,3,4,5,6,7},
                          {1,2,3,4,5,6,7,8},
                          {2,3,4,5,6,7,8,0},
                          {3,4,5,6,7,8,0,1},
                          {4,5,6,7,8,0,1,2},
                          {5,6,7,8,0,1,2,3},
                          {6,7,8,0,1,2,3,4},
                          {7,8,0,1,2,3,4,5},
                          {8,0,1,2,3,4,5,6},
                        };

//colsTest[8]={0,2,3,4,5,6,7,8};

LED_RGB color1={.red=150, .blue=0, .green=0};
LED_RGB color2={.red=0, .blue=50, .green=0};
LED_RGB color3={.red=0, .blue=0, .green=200};

/*******************************************************************************
 *******************************************************************************
                        GLOBAL FUNCTION DEFINITIONS
 *******************************************************************************
 ******************************************************************************/
/* Función que se llama 1 vez, al comienzo del programa */
void App_Init (void){
	timerInit();
	initMatrix();	
}

/* Función que se llama constantemente en un ciclo infinito */
void App_Run (void)
{
	/*while(1){
		fullMatrixON();
		timerDelay(TIMER_MS2TICKS(1000));
		clearMatrix(0);
		timerDelay(TIMER_MS2TICKS(1000));
	}*/


    for (uint8_t i = 0; i < 5; i++){
        increase_bright();
        fullMatrixON();
        timerDelay(TIMER_MS2TICKS(DEBUG_DELAY));

    }

   for (uint8_t i = 0; i < 10; i++){
        decrease_bright();
        fullMatrixON();
        timerDelay(TIMER_MS2TICKS(DEBUG_DELAY));

    }
    
    for (uint8_t i = 0; i < 5; i++){
        increase_bright();
        fullMatrixON();
        timerDelay(TIMER_MS2TICKS(DEBUG_DELAY));
    }
    
    clearMatrix(0);
    timerDelay(TIMER_MS2TICKS(DEBUG_DELAY));
    
    setLedMatrix(0, 0, &color1);
    timerDelay(TIMER_MS2TICKS(20));
    setLedMatrix(1,1, &color2);
    timerDelay(TIMER_MS2TICKS(20));
    setLedMatrix(5, 3, &color3);
    timerDelay(TIMER_MS2TICKS(3000));

    for(uint8_t i=0; i<COLS_CANT+1; i++){
        setColumnsMatrix(colsTest[i]);
        timerDelay(TIMER_MS2TICKS(3*DEBUG_DELAY));
    }

}


/*******************************************************************************
 *******************************************************************************
                        LOCAL FUNCTION DEFINITIONS
 *******************************************************************************
 ******************************************************************************/


/*******************************************************************************
 ******************************************************************************/
