/***************************************************************************//**
  @file     ADC.h
  @brief    +Descripcion del archivo+
  @author   KevinWahle
  @date		16 oct. 2022
 ******************************************************************************/

#ifndef SOURCES_TEMPLATE_ADC_H_
#define SOURCES_TEMPLATE_ADC_H_

/*******************************************************************************
 * INCLUDE HEADER FILES
 ******************************************************************************/

#include <stdint.h>
#include <stdbool.h>

/*******************************************************************************
 * ENUMERATIONS AND STRUCTURES AND TYPEDEFS
 ******************************************************************************/

typedef enum
{
	ADC_b8,
	ADC_b12,
	ADC_b10,
	ADC_b16,
} ADCBits_t;

typedef enum
{
	ADC_c24,
	ADC_c16,
	ADC_c10,
	ADC_c6,
	ADC_c4,
} ADCCycles_t;

typedef enum
{
	ADC_t4,
	ADC_t8,
	ADC_t16,
	ADC_t32,
	ADC_t1,
} ADCTaps_t;

typedef enum
{
	ADC_mA,
	ADC_mB,
} ADCMux_t;

typedef enum
{
	ADC0_t,
	ADC1_t,
} ADC_n;	//ADC_Type

typedef enum
{
	DIV_t1, // Clock
	DIV_t2,	// Clock/2
	DIV_t4,	// Clock/4
	DIV_t8,	// Clock/8

} ADCClkDiv_t;

typedef uint8_t ADCChannel_t; /* Channel 0-23 */
typedef uint16_t ADCData_t;
typedef void (*ADC_CB)(ADC_n adc_n, ADCData_t data);

/*******************************************************************************
 * FUNCTION PROTOTYPES WITH GLOBAL SCOPE
 ******************************************************************************/

/**
 * @brief initalize ADC module
 * @param adc_n number of adc
 * @param divider sample time divider to bus clock
 * @param bits number of bits per sample
 * @cycles number of cycles to complete a conversion
 */
void ADC_Init (ADC_n adc_n, ADCClkDiv_t divider, ADCBits_t bits, ADCCycles_t cycles);

/**
 * @brief starts sampling
 * @param adc_n number of adc
 * @param channel channel to sample from
 * @param mux mux to sample from
 */
void ADC_Start(ADC_n adc_n, ADCChannel_t channel, ADCMux_t mux);


void ADC_SetInterruptMode (ADC_n adc_n, bool);
void ADC_ClearInterruptFlag (ADC_n adc_n);

void ADC_SetResolution (ADC_n adc_n, ADCBits_t);
ADCBits_t ADC_GetResolution (ADC_n adc_n);
void ADC_SetCycles (ADC_n adc_n, ADCCycles_t);
ADCCycles_t ADC_GetCycles (ADC_n adc_n);
void ADC_SetHardwareAverage (ADC_n adc_n, ADCTaps_t);
ADCTaps_t ADC_GetHardwareAverage (ADC_n adc_n);

bool ADC_Calibrate (ADC_n adc_n);

bool ADC_IsReady (ADC_n adc_n);
ADCData_t ADC_getData (ADC_n adc_n);

#endif /* SOURCES_TEMPLATE_ADC_H_ */
