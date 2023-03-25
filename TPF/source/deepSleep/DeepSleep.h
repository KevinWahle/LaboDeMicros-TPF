/***************************************************************************//**
  @file     DeepSleep.h
  @brief    Funciones para entrar en bajo consumo
  @author   Grupo 5
  @date		  3 ene. 2023
 ******************************************************************************/

#ifndef DEEPSLEEP_H_
#define DEEPSLEEP_H_

/*******************************************************************************
 * INCLUDE HEADER FILES
 ******************************************************************************/
#include <stdbool.h>


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
