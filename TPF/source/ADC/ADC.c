
#include "ADC.h"
#include "MK64F12.h"
#include "../MCAL/gpio.h"

#define TWO_POW_NUM_OF_CAL (1 << 4)

#define BUS_CLK 0x00

bool ADC_interrupt[2] = {false, false};

typedef ADC_Type *ADC_t;

static ADC_Type* ADCPorts[] = ADC_BASE_PTRS;
//static PORT_Type* portPtrs[] = PORT_BASE_PTRS;



//ADC_CB callback;

typedef enum {PIN_DISABLE, ALTERNATIVE_1, ALTERNATIVE_2, ALTERNATIVE_3, ALTERNATIVE_4, 
									ALTERNATIVE_5, ALTERNATIVE_6, ALTERNATIVE_7} mux_alt;
typedef enum {OPEN_DRAIN, PUSH_PULL} pin_mode;



/*******************************************************************************
 *******************************************************************************
                        GLOBAL FUNCTION DEFINITIONS
 *******************************************************************************
 ******************************************************************************/

void ADC_Init (ADC_n adc_n, ADCClkDiv_t divider, ADCBits_t bits, ADCCycles_t cycles)
{
	ADC_t adc = ADCPorts[adc_n];

	if (!adc_n){
		SIM->SCGC6 |= SIM_SCGC6_ADC0_MASK;
		NVIC_EnableIRQ(ADC0_IRQn);
	}
	else{
		SIM->SCGC3 |= SIM_SCGC3_ADC1_MASK;
		NVIC_EnableIRQ(ADC1_IRQn);
	}

	adc->CFG1 = (adc->CFG1 & ~ADC_CFG1_ADICLK_MASK) | ADC_CFG1_ADICLK(BUS_CLK) ; //Use bus clock
	adc->CFG1 |= ADC_CFG1_ADIV(divider);		//Clock Divide Select

	ADC_SetResolution(adc_n, bits);
	ADC_SetCycles(adc_n, cycles);
	ADC_Calibrate(adc_n);

	adc->SC3 |= ADC_SC3_ADCO(true) ;  //Continous mode & ADC_SC3_AVGE(false)

}

void ADC_Start(ADC_n adc_n, ADCChannel_t channel, ADCMux_t mux){
	ADC_t adc = ADCPorts[adc_n];
	adc->CFG2 |= (adc->CFG2 & ~ADC_CFG2_MUXSEL_MASK) | ADC_CFG2_MUXSEL(mux);		// ADC Mux Select
	adc->SC1[adc_n] = ADC_SC1_AIEN(ADC_interrupt[adc_n]) | ADC_SC1_ADCH(channel);	// Interrupt Disable, Input channel select

}

void ADC_SetResolution (ADC_n adc_n, ADCBits_t bits)
{
	ADC_t adc = (adc_n==ADC0_t) ? ADC0 : ADC1;
	adc->CFG1 |= (adc->CFG1 & ~ADC_CFG1_MODE_MASK) | ADC_CFG1_MODE(bits);
}

void ADC_SetCycles (ADC_n adc_n, ADCCycles_t cycles)
{
	ADC_t adc = ADCPorts[adc_n];

	if (cycles & ~ADC_CFG2_ADLSTS_MASK)
	{
		adc->CFG1 &= ~ADC_CFG1_ADLSMP_MASK;		//Short sample time
	}
	else
	{
		adc->CFG1 |= ADC_CFG1_ADLSMP_MASK;		//Long sample time
		adc->CFG2 = (adc->CFG2 & ~ADC_CFG2_ADLSTS_MASK) | ADC_CFG2_ADLSTS(cycles);
	}
}



void ADC_SetInterruptMode (ADC_n adc_n, bool mode)
{
	ADC_interrupt[adc_n] = mode;
}



void ADC_ClearInterruptFlag (ADC_n adc_n)
{
	ADC_t adc = ADCPorts[adc_n];
	adc->SC1[adc_n] = 0x00;
}



ADCBits_t ADC_GetResolution (ADC_n adc_n)
{
	ADC_t adc = ADCPorts[adc_n];
	return adc->CFG1 & ADC_CFG1_MODE_MASK;
}



ADCCycles_t ADC_GetSCycles (ADC_n adc_n)
{
	ADC_t adc = ADCPorts[adc_n];
	if (adc->CFG1 & ADC_CFG1_ADLSMP_MASK)
		return ADC_c4;
	else
		return adc->CFG2 & ADC_CFG2_ADLSTS_MASK;
}

//Determines how many ADC conversions will be averaged to create the ADC average result
void ADC_SetHardwareAverage (ADC_n adc_n, ADCTaps_t taps)
{
	ADC_t adc = ADCPorts[adc_n];
	if (taps & ~ADC_SC3_AVGS_MASK)
	{
		adc->SC3 &= ~ADC_SC3_AVGE_MASK;
	}
	else
	{
		adc->SC3 |= ADC_SC3_AVGE_MASK;
		adc->SC3 |= (adc->SC3 & ~ADC_SC3_AVGS_MASK) | ADC_SC3_AVGS(taps);
	}
}

ADCTaps_t ADC_GetHardwareAverage (ADC_n adc_n)
{
	ADC_t adc = ADCPorts[adc_n];
	if (adc->SC3 & ADC_SC3_AVGE_MASK)
		return ADC_t1;
	else
		return adc->SC3 & ADC_SC3_AVGS_MASK;
}

bool ADC_Calibrate (ADC_n adc_n)
{
	ADC_t adc = ADCPorts[adc_n];
	int32_t  Offset		= 0;
	uint32_t Minus	[7] = {0,0,0,0,0,0,0};
	uint32_t Plus	[7] = {0,0,0,0,0,0,0};
	uint8_t  i;
	uint32_t scr3;

	/// SETUP
	adc->SC1[adc_n] |= 0x1F;
	scr3 = adc->SC3;
	adc->SC3 &= (ADC_SC3_AVGS(0x03) | ADC_SC3_AVGE_MASK);

	/// INITIAL CALIBRATION
	adc->SC3 &= ~ADC_SC3_CAL_MASK;
	adc->SC3 |=  ADC_SC3_CAL_MASK;
	while (!(adc->SC1[adc_n] & ADC_SC1_COCO_MASK));
	if (adc->SC3 & ADC_SC3_CALF_MASK)
	{
		adc->SC3 |= ADC_SC3_CALF_MASK;
		return false;
	}
	adc->PG  |= (0x8000 | ((adc->CLP0+adc->CLP1+adc->CLP2+adc->CLP3+adc->CLP4+adc->CLPS) >> (1 + TWO_POW_NUM_OF_CAL)));
	adc->MG  |= (0x8000 | ((adc->CLM0+adc->CLM1+adc->CLM2+adc->CLM3+adc->CLM4+adc->CLMS) >> (1 + TWO_POW_NUM_OF_CAL)));

	// FURTHER CALIBRATIONS
	for (i = 0; i < TWO_POW_NUM_OF_CAL; i++)
	{
		adc->SC3 &= ~ADC_SC3_CAL_MASK;
		adc->SC3 |=  ADC_SC3_CAL_MASK;
		while (!(adc->SC1[adc_n] & ADC_SC1_COCO_MASK));
		if (adc->SC3 & ADC_SC3_CALF_MASK)
		{
			adc->SC3 |= ADC_SC3_CALF_MASK;
			return 1;
		}
		Offset += (short)adc->OFS;
		Plus[0] += (unsigned long)adc->CLP0;
		Plus[1] += (unsigned long)adc->CLP1;
		Plus[2] += (unsigned long)adc->CLP2;
		Plus[3] += (unsigned long)adc->CLP3;
		Plus[4] += (unsigned long)adc->CLP4;
		Plus[5] += (unsigned long)adc->CLPS;
		Plus[6] += (unsigned long)adc->CLPD;
		Minus[0] += (unsigned long)adc->CLM0;
		Minus[1] += (unsigned long)adc->CLM1;
		Minus[2] += (unsigned long)adc->CLM2;
		Minus[3] += (unsigned long)adc->CLM3;
		Minus[4] += (unsigned long)adc->CLM4;
		Minus[5] += (unsigned long)adc->CLMS;
		Minus[6] += (unsigned long)adc->CLMD;
	}
	adc->OFS |= (Offset >> TWO_POW_NUM_OF_CAL);
	adc->PG  |= (0x8000 | ((Plus[0] +Plus[1] +Plus[2] +Plus[3] +Plus[4] +Plus[5] ) >> (1 + TWO_POW_NUM_OF_CAL)));
	adc->MG  |= (0x8000 | ((Minus[0]+Minus[1]+Minus[2]+Minus[3]+Minus[4]+Minus[5]) >> (1 + TWO_POW_NUM_OF_CAL)));
	adc->CLP0 |= (Plus[0] >> TWO_POW_NUM_OF_CAL);
	adc->CLP1 |= (Plus[1] >> TWO_POW_NUM_OF_CAL);
	adc->CLP2 |= (Plus[2] >> TWO_POW_NUM_OF_CAL);
	adc->CLP3 |= (Plus[3] >> TWO_POW_NUM_OF_CAL);
	adc->CLP4 |= (Plus[4] >> TWO_POW_NUM_OF_CAL);
	adc->CLPS |= (Plus[5] >> TWO_POW_NUM_OF_CAL);
	adc->CLPD |= (Plus[6] >> TWO_POW_NUM_OF_CAL);
	adc->CLM0 |= (Minus[0] >> TWO_POW_NUM_OF_CAL);
	adc->CLM1 |= (Minus[1] >> TWO_POW_NUM_OF_CAL);
	adc->CLM2 |= (Minus[2] >> TWO_POW_NUM_OF_CAL);
	adc->CLM3 |= (Minus[3] >> TWO_POW_NUM_OF_CAL);
	adc->CLM4 |= (Minus[4] >> TWO_POW_NUM_OF_CAL);
	adc->CLMS |= (Minus[5] >> TWO_POW_NUM_OF_CAL);
	adc->CLMD |= (Minus[6] >> TWO_POW_NUM_OF_CAL);

	/// UN-SETUP
	adc->SC3 = scr3;

	return true;
}



bool ADC_IsReady (ADC_n adc_n)
{
	ADC_t adc = ADCPorts[adc_n];
	bool is_ready = adc->SC1[adc_n] & ADC_SC1_COCO_MASK;		// Conversion Complete Flag
	return is_ready;
}



ADCData_t ADC_getData (ADC_n adc_n)
{
	ADC_t adc = ADCPorts[adc_n];
	ADCData_t data= adc->R[adc_n];
	return data;

}

/*
__ISR__ ADC0_IRQHandler(void){
	callback(ADC0_t, ADC_getData(ADC0_t));
}

__ISR__ ADC1_IRQHandler(void){
	callback(ADC1_t, ADC_getData(ADC1_t));
}
*/
