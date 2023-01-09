/***************************************************************************//**
  @file		Display.h
  @brief	HAL Display I2C
  @author	Grupo 5
  @date		29 Dic. 2022
 ******************************************************************************/

#ifndef _DISPLAY_H_
#define _DISPLAY_H_

/*******************************************************************************
 * INCLUDE HEADER FILES
 ******************************************************************************/
#include <stdint.h>
/*******************************************************************************
 * CONSTANT AND MACRO DEFINITIONS USING #DEFINE
 ******************************************************************************/
#define I2C_ID		3
#define DIS_ADDR    0x27
#define CANT_ROWS   2
#define CANT_COLS   16

/*******************************************************************************
 * FUNCTION PROTOTYPES WITH GLOBAL SCOPE
 ******************************************************************************/
void initDisplay();

// Muestra todo el contenido de text en una de la lineas
void displayLine(int row, char* text);

// Muestra text en una linea dada arrancando en position.
// Corta cuando se termina la línea
void displayText(int row, int column, char* text);

void clearDisplay();

/*******************************************************************************
 * Funciones no muy usadas en la FSM
 ******************************************************************************/

// Col va de 0 a 7; row de 0 a 1
void setCursor(uint8_t col, uint8_t row);

void displayOFF(void);
void displayON(void);
void cursorOFF(void);
void cursorON(void);
void blinkOFF(void);
void blinkON(void);
void backlightOFF(void);
void backlightON(void);

/*******************************************************************************
 ******************************************************************************/

#endif // _DISPLAY_H_
