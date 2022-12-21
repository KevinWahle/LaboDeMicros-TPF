/***************************************************************************//**
  @file     +event_queue.c+
  @brief    +Archivo fuente del módulo event_queue, para crear una cola de eventos+
  @author   +Grupo 5+
 ******************************************************************************/

/*******************************************************************************
 * INCLUDE HEADER FILES
 ******************************************************************************/

#include "event_queue.h"

#ifdef TEST     // Solo si se define TEST (-D TEST)
#include <stdio.h>  // Solo para TEST
#endif

/*******************************************************************************
 * STATIC VARIABLES AND CONST VARIABLES WITH FILE LEVEL SCOPE
 ******************************************************************************/

static event_t queue[MAX_EVENTS];               // Arreglo con la lista de eventos.

static unsigned long int top_of_queue = 0;      // Indicador de la cantidad de eventos guardados.

/*******************************************************************************
 *******************************************************************************
                        GLOBAL FUNCTION DEFINITIONS
 *******************************************************************************
 ******************************************************************************/

int add_event(event_t event) {
    if (top_of_queue < MAX_EVENTS) {        // Si hay lugar en la cola
        for (int i = top_of_queue; i > 0 ; i--) {
            queue[i] = queue[i-1];              // Muevo todos los eventos 1 posición
        }
        top_of_queue++;                         // Aumento tamaño de la cola (Apunta a proximo espacio libre)
        queue[0] = event;   // Agrego el nuevo evento al inicio
        return 0;   // Fin exitoso
    }    
    return 1;       // Error, no hay lugar
}

event_t get_next_event(void) {
    if (top_of_queue > 0) {     // Si hay eventos en la cola
        return queue[--top_of_queue];   // Devuelve ultimo evento y lo elimino de la cola
    }
    return NULL_EVENT;  // No hay eventos
}

int skip_event(void) {
    if (top_of_queue > 0) {     // Si hay elementos en la cola
        top_of_queue--;             // Se elimina el último elemento
        return 0;       // Fin exitoso
    }
    return 1;           // Error, no hay elementos
}

void empty_queue(void) {
    top_of_queue = 0;
}

int is_queue_empty(void) {
    return top_of_queue == 0;
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
