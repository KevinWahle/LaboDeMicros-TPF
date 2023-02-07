/*******************************************************************************
  @file     keypad.c
  @brief    Keypad functions
  @author   Grupo 5
  @date		13 sep. 2022
 ******************************************************************************/


/*******************************************************************************
 * INCLUDE HEADER FILES
 ******************************************************************************/
#include "../MCAL/gpio.h"
#include "../const.h"
#include "keypad.h"
#include "timer/timer.h"

/*******************************************************************************
 * CONSTANT AND MACRO DEFINITIONS USING #DEFINE
 ******************************************************************************/

#define BTN_SLEEP_PIN     	PORTNUM2PIN(PD,1)
#define BTN_VOLDOWN_PIN   	PORTNUM2PIN(PD,3)
#define BTN_VOLUP_PIN     	PORTNUM2PIN(PD,2)
#define BTN_PAUSE_PIN    	PORTNUM2PIN(PD,0)  // boton de pausa/reanudacion

#define LONG_PRESS_TIME		500U	//ms
#define BURST_TIME			100U	//ms

#define BTN_ACTIVE		LOW

/*******************************************************************************
 * ROM CONST VARIABLES WITH FILE LEVEL SCOPE
 ******************************************************************************/
static void sleep_cb();
static void volup_cb();
static void voldown_cb();
static void pause_cb();

static void volUp_Timercb();
static void volDown_Timercb();
/*******************************************************************************
 * STATIC VARIABLES AND CONST VARIABLES WITH FILE LEVEL SCOPE
 ******************************************************************************/
static fun_ptr myevent;

static tim_id_t timerVolUp, timerVolDown;	// Timer para long press y burst

/*******************************************************************************
 *******************************************************************************
                        GLOBAL FUNCTION DEFINITIONS
 *******************************************************************************
 ******************************************************************************/
void keypadInit(fun_ptr event){

    myevent=event;
    // Seteamos I/O de los botones
    gpioMode(BTN_SLEEP_PIN, INPUT_PULLUP);
    gpioMode(BTN_VOLUP_PIN, INPUT_PULLUP);
    gpioMode(BTN_VOLDOWN_PIN, INPUT_PULLUP);
    gpioMode(BTN_PAUSE_PIN, INPUT_PULLUP);

    gpioSetFilter(BTN_SLEEP_PIN, 0U);
    gpioSetFilter(BTN_VOLUP_PIN, 0U);
    gpioSetFilter(BTN_VOLDOWN_PIN, 0U);
    gpioSetFilter(BTN_PAUSE_PIN, 0x1FU);	// Seteo el maximo tiempo (31ms). Es el mismo para todos

    // Activamos IRQ
    gpioIRQ(BTN_SLEEP_PIN, GPIO_IRQ_MODE_FALLING_EDGE, sleep_cb);	// Set btn falling edge interruption
    gpioIRQ(BTN_VOLUP_PIN, GPIO_IRQ_MODE_FALLING_EDGE, volup_cb);	// Set btn falling edge interruption
    gpioIRQ(BTN_VOLDOWN_PIN, GPIO_IRQ_MODE_FALLING_EDGE, voldown_cb);	// Set btn falling edge interruption
    gpioIRQ(BTN_PAUSE_PIN, GPIO_IRQ_MODE_FALLING_EDGE, pause_cb);	// Set btn falling edge interruption

    timerVolUp = timerGetId();
    timerVolDown = timerGetId();
    
}

static void sleep_cb(){
  myevent(BTN_SLEEP);
}

static void volup_cb(){
  myevent(VOL_UP);
  timerStart(timerVolUp, TIMER_MS2TICKS(LONG_PRESS_TIME), TIM_MODE_PERIODIC, volUp_Timercb);
}

static void voldown_cb(){
  myevent(VOL_DOWN);
  timerStart(timerVolDown, TIMER_MS2TICKS(LONG_PRESS_TIME), TIM_MODE_PERIODIC, volDown_Timercb);
}

static void pause_cb(){
  myevent(BTN_PAUSE);
}


// Si detecta long press, hace rafaga de eventos mientras siga presionado
static void volUp_Timercb(){
  if (gpioRead(BTN_VOLUP_PIN) == BTN_ACTIVE) {
	  myevent(VOL_UP);
	  timerStart(timerVolUp, TIMER_MS2TICKS(BURST_TIME), TIM_MODE_PERIODIC, volUp_Timercb);
  }
  else {
	  timerStop(timerVolUp);
  }
}

// Si detecta long press, hace rafaga de eventos mientras siga presionado
static void volDown_Timercb(){
  if (gpioRead(BTN_VOLDOWN_PIN) == BTN_ACTIVE) {
	  myevent(VOL_DOWN);
	  timerStart(timerVolDown, TIMER_MS2TICKS(BURST_TIME), TIM_MODE_PERIODIC, volDown_Timercb);
  }
  else {
	  timerStop(timerVolDown);
  }
}
