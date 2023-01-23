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

// +ej: static int temperaturas_actuales[4];+


/*******************************************************************************
 *******************************************************************************
                        GLOBAL FUNCTION DEFINITIONS
 *******************************************************************************
 ******************************************************************************/

void App_Init() {

    FIL fil;        /* File object */
//    FRESULT fr;     /* FatFs return code */


    /* Give a work area to the default drive */
    if (f_mount(&FatFs, "1:/", 0) == FR_OK) {
    	printf("FileSystem mount OK\n");
    }

	if (!MP3DecInit()) {
    	printf("MP3Dec Init OK\n");
    	printf("\nTemazo:\n");
    	if (f_open(&fil, "1:/temazo.mp3", FA_READ) == FR_OK) {
    		printf("File open OK");
    		MP3PlaySong(&fil);
    		f_close(&fil);
    	}
    	printf("\nDrum:\n");
    	if (f_open(&fil, "1:/drum.mp3", FA_READ) == FR_OK) {
    		printf("File open OK");
    		MP3PlaySong(&fil);
    		f_close(&fil);
    	}
		printf("\nQuack:\n");
    	if (f_open(&fil, "1:/quack.mp3", FA_READ) == FR_OK) {
    		printf("File open OK");
    		MP3PlaySong(&fil);
    		f_close(&fil);
    	}
	}
	else {
		printf("Error al inicializar MP3Dec\n");
	}


//
//
//    /* Close the file */
//    f_close(&fil);
}

void App_Run() {

}


/*******************************************************************************
 *******************************************************************************
                        LOCAL FUNCTION DEFINITIONS
 *******************************************************************************
 ******************************************************************************/




