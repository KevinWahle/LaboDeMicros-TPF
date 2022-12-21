/***************************************************************************//**
  @file     circular_buffer.h
  @brief    Funciones para manejo del buffer circular
  @author   Grupo 5
 ******************************************************************************/


#ifndef _CIRCULAR_BUFFER16_H_
#define _CIRCULAR_BUFFER16_H_


/*******************************************************************************
 * INCLUDE HEADER FILES
 ******************************************************************************/
#include <stdbool.h>
#include <stdint.h>

#define BUFFER_SIZE_16 200  // 12 bytes buffer size JUST 11 bytes can be used for store data without deleting older bytes

/*******************************************************************************
 * ENUMERATIONS AND STRUCTURES AND TYPEDEFS
 ******************************************************************************/

typedef struct{
	uint16_t buffer[BUFFER_SIZE_16];

	//private
	uint16_t head;
	uint16_t tail;
	uint16_t sizeInBytes;
} circularBuffer16;

/*******************************************************************************
 * FUNCTION PROTOTYPES WITH GLOBAL SCOPE
 ******************************************************************************/

/**
 * @brief builder
 * @param circularBuffer16 type
 */
void CBinit16(circularBuffer16 * CB, uint16_t sizeInBytes);

/**
 * @brief tells you whether there is new data
 * @param circularBuffer1616 type
 */
bool CBisEmpty16(circularBuffer16 * CB);

/**
 * @brief push a string into the buffer
 * @param circularBuffer16 type, data, data length in bytes
 */
void CBputChain16(circularBuffer16 * CB, const void * data, uint16_t bytesLen);

/**
 * @brief push a byte into the buffer
 * @param circularBuffer16 type, data
 */
void CBputByte16(circularBuffer16 * CB, uint16_t by);

/**
 * @brief gets a byte from the buffer
 * @param circularBuffer16 type
 * @return the byte or BUFFER_FULL special byte
 */
uint16_t CBgetByte16(circularBuffer16 * CB);

/**
 * @brief tells you the amount of unread bytes
 * @param circularBuffer16 type
 */
uint16_t CBgetBufferState16(circularBuffer16 * CB);

/** UNIMPLEMENTED
 * @brief gets a chain of bytes
 * @param
 */

//const uint16_t *  CBgetData(circularBuffer16 * CB, uint16_t bytesLen);

/**
 * @brief Set all bytes to 0
 * @param circularBuffer16 type
 */
void CBreset16 (circularBuffer16 * CB);

/*******************************************************************************
 ******************************************************************************/

#endif // _CIRCULAR_BUFFER_H_
