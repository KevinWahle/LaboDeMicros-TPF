/***************************************************************************//**
  @file     FTM.h
  @brief    Funciones para manejo de timer FTM
  @author   Grupo 5
  @date		17 oct. 2022
 ******************************************************************************/

#ifndef _FTM_H_
#define _FTM_H_

/*******************************************************************************
 * INCLUDE HEADER FILES
 ******************************************************************************/

#include <stdint.h>
#include <stdbool.h>

/*******************************************************************************
 * CONSTANT AND MACRO DEFINITIONS USING #DEFINE
 ******************************************************************************/

#define FTM_CH_COUNT	8

#define FTM_TICK_TIME	20	// ns
#define FTM_TICK2NS(x)	((x)*FTM_TICK_TIME)			// ns
#define FTM_TICK2US(x)	((x)*FTM_TICK_TIME/1000)			// us
#define FTM_TICK2MS(x)	((x)*FTM_TICK_TIME/1000000)	// ms

/*******************************************************************************
 * ENUMERATIONS AND STRUCTURES AND TYPEDEFS
 ******************************************************************************/

typedef enum {FTM_0, FTM_1, FTM_2, FTM_3, FTM_COUNT} FTM_MODULE;
typedef uint8_t FTM_CHANNEL;

typedef uintmax_t FTM_tick_t;

typedef enum {CAPTURE_RISING=0x1, CAPTURE_FALLING, CAPTURE_BOTH} IC_CAPTURE_EDGE;

//typedef void (*callbackICEdge) (FTM_tick_t ticks);
typedef void callbackICEdge (FTM_tick_t ticks);

/*******************************************************************************
 * VARIABLE PROTOTYPES WITH GLOBAL SCOPE
 ******************************************************************************/

// +ej: extern unsigned int anio_actual;+


/*******************************************************************************
 * FUNCTION PROTOTYPES WITH GLOBAL SCOPE
 ******************************************************************************/

/**
 * @brief Reinicia el contador
 * @param ftm: módulo FTM
*/
void FTMReset(FTM_MODULE ftm);

////// PWM //////

/**
 * @brief Inicializa un canal de PWM
 * @param ftm: módulo FTM
 * @param channel: Canal del modulo FTM
 * @param freq: frecuencua del PWM (763 - 50000000) Hz
 * @obs Todos los canales de un mismo modulo FTM comparten la frecuencia
*/
void PWMInit(FTM_MODULE ftm, FTM_CHANNEL channel, uint32_t freq);

/**
 * @brief Activa la salida PWM de un canal con el duty deseado
 * @param ftm: módulo FTM
 * @param channel: Canal del modulo FTM
 * @param duty: duty cycle del PWM (0.0 - 1.0)
*/
void PWMStart(FTM_MODULE ftm, FTM_CHANNEL channel, double duty);


/**
 * @brief Recibe el duty a traves de un doble puntero
 * @param ftm: módulo FTM
 * @param channel: Canal del modulo FTM
 * @param ptr: direccion del puntero que apuntaa al duty actual. (12 bits). NULL para desactivar
*/
void PWMFromPtr(FTM_MODULE ftm, FTM_CHANNEL channel, uint16_t** ptr);


////// Input Capture //////

/**
 * @brief Inicializa un canal FTM en Input Capture
 * @param ftm: módulo FTM
 * @param channel: Canal del modulo FTM
 * @param edge: capture edge
 * @param edgeCb: NULL o callback que se llama en cada flanco activo con la cantidad de ticks desde el ultimo.
*/
void ICInit(FTM_MODULE ftm, FTM_CHANNEL channel, IC_CAPTURE_EDGE edge, callbackICEdge edgeCb);

/**
 * @brief Devuelve el tiempo entre los ultimos flancos activos
 * @param ftm: módulo FTM
 * @param channel: Canal del modulo FTM
 * @return valore del contador del canal
*/
FTM_tick_t ICGetCont(FTM_MODULE ftm, FTM_CHANNEL channel);

/**
 * @brief Devuelve si hay un nuevo flanco detectado
 * @param ftm: módulo FTM
 * @param channel: Canal del modulo FTM
 * @return true si hay un nuevo flanco
*/
bool ICisEdge(FTM_MODULE ftm, FTM_CHANNEL channel);

/**
 * @brief Reinicia el contador y el valor del canal
 * @param ftm: módulo FTM
 * @param channel: Canal del modulo FTM
*/
void ICReset(FTM_MODULE ftm, FTM_CHANNEL channel);

/*******************************************************************************
 ******************************************************************************/

#endif // _FTM_H_
