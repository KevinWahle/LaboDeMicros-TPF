/***************************************************************************//**
  @file     App.c
  @brief    TP2: Comunicacion Serie
  @author   Grupo 5
 ******************************************************************************/

/*******************************************************************************
 * INCLUDE HEADER FILES
 ******************************************************************************/
#include "hardware.h"
#include "PIT/PIT.h"
#include "DMA.h"
#include <stdio.h>

#include "../MCAL/gpio.h"
/*******************************************************************************
 * CONSTANT AND MACRO DEFINITIONS USING #DEFINE
 ******************************************************************************/

//#define ENABLE_TP

#ifdef ENABLE_TP
#define TP_PIN	PORTNUM2PIN(PB, 19)
#endif



static uint16_t ** PPglobal;

void initDMA(uint16_t * DACptr, uint16_t **PP)
{
	#ifdef ENABLE_TP
		gpioMode(TP_PIN, OUTPUT);
		gpioWrite(TP_PIN, LOW);
	#endif
	//PITInit(PIT_0, PIT_MS2TICK(100), NULL);
	//PITStart(PIT_0);
	PPglobal = PP;

	/* Enable the clock for the PORT C*/
	SIM->SCGC5 |= SIM_SCGC5_PORTC_MASK;

	/* Enable the clock for the eDMA and the DMAMUX. */
	SIM->SCGC7 |= SIM_SCGC7_DMA_MASK;
	SIM->SCGC6 |= SIM_SCGC6_DMAMUX_MASK;

	DMAMUX->CHCFG[0] |= DMAMUX_CHCFG_ENBL_MASK | DMAMUX_CHCFG_TRIG_MASK | DMAMUX_CHCFG_SOURCE(63);

	/* Enable the interrupts for the channel 0. */
	/* Clear all the pending events. */
	NVIC_ClearPendingIRQ(DMA0_IRQn);
	/* Enable the DMA interrupts. */
	NVIC_EnableIRQ(DMA0_IRQn);

	/// ============= INIT TCD0 ===================//
	/* Set memory address for source and destination. */

	DMA0->TCD[0].SADDR= (uint32_t)(*PPglobal);
	DMA0->TCD[0].DADDR = (uint32_t)(DACptr);

		/* Set an offset for source and destination address. */
	DMA0->TCD[0].SOFF =0x01; // Source address offset of 2 bytes per transaction.
	DMA0->TCD[0].DOFF =0x01; // Destination address offset of 1 byte per transaction.

	/* Set source and destination data transfer size is 1 byte. */
	DMA0->TCD[0].ATTR = DMA_ATTR_SSIZE(0) | DMA_ATTR_DSIZE(0);

	/*Number of bytes to be transfered in each service request of the channel.*/
	DMA0->TCD[0].NBYTES_MLNO= 0x02;

	/* Current major iteration count (5 iteration of 1 byte each one). */
	DMA0->TCD[0].CITER_ELINKNO = DMA_CITER_ELINKNO_CITER(0x01);
	DMA0->TCD[0].BITER_ELINKNO = DMA_BITER_ELINKNO_BITER(0x01);

	DMA0->TCD[0].SLAST = -2;
	DMA0->TCD[0].DLAST_SGA = -2;

	/* Setup control and status register. */

	DMA0->TCD[0].CSR = DMA_CSR_INTMAJOR_MASK;	//Enable Major Interrupt.

	/* Enable request signal for channel 0. */
	DMA0->ERQ |= DMA_ERQ_ERQ0_MASK;

	/* Never leave main */
	return ;
}

__ISR__ DMA0_IRQHandler(void)
{
	#ifdef ENABLE_TP
		gpioWrite(TP_PIN, HIGH);
	#endif

	/* Clear the interrupt flag. */
	DMA0->CINT |= 0;

	DMA0->TCD[0].SADDR= (uint32_t)(*PPglobal);

	#ifdef ENABLE_TP
		gpioWrite(TP_PIN, LOW);
	#endif
}

/* The red LED is toggled when an error occurs. */
__ISR__ DMA_Error_IRQHandler(void)
{
	/* Clear the error interrupt flag.*/
	DMA0->CERR |= 0;
}
