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

extern STATE splash_state[];
//extern STATE main_screen_state[];
extern STATE menu_state[];
extern STATE eq_state[];
extern STATE bright_state[];
extern STATE sel_song_state[];
extern STATE song_info_state[];
extern STATE volume_state[];


STATE splash_state[] = {          
    {TIMEOUT, menu_state, menu_start},
    {NULL_EVENT, splash_state, doNothing}
};

/*STATE main_screen_state[] = {          
    {ENCODER_PRESS, menu_state, menu_start},
    {NULL_EVENT, main_screen_state, doNothing}
};*/

STATE menu_state[] = {          
    {ENCODER_LEFT, menu_state, upper_menu},
    {ENCODER_RIGHT, menu_state, down_menu}, 
    {ENCODER_PRESS, menu_state, sel_menu},
    {TIMEOUT, song_info_state, load_info},

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
    {ENCODER_LEFT, sel_song_state, last_song},    //  last song debería permitirte volver para el menu
    {ENCODER_RIGHT, sel_song_state, next_song}, 
    {ENCODER_PRESS, sel_song_state, sel_option}, // sel_option debería cargar el estado reproducción
    {PAUSE_BTN, sel_song_state, toggle_state}, // cambia entre reproducir y pausar
    {SONG_SELECTED, song_info_state, load_info},
    //agregar la transicion de reproduccion
    {NULL_EVENT, sel_song_state, doNothing}
};

STATE song_info_state[] = {          
    {PAUSE_BTN, song_info_state, toggle_state}, // cambia entre reproducir y pausar
    {ENCODER_PRESS, menu_state, menu_start},
    {SAMPLE_TIMER, song_info_state, sample_processing}, //TODO: Revisar como ver el tema de las samples
    {NULL_EVENT, song_info_state, doNothing}
};

STATE volume_state[] = {
    {VOL_UP, volume_state, inc_vol},
    {VOL_DOWN, volume_state, dec_vol},
    {TIMEOUT, volume_state, last_state},

    {ENCODER_PRESS, volume_state, last_state},  //last_state genera un evento para volver al estado anterior
    {SONG_INFO_EV, song_info_state, load_info},  
    {SONG_SEL_EV, sel_song_state, loadFileSystem}, 
    {MAIN_MENU_EV, menu_state, menu_start},  

    {NULL_EVENT, volume_state, doNothing}    
};


/*******************************************************************************
 ******************************************************************************/

#endif // FSM_table_H

//REVISAR: cual es el idle state si NO hay cancion cargada?