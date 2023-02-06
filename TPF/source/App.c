/***************************************************************************//**
  @file     App.c
  @brief    TPF: Reproductor MP3
  @author   Grupo 5
 ******************************************************************************/

/*******************************************************************************
 * INCLUDE HEADER FILES
 ******************************************************************************/

#include "const.h"
#include "FSM_table.h"
#include "FSM_routines.h"
#include "event_queue/event_queue.h"
#include "timer/timer.h"
#include "encoder/encoder_hal.h"
#include "Display_I2C/Display.h"
#include "Matrix/Matrix.h"
#include "Keypad/keypad.h"
#include "SpectrumAnalyzer/SpectrumAnalyzer.h"
#include "MP3Dec/Mp3Dec.h"
#include "DMA2/DMA2.h"

// DEBUG
#include "MCAL/gpio.h"
#define TESTPIN	PORTNUM2PIN(PB, 2)
/*******************************************************************************
 *******************************************************************************
                        	GLOBAL VARIABLES
 *******************************************************************************
 ******************************************************************************/


static STATE* p_tabla_estado_actual;

/*******************************************************************************
 *******************************************************************************
                        GLOBAL FUNCTION DEFINITIONS
 *******************************************************************************
 ******************************************************************************/

/* Función que se llama 1 vez, al comienzo del programa */
void App_Init (void)
{
	timerInit();

	// DEBUG
	gpioMode(TESTPIN, OUTPUT);
	gpioWrite(TESTPIN, LOW);
 
}

/* Función que se llama constantemente en un ciclo infinito */
void App_Run (void)
{

    initDisplay();
	encoderInit(encoderCallback);
	initMatrix();
	clearMatrix(0);		// TODO: Por que no lo hace la inicializacion???
    keypadInit(add_event);
    MP3DecInit();
	DMA_initPingPong_Dac();
	initEqualizer();
	setUpFilter(0, 0);
	setUpFilter(0, 1);
	setUpFilter(0, 2);
	setUpFilter(0, 3);

    //Splash Screen
	p_tabla_estado_actual = splash_state;
    displayLine(0, "Reproductor MP3");
    displayLine(1, "    Grupo 5    ");
    
    timerStart(timerGetId(), TIMER_MS2TICKS(3000), TIM_MODE_SINGLESHOT, addTimeout);


    while (1) {
        event_t evento = get_next_event();  // Tomo un nuevo evento de la cola de eventos.
        p_tabla_estado_actual = fsm_interprete(p_tabla_estado_actual, evento);  // Actualizo el estado
    }
}

/*******************************************************************************
 *******************************************************************************
                        LOCAL FUNCTION DEFINITIONS
 *******************************************************************************
 ******************************************************************************/

STATE *fsm_interprete(STATE * p_tabla_estado_actual, event_t evento_actual)
{
    while ((p_tabla_estado_actual -> evento) != evento_actual && (p_tabla_estado_actual -> evento) !=NULL_EVENT){
        ++p_tabla_estado_actual;
    } 

    (p_tabla_estado_actual -> p_rut_accion) (); // Ejecuta Rutina de accion correspondiente
    p_tabla_estado_actual = (p_tabla_estado_actual -> proximo_estado); // Encuentro próximo estado
    return (p_tabla_estado_actual);    
}

/*******************************************************************************
 ******************************************************************************/
