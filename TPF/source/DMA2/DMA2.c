/***************************************************************************//**
  @file     App.c
  @brief    TPF: Reproductor MP3
  @author   Grupo 5
 ******************************************************************************/

/*******************************************************************************
 * INCLUDE HEADER FILES
 ******************************************************************************/
#include "MCAL/gpio.h"
#include "MK64F12.h"
#include "DMA2.h"
#include "hardware.h"
#include "PIT/PIT.h"
#include <stddef.h>
#include "FTM2.h"
#include "PORT.h"
#include <math.h>
#include "DAC/DAC_hal.h"
#include <stdint.h>

#define PIN_RED_LED 		22     	//PTB22
#define PIN_BLUE_LED 		21     	//PTB21
#define PIN_GREEN_LED 		26 	   	//PTE26
#define PIN_SW3			  	4 		//PTA4
#define PIN_SW2			  	6 		//PTC6

/* Structure with the TCD fields. */
typedef struct
{
	uint32_t SADDR;
	uint16_t SOFF;
	uint16_t ATTR;
	union
	{
		uint32_t NBYTES_MLNO;
		uint32_t NBYTES_MLOFFNO;
		uint32_t NBYTES_MLOFFYES;
	};
	uint32_t SLAST;
	uint32_t DADDR;
	uint16_t DOFF;
	union
	{
		uint16_t CITER_ELINKNO;
		uint16_t CITER_ELINKYES;
	};
	uint32_t DLAST_SGA;
	uint16_t CSR;
	union
	{
		uint16_t BITER_ELINKNO;
		uint16_t BITER_ELINKYES;
	};
}TCD_t;

/* Enumeration to access the TCDs. */
enum{
	TCD0 = 0,
	TCD1,
	TCDs_AMOUNT
}TCD_enum;

void DMA_Test(void);

#define TABLE_LENGHT_BYTES (64*24*2)//(1584*2)
/*******************************************************************************
 *******************************************************************************
                        	GLOBAL VARIABLES
 *******************************************************************************
 ******************************************************************************/
/* Auxiliary variable used to modify the source buffer on each iteration. */

//uint16_t sourceBuffer[10] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
//uint16_t sourceBuffer[10] = {0x1234,0x6789,0x1122,0x2233,0x5588,0x2345,0x3145,0x8172,0x6183,0x3756};
uint8_t destinationBuffer[10];
uint16_t destinationNoBuffer;
uint16_t PWMList[1584 + 1 + 10];
static uint16_t sourceBuffer[] = {5, 10, 15, 20, 25, 50, 5, 10, 15, 20, 25, 50, 0};

static TCD_t tcdA  __attribute__ ((aligned (32)));
static TCD_t tcdB  __attribute__ ((aligned (32)));


typedef enum {FREE, BUSY} AvailableTable;
static AvailableTable table1 = FREE;
static AvailableTable table2 = FREE;
static uint16_t* table1Internal;
static uint16_t* table2Internal;


static ActualTable = 1;



#define TS 22680 // ns
/*******************************************************************************
 *******************************************************************************
                        GLOBAL FUNCTION DEFINITIONS
 *******************************************************************************
 ******************************************************************************/
uint16_t memDirTable1[512] = {0, 1, 2, 3, 4};
uint16_t memDirTable2[512] = {5, 6, 7, 8, 9};


/*

void App_Init (void)
{
	for(uint16_t i = 0; i < 512; i++){
		memDirTable1[i] = 2048*sin(2.0*3.1415*250.0*(double)i*22.68e-6) + 2048;
	}
	for(uint16_t j = 0; j < 512; j++){
		memDirTable2[j] = 2048*cos(2.0*3.1415*250.0*(double)j*22.68e-6) + 2048;
	}
}
void App_Run (void)
{
	/*
	DACh_Init();
	PITInit(PIT_0, PIT_NS2TICK(TS), NULL);
	DMA_initPingPong_Dac();

	DMA_pingPong_DAC((uint32_t)memDirTable1, (uint32_t)memDirTable2, (uint32_t)(&DAC0->DAT[0].DATL), 512);
	PITStart(PIT_0);
	while(1){
		//if( gpioRead(PORTNUM2PIN(PC, 6)) == 0){
		//	DMA_displayTable();
		//	while(gpioRead(PORTNUM2PIN(PC, 6)) == 0);
		//}
	}
	//PORT_Init();
	//FTM_Init ();
	//DMA_Test();
	//while(1);

	DMA_initPingPong_Dac();
	DMA_pingPong_DAC((uint32_t)memDirTable1, (uint32_t)memDirTable2, (uint32_t)(&DAC0->DAT[0].DATL), 512);

	while(1){
		if( gpioRead(PORTNUM2PIN(PC, 6)) == 0){
			DMA_pause_pingPong();
			while(gpioRead(PORTNUM2PIN(PC, 6)) == 0);
			DMA_continue_pingPong();
		}
	}
}

*/

void DMA_initPingPong_Dac(){
	PITInit(PIT_0, PIT_NS2TICK(TS), NULL);
	DACh_Init();
	/* Enable the clock for the PORT C*/
	SIM->SCGC5 |= SIM_SCGC5_PORTC_MASK;

	/* Configure the SW2 DMA request source on falling edge. */

	PORTC->PCR[6]=0x00;
	PORTC->PCR[6]|=PORT_PCR_MUX(PORT_mGPIO); 		       //Set MUX to GPIO;
	PORTC->PCR[6]|=PORT_PCR_PE(1);          		       //Pull UP/Down  Enable;
	PORTC->PCR[6]|=PORT_PCR_PS(1);          		       //Pull UP
	PORTC->PCR[6]|=PORT_PCR_IRQC(PORT_eDMAFalling);


	/* Enable the clock for the eDMA and the DMAMUX. */
	SIM->SCGC7 |= SIM_SCGC7_DMA_MASK;
	SIM->SCGC6 |= SIM_SCGC6_DMAMUX_MASK;


	/* Enable the eDMA channel 0 and set the PORTC as the DMA request source. */
	//DMAMUX->CHCFG[0] |= DMAMUX_CHCFG_ENBL_MASK | DMAMUX_CHCFG_SOURCE(51);
	DMAMUX->CHCFG[0] |= DMAMUX_CHCFG_ENBL_MASK | DMAMUX_CHCFG_TRIG_MASK | DMAMUX_CHCFG_SOURCE(63);

	/* Enable the interrupts for the channel 0. */
	/* Clear all the pending events. */
	NVIC_ClearPendingIRQ(DMA0_IRQn);
	/* Enable the DMA interrupts. */
	NVIC_EnableIRQ(DMA0_IRQn);
}
void DMA_pingPong_DAC(uint32_t memDirTable1, uint32_t memDirTable2, uint32_t dacAddress, uint16_t tableSize){

	PITStop(PIT_0);
	table1Internal = (uint16_t*)memDirTable1;
	table2Internal = (uint16_t*)memDirTable2;
	//==========================================
	//=================tcdA=====================
	//==========================================

	tcdA.SADDR= (uint32_t)memDirTable1;
	tcdA.DADDR = (uint32_t)dacAddress;

		/* Set an offset for source and destination address. */
	tcdA.SOFF =0x02; // Source address offset of 2 bytes per transaction.
	tcdA.DOFF =0x00; // Destination address offset of 1 byte per transaction.

	/* Set source and destination data transfer size is 1 byte. */
	tcdA.ATTR = DMA_ATTR_SSIZE(1) | DMA_ATTR_DSIZE(1);

	/*Number of bytes to be transfered in each service request of the channel.*/
	tcdA.NBYTES_MLNO= 0x02;

	/* Current major iteration count (5 iteration of 1 byte each one). */
	tcdA.CITER_ELINKNO = DMA_CITER_ELINKNO_CITER(tableSize);
	tcdA.BITER_ELINKNO = DMA_BITER_ELINKNO_BITER(tableSize);

	tcdA.SLAST = 0;
	tcdA.DLAST_SGA = (uint32_t)&tcdB;

	tcdA.CSR = DMA_CSR_ESG_MASK | DMA_CSR_INTMAJOR_MASK; //Enable the scatter/gather feature.

	//==========================================
	//=================tcdB=====================
	//==========================================

	tcdB.SADDR= (uint32_t)memDirTable2;
	tcdB.DADDR = (uint32_t)dacAddress;

		/* Set an offset for source and destination address. */
	tcdB.SOFF =0x02; // Source address offset of 2 bytes per transaction.
	tcdB.DOFF =0x00; // Destination address offset of 1 byte per transaction.

	/* Set source and destination data transfer size is 1 byte. */
	tcdB.ATTR = DMA_ATTR_SSIZE(1) | DMA_ATTR_DSIZE(1);

	/*Number of bytes to be transfered in each service request of the channel.*/
	tcdB.NBYTES_MLNO= 0x02;

	/* Current major iteration count (5 iteration of 1 byte each one). */
	tcdB.CITER_ELINKNO = DMA_CITER_ELINKNO_CITER(tableSize);
	tcdB.BITER_ELINKNO = DMA_BITER_ELINKNO_BITER(tableSize);

	tcdB.SLAST = 0;
	tcdB.DLAST_SGA = (uint32_t)&tcdA;

	tcdB.CSR = DMA_CSR_ESG_MASK | DMA_CSR_INTMAJOR_MASK; //Enable the scatter/gather feature.

	//=========================================
	//=========================================
	//=========================================

	DMA0->TCD[0].SADDR = tcdA.SADDR;
	DMA0->TCD[0].DADDR = tcdA.DADDR;

		/* Set an offset for source and destination address. */
	DMA0->TCD[0].SOFF =tcdA.SOFF; // Source address offset of 2 bytes per transaction.
	DMA0->TCD[0].DOFF =tcdA.DOFF; // Destination address offset of 1 byte per transaction.

	/* Set source and destination data transfer size is 1 byte. */
	DMA0->TCD[0].ATTR = tcdA.ATTR;

	/*Number of bytes to be transfered in each service request of the channel.*/
	DMA0->TCD[0].NBYTES_MLNO= tcdA.NBYTES_MLNO;

	/* Current major iteration count (5 iteration of 1 byte each one). */
	DMA0->TCD[0].CITER_ELINKNO = tcdA.CITER_ELINKNO;
	DMA0->TCD[0].BITER_ELINKNO = tcdA.BITER_ELINKNO;

	DMA0->TCD[0].SLAST = tcdA.SLAST;
	DMA0->TCD[0].DLAST_SGA = tcdA.DLAST_SGA;

	DMA0->TCD[0].CSR = tcdA.CSR; //Enable the scatter/gather feature.

	/* Enable request signal for channel 0. */
	DMA0->ERQ |= DMA_ERQ_ERQ0_MASK;

	PITStart(PIT_0);
	table1 = BUSY;
	table2 = FREE;
	return ;
}

uint16_t* DMA_availableTable_pingPong(){

	hw_DisableInterrupts();

	if(table1 == FREE){
		hw_EnableInterrupts();
		return table1Internal;
	}
	else if(table2 == FREE){
		hw_EnableInterrupts();
		return table2Internal;
	}
	return NULL;
}
void DMA_pause_pingPong(){
	PITStop(PIT_0);
	DAC0->DAT[0].DATL = 0;
	DAC0->DAT[0].DATH = 0;
}
void DMA_continue_pingPong(){
	PITStart(PIT_0);
}


//======================================================
//======================================================
//======================================================

static uint32_t global_memDirTable;
void DMA_initDisplayTable(uint32_t memDirTable){
	PORT_Init();
	FTM_Init (*((uint16_t*)memDirTable));
	hw_DisableInterrupts ();
	global_memDirTable = memDirTable;
	/**************************************************************************/
	/***** Configure the FRDM-K64F SW2 (PTC6) as the DMA request source. *****/

	/* Enable the clock for the PORT C*/
	SIM->SCGC5 |= SIM_SCGC5_PORTC_MASK;     // prendo port C

	/* Configure the SW2 DMA request source on falling edge. */

	PORTC->PCR[6]=0x00;
	PORTC->PCR[6]|=PORT_PCR_MUX(PORT_mGPIO); 		       //Set MUX to GPIO;
	PORTC->PCR[6]|=PORT_PCR_PE(1);          		       //Pull UP/Down  Enable;
	PORTC->PCR[6]|=PORT_PCR_PS(1);          		       //Pull UP
	PORTC->PCR[6]|=PORT_PCR_IRQC(PORT_eDMAFalling);

	/* Enable the clock for the eDMA and the DMAMUX. */
	SIM->SCGC7 |= SIM_SCGC7_DMA_MASK;
	SIM->SCGC6 |= SIM_SCGC6_DMAMUX_MASK;

	DMAMUX->CHCFG[1] |= DMAMUX_CHCFG_ENBL_MASK | DMAMUX_CHCFG_SOURCE(20);   // FTM0 CH0

	/* Enable the interrupts for the channel 1. */
	/* Clear all the pending events. */
	NVIC_ClearPendingIRQ(DMA1_IRQn);
	/* Enable the DMA interrupts. */
	NVIC_EnableIRQ(DMA1_IRQn);

	hw_EnableInterrupts();

	return ;
}
void DMA_displayTable(void){
	/// ============= INIT TCD0 ===================//
	/* Set memory address for source and destination. */

	DMA0->TCD[1].SADDR= (uint32_t)((uint16_t*)global_memDirTable + 1);
	DMA0->TCD[1].DADDR =(uint32_t)(&(FTM0->CONTROLS[0].CnV));

		/* Set an offset for source and destination address. */
	DMA0->TCD[1].SOFF =0x02; // Source address offset of 2 bytes per transaction.
	DMA0->TCD[1].DOFF =0x00; // Destination address offset of 0 bytes per transaction.

	/* Set source and destination data transfer size is 2 bytes. */
	DMA0->TCD[1].ATTR = DMA_ATTR_SSIZE(1) | DMA_ATTR_DSIZE(1);

	/*Number of bytes to be transfered in each service request of the channel.*/
	DMA0->TCD[1].NBYTES_MLNO= 0x02;

	/* Current major iteration count (5 iteration of 1 byte each one). */
	DMA0->TCD[1].CITER_ELINKNO = DMA_CITER_ELINKNO_CITER(TABLE_LENGHT_BYTES/2);
	DMA0->TCD[1].BITER_ELINKNO = DMA_BITER_ELINKNO_BITER(TABLE_LENGHT_BYTES/2);

	DMA0->TCD[1].SLAST = -TABLE_LENGHT_BYTES/2;
	DMA0->TCD[1].DLAST_SGA = 0x00;

	/* Setup control and status register. */

	DMA0->TCD[1].CSR = DMA_CSR_INTMAJOR_MASK;	//Enable Major Interrupt.

	DMA0->ERQ |= DMA_ERQ_ERQ1_MASK;
	/* Enable request signal for channel 0. */
	FTM_Init (*((uint16_t*)global_memDirTable));
	FTM_StartClock(FTM0);

	return ;
}

void DMA_Test(void)
{

	hw_DisableInterrupts ();
	/**************************************************************************/
	/***** Configure the FRDM-K64F LEDs (RED - PTB22, GREEN - PTE26,
	 * BLUE - PTB21) to report events. *****/

	/* Enable the clock for the PORT B and the PORT E. */

	SIM->SCGC5 |= SIM_SCGC5_PORTA_MASK;   // prendo los clocks de los puertos gpio
	SIM->SCGC5 |= SIM_SCGC5_PORTB_MASK;
	SIM->SCGC5 |= SIM_SCGC5_PORTE_MASK;

	/* Configure the MUX option. */

	//// NO VA PORTA->PCR[4] |= PORT_PCR_ISF_MASK;  //Clear mask

	PORTB->PCR[PIN_BLUE_LED] |= PORT_PCR_MUX(PORT_mGPIO);   // los configuro como gpio
	PORTB->PCR[PIN_RED_LED] |= PORT_PCR_MUX(PORT_mGPIO);
	PORTE->PCR[PIN_GREEN_LED] |= PORT_PCR_MUX(PORT_mGPIO);

	/* Select LEDs as outputs. */

	PTB->PDDR |= (1 << PIN_BLUE_LED) | (1 << PIN_RED_LED); // salidas
	PTE->PDDR |= (1 << PIN_GREEN_LED);


	/* Turn all the LEDs off. (Active low) */

	PTB->PSOR |= (1 << PIN_BLUE_LED) | (1 << PIN_RED_LED);  // los activo bajo
	PTE->PSOR |= (1 << PIN_GREEN_LED);

	/**************************************************************************/
	/***** Configure the FRDM-K64F SW2 (PTC6) as the DMA request source. *****/

	/* Enable the clock for the PORT C*/
	SIM->SCGC5 |= SIM_SCGC5_PORTC_MASK;     // prendo port C

	/* Configure the SW2 DMA request source on falling edge. */

	PORTC->PCR[6]=0x00;
	PORTC->PCR[6]|=PORT_PCR_MUX(PORT_mGPIO); 		       //Set MUX to GPIO;
	PORTC->PCR[6]|=PORT_PCR_PE(1);          		       //Pull UP/Down  Enable;
	PORTC->PCR[6]|=PORT_PCR_PS(1);          		       //Pull UP
	PORTC->PCR[6]|=PORT_PCR_IRQC(PORT_eDMAFalling);


	/* Enable the clock for the eDMA and the DMAMUX. */
	SIM->SCGC7 |= SIM_SCGC7_DMA_MASK;
	SIM->SCGC6 |= SIM_SCGC6_DMAMUX_MASK;

	/* Enable the eDMA channel 0 and set the PORTC as the DMA request source. */
	//DMAMUX->CHCFG[0] |= DMAMUX_CHCFG_ENBL_MASK | DMAMUX_CHCFG_SOURCE(51);

	//PIT as Trigger , Always Enabled as DMA Request
	//DMAMUX->CHCFG[0] |= DMAMUX_CHCFG_ENBL_MASK | DMAMUX_CHCFG_TRIG_MASK | DMAMUX_CHCFG_SOURCE(63);

	DMAMUX->CHCFG[0] |= DMAMUX_CHCFG_ENBL_MASK | DMAMUX_CHCFG_SOURCE(20);   // FTM0 CH0

	/* Enable the interrupts for the channel 0. */
	/* Clear all the pending events. */
	NVIC_ClearPendingIRQ(DMA0_IRQn);
	/* Enable the DMA interrupts. */
	NVIC_EnableIRQ(DMA0_IRQn);

	//###################
	//####### DMA #######
	//###################

	/// ============= INIT TCD0 ===================//
	/* Set memory address for source and destination. */

	DMA0->TCD[0].SADDR= (uint32_t)(sourceBuffer);
	DMA0->TCD[0].DADDR =(uint32_t)(&(FTM0->CONTROLS[0].CnV));

		/* Set an offset for source and destination address. */
	DMA0->TCD[0].SOFF =0x02; // Source address offset of 2 bytes per transaction.
	DMA0->TCD[0].DOFF =0x00; // Destination address offset of 1 byte per transaction.

	/* Set source and destination data transfer size is 1 byte. */
	DMA0->TCD[0].ATTR = DMA_ATTR_SSIZE(1) | DMA_ATTR_DSIZE(1);

	/*Number of bytes to be transfered in each service request of the channel.*/
	DMA0->TCD[0].NBYTES_MLNO= 0x02;

	/* Current major iteration count (5 iteration of 1 byte each one). */
	DMA0->TCD[0].CITER_ELINKNO = DMA_CITER_ELINKNO_CITER(5);
	DMA0->TCD[0].BITER_ELINKNO = DMA_BITER_ELINKNO_BITER(5);

	DMA0->TCD[0].SLAST = -10;
	DMA0->TCD[0].DLAST_SGA = 0x00;

	/* Setup control and status register. */

	DMA0->TCD[0].CSR = DMA_CSR_INTMAJOR_MASK;	//Enable Major Interrupt.

	/* Enable request signal for channel 0. */
	DMA0->ERQ |= DMA_ERQ_ERQ0_MASK;

	hw_EnableInterrupts();

	//#######################
	//########  PIT  ########
	//#######################

	/* Never leave main */
	return ;
}
__ISR__ DMA0_IRQHandler(void)
{
	/* Clear the interrupt flag. */
	//gpioWrite(PORTNUM2PIN(PB, 2), HIGH);
	DMA0->INT = DMA_INT_INT0(1);
	//PITStop(pitState);   // WHAT IF SE DISPARA EL PIT OTRA VEZ ANTES DE LLEGAR ACA???
	//gpioWrite(PORTNUM2PIN(PB, 2), LOW);
	/* Change the source buffer contents. */

	if(table1 == BUSY)
		table1 = FREE;
	else
		table1 = BUSY;

	if(table2 == BUSY)
		table2 = FREE;
	else
		table2 = BUSY;

	//FTM_StopClock (FTM0);
	//FTM0->CNT = 0X00;
}
__ISR__ DMA1_IRQHandler(void)
{
	/* Clear the interrupt flag. */
	//gpioWrite(PORTNUM2PIN(PB, 2), HIGH);
	//DMA0->CINT |= 0;
	DMA0->INT = DMA_INT_INT1(1);
	//PITStop(pitState);   // WHAT IF SE DISPARA EL PIT OTRA VEZ ANTES DE LLEGAR ACA???
	//gpioWrite(PORTNUM2PIN(PB, 2), LOW);
	/* Change the source buffer contents. */

	FTM_StopClock (FTM0);
	FTM0->CNT = 0X00;
}
/* The red LED is toggled when an error occurs. */
__ISR__ DMA_Error_IRQHandler(void)
{
	/* Clear the error interrupt flag.*/
	DMA0->CERR |= 0;

	/* Turn the red LED on. */
	PTB->PTOR |= (1 << PIN_RED_LED);
}
/*******************************************************************************
 *******************************************************************************
                        LOCAL FUNCTION DEFINITIONS
 *******************************************************************************
 ******************************************************************************/


/*******************************************************************************
 ******************************************************************************/
