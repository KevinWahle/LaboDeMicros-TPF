extern void displayLine(int line, char* text);
extern void displayText(int line, int position, char* text);
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
#include "FileSystem/FileSystem.h"
#include "Display_I2C/Display.h"
#include "Matrix/Matrix.h"
#include "const.h"
#include "timer/timer.h"
#include <stdio.h>

/*******************************************************************************
 * CONSTANT AND MACRO DEFINITIONS USING #DEFINE
 ******************************************************************************/
#define MAIN_MENU_LEN   (sizeof(main_menu)/sizeof(MENU_ITEM)) 


/*******************************************************************************
 * VARIABLES WITH GLOBAL SCOPE
 ******************************************************************************/

MENU_ITEM main_menu[] = {  
                            {.option = "LEER SD", .ID = READ_SD},
                            {.option = "ECUALIZACION", .ID = EQ_SELECTION},
                            {.option = "AJUS. BRILLO", .ID = ADJUST_BRIGHT},
                            {.option = "REPRODUCIR", .ID = SONG_SELECTION},
                        };

MENU_ITEM equ_menu[] = {  
                            {.option = "Band: 200-500Hz",   .ID = BACK_EQU},
                            {.option = "Band: 200-500Hz",   .ID = BAND200},
                            {.option = "Band: 500-1KHz",    .ID = BAND500},
                            {.option = "Band: 1KHz-1K5Hz",  .ID = BAND1K},
                            {.option = "Band: 1K5Hz-...",   .ID = BAND_OTHERS},
                            {.option = "Band: 200-500Hz",   .ID = RESET_EQU},
                        };

/*******************************************************************************
 * STATIC VARIABLES AND CONST VARIABLES WITH FILE LEVEL SCOPE
 ******************************************************************************/
static tim_id_t idTimer;

static uint8_t menu_pointer = 0;           // Variable que marca la opcion del menú seleccionada.
static uint8_t equ_pointer = 0;            // Variable que marca la opcion de ecualizacion.     
static uint8_t band_selected = 0; 

static char brightnesschar[4];              // DUDA: no puede ir la funcion int2chr directo?
static SONG_INFO_T* actual_song;
static char* sel_pointer;
static uint8_t from_state;

static bool sd_state = false; // TODO: Inicializar

static data* fileData;


/*
void update_display(uint8_t* arr, uint8_t counter, bool password);
void updateMenuDis(char* word);
void setIDTimer_cb();
*/

char* int2char(int brightness);

/*******************************************************************************
 *******************************************************************************
                        GLOBAL FUNCTION DEFINITIONS
 *******************************************************************************
 ******************************************************************************/

/**********************************************************
*******************   Splash Screen    ********************
**********************************************************/
void addTimeout(){
    add_event(TIMEOUT);
}

/**********************************************************
*******************   Main Menu    ************************
**********************************************************/
void update_menu(){
    //TODO: en el segundo renglon mostramos la sgte opcion o la hora?
    if(menu_pointer<(MAIN_MENU_LEN-2)){                     
        displayText(0, 0, ">");
        displayText(0, 1, main_menu[menu_pointer].option);
        displayLine(1, main_menu[menu_pointer+1].option);
    }

    else {
        displayLine(0, main_menu[menu_pointer-1].option);
        displayText(1, 0, ">");
        displayText(1, 1, main_menu[menu_pointer].option);
    }
}

void down_menu(){
    if(menu_pointer<(MAIN_MENU_LEN-1)){
        menu_pointer++;
        update_menu();
    } 
}

void up_menu(){
    if(menu_pointer>0){
        menu_pointer--;
        update_menu();
    }
}

void sel_menu(){
    add_event(main_menu[menu_pointer].ID);
}

/**********************************************************
*****************   Equalizer Menu    *********************
**********************************************************/
void update_eq_menu(){
    clearDisplay();
    displayLine(0, equ_menu[equ_pointer].option);

    // Si estoy mostrando una banda muestro la ganancia de la misma
    if(equ_menu[equ_pointer].ID!=BACK_EQU && equ_menu[equ_pointer].ID!=RESET_EQU){ 
        displayText(1, 0, "Gain:" );
        // displayText(1, 7, int2char(getGainBand(equ_pointer-BAND200)));
    } 

}

void down_eq(){
    if(!band_selected){
        if(equ_pointer<(EQ_OPTIONS-1)){
            equ_pointer++;
            update_eq_menu();
        }
    }
    else{
        // atenuateBand(equ_pointer-BAND200);
        update_eq_menu();
    }
}

void up_eq(){
    if(!band_selected){
        if(equ_pointer>1){
            equ_pointer--;
            update_eq_menu();
        }
    }
    else{
        // gainBand(equ_pointer-BAND200);
        update_eq_menu();
    }
}

void sel_eq(){
    if(equ_menu[equ_pointer].ID==BACK_EQU){
        add_event(BACK);
    }
    else if(equ_menu[equ_pointer].ID==RESET_EQU){
        // resetEqualizer();
        equ_pointer=0;
        update_eq_menu();
    }
    else{
        band_selected = !band_selected;
    }
}

/**********************************************************
******************   Matrix Bright    *********************
**********************************************************/
void update_bright(){
    displayLine(0, "    Brillo     ");
    displayLine(1,"NO DESARROLLADO");
    //displayText(1, 8, int2char(get_bright()));
    fullMatrixON();
}

void inc_brightness(){
    increase_bright();
    update_bright();
}

void dec_brightness(){
    decrease_bright();
    update_bright();
}

void sel_brightness(){
    update_menu();
}


/**********************************************************
************************  SD   ***************************
**********************************************************/
void loadSDWrapper(){
    if (init_filesys()){
        if(statrt_mapping()){
            sd_state = true;    
        }
        else{
            sd_state = false;
        }
    }
    else{
        sd_state = false;
    }
    
}



/**********************************************************
******************  SELECTION MENU   **********************
**********************************************************/
void loadFileSystem(){
    if ((sel_pointer = show_next())!= NULL){     // TODO: No entendi lo de volver al menu
        update_sel_menu();
    }
    else {
        //TODO: ERROR: Volver al menu
    }
}

void update_sel_menu(){
    displayLine(0, "Seleccionar archivo:");
    displayLine(1, sel_pointer);
}

void last_song(){
    // if(!sel_pointer->back){
    //     sel_pointer = sel_pointer->up_element;
    //     update_sel_menu();
    // }
    if ((sel_pointer = show_prev())!= NULL){     // TODO: No entendi lo de volver al menu
        update_sel_menu();
    }
    else {
        //TODO: ERROR: Volver al menu
    }
}

void next_song(){ 
    // if(sel_pointer->down_element != 0){ //DUDA: poner alguna const de ultimo elemento
    //     sel_pointer = sel_pointer->down_element;
    //     update_sel_menu();
    // }
    if ((sel_pointer = show_next())!= NULL){     // TODO: No entendi lo de volver al menu
        update_sel_menu();
    }
    else {
        //TODO: ERROR: Volver al menu
    }
}

void sel_option(){ 

    if((sel_pointer = open_folder())!= NULL){ //DUDA: poner alguna const de ultimo elemento
        update_sel_menu();
    }
    else if ((fileData = open_file())!= NULL) {
        add_event(SONG_SELECTED);
    }
}

void save_info(){
    // set_state(PLAY);
    // readMetaSong(actual_song);
    // load_info();
}

/**********************************************************
******************    INFO SCREEN    **********************
**********************************************************/
void load_info(){
    //TODO: Revisar que pingo de info queremos mostrar
    displayLine(0, actual_song->name);
    displayLine(1, actual_song->author);
}

/**********************************************************
******************    VOLUME CTRL    **********************
**********************************************************/
void inc_vol(){
    // if(volume_up()){
        // clearLine(1);
        // displayText(1, 7, int2char(get_volume()));
    // } 
}

void dec_vol(){
    // if(volume_down()){
        // clearLine(1);
        // displayText(1, 7, int2char(get_volume()));
    // }
}

void vol_inc_ss(){
    from_state=SONG_SELECTION;
    // inc_vol();
} 

void vol_dec_ss(){
    from_state=SONG_SELECTION;
    // dec_vol();
} 

void vol_inc_si(){
    from_state=SONG_SELECTED;
    // inc_vol();
} 

void vol_dec_si(){
    from_state=SONG_SELECTED;
    // dec_vol();
}  

void vol_inc_menu(){
    from_state=MAIN_MENU_EV;
    // inc_vol();
} 

void vol_dec_menu(){
    from_state=MAIN_MENU_EV;
    // dec_vol();
}  

void last_state(){
    add_event(from_state);
}

/**********************************************************
*********************  VARIOUS   **************************
**********************************************************/
void toggle_state(){
    // get_state()==PLAY? set_state(PAUSE):set_state(PLAY);
}

void doNothing() {
    return;
}

/**********************************************************
*****************  LOCAL FUNCTIONS   **********************
**********************************************************/
char* int2char(int brightness){
    brightnesschar[0] = '0'+ (int)((brightness)/100);
    brightnesschar[1] = '0'+ (int)((brightness%100)/10);
    brightnesschar[2] = '0'+ brightness%10;
    brightnesschar[3] = '\0';

    return brightnesschar;
}

//



/**********************************************************
********************  CALLBACKS  *************************
**********************************************************/

void encoderCallback(ENC_STATE state){
    switch(state){
        case ENC_LEFT:
            add_event(ENCODER_LEFT);
        break;

        case ENC_RIGHT:
            add_event(ENCODER_RIGHT);
        break;

        case ENC_CLICK:
            add_event(ENCODER_PRESS);
        break;

        case ENC_LONG:
            add_event(ENCODER_LONG);
        break;

        case ENC_DOUBLE:
            add_event(ENCODER_DOUBLE);
        break;

        default:
        break;
    }
}
