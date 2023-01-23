/***************************************************************************//**
  @file     Mp3Dec.h
  @brief    Funciones del decodificador MP3
  @author   Grupo 5
  @date		21 dic. 2022
 ******************************************************************************/

#ifndef _MP3DEC_H_
#define _MP3DEC_H_

/*******************************************************************************
 * INCLUDE HEADER FILES
 ******************************************************************************/

#include <stdbool.h>
#include "FileSystem/ff15/source/ff.h"

/*******************************************************************************
 * CONSTANT AND MACRO DEFINITIONS USING #DEFINE
 ******************************************************************************/



/*******************************************************************************
 * ENUMERATIONS AND STRUCTURES AND TYPEDEFS
 ******************************************************************************/


/*******************************************************************************
 * VARIABLE PROTOTYPES WITH GLOBAL SCOPE
 ******************************************************************************/

// +ej: extern unsigned int anio_actual;+


/*******************************************************************************
 * FUNCTION PROTOTYPES WITH GLOBAL SCOPE
 ******************************************************************************/

/**
 * @brief Inicializa funciones del decodificador MP3
 * @return Devuelve 0 si no hubo error
*/
bool MP3DecInit();

/**
 * @brief TODO: completar descripcion
 * @param param1 Descripcion parametro 1
 * @param param2 Descripcion parametro 2
 * @return Descripcion valor que devuelve
*/
void MP3PlaySong(FIL* mp3File);


/*******************************************************************************
 ******************************************************************************/

#endif // _MP3DEC_H_
