/***************************************************************************//**
  @file     circular_buffer.h
  @brief    Funciones para manejo del buffer circular
  @author   Grupo 5
 ******************************************************************************/


#ifndef _CIRCULAR_BUFFER_H_
#define _CIRCULAR_BUFFER_H_


/*******************************************************************************
 * INCLUDE HEADER FILES
 ******************************************************************************/
#include <stdbool.h>
#include <stdint.h>

#define BUFFER_SIZE 120  // 12 bytes buffer size JUST 11 bytes can be used for store data without deleting older bytes

/*******************************************************************************
 * ENUMERATIONS AND STRUCTURES AND TYPEDEFS
 ******************************************************************************/

typedef struct{
	uint8_t buffer[BUFFER_SIZE];

	//private
	uint8_t head;
	uint8_t tail;
	uint8_t sizeInBytes;
} circularBuffer;

/*******************************************************************************
 * FUNCTION PROTOTYPES WITH GLOBAL SCOPE
 ******************************************************************************/

/**
 * @brief builder
 * @param circularBuffer type
 */
void CBinit(circularBuffer * CB, uint8_t sizeInBytes);

/**
 * @brief tells you whether there is new data
 * @param circularBuffer type
 */
bool CBisEmpty(circularBuffer * CB);

/**
 * @brief push a string into the buffer
 * @param circularBuffer type, data, data length in bytes
 */
void CBputChain(circularBuffer * CB, const void * data, uint8_t bytesLen);

/**
 * @brief push a byte into the buffer
 * @param circularBuffer type, data
 */
void CBputByte(circularBuffer * CB, uint8_t by);

/**
 * @brief gets a byte from the buffer
 * @param circularBuffer type
 * @return the byte or BUFFER_FULL special byte
 */
uint8_t CBgetByte(circularBuffer * CB);

/**
 * @brief tells you the amount of unread bytes
 * @param circularBuffer type
 */
uint8_t CBgetBufferState(circularBuffer * CB);

/** UNIMPLEMENTED
 * @brief gets a chain of bytes
 * @param
 */

//const uint8_t *  CBgetData(circularBuffer * CB, uint8_t bytesLen);

/**
 * @brief Set all bytes to 0
 * @param circularBuffer type
 */
void CBreset(circularBuffer * CB);

/*******************************************************************************
 ******************************************************************************/

#endif // _CIRCULAR_BUFFER_H_
