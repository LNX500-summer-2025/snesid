#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/i2c.h>
#include <linux/i2c-dev.h>
#include "main.h"

static void controllerThreadProc( void* param );
static uint16_t controllerRead( void );
static void controllerPoll( void );

pthread_mutex_t controllerMutex = PTHREAD_MUTEX_INITIALIZER;

static pthread_t controllerThreadHandle = 0;
static uint16_t padState = 0;

static int i2cFd = -1;

static void controllerThreadProc( void* param ) {
    printf( "Entering controller thread\n" );

    do {
        pthread_mutex_lock( &controllerMutex );
            // Poll the controller and allow for some time before reading
            controllerPoll( );
            usleep( 1000 );

            padState = controllerRead( );
        pthread_mutex_unlock( &controllerMutex );

        // Poll the controller every (x) ms
        usleep( 1000 * config_Pad_Poll_Time );
    } while ( runFlag );

    printf( "Leaving controller thread\n" );
    pthread_exit( NULL );
}

int controllerInit( void ) {
    checkError( open, ( i2cFd = open( "/dev/i2c-1", O_RDWR ) ) == -1 );

    checkError( ioctl, ioctl( i2cFd, I2C_SLAVE, config_Controller_Address ) != 0 );

    checkError( pthread_mutex_init, pthread_mutex_init( &controllerMutex, NULL ) != 0 );
    checkError( pthread_create, pthread_create( &controllerThreadHandle, NULL, ( void* ) controllerThreadProc, NULL ) != 0 );
    checkError( pthread_detach, pthread_detach( controllerThreadHandle ) != 0 );

    return 0;
}

void controllerClose( void ) {
    pthread_mutex_destroy( &controllerMutex );

    if ( i2cFd != -1 ) {
        close( i2cFd );
        i2cFd = -1;
    }
}

uint16_t controllerState( void ) {
    uint16_t state = 0;

    pthread_mutex_lock( &controllerMutex );
        state = padState;
    pthread_mutex_unlock( &controllerMutex );

    return state;
}

static void controllerPoll( void ) {
    const uint8_t pollReq = 0xCA;

    if ( i2cFd != -1 )
        checkError( write, write( i2cFd, &pollReq, 1 ) != 1 );
}

static uint16_t controllerRead( void ) {
    uint16_t data = 0;
    uint8_t lo = 0;
    uint8_t hi = 0;

    if ( i2cFd != -1 ) {
        // NOTE FOR TARA:
        // Must do i2c reads in one batch
        checkError( read, read( i2cFd, &data, 2 ) != 2 );

        lo = ( data >> 8 ) & 0xFF;
        hi = ( data ) & 0xFF;
    }

    return ( lo << 8 ) | hi;
}
