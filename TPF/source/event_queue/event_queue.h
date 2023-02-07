/***************************************************************************//**
  @file     +event_queue.h+
  @brief    +Archivo header del módulo event_queue, para crear una cola de eventos+
  @author   +Grupo 5+
 ******************************************************************************/

#ifndef _EVENT_QUEUE_H_
#define _EVENT_QUEUE_H_

/*******************************************************************************
 * INCLUDE HEADER FILES
 ******************************************************************************/

#include <stdint.h>
#include <stdbool.h>

/*******************************************************************************
 * CONSTANT AND MACRO DEFINITIONS USING #DEFINE
 ******************************************************************************/

#define MAX_EVENTS	500                 // Máxima cantidad de eventos

#define NULL_EVENT	((event_t)(-1))     // Evento nulo (máximo valor de event_t)

/*******************************************************************************
 * ENUMERATIONS AND STRUCTURES AND TYPEDEFS
 ******************************************************************************/

typedef uint8_t event_t;

/*******************************************************************************
 * FUNCTION PROTOTYPES WITH GLOBAL SCOPE
 ******************************************************************************/

/**
 * @brief Inicializa la cola de eventos
 * @return false si hubo error
*/
bool init_queue();

/**
 * @brief Agrega un evento a la cola
 * @param event evento a añadir a la cola
 * @return false si la cola está llena
*/
bool add_event(event_t event);

/**
 * @brief Obtiene el próximo evento en la cola
 * @return El próximo evento en la cola, NULL_EVENT si la cola está vacía
*/
event_t get_next_event(void);

/**
 * @brief Omite el próximo elemento en la cola
 * @return false si la cola estaba vacia
*/
bool skip_event(void);

/**
 * @brief Vacía la cola
*/
void empty_queue(void);

/**
 * @brief Indica si la cola está vacía o no
 * @return true si no hay lugar en la cola
*/
bool is_queue_empty(void);

/*******************************************************************************
 ******************************************************************************/

#endif // _EVENT_QUEUE_H_
