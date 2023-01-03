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

static void SDSendCmd(uint8_t cmd, uint32_t *argument, uint8_t rspType, uint32_t* response);

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
		SD_PORT->PCR[CLK_PIN] = PORT_PCR_MUX(SD_MUX_ALT);

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

		SDSendCmd(0, NULL, SD_RESPONSE_NONE, NULL);
		uint32_t res;
		SDSendCmd(8, NULL, SD_RESPONSE_SHORT, &res);
		printf("Respuesta a CMD8: %lX", res);

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

static void SDSendCmd(uint8_t cmd, uint32_t *argument, uint8_t rspType, uint32_t* response) {

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

	if (!(SDHC->PRSSTAT & (SDHC_PRSSTAT_CIHB_MASK | SDHC_PRSSTAT_CDIHB_MASK | SDHC_PRSSTAT_WTA_MASK))) {	// Line ready

		SDHC->BLKATTR = SDHC_BLKATTR_BLKCNT(0U) | SDHC_BLKATTR_BLKSIZE(512U);

		if (argument != NULL) {
			SDHC->CMDARG = *argument;
		}

		SDHC->XFERTYP = SDHC_XFERTYP_CMDINX(cmd) | //SDHC_XFERTYP_DPSEL(data!=NULL) |
				SDHC_XFERTYP_RSPTYP(rspType) | // TODO: Ver bien para otros comandos
				SDHC_XFERTYP_BCEN_MASK;
		// TODO: Obs: No se activa CRC ni index check

		while(!(SDHC->IRQSTAT & SDHC_IRQSTAT_CC_MASK));	// Wait command complete

		SDHC->IRQSTAT |= SDHC_IRQSTAT_CC_MASK;

		if (rspType && response != NULL) {
			response[0] = SDHC->CMDRSP[0];
			if (rspType == SD_RESPONSE_LONG) {	// Long response
				response[1] = SDHC->CMDRSP[1];
				response[2] = SDHC->CMDRSP[2];
				response[3] = SDHC->CMDRSP[3];
			}
		}

		//TODO: Chequear error

	}

}


static bool isSDCardInserted() {
	return gpioRead(SD_DETECT_GPIO) == SD_CARD_PRESENT_STATE;
}
