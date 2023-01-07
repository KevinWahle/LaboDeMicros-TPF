/***************************************************************************//**
  @file     App.c
  @brief    Application functions
  @author   Grupo5
 ******************************************************************************/

/*******************************************************************************
 * INCLUDE HEADER FILES
 ******************************************************************************/

#include "..\MCAL\board.h"
#include "..\MCAL\gpio.h"
#include "keypad.h"
#include "..\event_queue\event_queue.h"
#include "const.h"
/*******************************************************************************
 * CONSTANT AND MACRO DEFINITIONS USING #DEFINE
 ******************************************************************************/
#define TRUE 1
#define FALSE 0
#define TIME 200000UL
/*******************************************************************************
 * FUNCTION PROTOTYPES FOR PRIVATE FUNCTIONS WITH FILE LEVEL SCOPE
 ******************************************************************************/
void toggle_led();

/*******************************************************************************
 *******************************************************************************
                        GLOBAL FUNCTION DEFINITIONS
 *******************************************************************************
 ******************************************************************************/

/* Función que se llama 1 vez, al comienzo del programa */
void App_Init (void)
{
    keypadInit(add_event);
    
    gpioMode(PIN_LED_RED, OUTPUT);
    gpioMode(PIN_LED_GREEN, OUTPUT);
    gpioMode(PIN_LED_BLUE, OUTPUT);
    gpioWrite(PIN_LED_GREEN,!LED_ACTIVE);
    gpioWrite(PIN_LED_RED,!LED_ACTIVE);
    gpioWrite(PIN_LED_BLUE,!LED_ACTIVE);
}


event_t event;

/* Función que se llama constantemente en un ciclo infinito */
void App_Run (void)
{
    if(!is_queue_empty()){
        event= get_next_event();
        
        switch (event){
        case BTN_PAUSE:                 
            gpioToggle(PIN_LED_RED);    //PC8
            break;

        case VOL_UP:
            gpioToggle(PIN_LED_BLUE);     //PB19 
            break;

        case VOL_DOWN:
            gpioToggle(PIN_LED_GREEN);  //PC1
            break;

        case BTN_SLEEP:                 
            gpioToggle(PIN_LED_RED);    //PB18
            break; 

        default:
            break;
        }
    }
	
}
