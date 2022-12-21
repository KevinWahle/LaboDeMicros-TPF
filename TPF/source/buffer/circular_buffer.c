/***************************************************************************//**
  @file     circular_buffer.c
  @brief    Funciones para manejo del buffer circular
  @author   Grupo 5
 ******************************************************************************/

#include "circular_buffer.h"
#include "hardware.h"
#include <stdint.h>
#include <stdbool.h>

static uint8_t getCircularPointer(uint8_t index, uint8_t sizeInBytes){
	return index % sizeInBytes;
}

void CBinit(circularBuffer * CB, uint8_t sizeInBytes){
	CB->head = 0;
	CB->tail = 0;
	CB->sizeInBytes = sizeInBytes;
}

bool CBisEmpty(circularBuffer * CB){
	return CB->head == CB->tail;
}

void CBputChain(circularBuffer * CB, const void * data, uint8_t bytesLen){
	for (uint8_t i = 0; i < bytesLen; ++i) {
		CB->buffer[CB->head] = ((uint8_t*)data)[i];
		CB->head = getCircularPointer(++CB->head, CB->sizeInBytes);
		if (CB->head == CB->tail) {
			CB->tail = getCircularPointer(++CB->tail, CB->sizeInBytes);;
		}
	}

}

void CBputByte(circularBuffer * CB, uint8_t by){
	CB->buffer[CB->head] = by;
	CB->head = getCircularPointer(++CB->head, CB->sizeInBytes);
	if (CB->head == CB->tail) {
		CB->tail = getCircularPointer(++CB->tail, CB->sizeInBytes);;
	}
}

uint8_t CBgetByte(circularBuffer * CB){
	if(CB->head != CB->tail){
		uint8_t data = CB->buffer[CB->tail];
		CB->tail = getCircularPointer(++CB->tail, CB->sizeInBytes);
		return data;
	}
	return 0;
}

uint8_t CBgetBufferState(circularBuffer * CB){
	hw_DisableInterrupts();
	if(CB->head >= CB->tail) {
		uint8_t temp = CB->head - CB->tail;
		hw_EnableInterrupts();
		return temp;
	}
	else {
		uint8_t temp = CB->sizeInBytes - CB->tail + CB->head;
		hw_EnableInterrupts();
		return temp;
	}
}
/*
const uint8_t * CBgetData(circularBuffer * CB, uint8_t bytesLen){
}
*/
void CBreset(circularBuffer * CB){
	for (uint8_t i= 0; i < BUFFER_SIZE; ++i){
		CB->buffer[i] = 0;
	}
	CB->tail = 0;
	CB->head = 0;
}
