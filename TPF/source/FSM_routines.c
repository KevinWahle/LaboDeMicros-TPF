/***************************************************************************//**
  @file     +FSM_routines.c+
  @brief    +Funciones que conforman las rutas de acción entre los diferentes
            estados de la FSM.+
  @author   +Grupo 5+
 ******************************************************************************/

/*******************************************************************************
 * INCLUDE HEADER FILES
 ******************************************************************************/

#include "FSM_routines.h"
#include "event_queue/event_queue.h"
#include "encoder/encoder_hal.h"
#include "const.h"
#include "timer/timer.h"
#include <stdio.h>

/*******************************************************************************
 * CONSTANT AND MACRO DEFINITIONS USING #DEFINE
 ******************************************************************************/

/*******************************************************************************
 * VARIABLES WITH GLOBAL SCOPE
 ******************************************************************************/

MENU_ITEM main_menu[] = {  
                            {.option = "LEER SD", .ID = LOAD_SD},
                            {.option = "ECUALIZACION", .ID = EQUILIZATION},
                            {.option = "AJUS. BRILLO", .ID = 	ADJUST_BRIGHT},
                            {.option = "REPRODUCIR", .ID = SONG_SELECTION},
                        };

/*******************************************************************************
 * STATIC VARIABLES AND CONST VARIABLES WITH FILE LEVEL SCOPE
 ******************************************************************************/
static tim_id_t idTimer;

static uint8_t actual_option = 0;           // Variable que marca la opcion del menú seleccionada.     

void update_display(uint8_t* arr, uint8_t counter, bool password);
void updateMenuDis(char* word);
void setIDTimer_cb();

/*******************************************************************************
 *******************************************************************************
                        GLOBAL FUNCTION DEFINITIONS
 *******************************************************************************
 ******************************************************************************/


/**********************************************************
*********************  VARIOUS   **************************
**********************************************************/
void doNothing() {
    return;
}

