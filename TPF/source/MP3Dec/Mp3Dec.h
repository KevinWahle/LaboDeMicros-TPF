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
#include <stdint.h>

/*******************************************************************************
 * CONSTANT AND MACRO DEFINITIONS USING #DEFINE
 ******************************************************************************/

// Cantidad de MUESTRAS en un frame
// Obs: Cada muestra ocupa 2 bytes
#define OUTBUFF_SIZE	1152U


/*******************************************************************************
 * FUNCTION PROTOTYPES WITH GLOBAL SCOPE
 ******************************************************************************/

/**
 * @brief Inicializa funciones del decodificador MP3
 * @return Devuelve 0 si no hubo error
*/
bool MP3DecInit();

/**
 * @brief Selecciona la cancion a reproducir
 * @param filePath: Path to mp3 file
 * @return Devuelve 0 si no hubo error
*/
bool MP3SelectSong(char* filePath);


/**
 * @brief Decodifica el siguiente frame de la cancion seleccionada
 * @param outBuff: Arreglo con al menos OUTBUFF_SIZE bytes disponibles, donde se guarda la salida decodificada
 * @return Cantidad de bytes escritos en outBuff (<= OUTBUFF_SIZE). 0 si hubo error.
*/
uint16_t MP3DecNextFrame(int16_t* outBuff);

/*******************************************************************************
 ******************************************************************************/

#endif // _MP3DEC_H_
