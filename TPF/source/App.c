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

	encoderInit(encoderCallback);

    //Splash Screen
	p_tabla_estado_actual = splash_state;
    displayLine(0, "Reproductor MP3");
    displayLine(1, "    Grupo 5    ");
    
    timerStart(timerGetId(), TIMER_MS2TICKS(3000), TIM_MODE_SINGLESHOT, addTimeout);
    
}

/* Función que se llama constantemente en un ciclo infinito */
void App_Run (void)
{
	event_t evento = get_next_event();  // Tomo un nuevo evento de la cola de eventos.
	p_tabla_estado_actual = fsm_interprete(p_tabla_estado_actual, evento);  // Actualizo el estado
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