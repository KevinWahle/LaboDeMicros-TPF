/***************************************************************************//**
  @file     encoder_hal.c
  @brief    Funciones para manejo del encoder
  @author   Grupo 5
 ******************************************************************************/

#include "encoder_hal.h"
#include <stdbool.h>
#include <stdint.h>
#include "MCAL/gpio.h"
#include "timer/timer.h"

#define LONG_PRESS_TIME 500
#define DOUBLE_CLICK_TIME 400

typedef enum {IDLE_ENCODER, LEFT_TRANS, RIGHT_TRANS} SM_ENCODER;

typedef enum {IDLE_BUTTON ,C1, C2, C3} SM_BUTTON; //States of the machine

static pin_t rcha = PORTNUM2PIN(PC, 16);	// PORTNUM2PIN(PD, 0);
static pin_t rchb = PORTNUM2PIN(PC, 17);	//PORTNUM2PIN(PD, 2);
static pin_t button = PORTNUM2PIN(PB, 9);	//PORTNUM2PIN(PD, 3);


// Timers
static tim_id_t longPressedTimerID;
static tim_id_t doublePressedTimerID;


/**
 * @brief FSM implementation
 */
static void smEnconder(void);

static encoderCb funCb;


bool encoderInit( encoderCb funCbParam ){
	timerInit();
	longPressedTimerID = timerGetId();
	doublePressedTimerID = timerGetId();
	gpioMode(rcha, INPUT);
	gpioMode(rchb, INPUT);
	gpioMode(button, INPUT);

	funCb = funCbParam;

	tim_id_t periodicTimerID = timerGetId();
	timerStart(periodicTimerID, TIMER_MS2TICKS(0.5), TIM_MODE_PERIODIC, smEnconder);  // Periodic ISR 500us

	return true;
}

static void smEnconder(void){

	static SM_ENCODER state_rotate = IDLE_ENCODER;
	static SM_BUTTON state_button = IDLE_BUTTON;

	switch(state_rotate){
	case IDLE_ENCODER:
		if( ( !gpioRead(rcha) && gpioRead(rchb) ) || ( gpioRead(rcha) && !gpioRead(rchb) )){
			if(!gpioRead(rcha))
				state_rotate = LEFT_TRANS;
			else
				state_rotate = RIGHT_TRANS;
		}
		break;
	case LEFT_TRANS:
		if(gpioRead(rcha) && gpioRead(rchb)){
			state_rotate = IDLE_ENCODER;
			funCb(ENC_LEFT);
		}
		break;
	case RIGHT_TRANS:
		if(gpioRead(rcha) && gpioRead(rchb)){
			state_rotate = IDLE_ENCODER;
			funCb(ENC_RIGHT);
		}
		break;
	}

	switch(state_button){
	case IDLE_BUTTON:
		if(!gpioRead(button)){
			state_button = C1;
			timerStart(longPressedTimerID, TIMER_MS2TICKS(LONG_PRESS_TIME), TIM_MODE_SINGLESHOT, NULL);
			timerStart(doublePressedTimerID, TIMER_MS2TICKS(DOUBLE_CLICK_TIME), TIM_MODE_SINGLESHOT, NULL);
		}
		break;
	case C1:
		if(gpioRead(button)){
			if(timerExpired(longPressedTimerID)){
				state_button = IDLE_BUTTON;
				funCb(ENC_LONG);
			}
			else if(timerExpired(doublePressedTimerID)){
				state_button = IDLE_BUTTON;
				funCb(ENC_CLICK);
			}
			else{
				state_button = C2;
				funCb(ENC_CLICK);
			}
		}
		break;
	case C2:
		if(timerExpired(doublePressedTimerID))
			state_button = IDLE_BUTTON;
		if(!gpioRead(button)){
			state_button = C3;
			timerStart(longPressedTimerID, TIMER_MS2TICKS(LONG_PRESS_TIME), TIM_MODE_SINGLESHOT, NULL);
		}
		break;
	case C3:
		if(gpioRead(button)){
			if(timerExpired(longPressedTimerID)){
				state_button = IDLE_BUTTON;
				funCb(ENC_LONG);
			}
			else if(!timerExpired(doublePressedTimerID)){
				state_button = IDLE_BUTTON;
				funCb(ENC_DOUBLE);
			}
			else{
				state_button = IDLE_BUTTON;
				funCb(ENC_CLICK);
			}
		}
		break;
	}

}

