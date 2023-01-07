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

/*******************************************************************************
 * CONSTANT AND MACRO DEFINITIONS USING #DEFINE
 ******************************************************************************/

#define BTN_SLEEP_PIN     	PORTNUM2PIN(PB,18)
#define BTN_VOLUP_PIN     	PORTNUM2PIN(PB,19)
#define BTN_VOLDOWN_PIN   	PORTNUM2PIN(PC,1)
#define BTN_PAUSE_PIN    	PORTNUM2PIN(PC,8)  // boton de pausa/reanudacion

/*******************************************************************************
 * ROM CONST VARIABLES WITH FILE LEVEL SCOPE
 ******************************************************************************/
void sleep_cb();
void volup_cb();
void voldown_cb();
void pause_cb();

/*******************************************************************************
 * STATIC VARIABLES AND CONST VARIABLES WITH FILE LEVEL SCOPE
 ******************************************************************************/
static fun_ptr myevent;
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

    // Activamos IRQ
    gpioIRQ(BTN_SLEEP_PIN, GPIO_IRQ_MODE_FALLING_EDGE, sleep_cb);	// Set btn falling edge interruption
    gpioIRQ(BTN_VOLUP_PIN, GPIO_IRQ_MODE_FALLING_EDGE, volup_cb);	// Set btn falling edge interruption
    gpioIRQ(BTN_VOLDOWN_PIN, GPIO_IRQ_MODE_FALLING_EDGE, voldown_cb);	// Set btn falling edge interruption
    gpioIRQ(BTN_PAUSE_PIN, GPIO_IRQ_MODE_FALLING_EDGE, pause_cb);	// Set btn falling edge interruption

    
}

void sleep_cb(){
  myevent(BTN_SLEEP);
}

void volup_cb(){
  myevent(VOL_UP);
}

void voldown_cb(){
  myevent(VOL_DOWN);
}

void pause_cb(){
  myevent(BTN_PAUSE);
}


 
