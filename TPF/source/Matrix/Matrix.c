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


/*******************************************************************************
 * CONSTANT AND MACRO DEFINITIONS USING #DEFINE
 ******************************************************************************/
#define BRIGHT_STEP     25
#define BRIGHT_MAX      255
#define BRIGHT_INIT     125

#define DUTYMAX		62          //cant_duties=f_clock*T_pwm
#define DUTY_0    	20			//62*0.33
#define DUTY_1    	40 			//62*0.65


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

static LED_DUTY matrixduty[LEDS_CANT+1];    // Matriz que tiene los duty a enviar
// Despues de mandar los dutys hay que mandamos una memoria más de 0 para el reset code

/*******************************************************************************
 *******************************************************************************
                        GLOBAL FUNCTION DEFINITIONS
 *******************************************************************************
 ******************************************************************************/
void initMatrix(){
    DMA_initDisplayTable((uint32_t) matrixduty);
    clearMatrix(1);
}

// Incrementa el brillo de la matriz
void increase_bright(){
	brightness=((brightness+BRIGHT_STEP)>BRIGHT_MAX)? BRIGHT_MAX:brightness+BRIGHT_STEP;
}

// decrementa el brillo de la matriz
void decrease_bright(){
	brightness=((brightness-BRIGHT_STEP)<0)? 0:brightness-BRIGHT_STEP;
}

uint8_t get_bright(){
    return brightness;
}

// Prender toda la matrix en blanco con el brightness local
void fullMatrixON(){
    
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

    DMA_displayTable();
}

// Prende columnas de la matriz de leds
void setColumnsMatrix(uint8_t* columnsValues){
    LED_RGB auxColor;
    uint8_t value;
    uint8_t led_base;
    uint8_t aux;

    // Apagamos todos los leds de la matriz
    for(uint8_t led=0; led< LEDS_CANT; led++){
            for(uint8_t bit=0; bit< 8; bit++){
                (matrixduty[led]).red[bit] = DUTY_0;
                (matrixduty[led]).green[bit] = DUTY_0;
                (matrixduty[led]).blue[bit] = DUTY_0;
            }
        }


    for(uint8_t column=0; column<8; column++){
        value=columnsValues[column];
        led_base = 8*column;	// Calc el primer led de la columna

        if(value){				// Si value==0, no se prenden leds en la columna
			value-=1;			// Compensacion para que los colores queden lindos
        	auxColor.blue=0;

        	aux= (255-value*31);					// Calculamos la intensidad de rojo en funcion
        	aux=(aux<15)? 0:aux;					// de value para hacer el degradado
			aux = (uint8_t)(aux*brightness/255.0);	// Ponderamos por la intensidad de brillo
			auxColor.red=(aux<15)? 0:aux;

			aux = value*31;							// Calculamos la intensidad de verde en funcion
			aux = (aux>240)? 255:aux;				// de value para hacer el degradado
			aux = (uint8_t) (aux*brightness/255.0);	// Ponderamos por la intensidad de brillo
			auxColor.green=(aux>240)? 255:aux;

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

    DMA_displayTable();
}

// Prende una columna puntual de la matriz de leds
void setColumnMatrix(uint8_t col, uint8_t value){
    uint8_t led_base=8*col;
    LED_RGB auxColor;
    uint8_t aux;

    for(uint8_t led=0; led< 8; led++){
		for(uint8_t bit=0; bit< 8; bit++){
			(matrixduty[led_base+led]).red[bit] = DUTY_0;
			(matrixduty[led_base+led]).green[bit] = DUTY_0;
			(matrixduty[led_base+led]).blue[bit] = DUTY_0;
		}
	}

    if(value){
		value-=1;
    	auxColor.blue=0;

    	aux= (255-value*31);
    	aux=(aux<15)? 0:aux;
		aux = (uint8_t)(aux*brightness/255.0);
		auxColor.red=(aux<15)? 0:aux;

		aux = value*31;
		aux = (aux>240)? 255:aux;
		aux = (uint8_t) (aux*brightness/255.0);
		auxColor.green=(aux>240)? 255:aux;

		for(uint8_t led=0; led< value+1; led++){
			for(uint8_t bit=0; bit< 8; bit++){
				(matrixduty[led_base+led]).red[7-bit] = (auxColor.red&(1<<bit))? DUTY_1:DUTY_0;
				(matrixduty[led_base+led]).green[7-bit] = (auxColor.green&(1<<bit))? DUTY_1:DUTY_0;
				(matrixduty[led_base+led]).blue[7-bit] = (auxColor.blue&(1<<bit))? DUTY_1:DUTY_0;
			}
		}
    }
    
    DMA_displayTable();
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

void clearMatrix(uint8_t init){
    // Cargamos 0's lógicos en la matriz
	for(uint8_t led=0; led< LEDS_CANT; led++){
        for(uint8_t bit=0; bit< 8; bit++){
            (matrixduty[led]).red[bit] = DUTY_0;
            (matrixduty[led]).green[bit] = DUTY_0;
            (matrixduty[led]).blue[bit] = DUTY_0;
        }
    }

    if(!init){ // Si estamos inicializando no podemos usar DMA
    	DMA_displayTable();
    }
}

/*******************************************************************************
 *******************************************************************************
                        LOCAL FUNCTION DEFINITIONS
 *******************************************************************************
 ******************************************************************************/
