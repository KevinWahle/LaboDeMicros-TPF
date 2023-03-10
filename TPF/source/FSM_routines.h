/***************************************************************************//**
  @file     +FSM_routines.h+
  @brief    +Funciones que conforman las rutas de acción entre los diferentes
            estados de la FSM.+
  @author   +Grupo 5+
 ******************************************************************************/

#ifndef _FSM_ROUTINES_H_
#define _FSM_ROUTINES_H_

/*******************************************************************************
 * INCLUDE HEADER FILES
 ******************************************************************************/
#include "encoder/encoder_hal.h"
#include "stdint.h"
#include "const.h"

#include <stdbool.h>

/*******************************************************************************
 * FUNCTION PROTOTYPES WITH GLOBAL SCOPE
 ******************************************************************************/

/**********************************************************
*******************   Splash Screen    ********************
**********************************************************/

/**********************************************************
*******************   Main Menu    ************************
**********************************************************/

// Actualiza el menu principal
void update_menu();

// Baja en el menu
void down_menu();

// Sube en el menu
void up_menu();

// Handler de eventos del menu
void sel_menu();

void update_sel_menu();
/**********************************************************
*****************   Equalizer Menu    *********************
**********************************************************/
void update_eq_menu();
void down_eq();
void up_eq();
void sel_eq();

/**********************************************************
******************   Matrix Bright    *********************
**********************************************************/
void update_bright();
void inc_brightness();
void dec_brightness();
void sel_brightness();

/**********************************************************
************************  SD   ****************************
**********************************************************/


/**********************************************************
******************  SELECTION MENU   **********************
**********************************************************/
void loadFileSystem();
void last_song();
void next_song();
void sel_option();
void back_song();

/**********************************************************
******************    INFO SCREEN    **********************
**********************************************************/
void load_info();

/**********************************************************
******************    VOLUME CTRL    **********************
**********************************************************/
void inc_vol();
void dec_vol();
void vol_inc_ss();
void vol_dec_ss();
void vol_inc_si();
void vol_dec_si();
void vol_inc_menu();
void vol_dec_menu();
void vol_last_state();

/**********************************************************
**********************   SLEEP   **************************
**********************************************************/
void go_sleep();

/**********************************************************
******************    ERROR CTRL    **********************
**********************************************************/
void error_last_state();

/**********************************************************
*********************  VARIOUS   **************************
**********************************************************/
void addTimeout();
void toggle_state();
bool check_SD();
void doNothing();

/**********************************************************
********************  CALLBACKS  *************************
**********************************************************/

void encoderCallback(ENC_STATE state);

/*******************************************************************************
 ******************************************************************************/

#endif // _FSM_ROUTINES_H_
