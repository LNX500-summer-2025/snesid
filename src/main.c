/**
 * Copyright (c) 2025 Tara Keeling
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
 */

#include <signal.h>
#include <unistd.h>
#include "main.h"

static void signalHandler( int signal );
static void cleanup( void );

pthread_mutex_t runFlagMutex = PTHREAD_MUTEX_INITIALIZER;
int runFlag = 1;

int main( void ) {
    int run = 0;

    signal( SIGINT, signalHandler );
    signal( SIGQUIT, signalHandler );

    atexit( cleanup );

    pthread_mutex_init( &runFlagMutex, NULL );
    
    if ( serverInit( ) == 0 ) {
        runFlag = 1;

        do {
            pthread_mutex_lock( &runFlagMutex );
                run = runFlag;
            pthread_mutex_unlock( &runFlagMutex );

            usleep( 1000 );            
        } while ( run );

        serverClose( );
    }

    printf( "Bye!\n" );

    return 0;
}

static void cleanup( void ) {
    serverClose( );
    pthread_mutex_destroy( &runFlagMutex );
}

static void signalHandler( int signal ) {
    pthread_mutex_lock( &runFlagMutex );
        runFlag = 0;
    pthread_mutex_unlock( &runFlagMutex );
}
