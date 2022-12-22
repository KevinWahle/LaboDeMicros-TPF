/*******************************************************************************
  @file     +FSM_table.h+
  @brief    +Tabla de estados de la maquina de estados+
  @author   +Grupo 5+
 ******************************************************************************/

#ifndef FSM_TABLE_H
#define FSM_TABLE_H

/*******************************************************************************
 * INCLUDE HEADER FILES
 ******************************************************************************/

#include "FSM_routines.h"
#include "event_queue/event_queue.h"
#include "encoder/encoder_hal.h"

/*******************************************************************************
 * ENUMERATIONS AND STRUCTURES AND TYPEDEFS
 *******************************************************************************/

typedef struct state_diagram_edge STATE;

struct state_diagram_edge {
    event_t evento;
	//Evento que activa la rutina de acción

    STATE *proximo_estado;
    // Puntero que apunta al estado al que hay que ir en caso
    //que se cumpla el evento correspondiente. 

    void (*p_rut_accion)(void);
    // Función a realizar durante la transición entre un estado y otro.
};



STATE *fsm_interprete(STATE * p_tabla_estado_actual, event_t evento_actual);

extern STATE sleep_state[];
extern STATE menu_state[];
extern STATE eq_state[];
extern STATE bright_state[];
extern STATE sel_song_state[];
extern STATE info_song_state[];

STATE menu_state[] = {          
    {ENCODER_LEFT, menu_state, upper_menu},
    {ENCODER_RIGHT, menu_state, down_menu}, 
    {ENCODER_PRESS, menu_state, sel_menu},
    {READ_SD, menu_state, loadSDWrapper},    //TODO: Completar el wrapper 
    {EQ_SELECTION, eq_state, eq_menu},    
    {AJ_BRILLO, bright_state, adj_bright},
    {SONG_SEL, sel_song_state, loadFileSystem},
    {NULL_EVENT, menu_state, doNothing}
};

STATE eq_state[] = {          
    {ENCODER_LEFT, eq_state, upper_eq},   // upper_eq debería permitirte volver para el menu
    {ENCODER_RIGHT, eq_state, down_eq}, 
    {ENCODER_PRESS, menu_state, sel_eq}, // sel_eq debería cargar el menu tmb
    {NULL_EVENT, eq_state, doNothing}
};

STATE bright_state[] = {          
    {ENCODER_LEFT, bright_state, inc_brightness},   // inc y dec bright debe actualizar la pantalla y la potencia
    {ENCODER_RIGHT, bright_state, dec_brightness}, 
    {ENCODER_PRESS, menu_state, sel_bri}, // sel_bri debería cargar el menu tmb
    {NULL_EVENT, bright_state, doNothing}
};

STATE sel_song_state[] = {          
    {ENCODER_LEFT, bright_state, last_song},    //  last song debería permitirte volver para el menu
    {ENCODER_RIGHT, bright_state, next_song}, 
    {ENCODER_PRESS, menu_state, sel_option}, // sel_option debería cargar el estado reproducción
    {NULL_EVENT, sel_song_state, doNothing}
};

STATE song_info_state[] = {          
    {ENCODER_LEFT, bright_state, pause_song},
    {ENCODER_RIGHT, bright_state, resume_song}, 
    {ENCODER_PRESS, menu_state, sel_option}, // sel_option debería cargar el estado reproducción
    {SAMPLE_TIMER, song_info_state, }
    {NULL_EVENT, song_info_state, doNothing}
};


/*******************************************************************************
 ******************************************************************************/

#endif // FSM_table_H
