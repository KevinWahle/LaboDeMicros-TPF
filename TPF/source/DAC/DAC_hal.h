/***************************************************************************//**
  @file     DAC_hal.h
  @brief    HAL del DAC
  @author   Grupo 5
  @date		  22 oct. 2022
 ******************************************************************************/

#ifndef DAC_DAC_HAL_H_
#define DAC_DAC_HAL_H_

/*******************************************************************************
 * INCLUDE HEADER FILES
 ******************************************************************************/
#include <stdint.h>
#include "../buffer/circular_buffer_16.h"


/*******************************************************************************
 * FUNCTION PROTOTYPES WITH GLOBAL SCOPE
 ******************************************************************************/

/**
 * @brief Initialize DAC
 * @param frec sample frecuency DAC in KHz
 * buff buffer to read data
*/
void DACh_Init (void);

/*******************************************************************************
 ******************************************************************************/

#endif // _DAC_DAC_HAL_H_
