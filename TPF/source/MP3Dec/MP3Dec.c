/***************************************************************************//**
  @file		MP3Dec.c
  @brief	+Descripcion del archivo+
  @author	Grupo 5
  @date		21 dic. 2022
 ******************************************************************************/

/*******************************************************************************
 * INCLUDE HEADER FILES
 ******************************************************************************/

#include "Mp3Dec.h"
#include "helix/pub/mp3dec.h"
//#include "FileSystem/ff15/source/ff.h"

#include <stddef.h>
#include <stdio.h>

/*******************************************************************************
 * CONSTANT AND MACRO DEFINITIONS USING #DEFINE
 ******************************************************************************/

#define BUFF_SIZE	512U	//2304U		// TODO: Ver mejor tamano de buffer (2304 bytes?)

/*******************************************************************************
 * ENUMERATIONS AND STRUCTURES AND TYPEDEFS
 ******************************************************************************/


/*******************************************************************************
 * VARIABLES WITH GLOBAL SCOPE
 ******************************************************************************/

// +ej: unsigned int anio_actual;+


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


static HMP3Decoder mp3Dec;


/*******************************************************************************
 *******************************************************************************
                        GLOBAL FUNCTION DEFINITIONS
 *******************************************************************************
 ******************************************************************************/

bool MP3DecInit() {

	mp3Dec = MP3InitDecoder();

	if (mp3Dec == NULL) return true;

	return false;

}


void MP3PlaySong(FIL* mp3File) {


    if (mp3File) {

    	BYTE buff[BUFF_SIZE];
    	UINT br;

    	if (f_read(mp3File, buff, BUFF_SIZE, &br) == FR_OK) {
    		int offset = MP3FindSyncWord(buff, br);
    		if (offset >= 0) {
				printf("Sync Word FOUND: %u\n", offset);
				MP3FrameInfo mp3Info;
				int err = MP3GetNextFrameInfo(mp3Dec, &mp3Info, buff+offset);
    			if (!err) {
    				printf("MP3 Frame OK. SR: %d\n", mp3Info.samprate);

//    		    	BYTE outBuff[];		// TODO: Ver tamaño buffer
//    				MP3Decode(mp3Dec, buff+offset, BUFF_SIZE-offset, outBuff, BUFF_SIZE);

    			}
    			else {
    				printf("MP3 Frame FAILED: %d\n", err);
    			}
    		}
    		else {
				printf("Sync Word NOT FOUND\n");
    		}
    	}
    	else {
        	printf("File read ERROR\n");
    	}
    }
    else {
    	printf("File pointer ERROR\n");
    }

}


/*******************************************************************************
 *******************************************************************************
                        LOCAL FUNCTION DEFINITIONS
 *******************************************************************************
 ******************************************************************************/



 
