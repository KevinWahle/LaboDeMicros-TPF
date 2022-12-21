/***************************************************************************//**
  @file		CAN.c
  @brief	HAL para comunicación CAN
  @author	Grupo 5
  @date		19 sep. 2022
 ******************************************************************************/

/*******************************************************************************
 * INCLUDE HEADER FILES
 ******************************************************************************/
#include "../SPI/SPI.h"
#include "CAN.h"
#include "../buffer/SPI_buffer.h"
#include "../MCAL/gpio.h"
#include <string.h>
#include <stdbool.h>

/*******************************************************************************
 * CONSTANT AND MACRO DEFINITIONS USING #DEFINE
 ******************************************************************************/

#define ENABLE_TP

#ifdef ENABLE_TP
#define TP_PIN	PORTNUM2PIN(PC, 9)
#endif

//Instructiones
#define READ        0x03
#define WRITE       0x02
#define BIT_MODIFY  0x05
#define RESET       0xC0

//Defines de direcciones de registros
#define RXF0SIDH_REG  0x00
#define RXF0SIDL_REG  0x01
#define BFPCTRL_REG   0x0C

#define RXM0SIDH_REG  0x20
#define RXM0SIDL_REG  0x21

#define CNF1_REG  0x2A
#define CNF2_REG  0x29
#define CNF3_REG  0x28

#define RXB0CTRL_REG  0x60
#define RXB0SIDH_REG  0x61
#define RXB0SIDL_REG  0x62
#define RXB0DLC_REG	  0x65

#define CANCTRL_REG     0x0F
#define CANINTE_REG     0x2B
#define CANINTF_REG     0x2C
#define TXRTSCTRL_REG   0x0D

#define TXB0CTRL_REG  0x30
#define TXB0SIDH_REG  0x31
#define TXB0SIDL_REG  0x32
#define TXB0DLC_REG   0x35

#define TXB0D0_REG  0x36
#define TXB0D1_REG  0x37
#define TXB0D2_REG  0x38
#define TXB0D3_REG  0x39
#define TXB0D4_REG  0x3A
#define TXB0D5_REG  0x3B
#define TXB0D6_REG  0x3C
#define TXB0D7_REG  0x3D

#define RXB0D0_REG  0x66
#define RXB0D1_REG  0x67
#define RXB0D2_REG  0x68
#define RXB0D3_REG  0x69
#define RXB0D4_REG  0x6A
#define RXB0D5_REG  0x6B
#define RXB0D6_REG  0x6C
#define RXB0D7_REG  0x6D


//Modes
#define WRITESIZE   3
#define READSIZE    3
#define BITMODSIZE  4

#define MAXBYTES 8
#define IRQ_CAN   PORTNUM2PIN(PC, 12) // PTC12
/*******************************************************************************
 * ENUMERATIONS AND STRUCTURES AND TYPEDEFS
 ******************************************************************************/


/*******************************************************************************
 * VARIABLES WITH GLOBAL SCOPE
*******************************************************************************/

static SPI_config_t config;
static SPI_config_t * myconfig;

static uint16_t myID;

static uint8_t interrupt;
static uint8_t RXdlc;
static uint8_t RXIDL, RXIDH;
static uint8_t Rxdata[8];
static CANMsg_t* MSGReceive; 

static bool done;


/*******************************************************************************
 * FUNCTION PROTOTYPES FOR PRIVATE FUNCTIONS WITH FILE LEVEL SCOPE
 ******************************************************************************/

void CANRead (uint8_t address, uint8_t* save, CBType mycb);
void CANBitModify(uint8_t address, uint8_t mask, uint8_t data);
void CANWrite (uint8_t address, uint8_t value);
void CANReset();

void CANReceive();
void viewinterrupt();
void readData();
void readEnd ();


/*******************************************************************************
 * ROM CONST VARIABLES WITH FILE LEVEL SCOPE
 ******************************************************************************/



/*******************************************************************************
 * STATIC VARIABLES AND CONST VARIABLES WITH FILE LEVEL SCOPE
 ******************************************************************************/


/*******************************************************************************
 *******************************************************************************
                        GLOBAL FUNCTION DEFINITIONS
 *******************************************************************************
 ******************************************************************************/
void SPI_init(){      //Configuración del SPI
    myconfig=&config;
    myconfig->type=MASTER;
    myconfig->PCS_inactive_state=1;
    myconfig->LSB_fist=0;
    myconfig->frame_size=8;
    myconfig->clk_pol=0;
    myconfig->clk_phase=0;
    myconfig->Baud_rate_scaler=0b0100;

    SPI_config(SPI_0,myconfig);
}

void CANInit(uint16_t ID, CANMsg_t* msgReceive){
    
    myID=ID; 
    MSGReceive=msgReceive;

#ifdef ENABLE_TP
	gpioMode(TP_PIN, OUTPUT);
	gpioWrite(TP_PIN, LOW);
#endif

    gpioMode(IRQ_CAN, INPUT);
    gpioIRQ(IRQ_CAN, GPIO_IRQ_MODE_FALLING_EDGE, CANReceive);   // Para interrupcion cuando llegue un mensaje
    
    SPI_init();
    CANReset();

    CANBitModify(CANCTRL_REG, 0xE0, 0x80); // Sets Configuration mode

    CANWrite(TXRTSCTRL_REG, 0x01); //Pin is used to request message transmission of TXB0 buffer (on falling edge)
    CANWrite(RXB0CTRL_REG, 0x60);  //Turns mask/filters off; receives any message.

    CANWrite(CNF1_REG, 0xC3); // SJW=4 BRP=3
    CANBitModify(CNF2_REG, 0x3F, 0x1E); //PHSEG1=(3+1) PRSEG=(6+1)     
    CANBitModify(CNF3_REG, 0x07, 0x03); //PHSEG2=(3+1)

    CANWrite(CANINTE_REG,0x01); //TX0IE: Transmit Buffer 0 Empty Interrupt Enable bit
                                //RX0IE: Receive Buffer 0 Full Interrupt Enable bit
 
    CANBitModify(CANCTRL_REG, 0xEF, 0x04); //Sets Normal Operation mode,One-Shot, Clock Enable and Preescaler
}

bool CANSend(uint8_t * data, uint8_t len){
  if (len>MAXBYTES){
    return false;
  }

  CANBitModify(TXB0DLC_REG, 0x0F, len); //Set Length (DLC)
  
  //Set ID
  uint8_t IDH = (uint8_t) ((myID>>3) & 0xFF);
  uint8_t IDL = (uint8_t) (myID & 0x07);

  CANBitModify(TXB0SIDH_REG, 0xFF, IDH); 
  CANBitModify(TXB0SIDL_REG, 0XE0, IDL<<5);

  //Set Data
  switch(len){
    case 8:
      CANWrite(TXB0D7_REG, data[7]);
    case 7:
      CANWrite(TXB0D6_REG, data[6]);
    case 6:
      CANWrite(TXB0D5_REG, data[5]);
    case 5:
      CANWrite(TXB0D4_REG, data[4]);
    case 4:
      CANWrite(TXB0D3_REG, data[3]);
    case 3:
      CANWrite(TXB0D2_REG, data[2]);
    case 2:
      CANWrite(TXB0D1_REG, data[1]);
    case 1:
      CANWrite(TXB0D0_REG, data[0]);
      break;

    default:
      break;
  }

  CANBitModify(TXB0CTRL_REG, 0x08, 0x08); //Buffer is currently pending transmission
  return true;
}

/*******************************************************************************
 *******************************************************************************
                        LOCAL FUNCTION DEFINITIONS
 *******************************************************************************
 ******************************************************************************/
void CANReset(){
  package mydata;

  mydata.msg=RESET;
  mydata.pSave=NULL;
  mydata.cb=NULL;
  mydata.read=false;
  mydata.cs_end=1;

  SPISend(SPI_0, &mydata, 1, 0);
}


void CANWrite (uint8_t address, uint8_t value){
  package data[WRITESIZE];
  
  data[0].msg=WRITE;
  data[0].pSave=NULL;
  data[0].cb=NULL;
  data[0].read=false;
  data[0].cs_end=0;
  
  data[1].msg=address;
  data[1].pSave=NULL;
  data[1].cb=NULL;
  data[1].read=false;
  data[1].cs_end=0;

  data[2].msg=value;
  data[2].pSave=NULL;
  data[2].cb=NULL;
  data[2].read=false;
  data[2].cs_end=1;

  SPISend(SPI_0, data, WRITESIZE, 0);
}


void CANRead (uint8_t address, uint8_t* save, CBType mycb){
  package data[READSIZE];
  
  data[0].msg=READ;
  data[0].pSave=NULL;
  data[0].cb=NULL;
  data[0].read=false;
  data[0].cs_end=0;
  
  data[1].msg=address;
  data[1].pSave=NULL;
  data[1].cb=NULL;
  data[1].read=false;
  data[1].cs_end=0;
  
  data[2].msg=0;
  data[2].pSave=save;
  data[2].cb=mycb;
  data[2].read=true;
  data[2].cs_end=1;

  SPISend(SPI_0, data, READSIZE, 0);
}


void CANBitModify(uint8_t address, uint8_t mask, uint8_t value){
  package data[BITMODSIZE];
  
  data[0].msg=BIT_MODIFY;
  data[0].pSave=NULL;
  data[0].cb=NULL;
  data[0].read=false;
  data[0].cs_end=0;
  
  data[1].msg=address;
  data[1].pSave=NULL;
  data[1].cb=NULL;
  data[1].read=false;
  data[1].cs_end=0;
  
  data[2].msg=mask;
  data[2].pSave=NULL;
  data[2].cb=NULL;
  data[2].read=false;
  data[2].cs_end=0;
  
  data[3].msg=value;
  data[3].pSave=NULL;
  data[3].cb=NULL;
  data[3].read=false;
  data[3].cs_end=1;
  SPISend(SPI_0, data, BITMODSIZE, 0);
}


void CANReceive(){
#ifdef ENABLE_TP
	gpioWrite(TP_PIN, HIGH);
#endif

  done=false; //Starting to receive
  CANRead(CANINTF_REG, &interrupt, viewinterrupt);

#ifdef ENABLE_TP
	gpioWrite(TP_PIN, LOW);
#endif
}

void viewinterrupt(){
//	CANWrite(CANINTF_REG, 0x00); // Apago todos los flags
	if ((interrupt%2)==1){  // Veo que sea la interrupcion que quiero (RX0IF)
		CANBitModify(CANINTF_REG, 0x01, 0x00); //Pongo el Flag en 0
		CANRead(RXB0DLC_REG, &RXdlc, readData); // Veo cuantos datos tengo
	}

}


void readData(){
  //Read data
  switch(RXdlc){
    case 8:
      CANRead(RXB0D7_REG, &Rxdata[7], NULL);
    case 7:
      CANRead(RXB0D6_REG, &Rxdata[6], NULL);
    case 6:
      CANRead(RXB0D5_REG, &Rxdata[5], NULL);
    case 5:
      CANRead(RXB0D4_REG, &Rxdata[4], NULL);
    case 4:
      CANRead(RXB0D3_REG, &Rxdata[3], NULL);
    case 3:
      CANRead(RXB0D2_REG, &Rxdata[2], NULL);
    case 2:
      CANRead(RXB0D1_REG, &Rxdata[1], NULL);
    case 1:
      CANRead(RXB0D0_REG, &Rxdata[0], NULL);
      break;
    default:
      break;
  }
    //Read ID 
    CANRead(RXB0SIDH_REG, &RXIDH, NULL);
    CANRead(RXB0SIDL_REG, &RXIDL, readEnd);

}


void readEnd (){
    RXIDL >>= 5;
    MSGReceive->ID = (RXIDH<<3) | RXIDL;
    MSGReceive->length = RXdlc;
    memcpy(MSGReceive->data, Rxdata, MAXBYTES);

    //Clear all
    RXIDL=0; RXIDH=0;
    RXdlc=0;
    for (int i=0; i<MAXBYTES; i++){
      Rxdata[i]=0;
    }

    done=true; //Reception done succesfull
}

bool newMsg(){
  bool newmsg=done;
  if(newmsg) done = false;
  return newmsg;
}







