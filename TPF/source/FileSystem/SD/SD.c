/***************************************************************************//**
  @file		SD.c
  @brief	+Descripcion del archivo+
  @author	Grupo 5
  @date		25 dic. 2022
 ******************************************************************************/

//#define SD_DEBUG

/*******************************************************************************
 * INCLUDE HEADER FILES
 ******************************************************************************/

#include "SD.h"
#include "MCAL/gpio.h"
#include "MK64F12.h"

#include <stddef.h>

#ifdef SD_DEBUG
#include <stdio.h>
#endif
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

#define SDHC_DTW_4BIT		(1U)

#define SDHC_IRQSTAT_ERRORS_MASK	(0x117F0000U)

#define SD_BLKSIZE			(512U)

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

#define ACMD6_4BIT_ARG		(2U)

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
				SDResponseDataSingle,
				SDResponseDataMulti
				} SD_RESPONSE_TYPE;

/*******************************************************************************
 * FUNCTION PROTOTYPES FOR PRIVATE FUNCTIONS WITH FILE LEVEL SCOPE
 ******************************************************************************/

static uint32_t SDSendCmd(uint8_t cmd, uint32_t argument, SD_RESPONSE_TYPE rspIndex, uint32_t* response);

static void cardInsertedCb();

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

bool SDInit() {

	static bool yaInit = false;		// Initialization flag

	if (!yaInit) {
			yaInit = true;
			// SD Pin Setup

			SD_PORT->PCR[DAT0_PIN] = PORT_PCR_MUX(SD_MUX_ALT) | PORT_PCR_PE_MASK | PORT_PCR_PS_MASK;
			SD_PORT->PCR[DAT1_PIN] = PORT_PCR_MUX(SD_MUX_ALT) | PORT_PCR_PE_MASK | PORT_PCR_PS_MASK;
			SD_PORT->PCR[DAT2_PIN] = PORT_PCR_MUX(SD_MUX_ALT) | PORT_PCR_PE_MASK | PORT_PCR_PS_MASK;
			SD_PORT->PCR[DAT3_PIN] = PORT_PCR_MUX(SD_MUX_ALT) | PORT_PCR_PE_MASK | PORT_PCR_PS_MASK;

			SD_PORT->PCR[CMD_PIN] = PORT_PCR_MUX(SD_MUX_ALT) | PORT_PCR_PE_MASK | PORT_PCR_PS_MASK;
			SD_PORT->PCR[CLK_PIN] = PORT_PCR_MUX(SD_MUX_ALT) | PORT_PCR_PE_MASK | PORT_PCR_PS_MASK;
	//		SD_PORT->PCR[CLK_PIN] = PORT_PCR_MUX(SD_MUX_ALT);

			gpioMode(SD_DETECT_GPIO, INPUT_PULLDOWN);	// Pull-down for card detect
			gpioIRQ(SD_DETECT_GPIO, GPIO_IRQ_MODE_BOTH_EDGES, cardInsertedCb);

			cardInsertedCb();  	// Call function to initialize SDState

			// Clock gating
			SIM->SOPT2 &= ~SIM_SOPT2_SDHCSRC_MASK;
			SIM->SCGC3 |= SIM_SCGC3_SDHC_MASK;
	}

	return false;
}

DSTATUS SD_disk_status () {
	return SDState;
}


DSTATUS SD_disk_initialize () {

	SDState |= STA_NOINIT;

	SDInit();	// Init HW

	SDHC->SYSCTL |= SDHC_SYSCTL_RSTA_MASK;	// Software Reset SDHC

	SDHC->PROCTL = SDHC_PROCTL_EMODE(2U);		// Little Endian and 1-bit mode
	SDHC->IRQSIGEN = 0U;			// Disable all IRQs
	SDHC->VENDOR = 0U;				// Disable external DMA
	SDHC->BLKATTR = SDHC_BLKATTR_BLKSIZE(SD_BLKSIZE);

	if(isSDCardInserted()) {	// Card inserted

		SDHC->SYSCTL = SDHC_SYSCTL_SDCLKFS(0x80) | SDHC_SYSCTL_SDCLKEN_MASK;	// approx 400kHz Clk
		while (SDHC->PRSSTAT & (SDHC_PRSSTAT_CIHB_MASK | SDHC_PRSSTAT_CDIHB_MASK));	// Wait CMD and DAT

		// SD disk initialization
		SDHC->SYSCTL |= SDHC_SYSCTL_INITA_MASK;	// Send INIT signal to card

		uint32_t res[4], err;
		err = SDSendCmd(0, 0x0, SDNoResponse, NULL);	// CMD0: GO_IDLE_STATE
#ifdef SD_DEBUG
		printf("Error CMD0: %lX\n", err);
#endif
		if (err) return SDState;

		err = SDSendCmd(8, CMD8_ARG, SDResponseR7, res);		// CMD8: SEND_IF_COND (check voltage)
#ifdef SD_DEBUG
		printf("Error CMD8: %08lX\n", err);
		printf("Respuesta a CMD8: %08lX\n", res[0]);
#endif
		if (res[0] != CMD8_ARG || err) {		// Voltage not supported (should echo)
			return SDState;
		}

		rca = 0x0U;				// RCA reset
		uint32_t tries = 1000;	// timeout for ACMD41

		do {
			err = SDSendCmd(55, RCA_ARG(rca), SDResponseR1, res);		// CMD55: APP_CMD
#ifdef SD_DEBUG
			printf("Error CMD55: %08lX\n", err);
			printf("Respuesta a CMD55: %08lX\n", res[0]);
#endif
			if (err) return SDState;

			err = SDSendCmd(41, ACMD41_ARG(1U, 1U, 0U, SDHC_OCR), SDResponseR3, res);		// CMD41: SD_SEND_OP_COND
#ifdef SD_DEBUG
			printf("Error CMD41: %08lX\n", err);
			printf("Respuesta a CMD41: %08lX\n", res[0]);
#endif
			if (!(res[0] & SDHC_OCR) || err) {		// Initialization failed
				return SDState;
			}

			uint32_t cont = 0xFFFFF;
			while (cont--);
		} while (!(res[0] & OCR_BUSY_MASK) && --tries);		// Repeat while busy

		err = SDSendCmd(2, 0x0U, SDResponseR2, res);
#ifdef SD_DEBUG
		printf("Error CMD2: %08lX\n", err);
		printf("Respuesta a CMD2: %06lX %08lX %08lX %08lX\n", res[3], res[2], res[1], res[0]);
#endif
		if (err) return SDState;

		err = SDSendCmd(3, 0x0U, SDResponseR6, res);
#ifdef SD_DEBUG
		printf("Error CMD3: %08lX\n", err);
		printf("Respuesta a CMD3: %08lX\n", res[0]);
#endif
		if (err) return SDState;
		rca = res[0] >> RCA_ARG_SHIFT;

		// Card identified and in stand-by state

		// Get card CSD
		err = SDSendCmd(9, RCA_ARG(rca), SDResponseR2, res);
#ifdef SD_DEBUG
		printf("Error CMD9: %08lX\n", err);
		printf("Respuesta a CMD9: %06lX %08lX %08lX %08lX\n", res[3], res[2], res[1], res[0]);
#endif
		if (err) return SDState;

		// Change CLK to 25MHz
		SDHC->SYSCTL &= ~SDHC_SYSCTL_SDCLKEN_MASK;		// Disable CLK
		SDHC->SYSCTL = SDHC_SYSCTL_SDCLKFS(2U) | SDHC_SYSCTL_DTOCV(0xEU) ;			// Set Frequency to 25MHz
		while (!(SDHC->PRSSTAT & SDHC_PRSSTAT_SDSTB_MASK));	// Wait stable CLK
		SDHC->SYSCTL |= SDHC_SYSCTL_SDCLKEN_MASK;		// Enable CLK


		// Card selection (go to transfer state)
		err = SDSendCmd(7, RCA_ARG(rca), SDResponseR1b, res);
#ifdef SD_DEBUG
		printf("Error CMD7: %08lX\n", err);
		printf("Respuesta a CMD7: %08lX\n", res[0]);
#endif
		if (err) return SDState;


		// Change to 4-bit mode
		err = SDSendCmd(55, RCA_ARG(rca), SDResponseR1, res);		// CMD55: APP_CMD
#ifdef SD_DEBUG
		printf("Error CMD55: %08lX\n", err);
		printf("Respuesta a CMD55: %08lX\n", res[0]);
#endif
		if (err) return SDState;
		err = SDSendCmd(6, ACMD6_4BIT_ARG, SDResponseR1, res);		// ACMD6: SET_BUS_WIDTH
		#ifdef SD_DEBUG
				printf("Error ACMD6: %08lX\n", err);
				printf("Respuesta a ACMD6: %08lX\n", res[0]);
		#endif
		if (err) return SDState;

		SDHC->PROCTL |= SDHC_PROCTL_DTW(SDHC_DTW_4BIT);		// Change SDHC to 4-bit

		// Set block size
		err = SDSendCmd(16, SD_BLKSIZE, SDResponseR1, res);
#ifdef SD_DEBUG
		printf("Error CMD16: %08lX\n", err);
		printf("Respuesta a CMD16: %08lX\n", res[0]);
#endif
		if (err) return SDState;

		SDState = 0U;	// Initialization succeed
	}
	else {		// No card
		SDState |= STA_NODISK;
	}


	return SDState;
}

DRESULT SD_disk_read (
  BYTE* buff,    /* [OUT] Pointer to the read data buffer */
  LBA_t sector,  /* [IN] Start sector number */
  UINT count     /* [IN] Number of sectors to read */
) {

	if (SDState & STA_NOINIT) return RES_NOTRDY;

	uint32_t res, err = 0xFFFFFFFF;

	UINT index = 0U;
	uint8_t rdwml = SDHC->WML & SDHC_WML_RDWML_MASK;

	if (count == 1) {
		// Single sector read

		err = SDSendCmd(17, sector, SDResponseDataSingle, &res);
#ifdef SD_DEBUG
		printf("Error CMD17: %08lX\n", err);
		printf("Respuesta a CMD17: %08lX\n", res);
#endif
		if (err) return RES_ERROR;

	}
	else {
		// Multiple sector read
		SDHC->BLKATTR = SDHC_BLKATTR_BLKCNT(count) | SDHC_BLKATTR_BLKSIZE(SD_BLKSIZE);

		err = SDSendCmd(18, sector, SDResponseDataMulti, &res);
#ifdef SD_DEBUG
		printf("Error CMD18: %08lX\n", err);
		printf("Respuesta a CMD18: %08lX\n", res);
#endif
		if (err) return RES_ERROR;
	}


	while (!(SDHC->IRQSTAT & SDHC_IRQSTAT_TC_MASK)) {
		if (SDHC->PRSSTAT & SDHC_PRSSTAT_BREN_MASK) {
			for (int i = 0; i < rdwml; i++) {
				((uint32_t*)buff)[index] = SDHC->DATPORT;		// 32-bit read
#ifdef SD_DEBUG
				printf("Leo data: %08lX\n", ((uint32_t*)buff)[index]);
#endif
				index++;
			}
		}
	}

	err = SDHC->IRQSTAT & SDHC_IRQSTAT_ERRORS_MASK;
	SDHC->IRQSTAT |= SDHC_IRQSTAT_TC_MASK;		// Clear flags

#ifdef SD_DEBUG
	printf("Error READ: %08lX\n", err);
	printf("Error CMD12: %08lX\n", SDHC->AC12ERR);
#endif

	return err ? RES_ERROR : RES_OK;
}

bool isSDCardInserted() {
	return !(SDState & STA_NODISK);
}


/*******************************************************************************
 *******************************************************************************
                        LOCAL FUNCTION DEFINITIONS
 *******************************************************************************
 ******************************************************************************/

// Envia comando, devuelve

static uint32_t SDSendCmd(uint8_t cmd, uint32_t argument, SD_RESPONSE_TYPE rspIndex, uint32_t* response) {

	uint32_t err = 0xFFFFFFFF;
	uint32_t xferType = SDHC_XFERTYP_CMDINX(cmd);

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

		case SDResponseDataSingle:
			xferType |= SDHC_XFERTYP_RSPTYP(SD_RESPONSE_SHORT) | SDHC_XFERTYP_CCCEN_MASK | SDHC_XFERTYP_CICEN_MASK |
						SDHC_XFERTYP_DPSEL_MASK | SDHC_XFERTYP_DTDSEL_MASK;
			rspLength = SD_RESPONSE_SHORT;
			break;

		case SDResponseDataMulti:
			xferType |= SDHC_XFERTYP_RSPTYP(SD_RESPONSE_SHORT) | SDHC_XFERTYP_CCCEN_MASK | SDHC_XFERTYP_CICEN_MASK |
						SDHC_XFERTYP_DPSEL_MASK | SDHC_XFERTYP_DTDSEL_MASK |
						SDHC_XFERTYP_BCEN_MASK | SDHC_XFERTYP_MSBSEL_MASK | SDHC_XFERTYP_AC12EN_MASK;
			rspLength = SD_RESPONSE_SHORT;
			break;

		default:
			return err;

	}


	if (!(SDHC->PRSSTAT & (SDHC_PRSSTAT_CIHB_MASK | SDHC_PRSSTAT_CDIHB_MASK | SDHC_PRSSTAT_WTA_MASK))) {	// Line ready

		SDHC->CMDARG = argument;

		SDHC->XFERTYP = xferType;

		while(!(SDHC->IRQSTAT & SDHC_IRQSTAT_CC_MASK));	// Wait command complete

		err = SDHC->IRQSTAT & SDHC_IRQSTAT_ERRORS_MASK;
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


static void cardInsertedCb() {

	if (gpioRead(SD_DETECT_GPIO) == SD_CARD_PRESENT_STATE) {
		SDState = STA_NOINIT;
#ifdef SD_DEBUG
		printf("Card inserted\n");
#endif
	}
	else {
		SDState = STA_NOINIT | STA_NODISK;
#ifdef SD_DEBUG
		printf("Card removed\n");
#endif
	}

}
