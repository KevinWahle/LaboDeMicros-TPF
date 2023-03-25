/***************************************************************************//**
  @file     filesystem.h
  @brief    +Descripcion del archivo+
  @author   KevinWahle
  @date		13 ene. 2023
 ******************************************************************************/

#ifndef FILESYSTEM_H_
#define FILESYSTEM_H_

/*******************************************************************************
 * INCLUDE HEADER FILES
 ******************************************************************************/
#include <stdbool.h>
#include <stdint.h>
/*******************************************************************************
 * CONSTANT AND MACRO DEFINITIONS USING #DEFINE
 ******************************************************************************/
#define BUFFER_SIZE (513U)


/*******************************************************************************
 * ENUMERATIONS AND STRUCTURES AND TYPEDEFS
 ******************************************************************************/
typedef struct {
	uint8_t g_bufferRead[BUFFER_SIZE];
	unsigned int bytes_read;
}data;

/*******************************************************************************
 * FUNCTION PROTOTYPES WITH GLOBAL SCOPE
 ******************************************************************************/

/**
 * @brief Initialize filesystem module
 * @return true if ok
*/
bool init_filesys (void); //Bloqueante

/**
 * @brief list al files and folders
 * @param dir directory to start listing (also their sub-dir)
 * @return true if ok
*/
bool list_file(char * dir);

/**
 * @brief starts mapping all files
 * @return true if ok
*/
bool statrt_mapping();

/**
 * @brief open folder
 * @return next file/folder name into folder. Return NULL if action can't be done
*/
char * open_folder();

/**
 * @brief close folder
 * @return next file/folder name in the previous folder. Return NULL if action can't be done
*/
char * close_folder();

/**
 * @brief opens file and reads data
 * @return file data
*/
data * open_file();

/**
 * @brief gives the path
 * @return path of actual file
*/
char * get_path();

/**
 * @brief go to next file/folder (circular)
 * @return next file/folder name. Return NULL if action can't be done
*/
char *show_next();

/**
 * @brief go to previous file/folder (circular)
 * @return previous file/folder name. Return NULL if action can't be done
*/
char *show_prev();


/*******************************************************************************
 ******************************************************************************/

#endif // _FILESYSTEM_H_
