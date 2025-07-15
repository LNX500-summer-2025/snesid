#include <stdio.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <poll.h>
#include "main.h"

#ifndef POLL_IN
#define POLL_IN POLLIN 
#endif

static void clientThreadProc( void* param );

static const char* serverSocketPath = config_Server_Socket_Path;
static int serverFd = -1;

int serverInit( void ) {
    struct sockaddr_un serverSockAddr;

    checkError( socket, ( serverFd = socket( AF_UNIX, SOCK_STREAM, 0 ) ) < 0 );

    unlink( serverSocketPath );
    memset( &serverSockAddr, 0, sizeof( struct sockaddr_un ) );

    strncpy( serverSockAddr.sun_path, serverSocketPath, strlen( serverSocketPath ) + 1 );
    serverSockAddr.sun_family = AF_UNIX;

    checkError( bind, bind( serverFd, ( const struct sockaddr* ) &serverSockAddr, sizeof( struct sockaddr_un ) ) != 0 );
    checkError( listen, listen( serverFd, config_Server_Backlog ) != 0 );

    return 0;
}

int serverClose( void ) {
    if ( serverFd ) {
        close( serverFd );
        serverFd = -1;
    }

    unlink( serverSocketPath );
    return 0;
}

/// @brief Runs one tick of the server
/// @param  
/// @return 1 If a client thread was created this tick
int serverTick( void ) {
    pthread_t clientThreadHandle = 0;
    struct pollfd fdes;
    int clientFd = -1;
    int res = 0;
    int* ptr = NULL;

    fdes.events = POLL_IN;
    fdes.revents = 0;
    fdes.fd = serverFd;

    if ( ( res = poll( &fdes, 1, config_Server_Poll_Timeout ) ) > 0 ) {
        if ( fdes.revents & POLL_IN ) {
            // New socket is available
            if ( ( clientFd = accept( serverFd, NULL, NULL ) ) >= 0 ) {
                printf( "Client connected\n" );

                if ( ( ptr = ( int* ) malloc( sizeof( int ) ) ) != NULL ) {
                    *ptr = clientFd;

                    checkError( pthread_create, pthread_create( &clientThreadHandle, NULL, ( void* ) clientThreadProc, ( void* ) ptr ) != 0 );
                    checkError( pthread_detach, pthread_detach( clientThreadHandle ) != 0 );

                    return 1;
                } else {
                    close( clientFd );
                }
            }
        }
    }

    return 0;
}

static void clientThreadProc( void* param ) {
    uint16_t state = 0;
    int clientFd = -1;
    uint8_t data = 0;
    uint8_t lo = 0;
    uint8_t hi = 0;
    uint8_t buf[ 2 ];
    struct pollfd fdes;
    int res = 0;

    printf( "Entered client thread\n" );

    if ( param ) {
        clientFd = *( ( int* ) param );
        free( param );

        do {
            fdes.events = POLL_IN;
            fdes.revents = 0;
            fdes.fd = clientFd;

            if ( poll( &fdes, 1, config_Server_Poll_Timeout ) > 0 ) {
                if ( read( clientFd, &data, 1 ) != 1 )
                    break;

                if ( data == 0x20 ) {
                    state = controllerState( );

                    hi = ( state >> 8 ) & 0xFF;
                    lo = ( state & 0xFF );

                    buf[ 0 ] = lo;
                    buf[ 1 ] = hi;

                    res = write( clientFd, buf, 2 );

                    //res = write( clientFd, &lo, 1 );
                    //res+= write( clientFd, &hi, 1 );

                    if ( res != 2 )
                        break;
                }
            }
        } while ( runFlag );

        close( clientFd );
    }

    printf( "Leaving client thread\n" );
    pthread_exit( NULL );
}
