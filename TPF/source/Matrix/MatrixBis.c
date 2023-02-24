/***************************************************************************//**
  @file		Matrix.c
  @brief	HAL de la matrix de LEDs
  @author	Grupo 5
  @date		28 Dic. 2022
 ******************************************************************************/


/*******************************************************************************
 * INCLUDE HEADER FILES
 ******************************************************************************/
#include "Matrix.h"
#include "../DMA2/DMA2.h"
#include "../timer/timer.h"
#include <stdbool.h>

/*******************************************************************************
 * CONSTANT AND MACRO DEFINITIONS USING #DEFINE
 ******************************************************************************/
#define BRIGHT_STEP     25
#define BRIGHT_MAX      255
#define BRIGHT_INIT     125

#define DUTYMAX		62          //cant_duties=f_clock*T_pwm
#define DUTY_0    	20			//62*0.33
#define DUTY_1    	40 			//62*0.65

#define ON_VALUE    (ROWS_CANT+1)   // Matriz completa prendida
#define OFF_VALUE   (ROWS_CANT+2)   // Matriz completa apagada

#define MATRIX_PERIOD 		((1.0/MATRIX_FPS)*1000)
#define INACTIVITY_COUNTER 	(SONG_TIMEOUT/MATRIX_PERIOD)
/*******************************************************************************
 * ENUMERATIONS AND STRUCTURES AND TYPEDEFS
 ******************************************************************************/
typedef struct{
	PWM_DUTY_t green[8];
    PWM_DUTY_t red[8];
    PWM_DUTY_t blue[8];
} LED_DUTY;


/*******************************************************************************
 * STATIC VARIABLES AND CONST VARIABLES WITH FILE LEVEL SCOPE
 ******************************************************************************/
static uint8_t brightness=BRIGHT_INIT;
static uint8_t lastBright=BRIGHT_INIT;

static LED_DUTY matrixduty[LEDS_CANT+1];    // Matriz que tiene los duty a enviar
// Despues de mandar los dutys hay que mandamos una memoria más de 0 para el reset code
static uint8_t targetColumns[8], actualColumns[8];

static tim_id_t timerMatrix;
static tim_id_t timeoutTimer;

void refreshMatrix();
void calcColumnsMatrix(uint8_t* columnsValues);
uint8_t isSongRunning(uint8_t* columnsValues);

/*******************************************************************************
 *******************************************************************************
                        GLOBAL FUNCTION DEFINITIONS
 *******************************************************************************
 ******************************************************************************/
void initMatrix(){
    DMA_initDisplayTable((uint32_t) matrixduty);
    timerMatrix = timerGetId();
    timeoutTimer = timerGetId();
    clearMatrix();
    timerStart(timerMatrix, TIMER_MS2TICKS(MATRIX_PERIOD), TIM_MODE_PERIODIC, refreshMatrix);
}

// Incrementa el brillo de la matriz
void increase_bright(){
	brightness=((brightness+BRIGHT_STEP)>BRIGHT_MAX)? BRIGHT_MAX:brightness+BRIGHT_STEP;
}

// decrementa el brillo de la matriz
void decrease_bright(){
	brightness=((brightness-BRIGHT_STEP)<=5)? 0:brightness-BRIGHT_STEP;
}

uint8_t get_bright(){
    return brightness;
}

// Prender toda la matrix en blanco con el brightness local
void fullMatrixON(){
	targetColumns[0]=ON_VALUE;
}

void clearMatrix(){
	targetColumns[0]=OFF_VALUE;
}


// Prende columnas de la matriz de leds
void setColumnsMatrix(uint8_t* columnsValues){
	targetColumns[0]=columnsValues[0];

	if(isSongRunning(columnsValues)){ // Si se está reproduciendo cancion
		for(uint8_t i=0; i<COLS_CANT; i++){	//Genero un "piso" de un led prendido
			targetColumns[i]=columnsValues[i]+1;
            targetColumns[i]=(targetColumns[i]>ROWS_CANT)? ROWS_CANT:targetColumns[i]; 
		}
	}

	else if(isVumeterMode()){					// Si no muestro nada
		for(uint8_t i=0; i<COLS_CANT; i++){		// pero estoy en modo vumetro
			targetColumns[i]=0;					// no prendo leds
		}
	}
}

// Prende un led de la matriz.
void setLedMatrix(uint8_t fila, uint8_t columna, LED_RGB* color){
	uint8_t led=8*fila+columna; // Calc numero de led

	// Calculamos intensidades segun el brillo
	LED_RGB aux;
	aux.blue= (uint8_t)(color->blue * brightness/255.0);
	aux.red= (uint8_t)(color->red * brightness/255.0);
	aux.green= (uint8_t)(color->green * brightness/255.0);

	//Convertimos a PWM y guaramos en la matriz
    for(uint8_t bit=0; bit< 8; bit++){
        (matrixduty[led]).red[7-bit] = (aux.red&(1<<bit))? DUTY_1:DUTY_0;
        (matrixduty[led]).green[7-bit] = (aux.green&(1<<bit))? DUTY_1:DUTY_0;
        (matrixduty[led]).blue[7-bit] = (aux.blue&(1<<bit))? DUTY_1:DUTY_0;
    }

    DMA_displayTable();
}

// La matriz esta en modo vumetro?
uint8_t isVumeterMode(){
	if(targetColumns[0]!=ON_VALUE && targetColumns[0]!=OFF_VALUE){
		return 1;
	}
	else{
		return 0;
	}
}

/*******************************************************************************
 *******************************************************************************
                        LOCAL FUNCTION DEFINITIONS
 *******************************************************************************
 ******************************************************************************/
void refreshMatrix(){
	static uint8_t auxCounter=0;
	static uint8_t init=1;

    // Revisamos si cambio el brillo
    uint8_t changedBright= (lastBright != brightness)? 1:0;
    lastBright = brightness;


    // Revisamos si llegamos al target
    uint8_t equals=1;
    for(uint8_t i; i<COLS_CANT; i++){
        if(actualColumns[i] != targetColumns[i]){
            equals=0;
            break;
        }
    }

    /*if(!isSongRunning(targetColumns)){
    	if(auxCounter++>=INACTIVITY_COUNTER){
    		clearMatrix();
    		auxCounter=0;
    	}
    }
    else{
    	auxCounter=0;
    }*/

    // Si no llegamos al target o cambio el brillo:
    //Actualizamos el proximo valor de cada columna
    if(!equals || changedBright || init){
    	init=0;
        if(!isVumeterMode()){  						// A ON / OFF
                actualColumns[0]=targetColumns[0];	// hago transicion brusca
        }

         else if(actualColumns[0]==ON_VALUE || actualColumns[0]==OFF_VALUE){
            for(uint8_t i=0; i<COLS_CANT; i++){			// A vúmetro desde ON/OFF
                actualColumns[i]=targetColumns[i];
            }
        }

        else {
            for(uint8_t i=0; i<COLS_CANT; i++){             // A vúmetro:
                if(actualColumns[i]<targetColumns[i]){      // desde vúmetro
                    actualColumns[i]++;
                }
                else if(actualColumns[i]>targetColumns[i]){
                    actualColumns[i]--;
                }
            }

        }

        // Calculamos y mandamos la nueva matriz de PWMs
        calcColumnsMatrix(actualColumns);
        DMA_displayTable();
    }
}

// Calcula los valores de PWMs a mandar
void calcColumnsMatrix(uint8_t* columnsValues){
    LED_RGB auxColor;
    uint8_t value;
    uint8_t led_base;
    uint8_t aux;

    if(columnsValues[0] != ON_VALUE && columnsValues[0] != OFF_VALUE){       // CASO VUMETRO

        // Apagamos todos los leds de la matriz
        for(uint8_t led=0; led< LEDS_CANT; led++){
                for(uint8_t bit=0; bit< 8; bit++){
                    (matrixduty[led]).red[bit] = DUTY_0;
                    (matrixduty[led]).green[bit] = DUTY_0;
                    (matrixduty[led]).blue[bit] = DUTY_0;
                }
            }


        for(uint8_t column=0; column<COLS_CANT; column++){
            value=columnsValues[column];
            led_base = ROWS_CANT*column;	// Calc el primer led de la columna

			value-=1;			// Compensacion para que value=1 sea verde puro
			auxColor.blue=0;

			aux= (255-value*31);					// Calculamos la intensidad de rojo en funcion
			aux=(aux<15)? 0:aux;					// de value para hacer el degradado
			aux = (uint8_t)(aux*brightness/255.0);	// Ponderamos por la intensidad de brillo
			auxColor.green=(aux<15)? 0:aux;

			aux = value*31;							// Calculamos la intensidad de verde en funcion
			aux = (aux>240)? 255:aux;				// de value para hacer el degradado
			aux = (uint8_t) (aux*brightness/255.0);	// Ponderamos por la intensidad de brillo
			auxColor.red=(aux>240)? 255:aux;

			for(uint8_t led=0; led< value+1; led++){
				for(uint8_t bit=0; bit< 8; bit++){
					// Calc y guardamos en la matriz los valores de los bits a encender
					//en funcion del color deseado
					(matrixduty[led_base+led]).red[7-bit] = (auxColor.red&(1<<bit))? DUTY_1:DUTY_0;
					(matrixduty[led_base+led]).green[7-bit] = (auxColor.green&(1<<bit))? DUTY_1:DUTY_0;
					(matrixduty[led_base+led]).blue[7-bit] = (auxColor.blue&(1<<bit))? DUTY_1:DUTY_0;
				}

            }
        }
    }

    else if(columnsValues[0] == ON_VALUE){      // CASO PRENDER
        PWM_DUTY_t aux[8];
        for(uint8_t bit=0; bit<8; bit++){
            aux[7-bit]= (brightness&(1<<bit))? DUTY_1:DUTY_0;
        }

        for(uint8_t led=0; led< LEDS_CANT; led++){
            for(uint8_t bit=0; bit< 8; bit++){
                (matrixduty[led]).red[bit] = aux[bit];
                (matrixduty[led]).green[bit] = aux[bit];
                (matrixduty[led]).blue[bit] = aux[bit];

            }
        }
    }

    else{                                       // CASO APAGAR
        // Cargamos 0's lógicos en la matriz
	    for(uint8_t led=0; led< LEDS_CANT; led++){
            for(uint8_t bit=0; bit< 8; bit++){
                (matrixduty[led]).red[bit] = DUTY_0;
                (matrixduty[led]).green[bit] = DUTY_0;
                (matrixduty[led]).blue[bit] = DUTY_0;
            }
        }
    }
}

// Verifico si hay alguna columna distinta de 0 en modo vúmetro
uint8_t isSongRunning(uint8_t* columnsValues){
	if(columnsValues[0]==OFF_VALUE || columnsValues[0]==ON_VALUE){
		return 0;
	}

	for(uint8_t i=0; i<COLS_CANT; i++){
		if(!columnsValues[i]){
			return 1;
		}
	}
	return 0;
}
