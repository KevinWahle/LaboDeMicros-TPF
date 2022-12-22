/***************************************************************************//**
  @file     +const.h+
  @brief    +Constantes en comun para la aplicacion+
  @author   +Grupo 5+
 ******************************************************************************/

#ifndef CONST_H
#define CONST_H

/*******************************************************************************
 * INCLUDE HEADER FILES
 ******************************************************************************/

#include <stdint.h>
#include <stdbool.h>

/*******************************************************************************
 * CONSTANT AND MACRO DEFINITIONS USING #DEFINE
 ******************************************************************************/
//#define DEBUG 1

enum EVENTS {
	//Eventos de Software
	READ_SD,
	EQ_SELECTION,
	AJ_BRILLO,
	SONG_SEL,
	SONG_SELECTED,

	// Para retornar de Vol Adj
	SONG_INFO_EV,
	SONG_SEL_EV,
	MAIN_MENU_EV,
	MAIN_SCREEN_EV,

	// Miscelaneos
	TIMEOUT,
	BACK,	// Para volver atras una vez checkeado cierto dato
	WAIT,
	RESET,

	// Evento de Hardware
	// Botones
	BTN_PRESS,
	BTN_RELEASE,
	BTN_LONG,
	BTN_PAUSE,
	BTN_SLEEP,
	VOL_UP,
	VOL_DOWN,


	// Encoder
    ENCODER_LEFT,
    ENCODER_RIGHT,
	ENCODER_PRESS,
	ENCODER_RELEASE,
	ENCODER_DOUBLE,
	ENCODER_LONG,

};

enum MAIN_MENU_OPT{
	LOAD_SD,
	EQUILIZATION,
	ADJUST_BRIGHT,
	SONG_SELECTION,
	MAIN_SCREEN,
};


/*******************************************************************************
 * ENUMERATIONS AND STRUCTURES AND TYPEDEFS
 ******************************************************************************/

// Estructura utilizada para cada una de las opciones clickeables 
// en los diferentes menúes.
typedef struct {
	char* option;
	int ID;
} MENU_ITEM;


/*******************************************************************************
 ******************************************************************************/

#endif // CONST_H
