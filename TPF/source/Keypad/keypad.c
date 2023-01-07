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

#define BTN_SLEEP     PORTNUM2PIN(PB,18)
#define BTN_VOLUP     PORTNUM2PIN(PB,19)
#define BTN_VOLDOWN   PORTNUM2PIN(PC,1)
#define BTN_PAUSE     PORTNUM2PIN(PC,8)  // boton de pausa/reanudacion

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
    gpioMode(BTN_SLEEP, INPUT_PULLUP);
    gpioMode(BTN_VOLUP, INPUT_PULLUP);
    gpioMode(BTN_VOLDOWN, INPUT_PULLUP);
    gpioMode(BTN_PAUSE, INPUT_PULLUP);

    // Activamos IRQ
    gpioIRQ(BTN_SLEEP, GPIO_IRQ_MODE_FALLING_EDGE, sleep_cb);	// Set btn falling edge interruption
    gpioIRQ(BTN_VOLUP, GPIO_IRQ_MODE_FALLING_EDGE, volup_cb);	// Set btn falling edge interruption
    gpioIRQ(BTN_VOLDOWN, GPIO_IRQ_MODE_FALLING_EDGE, voldown_cb);	// Set btn falling edge interruption
    gpioIRQ(BTN_PAUSE, GPIO_IRQ_MODE_FALLING_EDGE, pause_cb);	// Set btn falling edge interruption

    
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


 
