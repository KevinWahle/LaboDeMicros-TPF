/***************************************************************************//**
  @file     circular_buffer.c
  @brief    Funciones para manejo del buffer circular
  @author   Grupo 5
 ******************************************************************************/

#include "circular_buffer_16.h"
#include <stdint.h>
#include <stdbool.h>

static uint16_t getCircularPointer(uint16_t index, uint16_t sizeInBytes){
	return index % sizeInBytes;
}

void CBinit16(circularBuffer16 * CB, uint16_t sizeInBytes){
	CB->head = 0;
	CB->tail = 0;
	CB->sizeInBytes = sizeInBytes;
}

bool CBisEmpty16(circularBuffer16 * CB){
	return CB->head == CB->tail;
}

void CBputChain16(circularBuffer16 * CB, const void * data, uint16_t bytesLen){
	for (uint16_t i = 0; i < bytesLen; ++i) {
		CB->buffer[CB->head] = ((uint16_t*)data)[i];
		CB->head = getCircularPointer(++CB->head, CB->sizeInBytes);
		if (CB->head == CB->tail) {
			CB->tail = getCircularPointer(++CB->tail, CB->sizeInBytes);;
		}
	}

}

void CBputByte16(circularBuffer16 * CB, uint16_t by){
	CB->buffer[CB->head] = by;
	CB->head = getCircularPointer(++CB->head, CB->sizeInBytes);
	if (CB->head == CB->tail) {
		CB->tail = getCircularPointer(++CB->tail, CB->sizeInBytes);;
	}
}

uint16_t CBgetByte16(circularBuffer16 * CB){
	if(CB->head != CB->tail){
		uint16_t data = CB->buffer[CB->tail];
		CB->tail = getCircularPointer(++CB->tail, CB->sizeInBytes);
		return data;
	}
	return 0;
}

uint16_t CBgetBufferState16(circularBuffer16 * CB){
	if(CB->head >= CB->tail)
		return CB->head - CB->tail;
	else
		return CB->sizeInBytes - CB->tail + CB->head;
}
/*
const uint16_t * CBgetData(circularBuffer16 * CB, uint16_t bytesLen){
}
*/
void CBreset16(circularBuffer16 * CB){
	for (uint16_t i= 0; i < BUFFER_SIZE_16; ++i){
		CB->buffer[i] = 0;
	}
	CB->tail = 0;
	CB->head = 0;
}
