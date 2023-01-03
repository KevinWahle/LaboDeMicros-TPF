/***************************************************************************//**
  @file     SD.h
  @brief    Funciones de la SD
  @author   Grupo 5
  @date		25 dic. 2022
 ******************************************************************************/

#ifndef _SD_SD_H_
#define _SD_SD_H_

/*******************************************************************************
 * INCLUDE HEADER FILES
 ******************************************************************************/

#include "FileSystem/ff15/source/diskio.h"

/*******************************************************************************
 * CONSTANT AND MACRO DEFINITIONS USING #DEFINE
 ******************************************************************************/

#define SD_DISK	0

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

DSTATUS SD_disk_status (
  BYTE pdrv     /* [IN] Physical drive number */
);


DSTATUS SD_disk_initialize (
  BYTE pdrv           /* [IN] Physical drive number */
);

DRESULT SD_disk_read (
  BYTE pdrv,     /* [IN] Physical drive number */
  BYTE* buff,    /* [OUT] Pointer to the read data buffer */
  LBA_t sector,  /* [IN] Start sector number */
  UINT count     /* [IN] Number of sectros to read */
);

/*******************************************************************************
 ******************************************************************************/

#endif // _SD_SD_H_
