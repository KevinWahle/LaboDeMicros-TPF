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

#define SD_R1_LENGTH	SD_RESPONSE_SHORT

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

static uint32_t SDSendCmd(uint8_t cmd, uint32_t argument, uint8_t rspType, uint32_t* response);

static bool isSDCardInserted();

/*******************************************************************************
 * ROM CONST VARIABLES WITH FILE LEVEL SCOPE
 ******************************************************************************/

// +ej: static const int temperaturas_medias[4] = {23, 26, 24, 29};+


/*******************************************************************************
 * STATIC VARIABLES AND CONST VARIABLES WITH FILE LEVEL SCOPE
 ******************************************************************************/

static DSTATUS SDState = STA_NOINIT | STA_NODISK;	// Set NOINIT on system reset

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

	if(isSDCardInserted()) {	// Card inserted

		SDHC->SYSCTL = SDHC_SYSCTL_SDCLKFS(0x80) | SDHC_SYSCTL_SDCLKEN_MASK;	// approx 400kHz Clk
		while (SDHC->PRSSTAT & (SDHC_PRSSTAT_CIHB_MASK | SDHC_PRSSTAT_CDIHB_MASK));	// Wait CMD and DAT

		// SD disk initialization
		SDHC->SYSCTL |= SDHC_SYSCTL_INITA_MASK;	// Send INIT signal to card

		uint32_t res, err;
		err = SDSendCmd(0, 0x0, SD_RESPONSE_NONE, NULL);	// CMD0: GO_IDLE_STATE
//		SDSendCmd(0, NULL, SD_RESPONSE_SHORT, &res);
//		printf("Error CMD0: %lX\n", err);
		if (err) return SDState;

		SDSendCmd(8, 0x1AAU, SD_RESPONSE_SHORT, &res);		// CMD8: SEND_IF_COND (check voltage)
		printf("Error CMD8: %lX\n", err);
		printf("Respuesta a CMD8: %lX\n", res);

		if (res != 0x1AAU || err) {		// Voltage not supported
			return SDState;
		}

//		SDSendCmd(2, 0x0, SD_RESPONSE_SHORT, &res);
//		printf("Error CMD2: %lX\n", err);
//		printf("Respuesta a CMD2: %lX\n", res);
//		SDSendCmd(13, 0x0, SD_R1_LENGTH, &res);
//		printf("Error CMD13: %lX\n", err);
//		printf("Respuesta a CMD13: %lX\n", res);
//		SDSendCmd(5, 0x0, SD_RESPONSE_SHORT, &res);
//		printf("Error CMD5: %lX\n", err);
//		printf("Respuesta a CMD5: %lX\n", res);










//
//		voltage_validation(voltage_range_arguement)
//		{
//		label the card as UNKNOWN;
//		send_command(IO_SEND_OP_COND, 0x0, <other parameters are omitted>); // CMD5, check SDIO
//		operation voltage, command argument is zero
//		if (RESP_TIMEOUT != wait_for_response(IO_SEND_OP_COND)) { // SDIO command is accepted
//		 if (0 < number of IO functions) {
//		 label the card as SDIO;
//		 IORDY = 0;
//		 while (!(IORDY in IO OCR response)) { // set voltage range for each IO function
//		 send_command(IO_SEND_OP_COND, <voltage range>, <other parameter>);
//		 wait_for_response(IO_SEND_OP_COND);
//		 } // end of while ...
//		 } // end of if (0 < ...
//		 if (memory part is present inside SDIO card) Label the card as SDCombo; // this is an
//		SD-Combo card
//		} // end of if (RESP_TIMEOUT ...
//		if (the card is labelled as SDIO card) return; // card type is identified and voltage range
//		is
//		set, so exit the function;
//		send_command(APP_CMD, 0x0, <other parameters are omitted>); // CMD55, Application specific
//		CMD
//		prefix
//		if (no error calling wait_for_response(APP_CMD, <...>) { // CMD55 is accepted
//		 send_command(SD_APP_OP_COND, <voltage range>, <...>); // ACMD41, to set voltage range
//		for memory part or SD card
//		 wait_for_response(SD_APP_OP_COND); // voltage range is set
//		 if (card type is UNKNOWN) label the card as SD;
//		Initialization/application of SDHC
//		K64 Sub-Family Reference Manual, Rev. 4, Oct 2019
//		1732 NXP Semiconductors
//		 return; //
//		} // end of if (no error ...
//		else if (errors other than time-out occur) { // command/response pair is corrupted
//		 deal with it by program specific manner;
//		} // of else if (response time-out
//		else { // CMD55 is refuse, it must be MMC card or CE-ATA card
//		 if (card is already labelled as SDCombo) { // change label
//		 re-label the card as SDIO;
//		 ignore the error or report it;
//		 return; // card is identified as SDIO card
//		 } // of if (card is ...
//		 send_command(SEND_OP_COND, <voltage range>, <...>);
//		 if (RESP_TIMEOUT == wait_for_response(SEND_OP_COND)) { // CMD1 is not accepted, either
//		 label the card as UNKNOWN;
//		 return;
//		 } // of if (RESP_TIMEOUT ...
//		 if (check for CE-ATA signature succeeded) { // the card is CE-ATA
//		 store CE-ATA specific info from the signature;
//		 label the card as CE-ATA;
//		 } // of if (check for CE-ATA ...
//		 else label the card as MMC;
//		} // of else
//		}


		SDState = 0U;	//// ?????????
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

	// TODO

	return SD_disk_status(pdrv);
}


/*******************************************************************************
 *******************************************************************************
                        LOCAL FUNCTION DEFINITIONS
 *******************************************************************************
 ******************************************************************************/

// Envia comando, devuelve

static uint32_t SDSendCmd(uint8_t cmd, uint32_t argument, uint8_t rspType, uint32_t* response) {

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

	if (!(SDHC->PRSSTAT & (SDHC_PRSSTAT_CIHB_MASK | SDHC_PRSSTAT_CDIHB_MASK | SDHC_PRSSTAT_WTA_MASK))) {	// Line ready

		SDHC->BLKATTR = SDHC_BLKATTR_BLKCNT(0U) | SDHC_BLKATTR_BLKSIZE(512U);

		SDHC->CMDARG = argument;

		SDHC->XFERTYP = SDHC_XFERTYP_CMDINX(cmd) | //SDHC_XFERTYP_DPSEL(data!=NULL) |
				SDHC_XFERTYP_RSPTYP(rspType) | // TODO: Ver bien para otros comandos
				SDHC_XFERTYP_BCEN_MASK |
				SDHC_XFERTYP_CCCEN(rspType!=SD_RESPONSE_NONE) | SDHC_XFERTYP_CICEN(rspType!=SD_RESPONSE_NONE);
//				SDHC_XFERTYP_CCCEN_MASK | SDHC_XFERTYP_CICEN_MASK;

		while(!(SDHC->IRQSTAT & SDHC_IRQSTAT_CC_MASK));	// Wait command complete

//		SDHC->IRQSTAT |= SDHC_IRQSTAT_CC_MASK;
		err = SDHC->IRQSTAT;
		SDHC->IRQSTAT |= err;
		err &= 0xFFFFFFFE;

		if (rspType && response != NULL) {
			response[0] = SDHC->CMDRSP[0];
			if (rspType == SD_RESPONSE_LONG) {	// Long response
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
