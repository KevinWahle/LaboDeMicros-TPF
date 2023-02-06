/***************************************************************************//**
  @file		ADC_hall.c
  @brief	+Descripcion del archivo+
  @author	KevinWahle
  @date		16 oct. 2022
 ******************************************************************************/

/*******************************************************************************
 * INCLUDE HEADER FILES
 ******************************************************************************/

// +Incluir el header propio (ej: #include "template.h")+
#include "ADC/ADC_hal.h"
#include "ADC.h"
#include "../MCAL/gpio.h"
#include "hardware.h"

#include "timer/timer.h"
#include "../MCAL/gpio.h"
/*******************************************************************************
 * CONSTANT AND MACRO DEFINITIONS USING #DEFINE
 ******************************************************************************/

//#define NBITS 12 //12 bits implementation
#define PTB2 PORTNUM2PIN(PB,2)

/*******************************************************************************
 * ENUMERATIONS AND STRUCTURES AND TYPEDEFS
 ******************************************************************************/


/*******************************************************************************
 * VARIABLES WITH GLOBAL SCOPE
 ******************************************************************************/



/*******************************************************************************
 * FUNCTION PROTOTYPES FOR PRIVATE FUNCTIONS WITH FILE LEVEL SCOPE
 ******************************************************************************/

static void add_buff_cb ();


/*******************************************************************************
 * ROM CONST VARIABLES WITH FILE LEVEL SCOPE
 ******************************************************************************/



/*******************************************************************************
 * STATIC VARIABLES AND CONST VARIABLES WITH FILE LEVEL SCOPE
 ******************************************************************************/
static PORT_Type* const portPtr[] = PORT_BASE_PTRS;

static tim_id_t timer_id;

static circularBuffer16 * mybuff;

//#define ENABLE_TP

#ifdef ENABLE_TP
#define TP_PIN	PORTNUM2PIN(PC, 1)
#endif


/*******************************************************************************
 *******************************************************************************
                        GLOBAL FUNCTION DEFINITIONS
 *******************************************************************************
 ******************************************************************************/

void ADCh_Init (ADChClkDiv_t divider, circularBuffer16 * buff){

	#ifdef ENABLE_TP
		gpioMode(TP_PIN, OUTPUT);
		gpioWrite(TP_PIN, LOW);
	#endif

	ADC_Init(ADC0_t, divider, ADC_b12 , ADC_c24);

	//CBinit(&buff,200);

	mybuff=buff;
	
	timerInit();
	timer_id = timerGetId();


}
void ADCh_Start(uint16_t frec){
	ADC_Start(ADC0_t, 0x0C, ADC_mA); //Channel 12
									 //mux A selected
	portPtr[PIN2PORT(PTB2)]->PCR[PIN2NUM(PTB2)]=PORT_PCR_MUX(0x00); //PTB2

	timerStart(timer_id, TIMER_MS2TICKS(1.0/frec), TIM_MODE_PERIODIC, add_buff_cb);
	
}


/*bool ADCh_IsReady(){
	return CBisEmpty16(mybuff);
}

uint16_t get_ADCh(){
	return CBgetByte16(mybuff);
}*/



/*******************************************************************************
 *******************************************************************************
                        LOCAL FUNCTION DEFINITIONS
 *******************************************************************************
 ******************************************************************************/
void add_buff_cb (){

	#ifdef ENABLE_TP
		gpioWrite(TP_PIN, HIGH);
	#endif
	if (ADC_IsReady(ADC0_t)){
		ADCData_t data =ADC_getData(ADC0_t);
		CBputByte16(mybuff, data);
	}
	#ifdef ENABLE_TP
		gpioWrite(TP_PIN, LOW);
	#endif
}



 
