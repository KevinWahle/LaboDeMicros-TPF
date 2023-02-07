/***************************************************************************//**
  @file     +event_queue.c+
  @brief    +Archivo fuente del módulo event_queue, para crear una cola de eventos+
  @author   +Grupo 5+
 ******************************************************************************/

/*******************************************************************************
 * INCLUDE HEADER FILES
 ******************************************************************************/

#include "event_queue.h"
#include "buffer/generic_circular_buffer.h"

#ifdef TEST     // Solo si se define TEST (-D TEST)
#include <stdio.h>  // Solo para TEST
#endif

/*******************************************************************************
 * STATIC VARIABLES AND CONST VARIABLES WITH FILE LEVEL SCOPE
 ******************************************************************************/

static genericCircularBuffer queue;               // Arreglo con la lista de eventos.

/*******************************************************************************
 *******************************************************************************
                        GLOBAL FUNCTION DEFINITIONS
 *******************************************************************************
 ******************************************************************************/

bool init_queue() {
	return GCBinit(&queue, sizeof(event_t), MAX_EVENTS);
}

bool add_event(event_t event) {

	if (GCBgetBufferState(&queue) >= MAX_EVENTS) return false;		// No hay lugar

	GCBputData(&queue, &event);

	return true;

}

event_t get_next_event(void) {

	event_t event = NULL_EVENT;

	GCBgetData(&queue, &event);

	return event;

}

bool skip_event(void) {
	return get_next_event() != NULL_EVENT;
}

void empty_queue(void) {
    GCBreset(&queue);
}

bool is_queue_empty(void) {
    return GCBisEmpty(&queue);
}

/*******************************************************************************
 *******************************************************************************
                        LOCAL FUNCTION DEFINITIONS
 *******************************************************************************
 ******************************************************************************/

#ifdef TEST     // Sólo si se define TEST
                // Es la prueba para verificar el funcionamiento de la librería.

    static void printArray (event_t array[], unsigned int size) {
        printf("[ ");
        for (int i = 0; i < size; i++) {
            printf("%u ", array[i]);
        }
        printf("]\n");
    }

    int main() {
        for (int i = 1 ; i < 10 ; i++) {
            add_event(i);
            printArray(queue, 10);
            printf("%I64u", top_of_queue);
            for (int j = 0; j < 2*top_of_queue+1; j++)
                printf(" ");
            printf("^\n");
        }
        printf("\n\n");
        for (int i = 0 ; i < 10 ; i++) {
            printf("%u\n", get_next_event());
            printArray(queue, 10);
            printf("%I64u", top_of_queue);
            for (int j = 0; j < 2*top_of_queue+1; j++)
                printf(" ");
            printf("^\n\n");
        }
        add_event(7);
        printArray(queue, 10);
        printf("%I64u", top_of_queue);
        for (int j = 0; j < 2*top_of_queue+1; j++)
            printf(" ");
        printf("^\n");
        printf("empty: %d\n\n", is_queue_empty());

        empty_queue();
        printArray(queue, 10);
        printf("%I64u", top_of_queue);
        for (int j = 0; j < 2*top_of_queue+1; j++)
            printf(" ");
        printf("^\n");
        printf("empty: %d\n", is_queue_empty());

        return 0;
    }

#endif  //TEST
