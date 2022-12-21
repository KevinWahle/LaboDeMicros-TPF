/***************************************************************************//**
  @file		CMP.c
  @brief	+Descripcion del archivo+
  @author	KevinWahle
  @date		23 oct. 2022
 ******************************************************************************/

/*******************************************************************************
 * INCLUDE HEADER FILES
 ******************************************************************************/

#include "MK64F12.h"
#include "CMP.h"
#include "../MCAL/gpio.h"

/*******************************************************************************
 * CONSTANT AND MACRO DEFINITIONS USING #DEFINE
 ******************************************************************************/

#define CMP_ANALOG_IN 1U // PTC7, CMP0_IN1				//PTC3, CMP1_IN1
#define CMP_ANALOG_REF 7U	//DAC
#define DAC_VALUE 31U

/*******************************************************************************
 * ENUMERATIONS AND STRUCTURES AND TYPEDEFS
 ******************************************************************************/

typedef CMP_Type * CMP_t;


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
								//CMP0_IN1			//CMP1_IN1
static uint8_t CMP_in_pins[] = {PORTNUM2PIN(PC, 7), PORTNUM2PIN(PC, 3)};

								//CMP0_OUT			//CMP1_OUT
static uint8_t CMP_out_pins[] = {PORTNUM2PIN(PC, 5), PORTNUM2PIN(PC, 4)};

static PORT_Type* const portPtr[] = PORT_BASE_PTRS;

static CMP_Type * CMPPorts[] = CMP_BASE_PTRS;
//static IRQn_Type CMP_IRQ[] = CMP_IRQS;

/*******************************************************************************
 *******************************************************************************
                        GLOBAL FUNCTION DEFINITIONS
 *******************************************************************************
 ******************************************************************************/

void CMP_Init (CMP_n cmp_n, HYST_LEVEL hyst_level, INVERT polarity){ //Number of consecutive samples that must agree
	CMP_t cmp = CMPPorts[cmp_n];

	SIM->SCGC4 |=SIM_SCGC4_CMP_MASK;

	cmp->CR0 = CMP_CR0_HYSTCTR(hyst_level) | CMP_CR0_FILTER_CNT_MASK;
	cmp->FPR = CMP_FPR_FILT_PER_MASK;		// Maximo periodo (5.1us)
//	cmp->CR0 = CMP_CR0_HYSTCTR(hyst_level);
	cmp->CR1 = CMP_CR1_INV(polarity) | CMP_CR1_OPE_MASK;

	portPtr[PIN2PORT(CMP_in_pins[cmp_n])]->PCR[PIN2NUM(CMP_in_pins[cmp_n])]=PORT_PCR_MUX(0x00); //In
	portPtr[PIN2PORT((CMP_out_pins[cmp_n]))]->PCR[PIN2NUM(CMP_out_pins[cmp_n])]=PORT_PCR_MUX(0x06); //Out

	//cmp->CR1 |= CMP_CR1_COS_MASK;

	cmp->DACCR = CMP_DACCR_VOSEL(DAC_VALUE) | CMP_DACCR_DACEN_MASK | CMP_DACCR_VRSEL_MASK;	//VOSEL=1/2 VCC

	cmp->MUXCR = CMP_MUXCR_MSEL(CMP_ANALOG_REF) | CMP_MUXCR_PSEL(CMP_ANALOG_IN);// | CMP_MUXCR_PSTM_MASK;

	//cmp->FPR = CMP_FPR_FILT_PER(0x01);

	cmp->CR1 |= CMP_CR1_EN_MASK;	//Enable Module

	//cmp->SCR |= (CMP_SCR_IEF_MASK | CMP_SCR_IER_MASK); //Interrupt falling and Rising

	//NVIC_EnableIRQ(CMP_IRQ[cmp_n]);

}

/*******************************************************************************
 *******************************************************************************
                        LOCAL FUNCTION DEFINITIONS
 *******************************************************************************
 ******************************************************************************/

/*
void CMP0_IRQHandler(void){
	for (int i=0;i<3; i++){
		i++;
	}
	CMP_t cmp = CMPPorts[CMP0_t];
	cmp->SCR |= (CMP_SCR_CFR_MASK | CMP_SCR_CFF_MASK);

}*/
 
