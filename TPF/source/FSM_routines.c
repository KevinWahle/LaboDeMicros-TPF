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
#include "MP3Dec/Mp3Dec.h"
#include "SpectrumAnalyzer/SpectrumAnalyzer.h"
#include "Matrix/Matrix.h"
#include "DMA2/DMA2.h"
#include "const.h"
#include "timer/timer.h"
#include <stdio.h>

#include "MK64F12.h"	//TODO: Sacar esto cuanto antes

// DEBUG
#include "MCAL/gpio.h"
#define TESTPIN	PORTNUM2PIN(PB, 2)
/*******************************************************************************
 * CONSTANT AND MACRO DEFINITIONS USING #DEFINE
 ******************************************************************************/
#define MAIN_MENU_LEN   (sizeof(main_menu)/sizeof(MENU_ITEM))

#define EQU_BAND_COUNT	4

#define MIN_VOLUME	0
#define MAX_VOLUME	100

#define VOLUME_TIMEOUT	2000	// ms

/*******************************************************************************
 * VARIABLES WITH GLOBAL SCOPE
 ******************************************************************************/

MENU_ITEM main_menu[] = {
//                            {.option = "LEER SD", .ID = READ_SD},
                            {.option = "ECUALIZACION", .ID = EQ_SELECTION},
                            {.option = "AJUS. BRILLO", .ID = ADJUST_BRIGHT},
                            {.option = "REPRODUCIR", .ID = SONG_SELECTION},
                        };

MENU_ITEM equ_menu[] = {  
                            {.option = "VOLVER",   .ID = BACK_EQU},
                            {.option = "Band: 200-500Hz",   .ID = BAND200},
                            {.option = "Band: 500-1KHz",    .ID = BAND500},
                            {.option = "Band: 1KHz-1K5Hz",  .ID = BAND1K},
                            {.option = "Band: 1K5Hz-...",   .ID = BAND_OTHERS},
                            {.option = "DEFAULT",   .ID = RESET_EQU},
                        };

/*******************************************************************************
 * STATIC VARIABLES AND CONST VARIABLES WITH FILE LEVEL SCOPE
 ******************************************************************************/
//static tim_id_t idTimer;
static tim_id_t timerMP3;

static uint8_t menu_pointer = 0;           // Variable que marca la opcion del menú seleccionada.
static uint8_t equ_pointer = 1;            // Variable que marca la opcion de ecualizacion.
static uint8_t band_selected = 0; 

static int8_t equGaindB[EQU_BAND_COUNT];

static char brightnesschar[4];              // DUDA: no puede ir la funcion int2chr directo?
static SONG_INFO_T* actual_song;
static char* sel_pointer;
static uint8_t from_state;

static bool sd_state = false;

static data* fileData;

// MP3

static int16_t MP3Tables[2][OUTBUFF_SIZE];
static float32_t MP3FloatTables[2][OUTBUFF_SIZE];		// 0 source, 1 dest

static int16_t* pMP3Table;
static int16_t* pPrevMP3Table;

static MUSIC_STATES songState = STOP;

// Vumetro

static uint8_t vumetValues[8];

// Volumen

static uint8_t volume = 50;
static tim_id_t timerVolume = 0;



/*
void update_display(uint8_t* arr, uint8_t counter, bool password);
void updateMenuDis(char* word);
void setIDTimer_cb();
*/

char* int2char(int brightness);

static void resetEqualizer();
static void timerMP3Cb();

static void resetVolumeTimer();
static void timerVolumeCb();

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
    	char temp[17];
    	sprintf(temp, "Gain: %d", equGaindB[equ_pointer-BAND200]);
    	displayText(1, 0, temp);
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
    	//TODO: hacerlo bien, limite, overflow, etc
    	setUpFilter(equGaindB[equ_pointer-BAND200] += 1, equ_pointer-BAND200);
        update_eq_menu();
    }
}

void up_eq(){
    if(!band_selected){
        if(equ_pointer>0){
            equ_pointer--;
            update_eq_menu();
        }
    }
    else{
        // gainBand(equ_pointer-BAND200);
    	//TODO: hacerlo bien, limite, overflow, etc
    	setUpFilter(equGaindB[equ_pointer-BAND200] -= 1, equ_pointer-BAND200);
        update_eq_menu();
    }
}

void sel_eq(){
    if(equ_menu[equ_pointer].ID==BACK_EQU){
        add_event(BACK);
    }
    else if(equ_menu[equ_pointer].ID==RESET_EQU){
        resetEqualizer();
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

	char temp[17];
	sprintf(temp, "Brillo: %u", get_bright());
    clearDisplay();
    displayLine(0, temp);
//    displayLine(1,"NO DESARROLLADO");
    //displayText(1, 8, int2char(get_bright()));
//    fullMatrixON();		// TODO: Solo es necesario para la primera vez
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
	clearMatrix(0);
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

	clearDisplay();

	if (!sd_state) loadSDWrapper();

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
	char * temp;
    if((temp = open_folder())!= NULL){ //DUDA: poner alguna const de ultimo elemento
        sel_pointer = temp;
    	update_sel_menu();
    }
    else if ((fileData = open_file())!= NULL) {	// Es archivo

    	// Chequeo de la cancion

    	if (MP3SelectSong(get_path())) {
    	    displayLine(0, "ERROR:");
    	    displayLine(1, "SONG ERROR");
    	}
    	else {		// Iniciar la reproduccion de la cancion

    		pMP3Table = MP3Tables[0];	// Seleccionamos primer tabla

    		uint16_t br = MP3DecNextFrame(pMP3Table);

    		if (!br) {
				displayLine(0, "ERROR:");
				displayLine(1, "FRAME ERROR");
    		}

    		//TODO: Hacer en otro lado
    		// Pasa a float
    		for (int i = 0; i < br; i++) {
    			MP3FloatTables[0][i] = (float32_t)pMP3Table[i];
    		}

    		blockEqualizer(MP3FloatTables[0], MP3FloatTables[1], OUTBUFF_SIZE);

    		// 16 bit to 12 bit and shifting
    		for (int i = 0; i < OUTBUFF_SIZE; i++) {
    			pMP3Table[i] = (int16_t)MP3FloatTables[1][i];
    			pMP3Table[i] += 0x8000U;
    			pMP3Table[i] *= (double)0xFFFU / 0xFFFFU;
    		}

    		startAnalyzer(MP3FloatTables[1], 512U);
    		getAnalyzer(vumetValues);
			setColumnsMatrix(vumetValues);

    		// TODO: Que no haya que poner el DAC aca
    		DMA_pingPong_DAC(MP3Tables[0], MP3Tables[1], (uint32_t)(&DAC0->DAT[0].DATL), OUTBUFF_SIZE);

    		// Start periodic timer to update tables
    		timerMP3 = timerGetId();
    		timerStart(timerMP3, TIMER_MS2TICKS(5U), TIM_MODE_PERIODIC, timerMP3Cb);

    		songState = PLAY;
			add_event(SONG_SELECTED);
    	}
    }
}

void save_info(){
//     set_state(PLAY);
    // readMetaSong(actual_song);
//     load_info();

    displayLine(0, "Reproducioendo");
    displayLine(1, sel_pointer);


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

	resetVolumeTimer();

	if(volume < MAX_VOLUME){
//	 clearLine(1);
//	 displayText(1, 7, int2char(get_volume()));
		volume++;
		char temp[17];
		sprintf(temp, "Volumen: %u", volume);
		clearDisplay();
		displayLine(0, temp);
	}
}

void dec_vol(){

	resetVolumeTimer();

//	if(volume_down()){
//	 clearLine(1);
//	 displayText(1, 7, int2char(get_volume()));
//	}
//
	if(volume > MIN_VOLUME){
		volume--;
		char temp[17];
		sprintf(temp, "Volume: %u", volume);
		clearDisplay();
		displayLine(0, temp);
	}

}

void vol_inc_ss(){
    from_state=SONG_SELECTION;
    inc_vol();
} 

void vol_dec_ss(){
    from_state=SONG_SELECTION;
    dec_vol();
} 

void vol_inc_si(){
    from_state=SONG_SELECTED;
    inc_vol();
} 

void vol_dec_si(){
    from_state=SONG_SELECTED;
    dec_vol();
}  

void vol_inc_menu(){
    from_state=MAIN_MENU_EV;
    inc_vol();
} 

void vol_dec_menu(){
    from_state=MAIN_MENU_EV;
    dec_vol();
}  

void last_state(){
	if (timerVolume) timerStop(timerVolume);	// Detengo el contador de timeout
    add_event(from_state);
}

/**********************************************************
*********************  VARIOUS   **************************
**********************************************************/
void toggle_state(){
     if (songState==PLAY) {
    	 timerStop(timerMP3);
    	 DMA_pause_pingPong();
    	 songState = PAUSE;
     }
     else {
    	 timerStart(timerMP3, TIMER_MS2TICKS(5U), TIM_MODE_PERIODIC, timerMP3Cb);
    	 DMA_continue_pingPong();
    	 songState = PLAY;
     }
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


static void resetEqualizer() {
	for (int i = 0; i < EQU_BAND_COUNT; i++) {
		equGaindB[i] = 0;
		setUpFilter(0, i);
	}
}

static void resetVolumeTimer() {

	if (!timerVolume) {		// timer no inicializado
		timerVolume = timerGetId();
	}

	timerStart(timerVolume, TIMER_MS2TICKS(VOLUME_TIMEOUT), TIM_MODE_SINGLESHOT, timerVolumeCb);

}

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


static void timerMP3Cb() {
	pMP3Table = (int16_t*)DMA_availableTable_pingPong();

	if (pMP3Table != pPrevMP3Table) {	// Hay nueva tabla disponible para llenar
		pPrevMP3Table = pMP3Table;

		gpioWrite(TESTPIN, HIGH);
		uint16_t br = MP3DecNextFrame(pMP3Table);

		if (br > 0) {
    		//TODO: Hacer en otro lado
    		// Pasa a float
    		for (int i = 0; i < br; i++) {
    			MP3FloatTables[0][i] = (float32_t)pMP3Table[i];
    		}

    		blockEqualizer(MP3FloatTables[0], MP3FloatTables[1], OUTBUFF_SIZE);

    		// 16 bit to 12 bit and shifting
    		for (int i = 0; i < OUTBUFF_SIZE; i++) {
    			pMP3Table[i] = (int16_t)MP3FloatTables[1][i];
    			pMP3Table[i] += 0x8000U;
    			pMP3Table[i] *= (double)0xFFFU / 0xFFFFU;
    		}

    		startAnalyzer(MP3FloatTables[1], 512U);
    		getAnalyzer(vumetValues);
    		setColumnsMatrix(vumetValues);

		}
		else {
			//TODO: para instantaneamente, hay que hacer una vuelta mas con 0s
			DMA_pause_pingPong();
			timerStop(timerMP3);
		}
		gpioWrite(TESTPIN, LOW);
	}

}


static void timerVolumeCb() {
	add_event(TIMEOUT);		//TODO: Evento de TIMEOUT propio de cada timer???
}
