#include "main.h"

static pthread_mutex_t controllerStateMutex = PTHREAD_MUTEX_INITIALIZER;
static uint16_t padState = 0;

int controllerInit( void ) {
    checkError( pthread_mutex_init, pthread_mutex_init( &controllerStateMutex, NULL ) != 0 );

    return 0;
}

void controllerClose( void ) {
    pthread_mutex_destroy( &controllerStateMutex );
}

uint16_t controllerState( void ) {
    uint16_t state = 0;

    pthread_mutex_lock( &controllerStateMutex );
        state = padState;
    pthread_mutex_unlock( &controllerStateMutex );

    return state;
}
