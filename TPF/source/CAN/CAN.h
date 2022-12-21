/***************************************************************************//**
  @file     CAN.h
  @brief    HAL para comunicación CAN
  @author   Nicolás Magliola
 ******************************************************************************/

#ifndef _CAN_H_
#define _CAN_H_

/*******************************************************************************
 * INCLUDE HEADER FILES
 ******************************************************************************/
#include <stdint.h>
/*******************************************************************************
 * CONSTANT AND MACRO DEFINITIONS USING #DEFINE
 ******************************************************************************/

/*******************************************************************************
 * FUNCTION PROTOTYPES WITH GLOBAL SCOPE
 ******************************************************************************/
typedef struct
{
  uint16_t ID;
  uint8_t length;
  uint8_t data [8];
}CANMsg_t;

/**
 * @brief Set up del CAN
 * @param ID Id con el que se visualizará a la placa
 * @param msgReceive Donde iran los parametros leídos
*/
void CANInit(uint16_t ID, CANMsg_t* msgReceive);

/**
 * @brief Envío de mensaje CAN
 * @param data Data a envíar
 * @param len Largo de la Data
 * @return True si los parámetros son correctos
*/
bool CANSend(uint8_t * data, uint8_t len);

/**
 * @brief Verificación de recepción de mensaje CAN
 * @return True si llegó un mensaje
*/
bool newMsg();

/*******************************************************************************
 ******************************************************************************/

#endif // _CAN_H_
