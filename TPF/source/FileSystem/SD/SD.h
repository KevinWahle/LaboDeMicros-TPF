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
#include <stdbool.h>

/*******************************************************************************
 * CONSTANT AND MACRO DEFINITIONS USING #DEFINE
 ******************************************************************************/

#define SD_DISK	1

/*******************************************************************************
 * FUNCTION PROTOTYPES WITH GLOBAL SCOPE
 ******************************************************************************/

// Hardware initialization. Called inside SD_disk_status if necessary.
// @return true if error
bool SDInit();

// Return true if card is present
// SDInit must be called before calling isSDCardInserted
bool isSDCardInserted();

DSTATUS SD_disk_status ();

DSTATUS SD_disk_initialize ();

DRESULT SD_disk_read (
  BYTE* buff,    /* [OUT] Pointer to the read data buffer */
  LBA_t sector,  /* [IN] Start sector number */
  UINT count     /* [IN] Number of sectros to read */
);

/*******************************************************************************
 ******************************************************************************/

#endif // _SD_SD_H_
