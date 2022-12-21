/***************************************************************************//**
  @file		ADC_test.c
  @brief	+Descripcion del archivo+
  @author	KevinWahle
  @date		21 oct. 2022
 ******************************************************************************/

/*******************************************************************************
 * INCLUDE HEADER FILES
 ******************************************************************************/

#include "ADC_hal.h"
//#include "../UART/uart.h"
#include <stdio.h>
#include "../DAC/DAC_hal.h"
/*******************************************************************************
 * CONSTANT AND MACRO DEFINITIONS USING #DEFINE
 ******************************************************************************/

//#define UART_ID			0
//#define UART_BAUDRATE	9600

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

// +ej: static void falta_envido (int);+


/*******************************************************************************
 * ROM CONST VARIABLES WITH FILE LEVEL SCOPE
 ******************************************************************************/

// +ej: static const int temperaturas_medias[4] = {23, 26, 24, 29};+


/*******************************************************************************
 * STATIC VARIABLES AND CONST VARIABLES WITH FILE LEVEL SCOPE
 ******************************************************************************/


circularBuffer16 buff;

void App_Init (void)
{
	CBinit16(&buff,200);
	//uart_cfg_t cfg = {.MSBF = false, .baudrate = UART_BAUDRATE, .parity = NO_PARITY};
	 ADCh_Init(DIVh_t8,&buff);
	 ADCh_Start(10);
	 DACh_Init(10,&buff);

}

void App_Run (void)
{


  while(1){

  }
}

 
