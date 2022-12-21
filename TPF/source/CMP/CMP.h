/***************************************************************************//**
  @file     CMP.h
  @brief    +Descripcion del archivo+
  @author   KevinWahle
  @date		24 oct. 2022
 ******************************************************************************/

#ifndef CMP_CMP_H_
#define CMP_CMP_H_

/*******************************************************************************
 * INCLUDE HEADER FILES
 ******************************************************************************/
#include <stdbool.h>
#include <stdint.h>

/*******************************************************************************
 * CONSTANT AND MACRO DEFINITIONS USING #DEFINE
 ******************************************************************************/



/*******************************************************************************
 * ENUMERATIONS AND STRUCTURES AND TYPEDEFS
 ******************************************************************************/
typedef enum
{
	CMP0_t,
	CMP1_t
} CMP_n;	//CMP_Type

typedef enum
{
	Disable=0,
	cons_1,
	cons_2,
	cons_3,
	cons_4,
	cons_5,
	cons_6,
	cons_7
} CONSEC_SAMP;	//Sample Count

typedef enum
{
	level_0,	// 5mV
	level_1,	// 10mV
	level_2,	// 20mV
	level_3		// 30mV
} HYST_LEVEL; //Hysteresis Level

typedef enum
{
	no_inv,
	inv
} INVERT; //Polarity Invert


/*******************************************************************************
 * VARIABLE PROTOTYPES WITH GLOBAL SCOPE
 ******************************************************************************/

// +ej: extern unsigned int anio_actual;+


/*******************************************************************************
 * FUNCTION PROTOTYPES WITH GLOBAL SCOPE
 ******************************************************************************/


/**
 * @brief initialize comparator
 * @param cmp_n number of cmp to use
 * @param hyst_level hysteresis level (5mV,10mV,20mV,30mV)
 * @param polarity decide if inverted output or not
*/
void CMP_Init (CMP_n cmp_n, HYST_LEVEL hyst_level, INVERT polarity);


/*******************************************************************************
 ******************************************************************************/

#endif // _CMP_CMP_H_
