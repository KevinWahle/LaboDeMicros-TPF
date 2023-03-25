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
extern STATE error_state[];


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
	{BTN_PAUSE, menu_state, toggle_state},

    {EQ_SELECTION, eq_state, update_eq_menu},    
    {ADJUST_BRIGHT, bright_state, update_bright},
    {SONG_SELECTION, sel_song_state, loadFileSystem},
    
    {VOL_UP, volume_state, vol_inc_menu},
    {VOL_DOWN, volume_state, vol_dec_menu},

    {BTN_SLEEP, menu_state, go_sleep},

    {ERROR_EV, error_state, doNothing},

    {NULL_EVENT, menu_state, doNothing}
};

STATE eq_state[] = {
    {ENCODER_LEFT, eq_state, up_eq},
    {ENCODER_RIGHT, eq_state, down_eq},
    {ENCODER_PRESS, eq_state, sel_eq}, // sel_eq debería cargar el menu tmb
    {ENCODER_LONG, menu_state, update_menu},
	{BTN_PAUSE, eq_state, toggle_state},

    {VOL_UP, volume_state, vol_inc_menu},
    {VOL_DOWN, volume_state, vol_dec_menu},

    {BTN_SLEEP, menu_state, go_sleep},

    {ERROR_EV, error_state, doNothing},
    
    {NULL_EVENT, eq_state, doNothing}
};

STATE bright_state[] = {
    {ENCODER_LEFT, bright_state, dec_brightness},
    {ENCODER_RIGHT, bright_state, inc_brightness},
    {ENCODER_PRESS, menu_state, sel_brightness},
    {ENCODER_LONG, menu_state, sel_brightness},
	{BTN_PAUSE, bright_state, toggle_state},

    {VOL_UP, volume_state, vol_inc_menu},
    {VOL_DOWN, volume_state, vol_dec_menu},

    {BTN_SLEEP, menu_state, go_sleep},

    {ERROR_EV, error_state, doNothing},

    {NULL_EVENT, bright_state, doNothing}
};

STATE sel_song_state[] = {       
    {ENCODER_LONG, sel_song_state, back_song},
    {BACK, menu_state, update_menu},

    {ENCODER_LEFT, sel_song_state, last_song},
    {ENCODER_RIGHT, sel_song_state, next_song}, 
    {ENCODER_PRESS, sel_song_state, sel_option},
    {BTN_PAUSE, sel_song_state, toggle_state},      // cambia entre reproducir y pausar

    {SONG_SELECTED, song_info_state, load_info},
    //agregar la transicion de reproduccion


    {VOL_UP, volume_state, vol_inc_ss},
    {VOL_DOWN, volume_state, vol_dec_ss},

    {BTN_SLEEP, menu_state, go_sleep},

    {ERROR_EV, error_state, doNothing},

    {NULL_EVENT, sel_song_state, doNothing}
};

STATE song_info_state[] = {    
    {ENCODER_LONG, sel_song_state, update_sel_menu},
    {ENCODER_LEFT, sel_song_state, update_sel_menu},
    {ENCODER_RIGHT, sel_song_state, update_sel_menu},
    
    {BTN_PAUSE, song_info_state, toggle_state}, // cambia entre reproducir y pausar
    
    {VOL_UP, volume_state, vol_inc_si},
    {VOL_DOWN, volume_state, vol_dec_si},

    {BTN_SLEEP, menu_state, go_sleep},

    {ERROR_EV, error_state, doNothing},

    {NULL_EVENT, song_info_state, doNothing}
};

STATE volume_state[] = {
    {VOL_UP, volume_state, inc_vol},
    {VOL_DOWN, volume_state, dec_vol},
    {TIMEOUT, volume_state, vol_last_state},
    
    {SONG_SELECTED, song_info_state, load_info},  
    {SONG_SELECTION, sel_song_state, loadFileSystem}, 
    {MAIN_MENU_EV, menu_state, update_menu},
    {EQ_SELECTION, eq_state, update_eq_menu},    
    {ADJUST_BRIGHT, bright_state, update_bright},  
    {BTN_PAUSE, volume_state, toggle_state}, // cambia entre reproducir y pausar
    {BTN_SLEEP, menu_state, go_sleep},
    
    {ERROR_EV, error_state, doNothing},
    
    {NULL_EVENT, volume_state, doNothing}    
};

STATE error_state[] = {
    {TIMEOUT, menu_state, update_menu}, 
    {ENCODER_LONG, menu_state, update_menu},
    {ENCODER_PRESS, menu_state, update_menu},

    {NULL_EVENT, error_state, doNothing}    
};


/*******************************************************************************
 ******************************************************************************/
#endif // FSM_table_H

