/***************************************************************************//**
  @file     SPECTRUMANALYZER.h
  @brief    Funciones para manejo de DMA
  @author   Grupo 5
  @date		29 dic. 2022
 ******************************************************************************/

#ifndef _DMA2_H_
#define _DMA2_H_
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

/*******************************************************************************
 * VARIABLE PROTOTYPES WITH GLOBAL SCOPE
 ******************************************************************************/
//1584
/*******************************************************************************
 * FUNCTION PROTOTYPES WITH GLOBAL SCOPE
 ******************************************************************************/
void DMA_initPingPong_Dac();
void DMA_pingPong_DAC(uint32_t memDirTable1, uint32_t memDirTable2, uint32_t dacAddress,uint16_t tableSize);
uint16_t* DMA_availableTable_pingPong();   // EL puntero no es local a esta funcion, es el puntero cargado por void DMA_pingPong_DAC
void DMA_pause_pingPong();
void DMA_continue_pingPong();


void DMA_initDisplayTable(uint32_t memDirTable);
void DMA_displayTable();
bool DMA_status();
/*******************************************************************************
 ******************************************************************************/

#endif // _DMA2_H_
