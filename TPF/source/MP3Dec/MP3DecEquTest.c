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
#include "DMA2/DMA2.h"
#include "MK64F12.h"
#include "SpectrumAnalyzer/SpectrumAnalyzer.h"
#include "MCAL/gpio.h"

/*******************************************************************************
 * CONSTANT AND MACRO DEFINITIONS USING #DEFINE
 ******************************************************************************/

#define TESTPIN	PORTNUM2PIN(PB, 2)

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
 * STATIC VARIABLES AND CONST VARIABLES WITH FILE LEVEL SCOPE
 ******************************************************************************/


/*******************************************************************************
 *******************************************************************************
                        GLOBAL FUNCTION DEFINITIONS
 *******************************************************************************
 ******************************************************************************/

void App_Init() {

	gpioMode(TESTPIN, OUTPUT);
	gpioWrite(TESTPIN, LOW);

	/* Give a work area to the default drive */
    if (f_mount(&FatFs, "1:/", 0) == FR_OK) {
    	printf("FileSystem mount OK\n");
    }

	if (MP3DecInit()) {
    	printf("MP3 Init Error\n");
	}

}

void App_Run() {

	DMA_initPingPong_Dac();
	initEqualizer();
	setUpFilter(0, 0);
	setUpFilter(0, 1);
	setUpFilter(0, 2);
	setUpFilter(0, 3);
	
	int16_t buff1[OUTBUFF_SIZE];
	int16_t buff2[OUTBUFF_SIZE];

	float32_t buffFloatSrc[OUTBUFF_SIZE], buffFloatDest[OUTBUFF_SIZE];

	if (MP3SelectSong("1:/tono.mp3")) {
    	printf("ERROR: Song not selected\n");
    	return;
	}

	int16_t* pTable = buff1;
	int16_t* pPrevTable = pTable;

	if (!MP3DecNextFrame(pTable)) {
    	printf("FRAME ERROR\n");
	}

	// Pasa a float
	for (int i = 0; i < OUTBUFF_SIZE; i++) {
		buffFloatSrc[i] = (float32_t)pTable[i];
	}

	blockEqualizer(buffFloatSrc, buffFloatDest, OUTBUFF_SIZE);

	// 16 bit to 12 bit and shifting
	for (int i = 0; i < OUTBUFF_SIZE; i++) {
		pTable[i] = (int16_t)buffFloatDest[i];
		pTable[i] += 0x8000U;
		pTable[i] *= (double)0xFFFU / 0xFFFFU;
	}

	uint16_t br = 1;

	int32_t temp;

	DMA_pingPong_DAC(buff1, buff2, (uint32_t)(&DAC0->DAT[0].DATL), OUTBUFF_SIZE);

	while(br) {

		pTable = (int16_t*)DMA_availableTable_pingPong();
		if (pTable == NULL) {
	    	printf("Table Pointer ERROR\n");
		}

		if (pTable != pPrevTable) {
			pPrevTable = pTable;

			gpioWrite(TESTPIN, HIGH);
			br = MP3DecNextFrame(pTable);

			// Pasa a float
			for (int i = 0; i < OUTBUFF_SIZE; i++) {
				buffFloatSrc[i] = (float32_t)pTable[i];
			}

			blockEqualizer(buffFloatSrc, buffFloatDest, OUTBUFF_SIZE);

			// 16 bit to 12 bit and shifting
			for (int i = 0; i < OUTBUFF_SIZE; i++) {
				pTable[i] = (int16_t)buffFloatDest[i];
				pTable[i] += 0x8000U;
				pTable[i] *= (double)0xFFFU / 0xFFFFU;
			}
			gpioWrite(TESTPIN, LOW);

		}
	}
	DMA_pause_pingPong();

	while(1);
}


/*******************************************************************************
 *******************************************************************************
                        LOCAL FUNCTION DEFINITIONS
 *******************************************************************************
 ******************************************************************************/

