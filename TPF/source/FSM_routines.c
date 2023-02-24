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
#include "deepSleep/DeepSleep.h"
#include "FileSystem/SD/SD.h"
#include <stdio.h>


// DEBUG
#include "MCAL/gpio.h"
#define TESTPIN	PORTNUM2PIN(PB, 2)
/*******************************************************************************
 * CONSTANT AND MACRO DEFINITIONS USING #DEFINE
 ******************************************************************************/
#define MAIN_MENU_LEN   (sizeof(main_menu)/sizeof(MENU_ITEM))

// VOLUMEN
#define VOLMAX		100
#define VOLMIN		0
#define VOLVALUES	100
#define VOLSTEP		((VOLMAX-VOLMIN)/VOLVALUES)
#define VOL_INIT	((VOLMAX+VOLMIN)/2)
#define VOLUME_TIMEOUT	2000	// ms

// Ecualizacion
#define GAINMAX     20
#define GAINMIN     -20
#define GAINSTEP    1
#define CANT_BANDS  (BAND_OTHERS+1-BAND200)

// ERROR
#define ERROR_DELAY  3000

extern uint8_t sleep_reset;
/*******************************************************************************
 * VARIABLES WITH GLOBAL SCOPE
 ******************************************************************************/

MENU_ITEM main_menu[] = {
                            {.option = "ECUALIZACION", .ID = EQ_SELECTION},
                            {.option = "AJUS. BRILLO", .ID = ADJUST_BRIGHT},
                            {.option = "REPRODUCIR", .ID = SONG_SELECTION},
                        };

MENU_ITEM equ_menu[] = {  
                            {.option = "Band: 200-500Hz",   .ID = BAND200},
                            {.option = "Band: 500-1KHz",    .ID = BAND500},
                            {.option = "Band: 1KHz-1K5Hz",  .ID = BAND1K},
                            {.option = "Band: 1K5Hz-...",   .ID = BAND_OTHERS},
                            {.option = "Reset Bands",   .ID = RESET_EQU},
                        };



/*******************************************************************************
 * STATIC VARIABLES AND CONST VARIABLES WITH FILE LEVEL SCOPE
 ******************************************************************************/
// FSM VARIABLES
static tim_id_t FSMTimerID;
static uint8_t menu_pointer = 0;           // Variable que marca la opcion del menú seleccionada.
static uint8_t equ_pointer = BAND200;      // Variable que marca la opcion de ecualizacion.
static uint8_t band_selected = 0; 
static uint8_t vol_return_state;
static uint8_t err_return_state;
//static uint8_t error_type;

// Ecualizador
static int8_t equGaindB[CANT_BANDS];

// FileSystem
static char* sel_pointer;
static data* fileData;

//SD
static bool sd_state = false;

// MP3
static tim_id_t timerMP3;

static int16_t MP3Tables[2][OUTBUFF_SIZE];
static float32_t MP3FloatTables[2][OUTBUFF_SIZE];		// 0 source, 1 dest

static int16_t* pMP3Table;
static int16_t* pPrevMP3Table;

static MUSIC_STATES songState = STOP;


// Vumetro
static uint8_t vumetValues[8];

// Volumen
static uint8_t volume = VOL_INIT;
static tim_id_t timerVolume = 0;

/*******************************************************************************
 * LOCAL FUNCTION DEFINITION
 ******************************************************************************/

static void resetEqualizer();
static void timerMP3Cb();

static void resetVolumeTimer();
static void timerVolumeCb();
static void add_error(uint8_t error_type);

void loadSDWrapper();
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
    if(menu_pointer<(MAIN_MENU_LEN-2)){                     
    	displayLine(0, ">");
        displayText(0, 1, main_menu[menu_pointer].option);
        displayLine(1, main_menu[menu_pointer+1].option);
    }

    else {
        displayLine(0, main_menu[menu_pointer-1].option);
        displayLine(1, ">");
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
	clearScreen();
    displayLine(0, equ_menu[equ_pointer].option);

    // Si estoy mostrando una banda muestro la ganancia de la misma
    if(equ_menu[equ_pointer].ID!=RESET_EQU){
    	char temp[17];
    	sprintf(temp, "Gain: %d", equGaindB[equ_pointer]);
    	displayLine(1, temp);
    } 

}

void down_eq(){
    if(!band_selected){
        if(equ_pointer<EQ_OPTIONS-1){
            equ_pointer++;
            update_eq_menu();
        }
    }
    else{
        uint8_t band = equ_pointer;
        equGaindB[band] = (equGaindB[band]+GAINSTEP <= GAINMAX)? equGaindB[band]+GAINSTEP: GAINMAX;
        setUpFilter(equGaindB[band], band);  // Actualizamos en el ecualizador
        update_eq_menu();
    }
}

void up_eq(){
    if(!band_selected){
        if(equ_pointer>BAND200){
            equ_pointer--;
            update_eq_menu();
        }
    }
    else{
        uint8_t band = equ_pointer;
        equGaindB[band] = (equGaindB[band]-GAINSTEP >= GAINMIN)? equGaindB[band]-GAINSTEP: GAINMIN; // Atenuamos dentro de los limites
        setUpFilter(equGaindB[band], band);
        update_eq_menu();
    }
}

void sel_eq(){
    if(equ_menu[equ_pointer].ID==RESET_EQU){
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
    clearScreen();
	char temp[16];
	sprintf(temp, "Brillo: %u", get_bright());
    displayLine(0, temp);
    if(!isVumeterMode()){
    	fullMatrixON();
    }
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
	if(!isVumeterMode()){
		clearMatrix();
	}
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

	clearScreen();

	if (!sd_state) loadSDWrapper(); // Inicializo si no esta inicializado
    
    if(check_SD()){
        if ((sel_pointer = show_next())!= NULL){
            update_sel_menu();
        }
        else {
            add_error(SONG_ERROR);
        }
    }
}

void update_sel_menu(){
    if(check_SD()){
        displayLine(0, "Archivo:");
        displayLine(1, sel_pointer);
    }
}

void last_song(){
    if(check_SD()){
        if ((sel_pointer = show_prev())!= NULL){
            update_sel_menu();
        }
        else {
        	add_error(SONG_ERROR);
        }
    }
}

void next_song(){ 
    if(check_SD()){
        if ((sel_pointer = show_next())!= NULL){
            update_sel_menu();
        }
        else {
        	add_error(SONG_ERROR);
        }
    }
}

void back_song(){
    if(check_SD()){
        if ((sel_pointer = close_folder())!= NULL){
            update_sel_menu();
        }
        else {
            add_event(BACK);
        }
    } 
}

void sel_option(){

    if(check_SD()){
        char * temp;

        // Pruebo de abrirlo como una carpeta y si abre apunto al primer archivo
        if((temp = open_folder())!= NULL){
            sel_pointer = temp;
            update_sel_menu();
        }

        else if ((fileData = open_file())!= NULL) {	// Es archivo

            if (MP3SelectSong(get_path())) {    //Error al cargar la cancion
            	add_error(SONG_ERROR);
                return;    
            }
            else {		// Iniciar la reproduccion de la cancion

                pMP3Table = MP3Tables[0];	// Seleccionamos primer tabla

                uint16_t br = MP3DecNextFrame(pMP3Table);

                if (!br) {
                    // TODO: Stopear todo??
                	add_error(FRAME_ERROR);
                    return;
                }

                //TODO: Hacer en otro lado
                // Pasa a float
                for (int i = 0; i < br; i++) {
                    MP3FloatTables[0][i] = (float32_t)pMP3Table[i];
                }

                blockEqualizer(MP3FloatTables[0], MP3FloatTables[1], OUTBUFF_SIZE);

                // float to 12 bit and shifting
                for (int i = 0; i < OUTBUFF_SIZE; i++) {
                    // escalado de 16 bits a 12 bits y por volumen
                    float32_t temp = MP3FloatTables[1][i]*0x7FF/0x7FFF*volume/VOLMAX;
                    if (temp > (int16_t)0x7FF) temp = 0x7FF;	// Saturacion
                    else if (temp < (int16_t)0xF800) temp = 0xF800;
                    ((uint16_t*)pMP3Table)[i] = (uint16_t)((int16_t)temp + (int16_t)0x800);
                }

                startAnalyzer(MP3FloatTables[1], 512U); // Vumetro
                getAnalyzer(vumetValues);
                setColumnsMatrix(vumetValues);

                // TODO: Que no haya que poner el DAC aca
                DMA_pingPong_DAC((uint16_t*)MP3Tables[0], (uint16_t*)MP3Tables[1], OUTBUFF_SIZE);

                // Start periodic timer to update tables
                timerMP3 = timerGetId();
                timerStart(timerMP3, TIMER_MS2TICKS(5U), TIM_MODE_PERIODIC, timerMP3Cb);

                songState = PLAY;
                add_event(SONG_SELECTED);
            }
        }
    }
}

/**********************************************************
******************    INFO SCREEN    **********************
**********************************************************/
void load_info(){
    displayLine(0, "Reproduciendo");
    displayLine(1, sel_pointer);
}

/**********************************************************
******************    VOLUME CTRL    **********************
**********************************************************/
void inc_vol(){

	resetVolumeTimer();

	if(volume+VOLSTEP <= VOLMAX){
		volume +=VOLSTEP;
		char temp[17];
		sprintf(temp, "Volumen: %u", volume);
		clearScreen();
		displayLine(0, temp);
	}
}

void dec_vol(){

	resetVolumeTimer();

	if(volume-VOLSTEP >= VOLMIN){
		volume-=VOLSTEP;
		char temp[17];
		sprintf(temp, "Volumen: %u", volume);
		clearScreen();
		displayLine(0, temp);
	}

}

void vol_inc_ss(){
    vol_return_state=SONG_SELECTION;
    inc_vol();
} 

void vol_dec_ss(){
    vol_return_state=SONG_SELECTION;
    dec_vol();
} 

void vol_inc_si(){
    vol_return_state=SONG_SELECTED;
    inc_vol();
} 

void vol_dec_si(){
    vol_return_state=SONG_SELECTED;
    dec_vol();
}  

void vol_inc_menu(){
    vol_return_state=MAIN_MENU_EV;
    inc_vol();
} 

void vol_dec_menu(){
    vol_return_state=MAIN_MENU_EV;
    dec_vol();
}  

void vol_last_state(){
	if (timerVolume) timerStop(timerVolume);	// Detengo el contador de timeout
    add_event(vol_return_state);
}

/**********************************************************
******************    ERROR CTRL    **********************
**********************************************************/
void err_last_state(){
	if (FSMTimerID) timerStop(FSMTimerID);	// DUDA: un timer general para la FSM?
    add_event(err_return_state);
}

void add_error(uint8_t error_type){
	clearMatrix();
    displayLine(0, "      ERROR");

    switch(error_type){
        case SD_ERROR:
            err_return_state= MAIN_MENU_EV;	// TODO: err_return_state no sirve
            displayLine(1, "  INSERT SD");
            break;
	    
        case SONG_ERROR:
            err_return_state = SONG_SELECTION;
            displayLine(1, "  WRONG FILE");
            break;

		case FRAME_ERROR:
            err_return_state = SONG_SELECTION;
		    displayLine(1, "  FRAME ERROR");
            break;

        default:
        	break;
    }

    if(!FSMTimerID) FSMTimerID=timerGetId();
    timerStart(FSMTimerID, TIMER_MS2TICKS(ERROR_DELAY), TIM_MODE_SINGLESHOT, addTimeout);
    
    add_event(ERROR_EV);
}

/**********************************************************
*********************   SLEEP    **************************
**********************************************************/
void go_sleep(){

	ready2sleep(); //Wait data to flows
	LLS_start();
	aftersleep();
    update_menu();


    //TODO: Agregar para prender/apagar led cuando se entra en sleep
}

/**********************************************************
*********************  VARIOUS   **************************
**********************************************************/
void toggle_state(){
    if(check_SD()){
        if (songState==PLAY) {
            timerStop(timerMP3);
            DMA_pause_pingPong();
            songState = PAUSE;
        }
        else if (songState == PAUSE){
            songState = PLAY;
            timerMP3Cb();
            DMA_continue_pingPong();
            timerStart(timerMP3, TIMER_MS2TICKS(5U), TIM_MODE_PERIODIC, timerMP3Cb);
        }
    }
}

bool check_SD(){
    if (!isSDCardInserted()){
        add_error(SD_ERROR);
        add_event(ERROR_EV);
        return false;
    }
    return true;
}
void doNothing() {
    return;
}

/**********************************************************
*****************  LOCAL FUNCTIONS   **********************
**********************************************************/
static void resetEqualizer() {
	for (int i = 0; i < CANT_BANDS; i++) {
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

    		// float to 12 bit and shifting
    		for (int i = 0; i < OUTBUFF_SIZE; i++) {
    			// escalado de 16 bits a 12 bits y por volumen
    			float32_t temp = MP3FloatTables[1][i]*0x7FF/0x7FFF*volume/VOLMAX;
    			if (temp > (int16_t)0x7FF) temp = 0x7FF;	// Saturacion
    			else if (temp < (int16_t)0xF800) temp = 0xF800;
    			((uint16_t*)pMP3Table)[i] = (uint16_t)((int16_t)temp + (int16_t)0x800);
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
