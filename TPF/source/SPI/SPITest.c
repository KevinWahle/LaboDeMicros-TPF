/***************************************************************************//**
  @file     App.c
  @brief    Application functions
  @author   Nicolás Magliola
 ******************************************************************************/

/*******************************************************************************
 * INCLUDE HEADER FILES
 ******************************************************************************/

#include "../MCAL/board.h"
#include "SPI.h"
#include "../buffer/SPI_buffer.h"
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

/*******************************************************************************
 * CONSTANT AND MACRO DEFINITIONS USING #DEFINE
 ******************************************************************************/


/*******************************************************************************
 * FUNCTION PROTOTYPES FOR PRIVATE FUNCTIONS WITH FILE LEVEL SCOPE
 ******************************************************************************/


/*******************************************************************************
 *******************************************************************************
                        GLOBAL FUNCTION DEFINITIONS
 *******************************************************************************
 ******************************************************************************/
static SPI_config_t config;
static SPI_config_t * myconfig;

static package mypkg[4];

uint8_t first, second;


bool done;

void myreadCB();
void myreadCB2();

/* Función que se llama 1 vez, al comienzo del programa */
void App_Init (void)
{
    gpioMode(PIN_SW3, INPUT);					//Ya es pullup electricamente
    gpioMode(PIN_SW2, INPUT_PULLUP);

    myconfig=&config;
    myconfig->type=MASTER;
    myconfig->PCS_inactive_state=1;
    myconfig->LSB_fist=0;
    myconfig->frame_size=8;
    myconfig->clk_pol=0;
    myconfig->clk_phase=0;
    myconfig->Baud_rate_scaler=0b0011;

    SPI_config(SPI_0,myconfig);

    done=0;
}

/* Función que se llama constantemente en un ciclo infinito */
void App_Run (void)
{
	//if(SPITransferCompleteFlag(SPI_0)){
    //SPIRead(SPI_0);
	//}

    

	if (!gpioRead(PIN_SW3)){            //Escribo
		while (!gpioRead(PIN_SW3));
			sendEscritura();
			sendLectura();
	}

	if(done){
		printf("%c,%c\n", first, second);
		first='Z'; second='K';
		done=0;
	}
}

void myreadCB(){
    done=1;
}


/*******************************************************************************
 *******************************************************************************
                        LOCAL FUNCTION DEFINITIONS
 *******************************************************************************
 ******************************************************************************/
void sendEscritura(){
	mypkg[0].msg = 'a';
	mypkg[0].pSave = NULL;
	mypkg[0].cb = NULL;
	mypkg[0].read = 0;
	mypkg[0].cs_end = 0;

	mypkg[1].msg = 't';
	mypkg[1].pSave = NULL;
	mypkg[1].cb = NULL;
	mypkg[1].read = 0;
	mypkg[1].cs_end = 0;

	mypkg[2].msg = 'f';
	mypkg[2].pSave = NULL;
	mypkg[2].cb = 0;
	mypkg[2].read = 0;
	mypkg[2].cs_end = 1;

	SPISend(SPI_0, mypkg, 3, 0);
}

void sendLectura(){
	mypkg[0].msg = 'a';
	mypkg[0].pSave = NULL;
	mypkg[0].cb = NULL;
	mypkg[0].read = 0;
	mypkg[0].cs_end = 0;

	mypkg[1].msg = 'b';
	mypkg[1].pSave = &first;
	mypkg[1].cb = NULL;
	mypkg[1].read = 1;
	mypkg[1].cs_end = 0;

	mypkg[2].msg = 'f';
	mypkg[2].pSave = &second;
	mypkg[2].cb = myreadCB;
	mypkg[2].read = 1;
	mypkg[2].cs_end = 1;

	SPISend(SPI_0, mypkg, 3, 0);
}
/*******************************************************************************
 ******************************************************************************/
