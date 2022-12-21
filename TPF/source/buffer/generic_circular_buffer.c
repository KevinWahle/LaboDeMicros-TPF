/***************************************************************************//**
  @file     circular_buffer.c
  @brief    Funciones para manejo del buffer circular
  @author   Grupo 5
 ******************************************************************************/

#include "generic_circular_buffer.h"
#include <stdint.h>
#include <stdbool.h>

static uint8_t getCircularPointer(uint8_t index){
	return index % BUFFER_BYTES_SIZE;
}

void GCBinit(genericCircularBuffer * CB, uint8_t sizeDataType){
	CB->head = 0;
	CB->tail = 0;
	CB->sizeDataType = sizeDataType;
}

bool GCBisEmpty(genericCircularBuffer * CB){
	return CB->head == CB->tail;
}

void GCBputDataChain(genericCircularBuffer * CB, const void * dataChain, uint8_t AmountOfData){
	for (uint8_t i = 0; i < AmountOfData; ++i) {
		GCBputData(CB, &(((uint8_t*)dataChain)[i*CB->sizeDataType]));
	}

}

void GCBputData(genericCircularBuffer * CB, void* dataToPush){
	for (uint8_t i = 0; i < CB->sizeDataType; ++i) {
		CB->buffer[CB->head] = ((uint8_t*)dataToPush)[i];
		CB->head = getCircularPointer(++CB->head);
		if (CB->head == CB->tail) {
			CB->tail = getCircularPointer(CB->tail + CB->sizeDataType);
		}
	}
}


void GCBgetData(genericCircularBuffer * CB, void* dataReturn){
	if(CB->head != CB->tail){
		for (uint8_t i = 0; i < CB->sizeDataType; i++){
			((uint8_t*)dataReturn)[i] = CB->buffer[CB->tail];
			CB->tail = getCircularPointer(++CB->tail);
		}
	}
}

void GCBreset(genericCircularBuffer * CB){
	for (uint8_t i= 0; i < BUFFER_BYTES_SIZE; ++i){
		CB->buffer[i] = 0;
	}
	CB->tail = 0;
	CB->head = 0;
}


uint8_t GCBgetBufferState(genericCircularBuffer * CB){
	if(CB->head >= CB->tail)
		return (CB->head - CB->tail)/CB->sizeDataType;  // Deberia dar siempre entero
	else
		return (BUFFER_BYTES_SIZE - CB->tail + CB->head)/CB->sizeDataType;
}
/*
const uint8_t * CBgetData(circularBuffer * CB, uint8_t bytesLen){
}
*/

