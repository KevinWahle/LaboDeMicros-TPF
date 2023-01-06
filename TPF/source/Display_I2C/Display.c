/***************************************************************************//**
  @file		Matrix.c
  @brief	HAL Display I2C
  @author	Grupo 5
  @date		29 Dic. 2022
 ******************************************************************************/


/*******************************************************************************
 * INCLUDE HEADER FILES
 ******************************************************************************/
#include "Display.h"
#include "../I2Cm/I2Cm.h"
#include "../timer/timer.h"

/*******************************************************************************
 * CONSTANT AND MACRO DEFINITIONS USING #DEFINE
 ******************************************************************************/
#define DEBUG_DELAY	15

// commands (Pag 24-28)
#define LCD_CLEARDISPLAY 0x01
#define LCD_RETURNHOME 0x02
#define LCD_ENTRYMODESET 0x04
#define LCD_DISPLAYCONTROL 0x08
#define LCD_CURSORSHIFT 0x10
#define LCD_FUNCTIONSET 0x20
#define LCD_SETCGRAMADDR 0x40
#define LCD_SETDDRAMADDR 0x80

// flags for display entry mode
#define LCD_ENTRYRIGHT 0x00
#define LCD_ENTRYLEFT 0x02
#define LCD_ENTRYSHIFTENABLE 0x01
#define LCD_ENTRYSHIFTDISABLE 0x00

// flags for display on/off control
#define LCD_DISPLAYON 0x04
#define LCD_DISPLAYOFF 0x00
#define LCD_CURSORON 0x02
#define LCD_CURSOROFF 0x00
#define LCD_BLINKON 0x01
#define LCD_BLINKOFF 0x00

// flags for display/cursor shift
#define LCD_DISPLAYMOVE 0x08
#define LCD_CURSORMOVE 0x00
#define LCD_MOVERIGHT 0x04
#define LCD_MOVELEFT 0x00

// flags for function set
#define LCD_8BITMODE 0x10
#define LCD_4BITMODE 0x00
#define LCD_2LINE 0x08
#define LCD_1LINE 0x00
#define LCD_5x10DOTS 0x04
#define LCD_5x8DOTS 0x00

#define MY_LCD_CONFIG (0x08)

// flags for backlight control
#define LCD_BACKLIGHT 0x08
#define LCD_NOBACKLIGHT 0x00

#define En 0x04  // Enable bit
#define Rw 0x02  // Read/Write bit
#define Rs 0x01  // Register select bit


/*******************************************************************************
 * ENUMERATIONS AND STRUCTURES AND TYPEDEFS
 ******************************************************************************/


/*******************************************************************************
 * VARIABLES WITH GLOBAL SCOPE
 ******************************************************************************/

// +ej: unsigned int anio_actual;+


/*******************************************************************************
 * FUNCTION PROTOTYPES FOR PRIVATE FUNCTIONS WITH FILE LEVEL SCOPE
 ******************************************************************************/
void homeDisplay();

// Col va de 0 a 7; row de 0 a 1 
void setCursor(uint8_t col, uint8_t row);

void displayOFF();
void displayON();
void cursorOFF();
void cursorON();
void blinkOFF();
void blinkON();

void scrollLeft(void);
void scrollRight(void);

// Cursor desp hacia derecha por caracter
void leftToRight(void);
// Cursor desp hacia izq por caracter
void rightToLeft(void);

// El cursor se fija y se desplaza el disp al escribir
void autoscrollON(void);
void autoscrollOFF(void);

void backlightOFF(void);
void backlightON(void);

void writeText(char* text, uint8_t cant);

//MCAL
void I2CSendCommand(uint8_t msg, uint8_t metadata);
void I2CSendNybble(uint8_t nybble);
void pulseEnable(uint8_t msg);
/*******************************************************************************
 * ROM CONST VARIABLES WITH FILE LEVEL SCOPE
 ******************************************************************************/

/*******************************************************************************
 * STATIC VARIABLES AND CONST VARIABLES WITH FILE LEVEL SCOPE
 ******************************************************************************/
static uint8_t writeBuffer;
static uint8_t dispControl, displayMode, backlightState;
uint8_t readBuff;
/*******************************************************************************
 *******************************************************************************
                        GLOBAL FUNCTION DEFINITIONS
 *******************************************************************************
 ******************************************************************************/
void initDisplay(){
    
	I2CmInit(I2C_ID);

    // Esperar a que todo el display se alimente bien
    timerDelay(TIMER_MS2TICKS(50));

    // Activamos los pines de control
    backlightState=LCD_NOBACKLIGHT;        //REVISAR: no debería ir el backlight?

    I2CmStartTransaction(I2C_ID, DIS_ADDR, &backlightState, 1, NULL, 0);
    timerDelay(TIMER_MS2TICKS(DEBUG_DELAY));	//TODO: borrar

    //Entramos en modo 4-Bits (Pag 46 datasheet)
    I2CSendNybble(0x30);    // Primer attemp
    timerDelay(TIMER_MS2TICKS(DEBUG_DELAY));	// Delay de 10
    I2CSendNybble(0x30);    // Segundo attemp
    timerDelay(TIMER_MS2TICKS(DEBUG_DELAY));	// Delay de 1
    I2CSendNybble(0x30);    // Tercer attemp
    timerDelay(TIMER_MS2TICKS(DEBUG_DELAY));	// Delay de 0.5
    
    // Arrancamos a configurar
    I2CSendNybble(0x20);
    I2CSendCommand(LCD_FUNCTIONSET| MY_LCD_CONFIG, 0);
    
    dispControl = LCD_DISPLAYON | LCD_CURSOROFF | LCD_BLINKOFF;
    I2CSendCommand(LCD_DISPLAYCONTROL | dispControl, 0);
    
    displayMode = LCD_ENTRYLEFT | LCD_ENTRYSHIFTDISABLE; 
    I2CSendCommand(LCD_ENTRYMODESET |  displayMode, 0);
    
    clearDisplay();
    homeDisplay();

}

void displayLine(int row, char* text){
    setCursor(0, row);
    writeText(text, CANT_COLS);
}

void displayText(int row, int column, char* text){
    setCursor(column, row);
    writeText(text, CANT_ROWS*CANT_COLS*3); // La idea es que corte por terminador 
}

/*******************************************************************************
 *******************************************************************************
                        LOCAL FUNCTION DEFINITIONS
 *******************************************************************************
 ******************************************************************************/

/**********************************************************
*****************     HIGH LEVEL      *********************
**********************************************************/
void clearDisplay(){
    I2CSendCommand(LCD_CLEARDISPLAY, 0);
    timerDelay(TIMER_MS2TICKS(1.52));
}

void homeDisplay(){
    I2CSendCommand(LCD_RETURNHOME, 0);
    timerDelay(TIMER_MS2TICKS(1.52));
}

// Col va de 0 a 7; row de 0 a 1 
void setCursor(uint8_t col, uint8_t row){
    uint8_t row_offset = (!row)? 0x00: 0x40;
	I2CSendCommand(LCD_SETDDRAMADDR | (col + row_offset), 0);    
}

void displayOFF() {
	dispControl &= ~LCD_DISPLAYON;
	I2CSendCommand(LCD_DISPLAYCONTROL | dispControl, 0);
}
void displayON() {
	dispControl |= LCD_DISPLAYON;
	I2CSendCommand(LCD_DISPLAYCONTROL | dispControl, 0);
}

void cursorOFF() {
	dispControl &= ~LCD_CURSORON;
	I2CSendCommand(LCD_DISPLAYCONTROL | dispControl, 0);
}
void cursorON() {
	dispControl |= LCD_CURSORON;
	I2CSendCommand(LCD_DISPLAYCONTROL | dispControl, 0);
}

void blinkOFF() {
	dispControl &= ~LCD_BLINKON;
	I2CSendCommand(LCD_DISPLAYCONTROL | dispControl, 0);
}
void blinkON() {
	dispControl |= LCD_BLINKON;
	I2CSendCommand(LCD_DISPLAYCONTROL | dispControl, 0);
}

void scrollLeft() {
	I2CSendCommand(LCD_CURSORSHIFT | LCD_DISPLAYMOVE | LCD_MOVELEFT, 0);
}
void scrollRight() {
	I2CSendCommand(LCD_CURSORSHIFT | LCD_DISPLAYMOVE | LCD_MOVERIGHT, 0);
}

// Cursor desp hacia derecha por caracter
void leftToRight() {
	displayMode |= LCD_ENTRYLEFT;
	I2CSendCommand(LCD_ENTRYMODESET |  displayMode, 0);
}
// Cursor desp hacia izq por caracter
void rightToLeft() {
	displayMode &= ~LCD_ENTRYLEFT;
	I2CSendCommand(LCD_ENTRYMODESET |  displayMode, 0);
}

// El cursor se fija y se desplaza el disp al escribir
void autoscrollON() {
	displayMode |= LCD_ENTRYSHIFTENABLE;
	I2CSendCommand(LCD_ENTRYMODESET |  displayMode, 0);
}
void autoscrollOFF() {
	displayMode &= ~LCD_ENTRYSHIFTENABLE;
	I2CSendCommand(LCD_ENTRYMODESET |  displayMode, 0);
}

void backlightOFF() {
	backlightState=LCD_NOBACKLIGHT;
    I2CmStartTransaction(I2C_ID, DIS_ADDR, &backlightState, 1, NULL, 0);
}
void backlightON() {
	backlightState=LCD_BACKLIGHT;
    I2CmStartTransaction(I2C_ID, DIS_ADDR, &backlightState, 1, NULL, 0);
}

void writeText(char* text, uint8_t cant){
    for(uint8_t cont=0; text[cont]!=0 && cont<cant; cont++){
        I2CSendCommand(text[cont], Rs);
    }
}
/**********************************************************
*****************     LOW LEVEL      *********************
**********************************************************/
// Para enviar comando de 8 bits
void I2CSendCommand(uint8_t msg, uint8_t metadata){
    // Mando primero el high nybble
    I2CSendNybble(  (msg & 0xF0) | metadata);
    // Mando luego el low nybble
    I2CSendNybble( ((msg<<4) & 0xF0) | metadata);
}

// El nybble debe estar en el nybble superior del uint8_t
void I2CSendNybble(uint8_t nybble){
    writeBuffer=nybble|backlightState;   // Agrego los pines de control
    I2CmStartTransaction(I2C_ID, DIS_ADDR, &writeBuffer, 1, NULL, 0);
    timerDelay(TIMER_MS2TICKS(DEBUG_DELAY));	//TODO: borrar
    pulseEnable(nybble);
}

void pulseEnable(uint8_t msg){
    uint8_t aux = msg | En;
    I2CmStartTransaction(I2C_ID, DIS_ADDR, &aux, 1, NULL, 0);
    timerDelay(TIMER_MS2TICKS(DEBUG_DELAY));	//TODO: borrar
    //TODO: Averiguar si va delay de 450ns y como hacerlo

    aux = msg & (~En);
    I2CmStartTransaction(I2C_ID, DIS_ADDR, &aux, 1, NULL, 0);
    timerDelay(TIMER_MS2TICKS(DEBUG_DELAY));	//TODO: borrar
    //TODO: Averiguar si va delay de 37us y como hacerlo    
}
