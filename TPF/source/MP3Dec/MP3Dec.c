/***************************************************************************//**
  @file		MP3Dec.c
  @brief	+Descripcion del archivo+
  @author	Grupo 5
  @date		21 dic. 2022
 ******************************************************************************/


#define MP3_DEBUG

/*******************************************************************************
 * INCLUDE HEADER FILES
 ******************************************************************************/

#include "Mp3Dec.h"
#include "helix/pub/mp3dec.h"
#include "FileSystem/ff15/source/ff.h"

#include <stddef.h>

#ifdef MP3_DEBUG
#include <stdio.h>
#endif

/*******************************************************************************
 * CONSTANT AND MACRO DEFINITIONS USING #DEFINE
 ******************************************************************************/

#define INBUFF_SIZE		MAINBUF_SIZE	// TODO: Ver mejor tamano de buffer

// Accepted sample rate
#define MP3_SAMPRATE	44100

// Accepted number of channels
#define MP3_NCHANS		1
// TODO: Ver tema stereo

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

static FIL mp3File;

static MP3FrameInfo mp3Info;

static bool fileOpen = false;

/*******************************************************************************
 *******************************************************************************
                        GLOBAL FUNCTION DEFINITIONS
 *******************************************************************************
 ******************************************************************************/

bool MP3DecInit() {

	mp3Dec = MP3InitDecoder();

	return mp3Dec == NULL;

}

bool MP3SelectSong(char* filePath) {

	if (fileOpen) {
		f_close(&mp3File);
		fileOpen = false;
	}

	if (f_open(&mp3File, filePath, FA_READ) == FR_OK) {
		fileOpen = true;
#ifdef MP3_DEBUG
		printf("File open OK\n");
#endif
		UINT br;
		BYTE readBuff[INBUFF_SIZE];	// Buffer to store file content
		int err = 1, offset;

		while (!f_eof(&mp3File) && err) {		// Read until Frame valid or EOF
			if (f_read(&mp3File, readBuff, INBUFF_SIZE, &br) == FR_OK) {	// Save file content in readBuff

#ifdef MP3_DEBUG
				printf("File read OK\n");
#endif
				offset = MP3FindSyncWord(readBuff, br);
				if (offset < 0) {
#ifdef MP3_DEBUG
					printf("Sync Word NOT FOUND\n");
#endif
					continue;	// Skip block
				}
#ifdef MP3_DEBUG
				printf("Sync Word FOUND: %u\n", offset);
#endif

				err = MP3GetNextFrameInfo(mp3Dec, &mp3Info, readBuff+offset);
				if (err) {
#ifdef MP3_DEBUG
					printf("Error getting Frame Info\n");
#endif
					f_lseek(&mp3File, f_tell(&mp3File) - br + offset + 1);	// Move file pointer next to last frame sync found
				}
			}
			else {		// Error reading file
#ifdef MP3_DEBUG
				printf("File read ERROR\n");
#endif
				break;
			}
    	}

		if (!err) {
#ifdef MP3_DEBUG
			printf("MP3 Frame OK.\n"
					"\tbitrate: %d\n"
					"\tnChans: %d\n"
					"\tsamprate: %d\n"
					"\tbitsPerSample: %d\n"
					"\toutputSamps: %d\n"
					"\tlayer: %d\n"
					"\tversion: %d\n",
					mp3Info.bitrate, mp3Info.nChans, mp3Info.samprate, mp3Info.bitsPerSample, mp3Info.outputSamps, mp3Info.layer, mp3Info.version
					);
#endif
			// Check if audio is accepted
			if (mp3Info.samprate == MP3_SAMPRATE && mp3Info.nChans == MP3_NCHANS) {
				if (f_lseek(&mp3File, f_tell(&mp3File) - br + offset) == FR_OK) {	// File pointer to Start Of Frame
#ifdef MP3_DEBUG
					printf("File selected\n");
#endif
					return 0;
				}
			}
		}

    	f_close(&mp3File);
		fileOpen = false;
    }
    else {
#ifdef MP3_DEBUG
    	printf("File open ERROR\n");
#endif
    }

	return 1;	// Error
}

uint16_t MP3DecNextFrame(int16_t* outBuff) {

	if (!fileOpen) {
#ifdef MP3_DEBUG
		printf("Error: No file selected\n");
#endif
		return 0;	// No hay archivo seleccionado
	}

	BYTE readBuff[INBUFF_SIZE];
	UINT br;
	int err, offset, bLeft;
	uint8_t* pDecBuff;

	bool needSync = false;	// Find frame sync

	while (!f_eof(&mp3File)) {		// Read until EOF or valid frame
		if (f_read(&mp3File, readBuff, INBUFF_SIZE, &br) == FR_OK) {	// Save file content in readBuff
#ifdef MP3_DEBUG
			printf("File read OK\n");
#endif
			bLeft = br;
			pDecBuff = readBuff;

			if (needSync) {
				offset = MP3FindSyncWord(pDecBuff, bLeft);
				if (offset < 0) {
#ifdef MP3_DEBUG
					printf("Sync Word NOT FOUND\n");
#endif
					continue;	// Skip block
				}
#ifdef MP3_DEBUG
				printf("Sync Word FOUND: %u\n", offset);
#endif
				needSync = false;
				bLeft -= offset;
				pDecBuff += offset;	// pDecBuff: Start of MP3 data frame
			}

			int checkpoint = bLeft;
			err = MP3Decode(mp3Dec, &pDecBuff, &bLeft, outBuff, 0U);

			switch (err) {
				case ERR_MP3_NONE:
					// Valid frame found
#ifdef MP3_DEBUG
					printf("MP3Decode OK. Bytes read: %d\tBytes left: %d\n", checkpoint-bLeft, bLeft);
#endif
					MP3GetLastFrameInfo(mp3Dec, &mp3Info);
					// Move file pointer next to end of frame
					f_lseek(&mp3File, f_tell(&mp3File) - bLeft);
					return mp3Info.outputSamps;
					break;
				case ERR_MP3_INDATA_UNDERFLOW:
				case ERR_MP3_MAINDATA_UNDERFLOW:
#ifdef MP3_DEBUG
					printf("MP3Decode UNDERFLOW ERROR: %d\n", err);
#endif
					if (!f_eof(&mp3File)) {		// Move file pointer to start of last frame
						f_lseek(&mp3File, f_tell(&mp3File) - checkpoint);
					}
					break;
				case ERR_MP3_INVALID_FRAMEHEADER:
				case ERR_MP3_INVALID_HUFFCODES:
#ifdef MP3_DEBUG
					printf("MP3Decode INVALID DATA ERROR: %d\n", err);
#endif
					// Move file pointer next to start of last frame and resync
					needSync = true;
					f_lseek(&mp3File, f_tell(&mp3File) - checkpoint + 1);
					break;

				default:
#ifdef MP3_DEBUG
					printf("MP3Decode UNKNOWN ERROR: %d\n", err);
#endif
					// Move file pointer next to start of last frame and resync
					needSync = true;
					f_lseek(&mp3File, f_tell(&mp3File) - checkpoint + 1);
					break;
			}

		}
		else {		// Error reading file
#ifdef MP3_DEBUG
			printf("File read ERROR\n");
#endif
			break;
		}
	}

	return 0;

}

/*******************************************************************************
 *******************************************************************************
                        LOCAL FUNCTION DEFINITIONS
 *******************************************************************************
 ******************************************************************************/



 
