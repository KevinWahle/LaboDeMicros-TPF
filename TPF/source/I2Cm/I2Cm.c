/***************************************************************************//**
  @file		I2Cm.c
  @brief	
  @author	Grupo 5
  @date		13 sep. 2022
 ******************************************************************************/

/*******************************************************************************
 * INCLUDE HEADER FILES
 ******************************************************************************/

#include "I2Cm.h"
#include "MCAL/gpio.h"
#include "MK64F12.h"
#include "hardware.h"
#include "stdbool.h"
#include "buffer/generic_circular_buffer.h"
#include "timer/timer.h"
/*******************************************************************************
 * CONSTANT AND MACRO DEFINITIONS USING #DEFINE
 ******************************************************************************/

#define ENABLE_TP

#ifdef ENABLE_TP
#define TP_PIN	PORTNUM2PIN(PC, 7)
#endif

#define I2C_COUNT	3

#define BUS_CLK	50000000UL

/*******************************************************************************
 * ROM CONST VARIABLES WITH FILE LEVEL SCOPE
 ******************************************************************************/

static I2C_Type* const I2CPtrs[] = I2C_BASE_PTRS;
static IRQn_Type const I2CIRQs[] = I2C_IRQS;

static PORT_Type* const portPtr[] = PORT_BASE_PTRS;

static const uint8_t I2CPinPorts[] =    { PB, PC, PA, PE };
static const uint8_t I2CPinPinsSCL[] =  { 2,  10, 12, 24 };
static const uint8_t I2CPinPinsSDA[] =  { 3,  11, 13, 25 };
static const uint8_t I2CPinAlts[] =     { 2,  2,  5,  5  };

static __IO uint32_t* const I2CClkSimPtr[] = {&(SIM->SCGC4), &(SIM->SCGC4), &(SIM->SCGC1), &(SIM->SCGC4)};
static const uint32_t I2CClkSimMask[] = {SIM_SCGC4_I2C0_MASK, SIM_SCGC4_I2C1_MASK, SIM_SCGC1_I2C2_MASK, SIM_SCGC4_I2C0_MASK};


/*******************************************************************************
 * STATIC VARIABLES AND CONST VARIABLES WITH FILE LEVEL SCOPE
 ******************************************************************************/

static genericCircularBuffer I2C_CircularBuffer;
static tim_id_t I2CTimerID;
static bool timerStarted = false;
/*******************************************************************************
 *******************************************************************************
                        GLOBAL FUNCTION DEFINITIONS
 *******************************************************************************
 ******************************************************************************/

void cbI2C(){  // LECTURA NO FUNCIONA
	if(!isI2CBusy(I2C_ACC)){       // I2C_ACC
		if(GCBgetBufferState(&I2C_CircularBuffer) > 0 ){
			Transaction_t tempT;
			GCBgetData(&I2C_CircularBuffer, (void*)(&tempT));
			I2CmStartTransaction(tempT.id, tempT.address, tempT.writeBuffer, tempT.writeSize, NULL, 0);
		}
	}
	return ;
}
void pushTransaction(Transaction_t * T){
	if(timerStarted == false){
		timerStart(I2CTimerID, TIMER_MS2TICKS(1), TIM_MODE_PERIODIC, cbI2C);
		timerStarted = true;
	}

	GCBputData(&I2C_CircularBuffer, (void*) T);

}


/**
 * @brief Inicializa el modulo I2C
 * @param id: Instancia del I2C [0 - 1]
*/
void I2CmInit(I2CPort_t id) {

	timerInit();
	I2CTimerID = timerGetId();
	GCBinit(&I2C_CircularBuffer, sizeof(Transaction_t), 10); // OJO QUE PUEDE TIRAR ERROR PORQUE EL BUFFER INTERNO ES CHICO


// Clock Gating

	*(I2CClkSimPtr[id]) |= I2CClkSimMask[id];

// Config pins (ALT, Open Drain, NO Pullup)

//	portPtr[I2CPinPorts[id]]->PCR[I2CPinPinsSCL[id]] = PORT_PCR_MUX(I2CPinAlts[id]) | PORT_PCR_ODE_MASK;
//	portPtr[I2CPinPorts[id]]->PCR[I2CPinPinsSDA[id]] = PORT_PCR_MUX(I2CPinAlts[id]) | PORT_PCR_ODE_MASK;

	// TEST:
	portPtr[I2CPinPorts[id]]->PCR[I2CPinPinsSCL[id]] = PORT_PCR_MUX(I2CPinAlts[id]) | PORT_PCR_ODE_MASK; //| PORT_PCR_PE_MASK | PORT_PCR_PS_MASK;
	portPtr[I2CPinPorts[id]]->PCR[I2CPinPinsSDA[id]] = PORT_PCR_MUX(I2CPinAlts[id]) | PORT_PCR_ODE_MASK; //| PORT_PCR_PE_MASK | PORT_PCR_PS_MASK;

// I2C Master config

	// Clock Divider
	I2CPtrs[id%I2C_COUNT]->F = I2C_F_ICR(0x2B);		// Divider to 512 with no multiplier	(SCL: 100k)
	//I2CPtrs[id%I2C_COUNT]->F = I2C_F_ICR(0x3F) | I2C_F_MULT(2);        // Divider to 3840 with 4 multiplier    (SCL: 3.26k)

	// Enable I2C. No Enable Master Mode and interrupts yet
	//	I2CPtrs[id%I2C_COUNT]->C1 = I2C_C1_IICEN_MASK;

	// Enable IRQ in NVIC

	NVIC_EnableIRQ(I2CIRQs[id%I2C_COUNT]);


#ifdef ENABLE_TP
	gpioMode(TP_PIN, OUTPUT);
//	gpioWrite(TP_PIN, LOW);
#endif

}

typedef struct{
	uint8_t writeSize;
	uint8_t writtenBytesCounter;
	uint8_t* writeBuffer;
	uint8_t address;
	bool repeatedStartRealeased;
	bool writeMode;
}writeState_t;

typedef struct{
	uint8_t readSize;
	uint8_t readBytesCounter;
	uint8_t* readBuffer;
}readState_t;

typedef enum {MASTER_RX, MASTER_TX, I2C_FAIL} I2C_STATES;

I2C_STATES i2cStates [] = {I2C_FAIL, I2C_FAIL};

static writeState_t writeState [I2C_COUNT];
static readState_t readState [I2C_COUNT];

static bool isBusBusy = false;

/**
 * @brief realiza una transmision y recepcion por I2C
 * @param address address del slave
 * @param writeBuffer buffer de escritura
 * @param writeSize Tamano del buffer de escritura
 * @param readBuffer buffer para guardar la lectura
 * @param readSize Tamano del buffer de lectura
*/
bool I2CmStartTransaction(I2CPort_t id, uint8_t address, uint8_t* writeBuffer, uint8_t writeSize, uint8_t* readBuffer, uint8_t readSize) {

	I2C_Type* pI2C = I2CPtrs[id%I2C_COUNT];
	//if(!(pI2C->S & I2C_S_BUSY_MASK)){  // hace la transaction solo si no esta busy
	if(!isI2CBusy(id)){
		writeState[id%I2C_COUNT].writeSize = writeSize;
		writeState[id%I2C_COUNT].writeBuffer = writeBuffer;
		writeState[id%I2C_COUNT].address = address;
		writeState[id%I2C_COUNT].writtenBytesCounter = 0;
		writeState[id%I2C_COUNT].repeatedStartRealeased = false;
		readState[id%I2C_COUNT].readSize = readSize;
		readState[id%I2C_COUNT].readBuffer = readBuffer;
		readState[id%I2C_COUNT].readBytesCounter = 0;

		uint8_t RWbit = writeSize > 0 ? 0 : 1; // bit de R/W luego del address. 0 si hay que escribir, 1 para leer
		writeState[id%I2C_COUNT].writeMode = !RWbit;
		i2cStates[id%I2C_COUNT] = MASTER_TX; // El master SIEMPRE serÃ¡ TX, pues debe enviar el address

		isBusBusy = true;
		// Enable I2C in Master Mode, transmit mode and interrupts
		pI2C->C1 |= I2C_C1_IICEN_MASK;
		pI2C->C1 |= I2C_C1_IICIE_MASK;
		pI2C->C1 |= I2C_C1_TX_MASK;
		pI2C->C1 |= I2C_C1_MST_MASK;

		pI2C->D = address << 1 | RWbit;		// Slave Address + RW bit

		return true;   // se hizo la transaction
	}
	return false;      // no se hizo la transaction por busy
}


bool isI2CBusy(I2CPort_t id){
	I2C_Type* pI2C = I2CPtrs[id%I2C_COUNT];
	if(!isBusBusy){
		return pI2C->S & I2C_S_BUSY_MASK;
	}
	return true;
}

/*******************************************************************************
 *******************************************************************************
                        LOCAL FUNCTION DEFINITIONS
 *******************************************************************************
 ******************************************************************************/

__ISR__ I2C0_IRQHandler() {

#ifdef ENABLE_TP
	gpioWrite(TP_PIN, HIGH);
#endif

	//I2C_IRQ();
	I2C_Type* pI2C = I2CPtrs[0];
 	pI2C->S |= I2C_S_IICIF_MASK;     // borro el flag de la interrupcion
	if(pI2C->S & I2C_S_TCF_MASK){     // Me fijo si la interrupcion fue porque se termino una transaccion y LLEGO un ACK/NACK
		switch(i2cStates[0]){
		case MASTER_TX:
			if(!writeState[0].writeMode){  // si se pide lectura
				i2cStates[0] = MASTER_RX;
				pI2C->C1 &= ~I2C_C1_TX_MASK;  // Cambio la direccion

				if(readState[0].readSize > 1) // Si es > 1, no debo responder con un NACK, si no con un ACK para seguir leyendo
					pI2C->C1 &= ~I2C_C1_TXAK_MASK;  // Envio el ACK
				else
					pI2C->C1 |= I2C_C1_TXAK_MASK;  // Envio el NACK

				uint8_t dummyData = pI2C->D;  // Leo dummy y disparo lectura
				dummyData++; // USING DUMMY

				#ifdef ENABLE_TP
					gpioWrite(TP_PIN, LOW);
				#endif
				return;
			}
			else if(writeState[0].writtenBytesCounter >= writeState[0].writeSize && readState[0].readSize == 0){  // si ya se escribio \todo lo que me pasaron y no hay nada para leer
				pI2C->C1 &= ~I2C_C1_MST_MASK;     // genero el Stop Signal
				isBusBusy = false;

				#ifdef ENABLE_TP
					gpioWrite(TP_PIN, LOW);
				#endif
				return;
			}
			else if(pI2C->S & I2C_S_RXAK_MASK){  // si entra, no me reconocio el ACK, => corto \todo
				pI2C->C1 &= ~I2C_C1_MST_MASK;    // genero el Stop Signal
				isBusBusy = false;

				#ifdef ENABLE_TP
					gpioWrite(TP_PIN, LOW);
				#endif
				return;
			}
			else if(!(pI2C->S & I2C_S_RXAK_MASK)){ //si entra, me envio el ACK
				if(!writeState[0].repeatedStartRealeased){ // si no habia hecho un repeated Start
					if(writeState[0].writtenBytesCounter < writeState[0].writeSize){ // si aun no escribi \todo
						pI2C->D = writeState[0].writeBuffer[writeState[0].writtenBytesCounter];
						writeState[0].writtenBytesCounter++;

						#ifdef ENABLE_TP
							gpioWrite(TP_PIN, LOW);
						#endif
						return;
					}
					else if(writeState[0].writtenBytesCounter == writeState[0].writeSize){	 // si escribi \todo, lanzo un Repeated start (ya se que readSize > 0, lo que implica un repeated start)
						pI2C->C1 |= I2C_C1_RSTA_MASK;              // hago un repeated start
						pI2C->D = writeState[0].address << 1 | 1;  // meto el address y ya FIJO lectura, si bien se puede escribir, por la simpleza de nuestra funcion se asume lectura ya
						writeState[0].repeatedStartRealeased = true;
					}
				}
				else{     // Debo cambiar el modo !! a RX, pues ya envie el Repeated start
					i2cStates[0] = MASTER_RX;
					pI2C->C1 &= ~I2C_C1_TX_MASK;  // Cambio la direccion

					if(readState[0].readSize > 1) // Si es > 1, no debo responder con un NACK, si no con un ACK para seguir leyendo
						pI2C->C1 &= ~I2C_C1_TXAK_MASK;  // Envio el ACK
					else
						pI2C->C1 |= I2C_C1_TXAK_MASK;  // Envio el NACK

					uint8_t dummyData = pI2C->D;  // Leo dummy y disparo lectura
					dummyData++; // USING DUMMY

					#ifdef ENABLE_TP
						gpioWrite(TP_PIN, LOW);
					#endif
					return;
				}
			}
			break;
		case MASTER_RX:
			if(readState[0].readBytesCounter < ( readState[0].readSize - 1 ) ){  // si leeiste \todos menos el ultimo => ahora vas a leer el ultimo

				if(readState[0].readBytesCounter < ( readState[0].readSize - 2 )) // ACK y NACK check
					pI2C->C1 &= ~I2C_C1_TXAK_MASK;  // Envio el ACK
				else
					pI2C->C1 |= I2C_C1_TXAK_MASK;  // Envio el NACK

			 	readState[0].readBuffer[readState[0].readBytesCounter] = pI2C->D;
				readState[0].readBytesCounter++;

				#ifdef ENABLE_TP
					gpioWrite(TP_PIN, LOW);
				#endif
				return;
			}
			else{
				pI2C->C1 &= ~I2C_C1_MST_MASK;   // Apago \todo
			 	readState[0].readBuffer[readState[0].readBytesCounter] = pI2C->D;  //
				readState[0].readBytesCounter++;
				isBusBusy = false;

				#ifdef ENABLE_TP
					gpioWrite(TP_PIN, LOW);
				#endif
				return;
			}
			break;
		default:
			break;
		}
	}

#ifdef ENABLE_TP
	gpioWrite(TP_PIN, LOW);
#endif

}
/*
__ISR__ I2C1_IRQHandler() {
#ifdef ENABLE_TP
	gpioWrite(TP_PIN, HIGH);
#endif
	I2C_IRQ();
#ifdef ENABLE_TP
	gpioWrite(TP_PIN, LOW);
#endif

}

__ISR__ I2C2_IRQHandler() {
#ifdef ENABLE_TP
	gpioWrite(TP_PIN, HIGH);
#endif
	I2C_IRQ();
#ifdef ENABLE_TP
	gpioWrite(TP_PIN, LOW);
#endif
}

static void I2C_IRQ() {

}
*/
