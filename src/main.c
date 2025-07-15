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

volatile sig_atomic_t runFlag = 1;

int main( void ) {
    signal( SIGINT, signalHandler );
    signal( SIGQUIT, signalHandler );

    atexit( cleanup );

    if ( controllerInit( ) == 0 && serverInit( ) == 0 ) {
        printf( "Entering main loop\n" );
        
        runFlag = 1;

        do {
            serverTick( );   
        } while ( runFlag );

        serverClose( );
        controllerClose( );
    }

    printf( "Bye!\n" );

    return 0;
}

static void cleanup( void ) {
    serverClose( );
    controllerClose( );
}

static void signalHandler( int signal ) {
    runFlag = 0;
}
