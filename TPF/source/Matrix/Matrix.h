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

#define MATRIX_FPS		20		// NO TOCAR
#define SONG_TIMEOUT	5000	//En ms
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


// Prende un led de la matriz. tanto fila como columna van de 0 a 7
void setLedMatrix(uint8_t fila, uint8_t columna, LED_RGB* color);

// Apaga la matriz.
// init=0 si no estamos dentro de la inicializacion
void clearMatrix();

uint8_t isVumeterMode();
/*******************************************************************************
 ******************************************************************************/

#endif // _MATRIX_H_
