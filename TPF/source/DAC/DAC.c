/***************************************************************************//**
  @file		DAC.c
  @brief	+Descripcion del archivo+
  @author	KevinWahle
  @date		15 oct. 2022
 ******************************************************************************/

/*******************************************************************************
 * INCLUDE HEADER FILES
 ******************************************************************************/

#include "../MCAL/gpio.h"
#include "MK64F12.h"
#include "hardware.h"
#include "DAC.h"
#include "DMA/DMA.h"

#include <stdint.h>
#include <stdio.h>
#include <string.h>

/*******************************************************************************
 * CONSTANT AND MACRO DEFINITIONS USING #DEFINE
 ******************************************************************************/

#define DAC_DATL_DATA0_WIDTH 8


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

// +ej: static int temperaturas_actuales[4];+
//static PORT_Type* const portPtr[] = PORT_BASE_PTRS;


/*******************************************************************************
 *******************************************************************************
                        GLOBAL FUNCTION DEFINITIONS
 *******************************************************************************
 ******************************************************************************/



static SIM_Type* sim_ptr = SIM;				// For clock enable




void DAC_Init (DAC_n dac_n)
{
	DAC_Type * DACn;
	if (dac_n==DAC_0){
		sim_ptr->SCGC2 |= SIM_SCGC2_DAC0_MASK;	// Clock Gating
		DACn=DAC0;
	}
	else{
		sim_ptr->SCGC2 |= SIM_SCGC2_DAC1_MASK;	// Clock Gating
		DACn=DAC1;
	}

	
	DACn->C0 = DAC_C0_DACEN_MASK | DAC_C0_DACRFS_MASK | DAC_C0_DACTRGSEL_MASK;	//DAC enable, DACREF_2 reference, software trigger	//TODO: por que DACREF_2?
	DACn->C1 &= ~DAC_C1_DACBFEN_MASK;	//Buffer disabled


}



void DAC_SetData (DAC_n dac_n, uint16_t **fskptr)		// 12bit length
{
	DAC_Type * DACn;
	if (dac_n==DAC_0){
		DACn=DAC0;
	}
	else{
		DACn=DAC1;
	}

	//initDMA(&(DACn->DAT[0].DATL), fskptr);
	/*
	DACn->DAT[0].DATL = DAC_DATL_DATA0(data);
	DACn->DAT[0].DATH = DAC_DATH_DATA1(data >> DAC_DATL_DATA0_WIDTH);*/
}


/*******************************************************************************
 *******************************************************************************
                        LOCAL FUNCTION DEFINITIONS
 *******************************************************************************
 ******************************************************************************/



 
