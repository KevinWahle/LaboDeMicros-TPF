/***************************************************************************//**
  @file     FTMTest.c
  @brief    Archivo de prueba para modulo FTM
  @author   Grupo 5
 ******************************************************************************/

/*******************************************************************************
 * INCLUDE HEADER FILES
 ******************************************************************************/

#include "FTM.h"
#include "timer/timer.h"
#include "UART/uart.h"
#include <stdio.h>

/*******************************************************************************
 * CONSTANT AND MACRO DEFINITIONS USING #DEFINE
 ******************************************************************************/

#define IC_FTM_MOD	FTM_2	// Pin PB 18
#define IC_FTM_CH	0

#define PWM_MOD	FTM_0		// Pin PC 3
#define PWM_CH	2

#define PWM_FREQ	800000	// Hz

#define UART_ID	0
#define UART_BAUDRATE	115200

/*******************************************************************************
 *******************************************************************************
                        GLOBAL VARIABLES
 *******************************************************************************
 ******************************************************************************/

uint8_t cont = 0;

bool flag = true;

uint16_t* ptr;

uint16_t arr[] = {2660, 2660, 2660, 2660, 2660, 2660, 2660, 2660,
				2660, 2660, 2660, 2660, 2660, 2660, 2660, 2660,
				2660, 2660, 2660, 2660, 2660, 2660, 2660, 2660};

uint8_t index = 0;

#define ARR_L	24

/*******************************************************************************
 *******************************************************************************
                        GLOBAL FUNCTION DEFINITIONS
 *******************************************************************************
 ******************************************************************************/

void inCapCb(FTM_tick_t ticks) {

	char msg[100];
//	uint8_t cant = sprintf(msg, "%llu\r\n", ticks);
	uint8_t cant = sprintf(msg, "%f\r\n", FTM_TICK2MS((double)ticks));
	uartWriteMsg(UART_ID, msg, cant);
}

void changePointer() {
	index = (index+1)%ARR_L;
	ptr = arr + index;
}

/* Función que se llama 1 vez, al comienzo del programa */
void App_Init (void)
{
//	timerInit();
//	uart_cfg_t config = {.baudrate=UART_BAUDRATE, .MSBF=false, .parity=NO_PARITY};
//	uartInit(UART_ID, config);
//	PWMInit(PWM_MOD, PWM_CH, PWM_FREQ);
//	PWMStart(PWM_MOD, PWM_CH, 0.5);

//	ICInit(IC_FTM_MOD, IC_FTM_CH, CAPTURE_BOTH, NULL);
	// ICInit(IC_FTM_MOD, IC_FTM_CH, CAPTURE_BOTH, inCapCb);

	// Pointer PWM Test
	timerInit();

	ptr = arr;
	PWMInit(PWM_MOD, PWM_CH, PWM_FREQ);
//	PWMFromPtr(PWM_MOD, PWM_CH, &ptr);
	PWMStart(PWM_MOD, PWM_CH, 0.33);

//	timerStart(timerGetId(), TIMER_MS2TICKS(1), TIM_MODE_PERIODIC, changePointer);

}

/* Función que se llama constantemente en un ciclo infinito */
void App_Run (void)
{
	// PWMStart(PWM_MOD, PWM_CH, 0.01*cont);

	// if (flag) cont++;
	// else cont--;

	// if (cont >= 100 || cont == 0) flag=!flag;
	// timerDelay(TIMER_MS2TICKS(100));

//	if (ICisEdge(IC_FTM_MOD, IC_FTM_CH)) {
//		FTM_tick_t ticks = ICGetCont(IC_FTM_MOD, IC_FTM_CH);
//		char msg[100];
//	//	uint8_t cant = sprintf(msg, "%llu\r\n", ticks);
//		uint8_t cant = sprintf(msg, "%f\r\n", FTM_TICK2MS((double)ticks));
//		uartWriteMsg(UART_ID, msg, cant);
//	}

}


/*******************************************************************************
 *******************************************************************************
                        LOCAL FUNCTION DEFINITIONS
 *******************************************************************************
 ******************************************************************************/


/*******************************************************************************
 ******************************************************************************/
