/***************************************************************************//**
  @file		SD.c
  @brief	+Descripcion del archivo+
  @author	Grupo 5
  @date		25 dic. 2022
 ******************************************************************************/

/*******************************************************************************
 * INCLUDE HEADER FILES
 ******************************************************************************/

#include "SD.h"
#include "MCAL/gpio.h"
#include "MK64F12.h"

#include <stddef.h>

#include <stdio.h>

/*******************************************************************************
 * CONSTANT AND MACRO DEFINITIONS USING #DEFINE
 ******************************************************************************/

// SD pinout

#define SD_PORT			PORTE
#define SD_PORT_NUM		PE

#define DAT0_PIN		1
#define DAT1_PIN		0
#define DAT2_PIN		5
#define DAT3_PIN		4

#define CMD_PIN			3
#define CLK_PIN			2

#define SD_DETECT_PIN	6

#define SD_DETECT_GPIO PORTNUM2PIN(SD_PORT_NUM, SD_DETECT_PIN)

#define SD_MUX_ALT		4

#define SD_CARD_PRESENT_STATE	HIGH

#define SD_RESPONSE_NONE		0UL		// No response expected
#define SD_RESPONSE_LONG		1UL		// Long response (136-bit)
#define SD_RESPONSE_SHORT		2UL		// Short response (48-bit)
#define SD_RESPONSE_SHORT_BUSY	3UL		// Short response with busy signal (48-bit)

//#define SD_R1_LENGTH	SD_RESPONSE_SHORT
//#define SD_R1b_LENGTH	SD_RESPONSE_SHORT_BUSY
//#define SD_R2_LENGTH	SD_RESPONSE_LONG
//#define SD_R6_LENGTH	SD_RESPONSE_SHORT

#define OCR_VDD32_33		(1U << 20U)
#define OCR_VDD33_34		(1U << 21U)
#define OCR_BUSY_MASK		(0x80000000U)

#define SDHC_OCR		(OCR_VDD32_33 | OCR_VDD33_34)

// SD Commands

#define RCA_ARG_SHIFT		(16U)
#define RCA_ARG(rca)		((rca) << RCA_ARG_SHIFT)

#define CMD8_ARG			(0x1AAU)

#define ACMD41_HCS_SHIFT	(30U)
#define ACMD41_HCS(x)		((x) << ACMD41_HCS_SHIFT)
#define ACMD41_XPC_SHIFT	(28U)
#define ACMD41_XPC(x)		((x) << ACMD41_XPC_SHIFT)
#define ACMD41_S18R_SHIFT	(24U)
#define ACMD41_S18R(x)		((x) << ACMD41_S18R_SHIFT)
#define ACMD41_OCR_SHIFT	(0U)
#define ACMD41_OCR(x)		((x) << ACMD41_OCR_SHIFT)
#define ACMD41_ARG(hcs, xpc, s18r, ocr)		(ACMD41_HCS(hcs) | ACMD41_XPC(xpc) | ACMD41_S18R(s18r) | ACMD41_OCR(ocr))


/*******************************************************************************
 * ENUMERATIONS AND STRUCTURES AND TYPEDEFS
 ******************************************************************************/

typedef enum {	SDNoResponse,
				SDResponseR1,
				SDResponseR1b,
				SDResponseR2,
				SDResponseR3,
				SDResponseR4,
				SDResponseR5,
				SDResponseR6,
				SDResponseR7,
				SDResponseData
				} SD_RESPONSE_TYPE;


/*******************************************************************************
 * VARIABLES WITH GLOBAL SCOPE
 ******************************************************************************/

// +ej: unsigned int anio_actual;+


/*******************************************************************************
 * FUNCTION PROTOTYPES FOR PRIVATE FUNCTIONS WITH FILE LEVEL SCOPE
 ******************************************************************************/

static uint32_t SDSendCmd(uint8_t cmd, uint32_t argument, SD_RESPONSE_TYPE rspIndex, uint32_t* response);

static bool isSDCardInserted();

/*******************************************************************************
 * ROM CONST VARIABLES WITH FILE LEVEL SCOPE
 ******************************************************************************/

// +ej: static const int temperaturas_medias[4] = {23, 26, 24, 29};+


/*******************************************************************************
 * STATIC VARIABLES AND CONST VARIABLES WITH FILE LEVEL SCOPE
 ******************************************************************************/

static DSTATUS SDState = STA_NOINIT | STA_NODISK;	// Set NOINIT on system reset

static uint16_t rca = 0x0U;		// RCA Address of SD Card

/*******************************************************************************
 *******************************************************************************
                        GLOBAL FUNCTION DEFINITIONS
 *******************************************************************************
 ******************************************************************************/


DSTATUS SD_disk_status (BYTE pdrv) {
	return SDState;
}


DSTATUS SD_disk_initialize (BYTE pdrv) {

	static bool yaInit = false;

	SDState = STA_NOINIT;

	// TODO: Ver de ponerlo en otra funcion
	if (!yaInit) {

		// SD Pin Setup

		SD_PORT->PCR[DAT0_PIN] = PORT_PCR_MUX(SD_MUX_ALT) | PORT_PCR_PE_MASK | PORT_PCR_PS_MASK;
		SD_PORT->PCR[DAT1_PIN] = PORT_PCR_MUX(SD_MUX_ALT) | PORT_PCR_PE_MASK | PORT_PCR_PS_MASK;
		SD_PORT->PCR[DAT2_PIN] = PORT_PCR_MUX(SD_MUX_ALT) | PORT_PCR_PE_MASK | PORT_PCR_PS_MASK;
		SD_PORT->PCR[DAT3_PIN] = PORT_PCR_MUX(SD_MUX_ALT) | PORT_PCR_PE_MASK | PORT_PCR_PS_MASK;

		SD_PORT->PCR[CMD_PIN] = PORT_PCR_MUX(SD_MUX_ALT) | PORT_PCR_PE_MASK | PORT_PCR_PS_MASK;
		SD_PORT->PCR[CLK_PIN] = PORT_PCR_MUX(SD_MUX_ALT) | PORT_PCR_PE_MASK | PORT_PCR_PS_MASK;
//		SD_PORT->PCR[CLK_PIN] = PORT_PCR_MUX(SD_MUX_ALT);

		gpioMode(SD_DETECT_GPIO, INPUT_PULLDOWN);	// Pull-down for card detect

		// Clock gating
		SIM->SOPT2 &= ~SIM_SOPT2_SDHCSRC_MASK;
		SIM->SCGC3 |= SIM_SCGC3_SDHC_MASK;
	}

		//TODO: SD Host Reset



		/*
		 *
		 * software_reset()
			{
			set_bit(SYSCTRL, RSTA); // software reset the Host
			set DTOCV and SDCLKFS bit fields to get the SD_CLK of frequency around 400kHz
			configure IO pad to set the power voltage of external card to around 3.0V
			poll bits CIHB and CDIHB bits of PRSSTAT to wait both bits are cleared
			set_bit(SYSCTRL, INTIA); // send 80 clock ticks for card to power up
			send_command(CMD_GO_IDLE_STATE, <other parameters>); // reset the card with CMD0
			or send_command(CMD_IO_RW_DIRECT, <other parameters>);
			}
		 */


	SDHC->SYSCTL |= SDHC_SYSCTL_RSTA_MASK;	// Software Reset SDHC

	SDHC->PROCTL = SDHC_PROCTL_EMODE(2U);		// Little Endian and 1-bit mode
//	SDHC->PROCTL = SDHC_PROCTL_EMODE(2) | SDHC_PROCTL_D3CD_MASK;		// Little Endian, DAT3 CD and 1-bit mode
//	SDHC->IRQSTATEN |= SDHC_IRQSTATEN_CRMSEN_MASK | SDHC_IRQSTATEN_CINSEN_MASK;	// Enable CD status flags
	SDHC->IRQSIGEN = 0U;			// Disable all IRQs
	SDHC->VENDOR = 0U;				// Disable external DMA

	if(isSDCardInserted()) {	// Card inserted

		SDHC->SYSCTL = SDHC_SYSCTL_SDCLKFS(0x80) | SDHC_SYSCTL_SDCLKEN_MASK;	// approx 400kHz Clk
		while (SDHC->PRSSTAT & (SDHC_PRSSTAT_CIHB_MASK | SDHC_PRSSTAT_CDIHB_MASK));	// Wait CMD and DAT

		// SD disk initialization
		SDHC->SYSCTL |= SDHC_SYSCTL_INITA_MASK;	// Send INIT signal to card

		uint32_t res[4], err;
		err = SDSendCmd(0, 0x0, SDNoResponse, NULL);	// CMD0: GO_IDLE_STATE
//		SDSendCmd(0, NULL, SD_RESPONSE_SHORT, &res);
//		printf("Error CMD0: %lX\n", err);
		if (err) return SDState;

		err = SDSendCmd(8, CMD8_ARG, SDResponseR7, res);		// CMD8: SEND_IF_COND (check voltage)
		printf("Error CMD8: %08lX\n", err);
		printf("Respuesta a CMD8: %08lX\n", res[0]);

		if (res[0] != CMD8_ARG || err) {		// Voltage not supported (should echo)
			return SDState;
		}

		rca = 0x0U;				// RCA reset
		uint32_t tries = 1000;	// timeout for ACMD41

		do {
			err = SDSendCmd(55, RCA_ARG(rca), SDResponseR1, res);		// CMD55: APP_CMD
			printf("Error CMD55: %08lX\n", err);
			printf("Respuesta a CMD55: %08lX\n", res[0]);
			if (err) return SDState;

			err = SDSendCmd(41, ACMD41_ARG(1U, 1U, 0U, SDHC_OCR), SDResponseR3, res);		// CMD41: SD_SEND_OP_COND
			printf("Error CMD41: %08lX\n", err);
			printf("Respuesta a CMD41: %08lX\n", res[0]);

			if (!(res[0] & SDHC_OCR) || err) {		// Initialization failed
				return SDState;
			}

			uint32_t cont = 0xFFFFF;		// TODO: Implementar con un delay mejor o sacar (10ms)
			while (cont--);
		} while (!(res[0] & OCR_BUSY_MASK) && --tries);		// Repeat while busy

		err = SDSendCmd(2, 0x0U, SDResponseR2, res);
		printf("Error CMD2: %08lX\n", err);
		printf("Respuesta a CMD2: %06lX %08lX %08lX %08lX\n", res[3], res[2], res[1], res[0]);
		if (err) return SDState;

		err = SDSendCmd(3, 0x0U, SDResponseR6, res);
		printf("Error CMD3: %08lX\n", err);
		printf("Respuesta a CMD3: %08lX\n", res[0]);
		if (err) return SDState;
		rca = res[0] >> 16U;

		// Card identified and in stand-by state

		// Get card CSD
		err = SDSendCmd(9, RCA_ARG(rca), SDResponseR2, res);
		printf("Error CMD9: %08lX\n", err);
		printf("Respuesta a CMD9: %06lX %08lX %08lX %08lX\n", res[3], res[2], res[1], res[0]);
		if (err) return SDState;

		// TODO: Chequeo de CSD...

		// Aca se puede hacer CMD4: SET_DSR


		// Card selection (go to transfer state)
		err = SDSendCmd(7, RCA_ARG(rca), SDResponseR1b, res);
		printf("Error CMD7: %08lX\n", err);
		printf("Respuesta a CMD7: %08lX\n", res[0]);
		if (err) return SDState;

		// TODO: CMD16 para setear cosas del bloque
//		err = SDSendCmd(16, 512U, SDResponseR1, res);
//		printf("Error CMD16: %08lX\n", err);
//		printf("Respuesta a CMD16: %08lX\n", res[0]);
//		if (err) return SDState;

		SDState = 0U;	// Clear flags
	}
	else {		// No card
		SDState |= STA_NODISK;
	}


	return SDState;
}

DRESULT SD_disk_read (
  BYTE pdrv,     /* [IN] Physical drive number */
  BYTE* buff,    /* [OUT] Pointer to the read data buffer */
  LBA_t sector,  /* [IN] Start sector number */
  UINT count     /* [IN] Number of sectors to read */
) {

	if (SDState & STA_NOINIT) return RES_NOTRDY;

	// TODO:	- Chequear que la SD este ready for data (CMD13)
	// 			- Ssetear block size??

	uint32_t res, err;

	if (count == 1) {
		// Single sector read

		err = SDSendCmd(17, sector, SDResponseData, &res);
		printf("Error CMD17: %08lX\n", err);
		printf("Respuesta a CMD17: %08lX\n", res);

//		if (err) return RES_ERROR;		// TODO: Mejorar

	}
	else {
		// Multiple sector read


	}

	UINT index = 0U;
	uint8_t water = 16U;//SDHC->WML & SDHC_WML_RDWML_MASK;	//TODO: Ver de cambiar y poner antes

	while (!(SDHC->IRQSTAT & SDHC_IRQSTAT_TC_MASK)) {
//		if (SDHC->IRQSTAT & SDHC_IRQSTAT_BRR_MASK) {
		if (SDHC->PRSSTAT & (SDHC_PRSSTAT_BREN_MASK | SDHC_PRSSTAT_RTA_MASK)) {
			for (int i = 0; i < water; i++) {
				((uint32_t*)buff)[index] = SDHC->DATPORT;		// 32-bit read
				printf("Leo data: %08lX\n", ((uint32_t*)buff)[index]);
				index++;
			}
		}
	}
	SDHC->IRQSTAT |= SDHC_IRQSTAT_TC_MASK;		// Clear flag


//		while (!(SDHC->PRSSTAT & SDHC_PRSSTAT_BREN_MASK));
//		while (SDHC->PRSSTAT & SDHC_PRSSTAT_BREN_MASK) {
//			((uint32_t*)buff)[index] = SDHC->DATPORT;		// 32-bit read
//			printf("Leo data: %08lX\n", ((uint32_t*)buff)[index]);
//			index++;
//		}


			// TODO: Borrar flags
		printf("Error READ: %08lX\n", SDHC->IRQSTAT);




	// Check status
	err = SDSendCmd(13, RCA_ARG(rca), SDResponseR1, &res);
	printf("Error CMD13: %08lX\n", err);
	printf("Respuesta a CMD13: %08lX\n", res);
	if (err) return RES_ERROR;





	return RES_OK;
}


/*******************************************************************************
 *******************************************************************************
                        LOCAL FUNCTION DEFINITIONS
 *******************************************************************************
 ******************************************************************************/

// Envia comando, devuelve

static uint32_t SDSendCmd(uint8_t cmd, uint32_t argument, SD_RESPONSE_TYPE rspIndex, uint32_t* response) {

//	send_command(cmd_index, cmd_arg, other requirements)
//	{
//	WORD wCmd; // 32-bit integer to make up the data to write into Transfer Type register, it is
//	recommended to implement in a bit-field manner
//	wCmd = (<cmd_index> & 0x3f) << 24; // set the first 8 bits as '00'+<cmd_index>
//	set CMDTYP, DPSEL, CICEN, CCCEN, RSTTYP, DTDSEL accorind to the command index;
//	if (internal DMA is used) wCmd |= 0x1;
//	if (multi-block transfer) {
//	set MSBSEL bit;
//	if (finite block number) {
//	set BCEN bit;
//	if (auto12 command is to use) set AC12EN bit;
//	}
//	}
//	write_reg(CMDARG, <cmd_arg>); // configure the command argument
//	write_reg(XFERTYP, wCmd); // set Transfer Type register as wCmd value to issue the command
//	}
//	wait_for_response(cmd_index)
//	{
//	while (CC bit in IRQ Status register is not set); // wait until Command Complete bit is set
//	read IRQ Status register and check if any error bits about Command are set
//	if (any error bits are set) report error;
//	write 1 to clear CC bit and all Command Error bits;

	uint32_t err = 0xFFFFFFFF;
	uint32_t xferType = SDHC_XFERTYP_CMDINX(cmd);// | //SDHC_XFERTYP_DPSEL(data!=NULL) |
//						SDHC_XFERTYP_RSPTYP(rspType) |
//						SDHC_XFERTYP_BCEN_MASK;
//						SDHC_XFERTYP_DPSEL(rspType == SD_DATA_RESPONSE) |
//						SDHC_XFERTYP_CCCEN(rspType!=SD_RESPONSE_NONE) | SDHC_XFERTYP_CICEN(rspType!=SD_RESPONSE_NONE);
			//				SDHC_XFERTYP_CCCEN_MASK | SDHC_XFERTYP_CICEN_MASK;

	uint8_t rspLength;

	switch (rspIndex) {

		case SDNoResponse:
			xferType |= SDHC_XFERTYP_RSPTYP(SD_RESPONSE_NONE);
			rspLength = SD_RESPONSE_NONE;
			break;

		case SDResponseR1:
		case SDResponseR5:
		case SDResponseR6:
		case SDResponseR7:
			xferType |= SDHC_XFERTYP_RSPTYP(SD_RESPONSE_SHORT) | SDHC_XFERTYP_CCCEN_MASK | SDHC_XFERTYP_CICEN_MASK;
			rspLength = SD_RESPONSE_SHORT;
			break;

		case SDResponseR1b:
			xferType |= SDHC_XFERTYP_RSPTYP(SD_RESPONSE_SHORT_BUSY) | SDHC_XFERTYP_CCCEN_MASK | SDHC_XFERTYP_CICEN_MASK;
			rspLength = SD_RESPONSE_SHORT;
			break;

		case SDResponseR2:
			xferType |= SDHC_XFERTYP_RSPTYP(SD_RESPONSE_LONG) | SDHC_XFERTYP_CCCEN_MASK;
			rspLength = SD_RESPONSE_LONG;
			break;

		case SDResponseR3:
		case SDResponseR4:
			xferType |= SDHC_XFERTYP_RSPTYP(SD_RESPONSE_SHORT);
			rspLength = SD_RESPONSE_SHORT;
			break;

		case SDResponseData:
			xferType |= SDHC_XFERTYP_RSPTYP(SD_RESPONSE_SHORT) | SDHC_XFERTYP_CCCEN_MASK | SDHC_XFERTYP_CICEN_MASK
						| SDHC_XFERTYP_DPSEL_MASK | SDHC_XFERTYP_DTDSEL_MASK;
			rspLength = SD_RESPONSE_SHORT;
			break;

		default:
			return err;

	}


	if (!(SDHC->PRSSTAT & (SDHC_PRSSTAT_CIHB_MASK | SDHC_PRSSTAT_CDIHB_MASK | SDHC_PRSSTAT_WTA_MASK))) {	// Line ready

		SDHC->BLKATTR = SDHC_BLKATTR_BLKCNT(1U) | SDHC_BLKATTR_BLKSIZE(512U);	// TODO: Ver de hacer afuera

		SDHC->CMDARG = argument;

		SDHC->XFERTYP = xferType;

		while(!(SDHC->IRQSTAT & SDHC_IRQSTAT_CC_MASK));	// Wait command complete

//		SDHC->IRQSTAT |= SDHC_IRQSTAT_CC_MASK;
		err = SDHC->IRQSTAT;
		SDHC->IRQSTAT |= err;
		err &= 0xFFFFFFFE;

		if (rspLength != SD_RESPONSE_NONE && response != NULL) {
			response[0] = SDHC->CMDRSP[0];
			if (rspLength == SD_RESPONSE_LONG) {	// Long response
				response[1] = SDHC->CMDRSP[1];
				response[2] = SDHC->CMDRSP[2];
				response[3] = SDHC->CMDRSP[3];
			}
		}


	}

	return err;

}


static bool isSDCardInserted() {
	return gpioRead(SD_DETECT_GPIO) == SD_CARD_PRESENT_STATE;
}
