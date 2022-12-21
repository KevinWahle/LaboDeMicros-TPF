/***************************************************************************//**
  @file		uart.c
  @brief	Funciones para comunicación por UART
  @author	Sergio Peralta
  @date		9 sep. 2022
 ******************************************************************************/

/*******************************************************************************
 * INCLUDE HEADER FILES
 ******************************************************************************/

#include "uart.h"
#include "hardware.h"
#include "buffer/circular_buffer.h"
#include "MCAL/gpio.h"

/*******************************************************************************
 * CONSTANT AND MACRO DEFINITIONS USING #DEFINE
 ******************************************************************************/

#define ENABLE_TP

#ifdef ENABLE_TP
#define TP_PIN	PORTNUM2PIN(PB, 18)
#endif

#define CORE_CLOCK	__CORE_CLOCK__

#define BUS_CLOCK	(CORE_CLOCK >> 1)

#define DEFAULT_BAUDRATE	115200

// Useful Macros

#define MIN(x, y)	((x) < (y) ? (x) : (y) )
#define MAX(x, y)	((x) > (y) ? (x) : (y) )


/*******************************************************************************
 * FUNCTION PROTOTYPES FOR PRIVATE FUNCTIONS WITH FILE LEVEL SCOPE
 ******************************************************************************/

static void UART_Rx_Tx_IRQ(uint8_t id);

/*******************************************************************************
 * ROM CONST VARIABLES WITH FILE LEVEL SCOPE
 ******************************************************************************/

static UART_Type* const UARTPorts[] = UART_BASE_PTRS;
static PORT_Type* const portPtr[] = PORT_BASE_PTRS;

static const uint32_t UARTClockSource[] = {CORE_CLOCK, CORE_CLOCK, BUS_CLOCK, BUS_CLOCK, BUS_CLOCK, BUS_CLOCK};

static __IO uint32_t* const UARTSIMR[] = {&(SIM->SCGC4), &(SIM->SCGC4), &(SIM->SCGC4), &(SIM->SCGC4), &(SIM->SCGC1)};
static const uint32_t UARTSIMMask[] = {SIM_SCGC4_UART0_MASK, SIM_SCGC4_UART1_MASK, SIM_SCGC4_UART2_MASK, SIM_SCGC4_UART3_MASK, SIM_SCGC1_UART4_MASK};

static const IRQn_Type UART_RX_TX_Vectors[] = UART_RX_TX_IRQS;
//static const IRQn_Type UART_ERR_Vectors[] = UART_ERR_IRQS;

static const uint8_t UARTPinPort[] = {PB, PC, PD, PB, PE};

static const uint8_t UARTPinNumRX[] = {16, 3, 2, 10, 24};
static const uint8_t UARTPinNumTX[] = {17, 4, 3, 11, 25};
static const uint8_t UARTPinMuxAlt[] = {3, 3, 3, 3, 3};

/*******************************************************************************
 * STATIC VARIABLES AND CONST VARIABLES WITH FILE LEVEL SCOPE
 ******************************************************************************/

static circularBuffer TxBuffer[UART_CANT_IDS];
static circularBuffer RxBuffer[UART_CANT_IDS];


/*******************************************************************************
 *******************************************************************************
                        GLOBAL FUNCTION DEFINITIONS
 *******************************************************************************
 ******************************************************************************/


/**
 * @brief Initialize UART driver
 * @param id UART's number
 * @param config UART's configuration (baudrate, parity, etc.)
*/
void uartInit (uint8_t id, uart_cfg_t config) {

	UART_Type* uart = UARTPorts[id];

// Clock Gating

	uint32_t clkFreq = UARTClockSource[id];
	*(UARTSIMR[id]) |= UARTSIMMask[id];


// Pinout setup

	//TODO: Enable CLK for PORTx
	portPtr[UARTPinPort[id]]->PCR[UARTPinNumRX[id]] = PORT_PCR_MUX(UARTPinMuxAlt[id]);		// Borra otros flags
	portPtr[UARTPinPort[id]]->PCR[UARTPinNumTX[id]] = PORT_PCR_MUX(UARTPinMuxAlt[id]);		// Borra otros flags

// Baud rate config

	uint32_t baudrate;
	uint16_t sbr, brfa;

	baudrate = config.baudrate;

	if (!baudrate) baudrate = DEFAULT_BAUDRATE;

	sbr = clkFreq / (baudrate << 4);
	brfa = (clkFreq << 1) / baudrate - (sbr << 5);

	// IMPORTANTE: Primero el HIGH
	uart->BDH = UART_BDH_SBR(sbr >> 8);			// Borra los otros flags
	uart->BDL = UART_BDL_SBR(sbr);
	uart->C4 = UART_C4_BRFA(brfa);				// Set BRFA, Clear MA and M10

// Enable NVIC Interrupts

	NVIC_EnableIRQ(UART_RX_TX_Vectors[id]);
	//NVIC_EnableIRQ(UART_ERR_Vectors[id]);

// Control

	uart->S2 = UART_S2_MSBF(config.MSBF);		// Set MSBF, polarity and disable break IRQs
	uart->C5 = 0x0;		// Disable DMA Requests
	uart->IR = 0x0;		// Disable IR
	uart->MODEM = 0x0;		// Disable CTS, RTS

	// Parity (Si hay paridad, habilitar 9 bits de data)
	uart->C1 = UART_C1_PE(config.parity != NO_PARITY) | UART_C1_PT(config.parity == ODD_PARITY) | UART_C1_M(config.parity != NO_PARITY);	// Borra los demas flags

	// Habilita interrupcion por Parity Error y Overrun
	//uart->C3 = UART_C3_ORIE(1) | UART_C3_PEIE(1);
	// Disable Error IRQs
	uart->C3 = 0x0;

	// Habilitar Rx e interrupciones
	uart->C2 = UART_C2_RIE_MASK | UART_C2_RE_MASK;


// Initialize Buffers
	CBinit(TxBuffer+id, BUFFER_SIZE);
	CBinit(RxBuffer+id, BUFFER_SIZE);

#ifdef ENABLE_TP
	gpioMode(TP_PIN, OUTPUT);
	gpioWrite(TP_PIN, LOW);
#endif

}


/**
 * @brief Check if a new byte was received
 * @param id UART's number
 * @return A new byte has being received
*/
bool uartIsRxMsg(uint8_t id){
	return !CBisEmpty(RxBuffer+id);
}


/**
 * @brief Check how many bytes were received
 * @param id UART's number
 * @return Quantity of received bytes
*/
uint8_t uartGetRxMsgLength(uint8_t id) {
	return CBgetBufferState(RxBuffer+id);
}


/**
 * @brief Read a received message. Non-Blocking
 * @param id UART's number
 * @param msg Buffer to paste the received bytes
 * @param cant Desired quantity of bytes to be pasted
 * @return Real quantity of pasted bytes
*/
uint8_t uartReadMsg(uint8_t id, char* msg, uint8_t cant) {
	
	uint8_t i = 0;

	hw_DisableInterrupts();
	while (i++ < cant && CBgetBufferState(RxBuffer+id)) {
		*(msg++) = CBgetByte(RxBuffer+id);			// Copy to msg
	}
	hw_EnableInterrupts();

	return i-1;

}


/**
 * @brief Write a message to be transmitted. Non-Blocking
 * @param id UART's number
 * @param msg Buffer with the bytes to be transfered
 * @param cant Desired quantity of bytes to be transfered
 * @return Real quantity of bytes to be transfered
*/
uint8_t uartWriteMsg(uint8_t id, const char* msg, uint8_t cant) {

	hw_DisableInterrupts();
	CBputChain(TxBuffer+id, msg, cant);
	hw_EnableInterrupts();

	// Enable Tx and their IRQs, this automatically starts sending data
	UARTPorts[id]->C2 |= UART_C2_TE_MASK | UART_C2_TIE_MASK;

	return CBgetBufferState(TxBuffer+id);

}


/**
 * @brief Check if all bytes were transfered
 * @param id UART's number
 * @return All bytes were transfered
*/
bool uartIsTxMsgComplete(uint8_t id) {
	return CBisEmpty(TxBuffer+id);
}


/*******************************************************************************
 *******************************************************************************
                        LOCAL FUNCTION DEFINITIONS
 *******************************************************************************
 ******************************************************************************/

static void UART_Rx_Tx_IRQ(uint8_t id) {

	if(UARTPorts[id]->S1 & UART_S1_TDRE_MASK) {		// Buffer empty
		if (!CBisEmpty(TxBuffer + id)) {				// Keep sending
			uint8_t data = CBgetByte(TxBuffer + id);
			UARTPorts[id]->D = data;						// Flag cleaned when writing in D buffer
		}
		else {		// Nothing else to send -> IDLE State
			UARTPorts[id]->C2 &= ~(UART_C2_TE_MASK | UART_C2_TIE_MASK | UART_C2_TCIE_MASK);		// Turn off Tx and disable their IRQs
//			UARTPorts[id]->C2 |= UART_C2_SBK_MASK	// Break character;
		}
	}

	if (UARTPorts[id]->S1 & UART_S1_RDRF_MASK) {	// Something received
		CBputByte(RxBuffer + id, UARTPorts[id]->D);		// Flag cleaned when reading D buffer
	}

}


__ISR__ UART0_RX_TX_IRQHandler(void){
#ifdef ENABLE_TP
	gpioWrite(TP_PIN, HIGH);
#endif

	UART_Rx_Tx_IRQ(0);

#ifdef ENABLE_TP
	gpioWrite(TP_PIN, LOW);
#endif
}

__ISR__ UART1_RX_TX_IRQHandler(void){
#ifdef ENABLE_TP
	gpioWrite(TP_PIN, HIGH);
#endif

	UART_Rx_Tx_IRQ(1);

#ifdef ENABLE_TP
	gpioWrite(TP_PIN, LOW);
#endif
}

__ISR__ UART2_RX_TX_IRQHandler(void){
#ifdef ENABLE_TP
	gpioWrite(TP_PIN, HIGH);
#endif

	UART_Rx_Tx_IRQ(2);

	#ifdef ENABLE_TP
	gpioWrite(TP_PIN, LOW);
#endif
}

__ISR__ UART3_RX_TX_IRQHandler(void){
#ifdef ENABLE_TP
	gpioWrite(TP_PIN, HIGH);
#endif

	UART_Rx_Tx_IRQ(3);

#ifdef ENABLE_TP
	gpioWrite(TP_PIN, LOW);
#endif
}

__ISR__ UART4_RX_TX_IRQHandler(void){
#ifdef ENABLE_TP
	gpioWrite(TP_PIN, HIGH);
#endif

	UART_Rx_Tx_IRQ(4);

#ifdef ENABLE_TP
	gpioWrite(TP_PIN, LOW);
#endif
}
