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
#include "../DMA2/DMA2.h"
#include "../MCAL/gpio.h"

/*******************************************************************************
 * CONSTANT AND MACRO DEFINITIONS USING #DEFINE
 ******************************************************************************/


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

static tim_id_t IDTimer;

LED_RGB color1={.red=150, .blue=0, .green=0};
LED_RGB color2={.red=0, .blue=50, .green=0};
LED_RGB color3={.red=0, .blue=0, .green=200};

static uint16_t matrizfacha[1585];
/*******************************************************************************
 *******************************************************************************
                        GLOBAL FUNCTION DEFINITIONS
 *******************************************************************************
 ******************************************************************************/
/* Función que se llama 1 vez, al comienzo del programa */
void App_Init (void){
    timerInit();
    IDTimer=timerGetId();
    gpioMode (PORTNUM2PIN(PC, 6), INPUT_PULLUP);
	//initMatrix();   // Pin PC1

    for(uint16_t i=0; i<1536; i++){
    	matrizfacha[i]=20;
    }
}

/* Función que se llama constantemente en un ciclo infinito */
void App_Run (void)
{
	/*initMatrix();
	while(1){
        if( gpioRead(PORTNUM2PIN(PC, 6)) == 0){
    		fullMatrixON();
            while(gpioRead(PORTNUM2PIN(PC, 6)) == 0);
        }
    }*/

	/*initMatrix();   // Pin PC1
	while(1){
		//fullMatrixON();
		//timerDelay(TIMER_MS2TICKS(1000));
		//clearMatrix();
		//timerDelay(TIMER_MS2TICKS(1000));
}*/

	increase_bright();
    for (uint8_t i = 0; i < 5; i++){
        increase_bright();
    }

    for (uint8_t i = 0; i < 10; i++){
        decrease_bright();
    }
    
    for (uint8_t i = 0; i < 5; i++){
        increase_bright();
    }

    for (uint8_t i = 0; i < 9; i++){
        setColumnsMatrix(colsTest[i]);
    }
    
    clearMatrix();
    setLedMatrix(0, &color1);
    setLedMatrix(20, &color2);
    setLedMatrix(40, &color3);
}


/*******************************************************************************
 *******************************************************************************
                        LOCAL FUNCTION DEFINITIONS
 *******************************************************************************
 ******************************************************************************/


/*******************************************************************************
 ******************************************************************************/
