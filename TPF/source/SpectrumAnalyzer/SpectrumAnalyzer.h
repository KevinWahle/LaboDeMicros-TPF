/***************************************************************************//**
  @file     SPECTRUMANALYZER.h
  @brief    Funciones para manejo spec
  @author   Grupo 5
  @date		27 dic. 2022
 ******************************************************************************/

#ifndef _SPECTRUM_ANALYZER_H_
#define _SPECTRUM_ANALYZER_H_
/*******************************************************************************
 * INCLUDE HEADER FILES
 ******************************************************************************/
#include <arm_math.h>
/*******************************************************************************
 * CONSTANT AND MACRO DEFINITIONS USING #DEFINE
 ******************************************************************************/

/*******************************************************************************
 * ENUMERATIONS AND STRUCTURES AND TYPEDEFS
 ******************************************************************************/

/*******************************************************************************
 * VARIABLE PROTOTYPES WITH GLOBAL SCOPE
 ******************************************************************************/

/*******************************************************************************
 * FUNCTION PROTOTYPES WITH GLOBAL SCOPE
 ******************************************************************************/
void startAnalyzer(float32_t * tableStart, uint16_t lenData);
void getAnalyzer(uint8_t data [8]);


void initEqualizer(void);

void setUpFilter(float32_t nuevaGananciadB, uint8_t nroBanda);

void blockEqualizer(const float32_t * pSrc, float32_t * pDst, uint32_t 	blockSize);



void set_pop();

void set_classic();

void set_rock();

void set_techno();
/*******************************************************************************
 ******************************************************************************/

#endif // _SPECTRUM_ANALYZER_H_
