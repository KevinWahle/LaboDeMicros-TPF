/***************************************************************************//**
  @file		Matrix.c
  @brief	HAL de la matrix de LEDs
  @author	Grupo 5
  @date		28 Dic. 2022
 ******************************************************************************/

#ifndef _MATRIX_H_
#define _MATRIX_H_

/*******************************************************************************
 * INCLUDE HEADER FILES
 ******************************************************************************/
#include <stdint.h>


/*******************************************************************************
 * CONSTANT AND MACRO DEFINITIONS USING #DEFINE
 ******************************************************************************/
#define LEDS_CANT   64
#define RESET_CANT  2

/*******************************************************************************
 * ENUMERATIONS AND STRUCTURES AND TYPEDEFS
 ******************************************************************************/
typedef uint16_t PWM_DUTY_t;

typedef struct
{
    uint8_t red;
    uint8_t blue;
    uint8_t green;
} LED_RGB;

/*******************************************************************************
 * VARIABLE PROTOTYPES WITH GLOBAL SCOPE
 ******************************************************************************/

// +ej: extern unsigned int anio_actual;+


/*******************************************************************************
 * FUNCTION PROTOTYPES WITH GLOBAL SCOPE
 ******************************************************************************/
void initMatrix();

// Incrementa o decrementa el brillo de la matriz
void increase_bright();
void decrease_bright();

// Getter del parametro de brillo
uint8_t get_bright();

// Prender toda la matrix en blanco con el brightness local
void fullMatrixON();

// Prende columnas de la matriz de leds
// columnsValues: arreglo de 8 uint8_t que indica la cantidad de 
//led a prender por columna
void setColumnsMatrix(uint8_t* columnsValues);

// Prende una columna puntual de la matriz de leds
// col va de 0 a 7 y value de 0 a 8 ambos incluidos
void setColumnMatrix(uint8_t col, uint8_t value);

// Prende un led de la matriz
void setLedMatrix(uint8_t led, LED_RGB* color);

// Apaga la matriz
void clearMatrix();
/*******************************************************************************
 ******************************************************************************/

#endif // _MATRIX_H_
