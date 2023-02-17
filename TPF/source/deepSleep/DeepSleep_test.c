/***************************************************************************//**
  @file     App.c
  @brief    Application functions
  @author   KevinWahle
 ******************************************************************************/

/*******************************************************************************
 * INCLUDE HEADER FILES
 ******************************************************************************/

#include "..\MCAL\board.h"
#include "..\MCAL\gpio.h"
#include "DeepSleep.h"


/*******************************************************************************
 * CONSTANT AND MACRO DEFINITIONS USING #DEFINE
 ******************************************************************************/
//#define EXTERN_LED	PIN_LED_RED
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
    gpioMode(PIN_LED_RED, OUTPUT);
    gpioMode(PIN_LED_GREEN, OUTPUT);
    gpioMode(PIN_LED_BLUE, OUTPUT);

    gpioMode(PIN_SW3, INPUT);			//Ya es pulldown electricamente
    gpioMode(PIN_SW2, INPUT_PULLUP);	//Esta como do not place (no tiene resistencia)
    //gpioMode(EXTERN_LED, OUTPUT);

    LLS_config();		//LLWU

    gpioWrite(PIN_LED_GREEN,!LED_ACTIVE);
    gpioWrite(PIN_LED_RED,!LED_ACTIVE);
    gpioWrite(PIN_LED_BLUE,!LED_ACTIVE);
}




/* Función que se llama constantemente en un ciclo infinito */
void App_Run (void)
{

	toggle_led();

	//LLS
	if (is_onoff_btn()){
		LLS_start();
	}

}


/*******************************************************************************
 *******************************************************************************
                        LOCAL FUNCTION DEFINITIONS
 *******************************************************************************
 ******************************************************************************/
void toggle_led(){
	static bool state=FALSE;
	static uint32_t veces;

	//Baliza
	switch (state) {
		case TRUE:
			veces--;
			if(!veces){
				gpioToggle(PIN_LED_BLUE);
				veces=TIME;
			}
			if (!gpioRead(PIN_SW3)){
				while (!gpioRead(PIN_SW3));
				state=FALSE;
				gpioWrite(PIN_LED_BLUE,LOW);
				gpioWrite(PIN_LED_RED,!LED_ACTIVE);
			}
			break;
		case FALSE:
			if (!gpioRead(PIN_SW3)){
				while (!gpioRead(PIN_SW3));
				state=TRUE;
				gpioWrite(PIN_LED_RED,LED_ACTIVE);
				veces = TIME;
			}
		default:
			break;
	}
}
/*******************************************************************************
 ******************************************************************************/
