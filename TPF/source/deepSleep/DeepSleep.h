/***************************************************************************//**
  @file     DeepSleep.h
  @brief    +Descripcion del archivo+
  @author   KevinWahle
  @date		3 ene. 2023
 ******************************************************************************/

#ifndef DEEPSLEEP_H_
#define DEEPSLEEP_H_

/*******************************************************************************
 * INCLUDE HEADER FILES
 ******************************************************************************/
#include <stdbool.h>

/*******************************************************************************
 * CONSTANT AND MACRO DEFINITIONS USING #DEFINE
 ******************************************************************************/



/*******************************************************************************
 * ENUMERATIONS AND STRUCTURES AND TYPEDEFS
 ******************************************************************************/


/*******************************************************************************
 * VARIABLE PROTOTYPES WITH GLOBAL SCOPE
 ******************************************************************************/

// +ej: extern unsigned int anio_actual;+


/*******************************************************************************
 * FUNCTION PROTOTYPES WITH GLOBAL SCOPE
 ******************************************************************************/

/**
 * @brief Initial configuration of sleep mode (LLS)
*/
void LLS_config (void);


/**
 * @brief LLS mode entry routine. Puts the processor into LLS mode from normal Run mode
*/
void LLS_start(void);


/**
 * @brief Check if btn is pressed
 * @return true if pressed
*/
bool is_onoff_btn(void);
/*******************************************************************************
 ******************************************************************************/

#endif // _DEEPSLEEP_H_
