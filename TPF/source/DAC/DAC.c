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

#include <stdint.h>
#include <stdio.h>
#include <string.h>

/*******************************************************************************
 * CONSTANT AND MACRO DEFINITIONS USING #DEFINE
 ******************************************************************************/

#define DAC_DATL_DATA0_WIDTH 8

/*******************************************************************************
 * STATIC VARIABLES AND CONST VARIABLES WITH FILE LEVEL SCOPE
 ******************************************************************************/

static SIM_Type* sim_ptr = SIM;				// For clock enable

/*******************************************************************************
 *******************************************************************************
                        GLOBAL FUNCTION DEFINITIONS
 *******************************************************************************
 ******************************************************************************/

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

	
	DACn->C0 = DAC_C0_DACEN_MASK | DAC_C0_DACRFS_MASK | DAC_C0_DACTRGSEL_MASK;	//DAC enable, DACREF_2 reference, software trigger
	DACn->C1 &= ~DAC_C1_DACBFEN_MASK;	//Buffer disabled


}
