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
#include "deepSleep/DeepSleep.h"

// DEBUG
#include "MCAL/gpio.h"
#define TESTPIN		PORTNUM2PIN(PB, 3)
#define TESTPIN2	PORTNUM2PIN(PB, 2)
/*******************************************************************************
 *******************************************************************************
                        	GLOBAL VARIABLES
 *******************************************************************************
 ******************************************************************************/

uint8_t sleep_reset;
static STATE* p_tabla_estado_actual;

tim_id_t timerSplash;

/*******************************************************************************
 *******************************************************************************
                        GLOBAL FUNCTION DEFINITIONS
 *******************************************************************************
 ******************************************************************************/

/* Función que se llama 1 vez, al comienzo del programa */
void App_Init (void)
{
	timerInit();

    timerSplash = timerGetId();

	// DEBUG
	gpioMode(TESTPIN, OUTPUT);
	gpioWrite(TESTPIN, LOW);

	gpioMode(TESTPIN2, OUTPUT);
	gpioWrite(TESTPIN2, LOW);
 
}

/* Función que se llama constantemente en un ciclo infinito */
void App_Run (void)
{
    LLS_config(); //Sleep mode
    initDisplay();
	encoderInit(encoderCallback);
    keypadInit(add_event);
    MP3DecInit();
	initMatrix();
	DMA_initPingPong_Dac();
	initEqualizer();
	setUpFilter(0, 0);
	setUpFilter(0, 1);
	setUpFilter(0, 2);
	setUpFilter(0, 3);
	init_queue();

    //Splash Screen + Matrix
	p_tabla_estado_actual = splash_state;
    displayLine(0, "Reproductor MP3");
    displayLine(1, "    Grupo 5    ");
    uint8_t splashColumns[8]={7,4,5,2,6,8,3,1};
    setColumnsMatrix(splashColumns);
    timerStart(timerSplash, TIMER_MS2TICKS(3000), TIM_MODE_SINGLESHOT, addTimeout);

    sleep_reset=false;

    while (!sleep_reset) {
        event_t evento = get_next_event();  // Tomo un nuevo evento de la cola de eventos.
        if (evento == NULL_EVENT) {
        	gpioWrite(TESTPIN, LOW);
        }
        else{
        	gpioWrite(TESTPIN, HIGH);
        }
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
