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
#include "timer/timer.h"

/*******************************************************************************
 * CONSTANT AND MACRO DEFINITIONS USING #DEFINE
 ******************************************************************************/
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

#define MY_LCD_CONFIG (0x08)    // LCD_4BITMODE | LCD_2LINE | LCD_5x8DOTS

// flags for backlight control
#define LCD_BACKLIGHT 0x08
#define LCD_NOBACKLIGHT 0x00

#define En 0x04  				// Enable bit
#define Rw 0x02  				// Read/Write bit
#define Rs 0x01  				// Register select bit
#define NO_CONTROL_PIN	0x00
#define DELAY_TX		0x08

#define NULL_LINE	("                ")
#define NULL_CHAR	(' ')

/*******************************************************************************
 * FUNCTION PROTOTYPES FOR PRIVATE FUNCTIONS WITH FILE LEVEL SCOPE
 ******************************************************************************/
void refreshScreen();
void homeDisplay();

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
static uint8_t dispControl, displayMode, backlightState, refresh;
static Tx_msg writeBuff;
static tim_id_t timerId;
static char screenText[CANT_ROWS][CANT_COLS];

/*******************************************************************************
 *******************************************************************************
                        GLOBAL FUNCTION DEFINITIONS
 *******************************************************************************
 ******************************************************************************/
void initDisplay(){
	I2CmInit(I2C_ID);
    timerId=timerGetId();

    //Entramos en modo 4-Bits (Pag 46 datasheet)
    I2CSendNybble(0x30);    				// Primer attemp
    timerDelay(TIMER_MS2TICKS(12));			// Delay de 10ms
    I2CSendNybble(0x30);    				// Segundo attemp
    timerDelay(TIMER_MS2TICKS(7));			// Delay de 5ms
    I2CSendNybble(0x30);    				// Tercer attemp
    timerDelay(TIMER_MS2TICKS(2));			// Delay de 0.5ms

    I2CSendNybble(0x20);

	// Prendemos la luz de fondo del display
    backlightState = LCD_BACKLIGHT;

    // Arrancamos a configurar
    I2CSendCommand(LCD_FUNCTIONSET| MY_LCD_CONFIG, NO_CONTROL_PIN);


    dispControl = LCD_DISPLAYON | LCD_CURSOROFF | LCD_BLINKOFF;
    I2CSendCommand(LCD_DISPLAYCONTROL | dispControl, NO_CONTROL_PIN);

    displayMode = LCD_ENTRYLEFT | LCD_ENTRYSHIFTDISABLE; 
    I2CSendCommand(LCD_ENTRYMODESET |  displayMode, NO_CONTROL_PIN);

	// Ponemos el display como nuevo
    clearDisplay();
    homeDisplay();

    timerDelay(TIMER_MS2TICKS(20));

    refresh=1;
    timerStart(timerId, TIMER_MS2TICKS(REFRESH_PERIOD_MS), TIM_MODE_PERIODIC, refreshScreen);
}

// Borra toda la linea row y escribe text encima
void displayLine(int row, char* text){
    clearLine(row);

	for(uint8_t i=0; i<CANT_COLS && text[i]!=0; i++){
        screenText[row][i]=text[i];
    }

}

// Pone el cursor en (row,column) y escribe text a partir de ahi
// Espera un terminador '\0' o el fin del renglon
void displayText(int row, int column, char* text){
    for(uint8_t i=0; (i < CANT_COLS-column) && text[i]!=0 ; i++){
        screenText[row][column+i]= text[i];
    }
}

void displayChar(int row, int column, char character){
    screenText[row][column]=character;
}

void clearLine(uint8_t row){
	for(uint8_t j=0; j<CANT_COLS; j++){
		screenText[row][j]=NULL_CHAR;
	}
}
void clearScreen(){
    for(uint8_t i=0; i<CANT_ROWS; i++){
        for(uint8_t j=0; j<CANT_COLS; j++){
            screenText[i][j]=NULL_CHAR;
        }
    }
}

void turnONRefresh(){
    refresh=1;
}

void turnOFFRefresh(){
    refresh=0;
}
/*******************************************************************************
 *******************************************************************************
                        LOCAL FUNCTION DEFINITIONS
 *******************************************************************************
 ******************************************************************************/
void refreshScreen(){
    if(refresh){
        setCursor(0, 0);                        // reseteo la pos. del cursor
        writeText(screenText[0], CANT_COLS);    // mando cada linea del display
        setCursor(0, 1);
        writeText(screenText[1], CANT_COLS);
    }
}

/**********************************************************
*****************     HIGH LEVEL      *********************
**********************************************************/
void clearDisplay(){
    I2CSendCommand(LCD_CLEARDISPLAY, NO_CONTROL_PIN);
    pushTransaction(DELAY_TX);
    pushTransaction(DELAY_TX);
    pushTransaction(DELAY_TX);
}

void homeDisplay(){
    I2CSendCommand(LCD_RETURNHOME, NO_CONTROL_PIN);
    pushTransaction(DELAY_TX);
    pushTransaction(DELAY_TX);
    pushTransaction(DELAY_TX);
}

// Col va de 0 a 7; row de 0 a 1 
void setCursor(uint8_t col, uint8_t row){
    uint8_t row_offset = (!row)? 0x00: 0x40;
	I2CSendCommand(LCD_SETDDRAMADDR | (col + row_offset), NO_CONTROL_PIN);
}

void displayOFF() {
	dispControl &= ~LCD_DISPLAYON;
	I2CSendCommand(LCD_DISPLAYCONTROL | dispControl, NO_CONTROL_PIN);
}
void displayON() {
	dispControl |= LCD_DISPLAYON;
	I2CSendCommand(LCD_DISPLAYCONTROL | dispControl, NO_CONTROL_PIN);
}

void cursorOFF() {
	dispControl &= ~LCD_CURSORON;
	I2CSendCommand(LCD_DISPLAYCONTROL | dispControl, NO_CONTROL_PIN);
}
void cursorON() {
	dispControl |= LCD_CURSORON;
	I2CSendCommand(LCD_DISPLAYCONTROL | dispControl, NO_CONTROL_PIN);
}

void blinkOFF() {
	dispControl &= ~LCD_BLINKON;
	I2CSendCommand(LCD_DISPLAYCONTROL | dispControl, NO_CONTROL_PIN);
}
void blinkON() {
	dispControl |= LCD_BLINKON;
	I2CSendCommand(LCD_DISPLAYCONTROL | dispControl, NO_CONTROL_PIN);
}

void scrollLeft() {
	I2CSendCommand(LCD_CURSORSHIFT | LCD_DISPLAYMOVE | LCD_MOVELEFT, NO_CONTROL_PIN);
}
void scrollRight() {
	I2CSendCommand(LCD_CURSORSHIFT | LCD_DISPLAYMOVE | LCD_MOVERIGHT, NO_CONTROL_PIN);
}

// Cursor desp hacia derecha por caracter
void leftToRight() {
	displayMode |= LCD_ENTRYLEFT;
	I2CSendCommand(LCD_ENTRYMODESET |  displayMode, NO_CONTROL_PIN);
}
// Cursor desp hacia izq por caracter
void rightToLeft() {
	displayMode &= ~LCD_ENTRYLEFT;
	I2CSendCommand(LCD_ENTRYMODESET |  displayMode, NO_CONTROL_PIN);
}

// El cursor se fija y se desplaza el disp al escribir
void autoscrollON() {
	displayMode |= LCD_ENTRYSHIFTENABLE;
	I2CSendCommand(LCD_ENTRYMODESET |  displayMode, NO_CONTROL_PIN);
}
void autoscrollOFF() {
	displayMode &= ~LCD_ENTRYSHIFTENABLE;
	I2CSendCommand(LCD_ENTRYMODESET |  displayMode, NO_CONTROL_PIN);
}

void backlightOFF() {
	backlightState=LCD_NOBACKLIGHT;
    writeBuff= backlightState;
	pushTransaction(writeBuff);
}
void backlightON() {
	backlightState=LCD_BACKLIGHT;
    writeBuff= backlightState;
	pushTransaction(writeBuff);
}

void writeText(char* text, uint8_t cant){
	for(uint8_t cont=0; cont<cant; cont++){
		// Es necesario mandar Rs entre los pines de control para formar
		//el comando de escribir.
        I2CSendCommand(text[cont], Rs);
    }
}

void ready2sleep(){
	displayOFF();
	turnOFFRefresh();
	while (!isBufEmpty());
	while (isI2CBusy(I2C_ID));
	disableI2C(I2C_ID);
}

void aftersleep(){
	enableI2C(I2C_ID);
	displayON();
	turnONRefresh();
}
/**********************************************************
*****************     LOW LEVEL      *********************
**********************************************************/
// Para enviar comando de 8 bits
// poner metadata en Rs si se quiere escribir texto
// blocking = 0 para stakear mensajes en el buffer
void I2CSendCommand(uint8_t msg, uint8_t metadata){

	// Mando primero el high nybble
    I2CSendNybble(  (msg & 0xF0) | metadata);

    // Mando luego el low nybble
    I2CSendNybble( ((msg<<4) & 0xF0) | metadata);
}

/* El nybble a mandar debe estar en el nybble superior del uint8_t
 * La idea es (Fig 25, pag 58):
 *  1ero: mandar los bits de control. (y de paso mandamos los bits de data)
 *  2do: Manteniendo los bits de control poner enable en 1
 *  3ero: Una vez aceptado el comando, poner enable en 0
 */
void I2CSendNybble(uint8_t nybble){

	writeBuff = nybble | backlightState;

	pushTransaction(writeBuff);
	pulseEnable(writeBuff);


}

/* Para que el display sepa que le estás hablando a él hay que mandar un pulso en 
1 en el pin ENABLE y despues un pulso en 0 en ENABLE. 
*/ 
 
void pulseEnable(uint8_t msg){
	writeBuff = msg | En;
	pushTransaction(writeBuff);

	writeBuff= msg & (~En);
	pushTransaction(writeBuff);
}
