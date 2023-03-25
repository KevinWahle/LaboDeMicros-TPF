/***************************************************************************//**
  @file     keypad.h
  @brief    Keypad functions
  @author   Grupo 5
  @date		13 sep. 2022
 ******************************************************************************/

#ifndef _KEYPAD_H_
#define _KEYPAD_H_

/*******************************************************************************
 * INCLUDE HEADER FILES
 ******************************************************************************/
#include <stdint.h>


/*******************************************************************************
 * ENUMERATIONS AND STRUCTURES AND TYPEDEFS
 ******************************************************************************/
typedef bool (*fun_ptr)(uint8_t event);

/*******************************************************************************
 * FUNCTION PROTOTYPES WITH GLOBAL SCOPE
 ******************************************************************************/
void keypadInit(fun_ptr event);

/*******************************************************************************
 ******************************************************************************/

#endif // _KEYPAD_H_
