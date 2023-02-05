/***************************************************************************//**
  @file     App.c
  @brief    TPF: Reproductor MP3
  @author   Grupo 5
 ******************************************************************************/

/*******************************************************************************
 * INCLUDE HEADER FILES
 ******************************************************************************/

#include "SpectrumAnalyzer.h"
#include "MCAL/board.h"
#include <arm_math.h>
#include <stdio.h>
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
#define T (1.0/44100.0) // s
#define F 1000 // Hz
float32_t pSrc[512];
float32_t pDst[512];
/* Función que se llama 1 vez, al comienzo del programa */
void App_Init (void)
{
	gpioMode(PORTNUM2PIN(PB, 2), OUTPUT);
	gpioWrite(PORTNUM2PIN(PB, 2), LOW);
}

/* Función que se llama constantemente en un ciclo infinito */
void App_Run (void)
{
	for(uint16_t n = 0; n < 512; n++){
		pSrc[n] = 32767*sin(2*PI*F*n*T)/2 + 32767*cos(2*PI*200*n*T)/4;
	}
	//initEqualizer();
	//setUpFilter(0, 0);
	//setUpFilter(0, 1);
	//setUpFilter(-6, 2);
	//setUpFilter(0, 3);
	//blockEqualizer(pSrc, pDst, 512);
	uint8_t data[8];
	startAnalyzer(pSrc, 512);
	getAnalyzer(data);
	printf("[");
	for(uint16_t i = 0; i < 512; i++){
		printf(" %f", pDst[i]);
	}
	printf("]");
	while(1);
}

/*******************************************************************************
 *******************************************************************************
                        LOCAL FUNCTION DEFINITIONS
 *******************************************************************************
 ******************************************************************************/


/*******************************************************************************
 ******************************************************************************/
