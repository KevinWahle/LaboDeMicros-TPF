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
#include "../FTM/FTM.h"
#include "../DMA/DMA.h"

/*******************************************************************************
 * CONSTANT AND MACRO DEFINITIONS USING #DEFINE
 ******************************************************************************/
#define BRIGHT_STEP     25
#define BRIGHT_MAX      255
#define BRIGHT_INIT     125

#define DUTYMAX (2<<12)
#define T0H 400
#define T0L 850
#define DUTY0   (T0H/(T0H+T0L))
#define CnV0    (DUTYMAX*DUTY0) 
#define T1H 800
#define T1L 450
#define DUTY1   (T1H/(T1H+T1L))
#define CnV1    (DUTYMAX*DUTY1) 


/*******************************************************************************
 * ENUMERATIONS AND STRUCTURES AND TYPEDEFS
 ******************************************************************************/
typedef struct{
    uint8_t green[8];
    uint8_t red[8];
    uint8_t blue[8];
} LED_DUTY;


/*******************************************************************************
 * VARIABLES WITH GLOBAL SCOPE
 ******************************************************************************/

// +ej: unsigned int anio_actual;+


/*******************************************************************************
 * FUNCTION PROTOTYPES FOR PRIVATE FUNCTIONS WITH FILE LEVEL SCOPE
 ******************************************************************************/


/*******************************************************************************
 * ROM CONST VARIABLES WITH FILE LEVEL SCOPE
 ******************************************************************************/

/*******************************************************************************
 * STATIC VARIABLES AND CONST VARIABLES WITH FILE LEVEL SCOPE
 ******************************************************************************/
uint8_t brightness=BRIGHT_INIT;

static LED_DUTY matrixduty[LEDS_CANT+2];    // Matriz que tiene los duty a enviar    
// Despues de mandar los dutys hay que mandar 50us de reset code = 2*T_leds
// Mandar GRB con high bit primero

/*******************************************************************************
 *******************************************************************************
                        GLOBAL FUNCTION DEFINITIONS
 *******************************************************************************
 ******************************************************************************/
void initMatrix(){
    //Init DMA

    

}

// Incrementa o decrementa el brillo de la matriz
void increase_bright(){
    if (brightness<BRIGHT_MAX-BRIGHT_STEP)
    {
        brightness+=BRIGHT_STEP;
        brightness=(brightness>240)? BRIGHT_MAX:brightness;
        fullMatrixON();
    }
    
}

void decrease_bright(){
    if (brightness>BRIGHT_STEP)
    {
        brightness-=BRIGHT_STEP;
        brightness=(brightness<15)? 0:brightness;
        fullMatrixON();
    }
}

uint8_t get_bright(){
    return brightness;
}

// Prender toda la matrix en blanco con el brightness local
void fullMatrixON(){
    
    uint8_t aux[8];
    for(uint8_t bit=0; bit<8; bit++){
        aux[7-bit]= (brightness|(1<<bit))? CnV1:CnV0; 
    }

    for(uint8_t led=0; led< LEDS_CANT; led++){
        for(uint8_t bit=0; bit< 8; bit++){
            (matrixduty[led]).red[bit] = aux[bit];
            (matrixduty[led]).green[bit] = aux[bit];
            (matrixduty[led]).blue[bit] = aux[bit];
        }
    }

    //TODO: Activar DMA
}

// Prende columnas de la matriz de leds
void setColumnsMatrix(uint8_t* columnsValues){
    LED_RGB auxColor;
    uint8_t value;
    uint8_t led_base;
    uint8_t aux;

    for(uint8_t column=0; column<8; column++){
        value=columnsValues[column];
        led_base = 8*column;

        auxColor.blue=0;
        
        aux = 255-value*31;              
        //TODO: Agregar regulacion de brillo
        //aux = (255-value*31)*(brightness/255);
        auxColor.red=(aux<10)? 0:aux;

        aux=value*31;                           
        //TODO: Agregar regulacion de brillo
        //aux = (value*31)*(brightness/255);
        auxColor.green=(aux>245)? 255:aux;

        for(uint8_t led=0; led< value; led++){
            for(uint8_t bit=0; bit< 8; bit++){
                (matrixduty[led_base+led]).red[bit] = (auxColor.red|(1<<bit))? CnV1:CnV0;
                (matrixduty[led_base+led]).green[bit] = (auxColor.green|(1<<bit))? CnV1:CnV0;
                (matrixduty[led_base+led]).blue[bit] = (auxColor.blue|(1<<bit))? CnV1:CnV0;
            }
        }
    }

    //TODO: Activar DMA
}

// Prende una columna puntual de la matriz de leds
void setColumnMatrix(uint8_t col, uint8_t value){
    uint8_t led_base=8*col;

    LED_RGB auxColor;
    auxColor.blue=0;

    uint8_t aux = 255-value*31;              //TODO: Agregar regulacion de brillo
    auxColor.red=(aux<10)? 0:aux;

    aux=value*31;                           //TODO: Agregar regulacion de brillo
    auxColor.green=(aux>245)? 255:aux;

    for(uint8_t led=0; led< value; led++){
        for(uint8_t bit=0; bit< 8; bit++){
            (matrixduty[led_base+led]).red[bit] = (auxColor.red|(1<<bit))? CnV1:CnV0;
            (matrixduty[led_base+led]).green[bit] = (auxColor.green|(1<<bit))? CnV1:CnV0;
            (matrixduty[led_base+led]).blue[bit] = (auxColor.blue|(1<<bit))? CnV1:CnV0;
        }
    }
    
    //TODO: Activar DMA
}

// Prende un led de la matriz
void setLedMatrix(uint8_t led, LED_RGB color){
    // Variar en funcion del brillo

    for(uint8_t bit=0; bit< 8; bit++){
        (matrixduty[led]).red[7-bit] = (color.red|(1<<bit))? CnV1:CnV0;
        (matrixduty[led]).green[7-bit] = (color.green|(1<<bit))? CnV1:CnV0;
        (matrixduty[led]).blue[7-bit] = (color.blue|(1<<bit))? CnV1:CnV0;
    }
    
    //TODO: Activar DMA
}

/*******************************************************************************
 *******************************************************************************
                        LOCAL FUNCTION DEFINITIONS
 *******************************************************************************
 ******************************************************************************/
void clearMatrix(){
    for(uint8_t led=0; led< LEDS_CANT; led++){
        for(uint8_t bit=0; bit< 8; bit++){
            (matrixduty[led]).red[bit] = CnV0;
            (matrixduty[led]).green[bit] = CnV0;
            (matrixduty[led]).blue[bit] = CnV0;
        }
    }


    //Agregamos los reset codes.
    for(uint8_t reset=0; reset< 2; reset++){
        for(uint8_t bit=0; bit< 8; bit++){
            (matrixduty[LEDS_CANT+reset]).red[bit] = 0;
            (matrixduty[LEDS_CANT+reset]).green[bit] = 0;
            (matrixduty[LEDS_CANT+reset]).blue[bit] = 0;
        }
    }

    //TODO: Activar DMA
}