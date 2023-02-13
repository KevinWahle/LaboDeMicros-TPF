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
#define COLS_CANT   8
#define ROWS_CANT   8

#define MATRIX_FPS    20
#define MATRIX_PERIOD 1/MATRIX_FPS
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
 * FUNCTION PROTOTYPES WITH GLOBAL SCOPE
 ******************************************************************************/
/// @brief Inicializa la matriz
void initMatrix();

/// @brief Incrementa internamente el brillo de la matriz
void increase_bright();

/// @brief Incrementa internamente el brillo de la matriz
void decrease_bright();

/// @brief getter del brillo de la matriz
/// @return uint8_t con el brillo de la matriz
uint8_t get_bright();

/// @brief Prende todas la matriz
void fullMatrixON();

/// @brief Setea los leds de todas las columnas
/// @param columnsValues uint8_t[8] de valores entre 0 y 8
void setColumnsMatrix(uint8_t* columnsValues);


/// @brief setea los leds de una columna
/// @param col entre 0 y 7
/// @param value entre 0 y 8
void setColumnMatrix(uint8_t col, uint8_t value);

/// @brief Prende un led de la matriz.
/// @param fila entre 0 y 7
/// @param columna entre 0 y 7
/// @param color configurar en una LED_RGB
void setLedMatrix(uint8_t fila, uint8_t columna, LED_RGB* color);

/// @brief Apaga la matriz.
/// @param init =0 si no estamos en la inicializacion
void clearMatrix(uint8_t init);

/*******************************************************************************
 ******************************************************************************/

#endif // _MATRIX_H_
