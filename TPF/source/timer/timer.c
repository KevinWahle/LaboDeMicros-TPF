/***************************************************************************//**
  @file     timer.c
  @brief    Timer driver. Advance implementation
  @author   Grupo 5
 ******************************************************************************/

/*******************************************************************************
 * INCLUDE HEADER FILES
 ******************************************************************************/

#include "timer.h"
#include "SysTick.h"

/*******************************************************************************
 * CONSTANT AND MACRO DEFINITIONS USING #DEFINE
 ******************************************************************************/

#define TIMER_DEVELOPMENT_MODE    0

#define TIMER_ID_INTERNAL   0

/*******************************************************************************
 * ENUMERATIONS AND STRUCTURES AND TYPEDEFS
 ******************************************************************************/

typedef struct {
	ttick_t             period;           // ticks hasta expiración
    ttick_t             cnt;              // ticks transcurridos
    tim_callback_t      callback;
    uint8_t             mode        : 1;
    uint8_t             running     : 1;
    uint8_t             expired     : 1;
    uint8_t             unused      : 5;
} timer_t;

/*******************************************************************************
 * FUNCTION PROTOTYPES FOR PRIVATE FUNCTIONS WITH FILE LEVEL SCOPE
 ******************************************************************************/

/**
 * @brief Periodic service
 */
static void timer_isr(void);

/*******************************************************************************
 * STATIC VARIABLES AND CONST VARIABLES WITH FILE LEVEL SCOPE
 ******************************************************************************/

static timer_t timers[TIMERS_MAX_CANT];
static tim_id_t timers_cant = TIMER_ID_INTERNAL+1;


/*******************************************************************************
 *******************************************************************************
                        GLOBAL FUNCTION DEFINITIONS
 *******************************************************************************
 ******************************************************************************/

void timerInit(void)
{
    static bool yaInit = false;
    if (yaInit)
        return;

    SysTick_Init(timer_isr); // init peripheral

    yaInit = true;
}


tim_id_t timerGetId(void)
{
#ifdef TIMER_DEVELOPMENT_MODE
    if (timers_cant >= TIMERS_MAX_CANT)
    {
        return TIMER_INVALID_ID;
    }
    else
#endif // TIMER_DEVELOPMENT_MODE
    {
        return timers_cant++;
    }
}


void timerStart(tim_id_t id, ttick_t ticks, uint8_t mode, tim_callback_t callback)
{
#ifdef TIMER_DEVELOPMENT_MODE
    if ((id < timers_cant) && (mode < CANT_TIM_MODES))
#endif // TIMER_DEVELOPMENT_MODE
    {
        // disable timer
        timers[id].running=0b0;

        // configure timer
        timers[id].period=ticks;
        timers[id].cnt=ticks;
        timers[id].callback=callback;
        timers[id].mode=mode;
        timers[id].expired=0b0;

        // enable timer
        timers[id].running=0b1;
    }
}


void timerStop(tim_id_t id)
{
    // Apago el timer
    timers[id].running = 0b0;

    // y bajo el flag
    timers[id].expired=0b0;
}


bool timerExpired(tim_id_t id)
{
    // Verifico si expiró el timer
    bool expired = timers[id].expired;

    // y bajo el flag
    if (expired)
    	timers[id].expired = 0b0;

    return expired;
}


void timerDelay(ttick_t ticks)
{
    timerStart(TIMER_ID_INTERNAL, ticks, TIM_MODE_SINGLESHOT, NULL);
    while (!timerExpired(TIMER_ID_INTERNAL))
    {
        // wait ...
    }
}


/*******************************************************************************
 *******************************************************************************
                        LOCAL FUNCTION DEFINITIONS
 *******************************************************************************
 ******************************************************************************/

static void timer_isr(void)
{
    for(tim_id_t id=TIMER_ID_INTERNAL; id<timers_cant; id++){
      // decremento los timers activos y si hubo timeout!
      if(timers[id].running && !(--timers[id].cnt)){

        // 1) execute action: callback or set flag
        if (timers[id].callback != NULL){
          timers[id].callback();
        }
        timers[id].expired=0b1;

        // 2) update state
        if(timers[id].mode){
        	timers[id].cnt=timers[id].period;
        }
        else{
        	timers[id].running=0;
        }
      }


    }
}


/******************************************************************************/
