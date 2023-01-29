/***************************************************************************//**
  @file		MP3DecTest.c
  @brief	Funciones de prueba para MP3
  @author	Grupo 5
  @date		22 ene. 2023
 ******************************************************************************/

/*******************************************************************************
 * INCLUDE HEADER FILES
 ******************************************************************************/

#include "Mp3Dec.h"
#include "FileSystem/ff15/source/ff.h"
#include <stdio.h>

/*******************************************************************************
 * CONSTANT AND MACRO DEFINITIONS USING #DEFINE
 ******************************************************************************/



/*******************************************************************************
 * ENUMERATIONS AND STRUCTURES AND TYPEDEFS
 ******************************************************************************/


/*******************************************************************************
 * VARIABLES WITH GLOBAL SCOPE
 ******************************************************************************/

FATFS FatFs;   /* Work area (filesystem object) for logical drive */

/*******************************************************************************
 * FUNCTION PROTOTYPES FOR PRIVATE FUNCTIONS WITH FILE LEVEL SCOPE
 ******************************************************************************/

// +ej: static void falta_envido (int);+


/*******************************************************************************
 * ROM CONST VARIABLES WITH FILE LEVEL SCOPE
 ******************************************************************************/

// +ej: static const int temperaturas_medias[4] = {23, 26, 24, 29};+


/*******************************************************************************
 * STATIC VARIABLES AND CONST VARIABLES WITH FILE LEVEL SCOPE
 ******************************************************************************/

static void testPlayMP3();

/*******************************************************************************
 *******************************************************************************
                        GLOBAL FUNCTION DEFINITIONS
 *******************************************************************************
 ******************************************************************************/

void App_Init() {

	/* Give a work area to the default drive */
    if (f_mount(&FatFs, "1:/", 0) == FR_OK) {
    	printf("FileSystem mount OK\n");
    }


}

void App_Run() {

	if (!MP3DecInit()) {
    	printf("MP3Dec Init OK\n");
		printf("\nTemazo:\n");
		MP3SelectSong("1:/temazo.mp3");
		testPlayMP3();
    	printf("\nDrum:\n");
		MP3SelectSong("1:/drum.mp3");
		testPlayMP3();
		printf("\nQuack:\n");
		MP3SelectSong("1:/quack.mp3");
		testPlayMP3();
		printf("\nSound:\n");
		MP3SelectSong("1:/sound.mp3");
		testPlayMP3();
	}
	else {
		printf("Error al inicializar MP3Dec\n");
	}


	while(1);
}


/*******************************************************************************
 *******************************************************************************
                        LOCAL FUNCTION DEFINITIONS
 *******************************************************************************
 ******************************************************************************/


static void testPlayMP3() {
	int16_t buff[OUTBUFF_SIZE];
	uint16_t br;
	do {
		br = MP3DecNextFrame(buff);
		printf("One frame read. Result: %u\n", br);
	} while(br);
}

