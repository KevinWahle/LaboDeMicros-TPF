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

#define INBUFF_SIZE		MAINBUF_SIZE	// TODO: Ver mejor tamano de buffer
#define OUTBUFF_SIZE	2304U			// TODO: Ver mejor tamano de buffer (2304 bytes?)

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


void MP3PlaySong(char* filePath) {

	FIL mp3File;

	if (f_open(&mp3File, filePath, FA_READ) == FR_OK) {
		printf("File open OK\n");

		MP3FrameInfo mp3Info;
    	BYTE readBuff[INBUFF_SIZE];
		short int outBuff[OUTBUFF_SIZE];		// TODO: Ver tamaño buffer (outputsamps)
    	UINT br;
    	int err, offset, bLeft;
    	uint8_t* pDecBuff;

    	bool needSync = true;	// Find sync of first frame

    	do {
    		//TODO: Alinear bien los frames
			if (f_read(&mp3File, readBuff, INBUFF_SIZE, &br) == FR_OK) {	// Save file content in readBuff
				printf("File read OK\n");
				bLeft = br;
				pDecBuff = readBuff;

				if (needSync) {
					needSync = false;
					offset = MP3FindSyncWord(pDecBuff, bLeft);
					if (offset < 0) {
						printf("Sync Word NOT FOUND\n");
						continue;	// Skip block
					}
					printf("Sync Word FOUND: %u\n", offset);
//					err = MP3GetNextFrameInfo(mp3Dec, &mp3Info, pDecBuff);	// TODO: Ver si poner solo aca
					bLeft -= offset;
					pDecBuff += offset;	// pDecBuff: Start of MP3 data frame
				}
				while (bLeft) {
					err = MP3GetNextFrameInfo(mp3Dec, &mp3Info, pDecBuff);	// TODO: Es necesario siempre??
						if (!err) {
	//						printf("MP3 Frame OK.\n"
	//								"\tbitrate: %d\n"
	//								"\tnChans: %d\n"
	//								"\tsamprate: %d\n"
	//								"\tbitsPerSample: %d\n"
	//								"\toutputSamps: %d\n"
	//								"\tlayer: %d\n"
	//								"\tversion: %d\n",
	//								mp3Info.bitrate, mp3Info.nChans, mp3Info.samprate, mp3Info.bitsPerSample, mp3Info.outputSamps, mp3Info.layer, mp3Info.version
	//								);
//							bLeft -= offset;
							int checkpoint = bLeft;		//TODO: Borrar, solo debug. O no?
							err = MP3Decode(mp3Dec, &pDecBuff, &bLeft, outBuff, 0U);
							if (!err) {
		//						MP3GetLastFrameInfo(mp3Dec, &mp3Info);
		//						printf("MP3 Last Frame Info:\n"
		//								"\tbitrate: %d\n"
		//								"\tnChans: %d\n"
		//								"\tsamprate: %d\n"
		//								"\tbitsPerSample: %d\n"
		//								"\toutputSamps: %d\n"
		//								"\tlayer: %d\n"
		//								"\tversion: %d\n",
		//								mp3Info.bitrate, mp3Info.nChans, mp3Info.samprate, mp3Info.bitsPerSample, mp3Info.outputSamps, mp3Info.layer, mp3Info.version
		//								);
								//TODO: Ver que hacer con la data
								printf("MP3Decode OK. Bytes read: %d\tBytes left: %d\n", checkpoint-bLeft, bLeft);
							}
							else {		// Error decoding
								printf("MP3Decode ERROR: %d\n", err);

//								switch (err) {
//
//								}

								// TODO: Diferenciar errores
								if (!f_eof(&mp3File)) {
									f_lseek(&mp3File, f_tell(&mp3File) - checkpoint);
								}

								break;
							}

						}
						else {		// Error getting frame info
							printf("MP3 Frame FAILED: %d\n", err);
							// TODO: En este caso se rompe todo. Solucionar
//							needSync = true;
//							continue;
						}
				}
			}
			else {		// Error reading file
				printf("File read ERROR\n");
//				break;
			}
    	} while (!f_eof(&mp3File));		// Read until EOF
		f_close(&mp3File);
    }
    else {
    	printf("File open ERROR\n");
    }

}


/*******************************************************************************
 *******************************************************************************
                        LOCAL FUNCTION DEFINITIONS
 *******************************************************************************
 ******************************************************************************/



 
