/***************************************************************************//**
  @file		DAC_hal.c
  @brief	+12 bits DAC+
  @author	KevinWahle
  @date		22 oct. 2022
 ******************************************************************************/

/*******************************************************************************
 * INCLUDE HEADER FILES
 ******************************************************************************/

#include "DAC_hal.h"
#include "DAC.h"
#include "timer/timer.h"

/*******************************************************************************
 * CONSTANT AND MACRO DEFINITIONS USING #DEFINE
 ******************************************************************************/


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

static void write_data_cb ();


/*******************************************************************************
 * ROM CONST VARIABLES WITH FILE LEVEL SCOPE
 ******************************************************************************/

// +ej: static const int temperaturas_medias[4] = {23, 26, 24, 29};+


/*******************************************************************************
 * STATIC VARIABLES AND CONST VARIABLES WITH FILE LEVEL SCOPE
 ******************************************************************************/

//static tim_id_t timer_id;
//
//static circularBuffer16 * mybuff;


/*******************************************************************************
 *******************************************************************************
                        GLOBAL FUNCTION DEFINITIONS
 *******************************************************************************
 ******************************************************************************/
//void DACh_Init (uint16_t frec, circularBuffer16 * buff){		// in KHz
void DACh_Init (void){
	DAC_Init (DAC_0);

	//CBinit16(&buff,200);

	//timerInit();
	//timer_id = timerGetId();

	//timerStart(timer_id, TIMER_MS2TICKS(1.0/frec), TIM_MODE_PERIODIC, write_data_cb);

}

/*void DACh_SetData(){
	CBputByte16(mybuff, data);
}*/

/*******************************************************************************
 *******************************************************************************
                        LOCAL FUNCTION DEFINITIONS
 *******************************************************************************
 ******************************************************************************/

void setDataDAC (uint16_t ** fskptr){
	DAC_SetData(DAC_0, fskptr);
}



 
