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
    {TIMEOUT, menu_state, update_menu},
    {NULL_EVENT, splash_state, doNothing}
};

/*STATE main_screen_state[] = {          
    {ENCODER_PRESS, menu_state, menu_start},
    {NULL_EVENT, main_screen_state, doNothing}
};*/

STATE menu_state[] = {          
    {ENCODER_LEFT, menu_state, up_menu},
    {ENCODER_RIGHT, menu_state, down_menu}, 
    {ENCODER_PRESS, menu_state, sel_menu},

    {READ_SD, menu_state, loadSDWrapper},    //TODO: Completar el wrapper 
    {EQ_SELECTION, eq_state, update_eq_menu},    
    {ADJUST_BRIGHT, bright_state, update_bright},
    {SONG_SELECTION, sel_song_state, loadFileSystem},
    {TIMEOUT, song_info_state, load_info},  // Revisar si conviene mandar a song info o a una main screen
    
    {VOL_UP, volume_state, vol_inc_menu},
    {VOL_DOWN, volume_state, vol_dec_menu},

    {NULL_EVENT, menu_state, doNothing}
};

STATE eq_state[] = {          
    {ENCODER_LEFT, eq_state, upper_eq},   // upper_eq debería permitirte volver para el menu
    {ENCODER_RIGHT, eq_state, down_eq}, 
    {ENCODER_PRESS, menu_state, sel_eq}, // sel_eq debería cargar el menu tmb
    {NULL_EVENT, eq_state, doNothing}
};

STATE bright_state[] = {          
    {ENCODER_LEFT, bright_state, inc_brightness},
    {ENCODER_RIGHT, bright_state, dec_brightness}, 
    {ENCODER_PRESS, menu_state, sel_brightness},
    {NULL_EVENT, bright_state, doNothing}
};

STATE sel_song_state[] = {          
    {ENCODER_LEFT, sel_song_state, last_song},      // last song debería permitirte volver para el menu
    {ENCODER_RIGHT, sel_song_state, next_song}, 
    {ENCODER_PRESS, sel_song_state, sel_option},    // sel_option debería cargar el estado reproducción
    {PAUSE_BTN, sel_song_state, toggle_state},      // cambia entre reproducir y pausar
    {SONG_SELECTED, song_info_state, save_info},
    //agregar la transicion de reproduccion
    {VOL_UP, volume_state, vol_inc_ss},
    {VOL_DOWN, volume_state, vol_dec_ss},
    {NULL_EVENT, sel_song_state, doNothing}
};

STATE song_info_state[] = {          
    {PAUSE_BTN, song_info_state, toggle_state}, // cambia entre reproducir y pausar
    {ENCODER_PRESS, menu_state, update_menu},
    {SAMPLE_TIMER, song_info_state, sample_processing}, //DUDA: Revisar como ver el tema de las samples
    
    {VOL_UP, volume_state, vol_inc_si},
    {VOL_DOWN, volume_state, vol_dec_si},
    {NULL_EVENT, song_info_state, doNothing}
};

STATE volume_state[] = {
    {VOL_UP, volume_state, inc_vol},
    {VOL_DOWN, volume_state, dec_vol},
    {TIMEOUT, volume_state, last_state},
    {ENCODER_PRESS, volume_state, last_state},  //last_state genera un evento para volver al estado anterior
    
    {SONG_SELECTED, song_info_state, load_info},  
    {SONG_SELECTION, sel_song_state, loadFileSystem}, 
    {MAIN_MENU_EV, menu_state, update_menu},  

    {NULL_EVENT, volume_state, doNothing}    
};


/*******************************************************************************
 ******************************************************************************/

#endif // FSM_table_H

//REVISAR: cual es el idle state si NO hay cancion cargada?