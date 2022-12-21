/***************************************************************************//**
  @file     I2C.h
  @brief    I2C Functions
  @author   Grupo 5
  @date		13 sep. 2022
 ******************************************************************************/

#ifndef _I2Cm_H_
#define _I2Cm_H_

/*******************************************************************************
 * INCLUDE HEADER FILES
 ******************************************************************************/

#include <stdbool.h>
#include <stdint.h>

/*******************************************************************************
 * CONSTANT AND MACRO DEFINITIONS USING #DEFINE
 ******************************************************************************/


/*******************************************************************************
 * ENUMERATIONS AND STRUCTURES AND TYPEDEFS
 ******************************************************************************/

// I2C_Acc uses I2C_0 with specific pins
typedef enum {I2C_0, I2C_1, I2C_2, I2C_ACC} I2CPort_t;

/*******************************************************************************
 * VARIABLE PROTOTYPES WITH GLOBAL SCOPE
 ******************************************************************************/

// +ej: extern unsigned int anio_actual;+


/*******************************************************************************
 * FUNCTION PROTOTYPES WITH GLOBAL SCOPE
 ******************************************************************************/

/**
 * @brief Inicializa el modulo I2C
 * @param id: Instancia del I2C [0 - 1]
*/
void I2CmInit(I2CPort_t id);

/**
 * @brief ASYNC. realiza una transmision y recepcion po I2C
 * @param address address del slave
 * @param writeBuffer buffer de escritura
 * @param writeSize Tamano del buffer de escritura
 * @param readBuffer buffer para guardar la lectura
 * @param readSize Tamano del buffer de lectura
*/
bool I2CmStartTransaction(I2CPort_t id, uint8_t address, uint8_t* writeBuffer, uint8_t writeSize, uint8_t* readBuffer, uint8_t readSize);
/**
 * @brief tells you whether the bus is busy
 * @return true if bus is busy
*/
bool isI2CBusy(I2CPort_t id);

/*******************************************************************************
 ******************************************************************************/

#endif // _I2Cm_H_
