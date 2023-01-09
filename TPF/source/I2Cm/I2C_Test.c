/***************************************************************************//**
  @file     App.c
  @brief    TP2: Comunicacion Serie
  @author   Grupo 5
 ******************************************************************************/

/*******************************************************************************
 * INCLUDE HEADER FILES
 ******************************************************************************/

#include "I2Cm/I2Cm.h"
#include "timer/timer.h"
#include "MCAL/gpio.h"
#include "MCAL/board.h"
/*******************************************************************************
 * CONSTANT AND MACRO DEFINITIONS USING #DEFINE
 ******************************************************************************/

#define I2C_ID		I2C_0

#define I2C_ADDR	0x1D
//#define I2C_ADDR	0xFF


/*******************************************************************************
 * FUNCTION PROTOTYPES FOR PRIVATE FUNCTIONS WITH FILE LEVEL SCOPE
 ******************************************************************************/

/*******************************************************************************
 *******************************************************************************
                        GLOBAL VARIABLES
 *******************************************************************************
 ******************************************************************************/


/*******************************************************************************
 *******************************************************************************
                        GLOBAL FUNCTION DEFINITIONS
 *******************************************************************************
 ******************************************************************************/
uint8_t readBuffer [10];
//uint8_t writeBuffer = 0x0D;  // WHO AM I -> DEVICE ID
/* Función que se llama 1 vez, al comienzo del programa */
void App_Init (void)
{
	gpioMode(PIN_LED_RED, OUTPUT);
	gpioWrite(PIN_LED_RED, LOW);

	//timerInit();
}

/* Función que se llama constantemente en un ciclo infinito */
void App_Run (void)
{
	I2CmInit(I2C_ACC);
	//I2CmStartTransaction(I2C_ACC, I2C_ADDR, &writeBuffer, 1, readBuffer, 1);

	uint8_t writeBuffer[2];
	writeBuffer[0] = 0x5B;
	writeBuffer[1] = 0x1F;

	Transaction_t T = { .id = I2C_ACC,
						.address = I2C_ADDR,
						.writeBuffer = writeBuffer,
						.writeSize = 2,
						.readBuffer = NULL,
						.readSize = 0
					  };
	pushTransaction(&T);

	writeBuffer[0] = 0xAA;
	writeBuffer[1] = 0xCC;
	pushTransaction(&T);

	writeBuffer[0] = 0xFF;
	writeBuffer[1] = 0x11;
	pushTransaction(&T);

	writeBuffer[0] = 0xFA;
	writeBuffer[1] = 0x12;
	pushTransaction(&T);

	writeBuffer[0] = 0xFB;
	writeBuffer[1] = 0x13;
	pushTransaction(&T);

	writeBuffer[0] = 0xFC;
	writeBuffer[1] = 0x14;
	pushTransaction(&T);


	timerDelay(TIMER_MS2TICKS(4));

	writeBuffer[0] = 0xBB;
	writeBuffer[1] = 0x14;
	pushTransaction(&T);

	writeBuffer[0] = 0xBA;
	writeBuffer[1] = 0x14;
	pushTransaction(&T);

	writeBuffer[0] = 0xBC;
	writeBuffer[1] = 0x14;
	pushTransaction(&T);

	writeBuffer[0] = 0xBD;
	writeBuffer[1] = 0x14;
	pushTransaction(&T);

	writeBuffer[0] = 0xBE;
	writeBuffer[1] = 0x14;
	pushTransaction(&T);

	writeBuffer[0] = 0xCA;
	writeBuffer[1] = 0x14;
	pushTransaction(&T);

	writeBuffer[0] = 0xCB;
	writeBuffer[1] = 0x14;
	pushTransaction(&T);

	while(1){
		//timerDelay(TIMER_MS2TICKS(1000));
	//	gpioToggle(PIN_LED_RED);
	}
}


/*******************************************************************************
 *******************************************************************************
                        LOCAL FUNCTION DEFINITIONS
 *******************************************************************************
 ******************************************************************************/


/*******************************************************************************
 ******************************************************************************/
