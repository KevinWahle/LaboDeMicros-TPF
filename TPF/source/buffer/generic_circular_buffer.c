/***************************************************************************//**
  @file     circular_buffer.c
  @brief    Funciones para manejo del buffer circular
  @author   Grupo 5
 ******************************************************************************/

#include "generic_circular_buffer.h"
#include <stdint.h>
#include <stdbool.h>
#include "hardware.h"

static uint16_t getCircularPointer(genericCircularBuffer * CB, uint16_t index){
	return index % CB->newBufferSize;
}

bool GCBinit(genericCircularBuffer * CB, uint8_t sizeDataType, uint16_t amount){
	if(amount*sizeDataType > BUFFER_BYTES_SIZE)
		return false;
	CB->head = 0;
	CB->tail = 0;
	CB->sizeDataType = sizeDataType;
	CB->newBufferSize = amount * sizeDataType;
	return true;
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
		CB->head = getCircularPointer(CB, ++CB->head);
		if (CB->head == CB->tail) {
			CB->tail = getCircularPointer(CB, CB->tail + CB->sizeDataType);
		}
	}

}


void GCBgetData(genericCircularBuffer * CB, void* dataReturn){

	if(CB->head != CB->tail){
		for (uint8_t i = 0; i < CB->sizeDataType; i++){
			((uint8_t*)dataReturn)[i] = CB->buffer[CB->tail];
			CB->tail = getCircularPointer(CB, ++CB->tail);
		}
	}

}

void GCBreset(genericCircularBuffer * CB){
	for (uint8_t i= 0; i < CB->newBufferSize; ++i){
		CB->buffer[i] = 0;
	}
	CB->tail = 0;
	CB->head = 0;
}


uint16_t GCBgetBufferState(genericCircularBuffer * CB){
	if(CB->head >= CB->tail)
		return (CB->head - CB->tail)/CB->sizeDataType;  // Deberia dar siempre entero
	else
		return (CB->newBufferSize - CB->tail + CB->head)/CB->sizeDataType;
}
/*
const uint8_t * CBgetData(circularBuffer * CB, uint8_t bytesLen){
}
*/
