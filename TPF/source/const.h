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


/*******************************************************************************
 * ENUMERATIONS AND STRUCTURES AND TYPEDEFS
 ******************************************************************************/
enum EVENTS {
	//Eventos de Software
	READ_SD,
	EQ_SELECTION,
	ADJUST_BRIGHT,
	SONG_SELECTION,
	SONG_SELECTED,

	// Para retornar de Vol Adj
	MAIN_MENU_EV,
	MAIN_SCREEN_EV,

	// Miscelaneos
	TIMEOUT,
	BACK,	// Para volver atras una vez checkeado cierto dato
	WAIT,
	RESET,
	ERROR_EV,

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

enum EQ_OPTIONS{
	BACK_EQU,
	BAND200,
	BAND500,
	BAND1K,
	BAND_OTHERS,
	RESET_EQU,
	EQ_OPTIONS,
};

enum ERRORS{
	SD_ERROR,
	SONG_ERROR,
	FRAME_ERROR,
};

typedef enum {
	PLAY,
	PAUSE,
	STOP,
} MUSIC_STATES;

typedef struct {
	char* option;
	int ID;
} MENU_ITEM;


//DUDA: De aca para abajo se va para un .h de SERMAN
 typedef uint8_t* SONG_FILE;

typedef struct {
	char* name;
	char* author;
	int duration;
	// referencia
} SONG_INFO_T;

/*******************************************************************************
 ******************************************************************************/

#endif // CONST_H
