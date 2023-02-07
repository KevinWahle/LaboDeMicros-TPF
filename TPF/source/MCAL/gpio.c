/***************************************************************************//**
  @file     gpio.c
  @brief    GPIO functions
  @author   Grupo 5
 ******************************************************************************/

/*******************************************************************************
 * INCLUDE HEADER FILES
 ******************************************************************************/

#include "gpio.h"
#include "MK64F12.h"
#include "hardware.h"
#include <stddef.h>

/*******************************************************************************
 * CONSTANT AND MACRO DEFINITIONS USING #DEFINE
 ******************************************************************************/

#define SIM_PORT_MASKS	{SIM_SCGC5_PORTA_MASK, SIM_SCGC5_PORTB_MASK, SIM_SCGC5_PORTC_MASK, SIM_SCGC5_PORTD_MASK, SIM_SCGC5_PORTE_MASK}

#define BUS_CLOCK 50e6

/*******************************************************************************
 * GLOBAL VARIABLES
 ******************************************************************************/

static GPIO_Type* const gpioPtr[] = GPIO_BASE_PTRS;	//TODO: Make global??	////////////////////////////
static PORT_Type* const portPtr[] = PORT_BASE_PTRS;	//TODO: Make global??	////////////////////////////
static const IRQn_Type portIRQs[] = PORT_IRQS;		//TODO: Make global??	////////////////////////////
static const uint32_t SIMPortMasks[] = SIM_PORT_MASKS;

pinIrqFun_t portIrqFunc[5][32];


/*******************************************************************************
 *******************************************************************************
                        GLOBAL FUNCTION DEFINITIONS
 *******************************************************************************
 ******************************************************************************/
bool gpioSetFilter(pin_t pin, uint8_t filterLength){

	uint8_t portn = PIN2PORT(pin);
	if (portn != PD) return false;		// Solo se admite en este PORT

	uint8_t pinn = PIN2NUM(pin);

	PORT_Type* port = portPtr[portn];

	uint32_t enablesMask = port->DFER | (1U << pinn);
	port->DFER = 0U;	// Disable all filters to change settings

	port->DFCR = PORT_DFCR_CS_MASK; // LPO clock
	port->DFWR = PORT_DFWR_FILT(filterLength);
	port->DFER = enablesMask;

	return true;

}


/**
 * @brief Configures the specified pin to behave either as an input or an output
 * @param pin the pin whose mode you wish to set (according PORTNUM2PIN)
 * @param mode INPUT (0), OUTPUT(1), INPUT_PULLUP(2) or INPUT_PULLDOWN(3).
 */
void gpioMode (pin_t pin, uint8_t mode) {


	uint8_t pinn = PIN2NUM(pin);

	uint8_t portn = PIN2PORT(pin);

	SIM->SCGC5 |= SIMPortMasks[portn];	// Enable Clock Gating for GPIO_PORTX

	GPIO_Type* gpio = gpioPtr[portn];

	PORT_Type* port = portPtr[portn];

	port->PCR[pinn] = PORT_PCR_MUX(0x1);		// Clear del PCR y ALT01:GPIO

	switch(mode) {

		case OUTPUT:
			gpio->PDDR |= GPIO_PDDR_PDD(1 << pinn);		// OUTPUT: Pin en 1
			break;

		case INPUT:
			gpio->PDDR &= ~GPIO_PDDR_PDD(1 << pinn);	// INPUT: Pin en 0
			port->PCR[pinn] &= ~PORT_PCR_PE(1);			// Pull Disable (PE=0)
			break;

		case INPUT_PULLDOWN:
			gpio->PDDR &= ~GPIO_PDDR_PDD(1 << pinn);	// INPUT: Pin en 0
			port->PCR[pinn] |= PORT_PCR_PE(1);			// Pull Enable (PE=1)
			port->PCR[pinn] &= ~PORT_PCR_PS(1);			// Pull Down (PS=0)
			break;

		case INPUT_PULLUP:
			gpio->PDDR &= ~GPIO_PDDR_PDD(1 << pinn);	// INPUT: Pin en 0
			port->PCR[pinn] |= PORT_PCR_PE(1);			// Pull Enable (PE=1)
			port->PCR[pinn] |= PORT_PCR_PS(1);			// Pull Up (PS=1)
			break;

		default:
			gpio->PDDR &= ~GPIO_PDDR_PDD(1 << pinn);	// NADA: Dejo como INPUT
			port->PCR[pinn] &= ~PORT_PCR_PE(1);
			break;

	}

}

/**
 * @brief Configures how the pin reacts when an IRQ event ocurrs
 * @param pin the pin whose IRQ mode you wish to set (according PORTNUM2PIN)
 * @param irqMode disable, risingEdge, fallingEdge or bothEdges
 * @param irqFun function to call on pin event
 * @return Registration succeed
 */
bool gpioIRQ (pin_t pin, uint8_t irqMode, pinIrqFun_t irqFun) {

	uint8_t pinn = PIN2NUM(pin);

	uint8_t portn = PIN2PORT(pin);

	PORT_Type* port = portPtr[portn];

	uint8_t mode;

	switch(irqMode) {

		case GPIO_IRQ_MODE_DISABLE:
			mode = 0;
			break;


		case GPIO_IRQ_MODE_RISING_EDGE:
			mode = 9;
			break;

		case GPIO_IRQ_MODE_FALLING_EDGE:
			mode = 10;
			break;

		case GPIO_IRQ_MODE_BOTH_EDGES:
			mode = 11;
			break;

		default: return false;

	}

	port->PCR[pinn] &= ~PORT_PCR_IRQC_MASK;	// Clear IRQC
	port->PCR[pinn] |= PORT_PCR_IRQC(mode);	// Set IRQC Mode

//	NVIC_SetVector(portIRQs[portn], (uint32_t)&irqFun);				// Esta bien setearlo asi?		///////////////////

	portIrqFunc[portn][pinn] = irqFun;

	if (mode) NVIC_EnableIRQ(portIRQs[portn]);

	return true;

}

/**
 * @brief Write a HIGH or a LOW value to a digital pin
 * @param pin the pin to write (according PORTNUM2PIN)
 * @param val Desired value (HIGH or LOW)
 */
void gpioWrite (pin_t pin, bool value) {

	uint8_t pinn = PIN2NUM(pin);

	uint8_t portn = PIN2PORT(pin);

	GPIO_Type* gpio = gpioPtr[portn];

	if (value) {
		gpio->PDOR |= GPIO_PDOR_PDO(1 << pinn);
	}
	else {
		gpio->PDOR &= ~GPIO_PDOR_PDO(1 << pinn);
	}

}

/**
 * @brief Toggle the value of a digital pin (HIGH<->LOW)
 * @param pin the pin to toggle (according PORTNUM2PIN)
 */
void gpioToggle (pin_t pin) {
		uint8_t pinn = PIN2NUM(pin);

		uint8_t portn = PIN2PORT(pin);

		GPIO_Type* gpio = gpioPtr[portn];

		gpio->PTOR = GPIO_PTOR_PTTO(1 << pinn);
}

/**
 * @brief Reads the value from a specified digital pin, either HIGH or LOW.
 * @param pin the pin to read (according PORTNUM2PIN)
 * @return HIGH or LOW
 */
bool gpioRead (pin_t pin) {

	bool state;

	uint8_t pinn = PIN2NUM(pin);

	uint8_t portn = PIN2PORT(pin);

	GPIO_Type* gpio = gpioPtr[portn];

	state = gpio->PDIR & GPIO_PDIR_PDI(1 << pinn);

	return state;

}


/*******************************************************************************
 *******************************************************************************
                        LOCAL FUNCTION DEFINITIONS
 *******************************************************************************
 ******************************************************************************/

__ISR__ PORTA_IRQHandler (void){
	for (int i = 0; i < 32; i++) {
		if (PORTA->ISFR & (1<<i)) {
			PORTA->ISFR = 1<<i;		// ISR Clear
			portIrqFunc[PA][i]();
		}
	}
}


__ISR__ PORTB_IRQHandler (void){
	for (int i = 0; i < 32; i++) {
		if (PORTB->ISFR & (1<<i)) {
			PORTB->ISFR = 1<<i;		// ISR Clear
			portIrqFunc[PB][i]();
		}
	}
}


__ISR__ PORTC_IRQHandler (void){
	for (int i = 0; i < 32; i++) {
		if (PORTC->ISFR & (1<<i)) {
			PORTC->ISFR = 1<<i;		// ISR Clear
			portIrqFunc[PC][i]();
		}
	}
}


__ISR__ PORTD_IRQHandler (void){
	for (int i = 0; i < 32; i++) {
		if (PORTD->ISFR & (1<<i)) {
			PORTD->ISFR = 1<<i;		// ISR Clear
			portIrqFunc[PD][i]();
		}
	}
}


__ISR__ PORTE_IRQHandler (void){
	for (int i = 0; i < 32; i++) {
		if (PORTE->ISFR & (1<<i)) {
			PORTE->ISFR = 1<<i;		// ISR Clear
			portIrqFunc[PE][i]();
		}
	}
}

/*******************************************************************************
 ******************************************************************************/
