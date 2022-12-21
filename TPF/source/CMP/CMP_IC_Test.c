/***************************************************************************//**
  @file		CMP_test.c
  @brief	+Descripcion del archivo+
  @author	Fachas
  @date		24 oct. 2022
 ******************************************************************************/

/*******************************************************************************
 * INCLUDE HEADER FILES
 ******************************************************************************/

// +Incluir el header propio (ej: #include "template.h")+
#include "CMP.h"
#include "FTM/FTM.h"
#include "UART/uart.h"
#include <stdio.h>
/*******************************************************************************
 * CONSTANT AND MACRO DEFINITIONS USING #DEFINE
 ******************************************************************************/

#define UART_ID			0
#define UART_BAUDRATE	115200

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

static void inCapCb(FTM_tick_t ticks);

/*******************************************************************************
 * ROM CONST VARIABLES WITH FILE LEVEL SCOPE
 ******************************************************************************/

// +ej: static const int temperaturas_medias[4] = {23, 26, 24, 29};+


/*******************************************************************************
 * STATIC VARIABLES AND CONST VARIABLES WITH FILE LEVEL SCOPE
 ******************************************************************************/

// +ej: static int temperaturas_actuales[4];+


void App_Init (void)
{
	CMP_Init(CMP0_t, level_3, no_inv);
	ICInit(FTM_1, 0, CAPTURE_BOTH, inCapCb);
	uart_cfg_t config = {.baudrate=UART_BAUDRATE, .MSBF=false, .parity=NO_PARITY};
	uartInit(UART_ID, config);
}

void App_Run (void)
{
	uint8_t i=0;
	while(1){
		i++;
	}
}


void inCapCb(FTM_tick_t ticks) {
	char msg[100];
//	uint8_t cant = sprintf(msg, "%llu\r\n", ticks);
	uint8_t cant = sprintf(msg, "%f\r\n", FTM_TICK2MS((double)ticks));
	uartWriteMsg(UART_ID, msg, cant);
}
 
