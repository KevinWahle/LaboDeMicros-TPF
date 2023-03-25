/***************************************************************************//**
  @file     PIT.h
  @brief    Funciones del timer PIT
  @author   Grupo 5
  @date		24 oct. 2022
 ******************************************************************************/

#ifndef _PIT_PIT_H_
#define _PIT_PIT_H_

/*******************************************************************************
 * INCLUDE HEADER FILES
 ******************************************************************************/

#include <stdint.h>

/*******************************************************************************
 * CONSTANT AND MACRO DEFINITIONS USING #DEFINE
 ******************************************************************************/

#define PIT_TICK_TIME	20UL	// ns

#define PIT_TICK2NS(x)	((x)*PIT_TICK_TIME)			// ns
#define PIT_TICK2MS(x)	((x)*PIT_TICK_TIME/1000000UL)	// ms

#define PIT_NS2TICK(x)	(((x)/PIT_TICK_TIME)-1)
#define PIT_US2TICK(x)	(((x)*(1000UL/PIT_TICK_TIME))-1UL)
#define PIT_MS2TICK(x)	(((x)*(1000000UL/PIT_TICK_TIME))-1UL)

/*******************************************************************************
 * ENUMERATIONS AND STRUCTURES AND TYPEDEFS
 ******************************************************************************/

typedef enum {PIT_0, PIT_1, PIT_2, PIT_3, PIT_COUNT} PIT_MOD;

//typedef enum {DMA_0_SRC, DMA_1_SRC, ADC0_SRC} PIT_SRC;

typedef uint32_t PITTick_t;

typedef void (*PITCallback_t) (void);

/*******************************************************************************
 * FUNCTION PROTOTYPES WITH GLOBAL SCOPE
 ******************************************************************************/

/**
 * @brief Inicializa un timer periodico. NO se inicia hasta llamar a PITStart
 * @param pit: modulol PIT a utilizar
 * @param ticks: cantidad de tiempo a contar, en ticks
 * @param cb: NULL o callback a llamar en cada periodo
*/
void PITInit(PIT_MOD pit, PITTick_t ticks, PITCallback_t cb);

/**
 * @brief Inicia la cuenta del timer
 * @param pit: modulol PIT a iniciar
*/
void PITStart(PIT_MOD pit);

/**
 * @brief Detiene un timer en curso
 * @param pit: modulol PIT a deteenr
*/
void PITStop(PIT_MOD pit);


/*******************************************************************************
 ******************************************************************************/

#endif // _PIT_PIT_H_
