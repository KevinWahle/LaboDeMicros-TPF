/***************************************************************************//**
  @file     SPI.h
  @brief    +Descripcion del archivo+
  @author   KevinWahle
  @date		10 sep. 2022
 ******************************************************************************/

#ifndef SPI_H_
#define SPI_H_

/*******************************************************************************
 * INCLUDE HEADER FILES
 ******************************************************************************/
#include <stdbool.h>
#include <stdint.h>
#include "../buffer/SPI_buffer.h"

/*******************************************************************************
 * CONSTANT AND MACRO DEFINITIONS USING #DEFINE
 ******************************************************************************/
typedef enum {SLAVE, MASTER} SPImode;
typedef enum {SPI_0, SPI_1, SPI_2} SPIport;

#define CS_ACTIVE	false
#define CS_INACTIVE	!CS_ACTIVE
/*******************************************************************************
 * ENUMERATIONS AND STRUCTURES AND TYPEDEFS
 ******************************************************************************/
typedef struct {
	uint8_t type              	:1 ;  // Master or Slave
	uint8_t clk_pol             :1 ;  // Clock Polarity
	uint8_t clk_phase           :1 ;  // Clock Phase
	uint8_t LSB_fist            :1 ;  // 0 Data is transferred MSB first.
										                // 1 Data is transferred LSB first.
	uint8_t PCS_inactive_state  :1 ;  // Peripheral Chip Select x Inactive State (1 The inactive state of PCSx is high)
	uint8_t frame_size          :4 ;  // Size of the package to send/receive
	uint8_t Baud_rate_scaler    :4 ;  // Baud Rate Scaler (Reference Manual Page 1492)

	uint8_t unused :7 ;

} SPI_config_t;


/*******************************************************************************
 * VARIABLE PROTOTYPES WITH GLOBAL SCOPE
 ******************************************************************************/



/*******************************************************************************
 * FUNCTION PROTOTYPES WITH GLOBAL SCOPE
 ******************************************************************************/

/**
 * @brief Set Up del SPI
 * @param SPI_n Numero de SPI a utilizar
 * @param config Opciones sobre la configuracion
 * @return True si la configuracion fue exitosa
*/
bool SPI_config (uint8_t SPI_n, SPI_config_t * config);

/**
 * @brief Se genera una opracion de envíar/recibir, encolando los mensajes
 * @param data Información sobre el mensaje a leer/escribir
 * @param len Largo del mensaje
*/
void SPISend(uint8_t SPI_n, package* data, uint8_t len, uint8_t PCS);

/*******************************************************************************
 ******************************************************************************/

#endif // _SPI_H_
