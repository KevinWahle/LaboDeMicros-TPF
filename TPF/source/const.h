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


//DUDA: Esto va en el modulo de Schembe
#define VOLMAX		100
#define VOLMIN		0
#define VOLVALUES	10
#define VOLSTEP		((VOLMAX-VOLMIN)/VOLVALUES)
#define VOL_INIT	((VOLMAX+VOLMIN)/2)

enum EQ_OPTIONS{
	JAZZ,
	ROCK,
	CLASSIC,
	HOUSE,
};



//DUDA: Para archivo de serman
enum MUSIC_STATES{
	PLAY,
	PAUSE,
	STOP,
}

/*******************************************************************************
 * ENUMERATIONS AND STRUCTURES AND TYPEDEFS
 ******************************************************************************/

// Estructura utilizada para cada una de las opciones clickeables 
// en los diferentes menúes.
typedef struct {
	char* option;
	int ID;
} MENU_ITEM;


//DUDA: De aca para abajo se va para un .h de SERMAN
typedef SONG_FILE uint8_t*;

typedef struct {
	char* name;
	char* author;
	int duration;
	// referencia
} SONG_INFO_T;

// Elemento del navegador del filesystem.
// El primer elemento de cada carpeta debería mandarte a un nodo que te permita volver si lo clickeas

// REVISAR: Cómo definir punteros a estructuras 
typedef struct {
	FS_ELEMENT_T* up_element;
	FS_ELEMENT_T* down_element;
	FS_ELEMENT_T* content;
	uint8_t song; // indica si es una cancion o una carpeta. 1 si true
	uint8_t back; // indica si es una opción para volver para atras. 1 si true
} FS_ELEMENT_T;


/*******************************************************************************
 ******************************************************************************/

#endif // CONST_H
