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
#include <stdbool.h>
typedef uint8_t Tx_msg;

/*******************************************************************************
 * CONSTANT AND MACRO DEFINITIONS USING #DEFINE
 ******************************************************************************/
#define DISPLAY_FPS 15
#define REFRESH_PERIOD_MS 1000/(DISPLAY_FPS)

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

// Escribe un solo caracter en la posicion pedida
void displayChar(int row, int column, char character);

// Limpia la copia local de la pantalla
void clearScreen();

// Limpia un renglon de la copia local de la pantalla
void clearLine(uint8_t row);
/*******************************************************************************
 * Funciones no muy usadas en la FSM
 ******************************************************************************/

// Col va de 0 a 7; row de 0 a 1
void setCursor(uint8_t col, uint8_t row);

void clearDisplay();
void displayOFF(void);
void displayON(void);
void cursorOFF(void);
void cursorON(void);
void blinkOFF(void);
void blinkON(void);
void backlightOFF(void);
void backlightON(void);

void ready2sleep();
void aftersleep();
/*******************************************************************************
 ******************************************************************************/

#endif // _DISPLAY_H_
