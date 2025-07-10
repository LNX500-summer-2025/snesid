#include <stdio.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <poll.h>
#include "main.h"

#ifndef POLL_IN
#define POLL_IN POLLIN 
#endif

static void serverThreadProc( void* param );
static void clientThreadProc( void* param );

static const char* serverSocketPath = config_Server_Socket_Path;
static int serverFd = -1;

static pthread_t serverThreadHandle = 0;

int serverInit( void ) {
    struct sockaddr_un serverSockAddr;

    checkError( socket, ( serverFd = socket( AF_UNIX, SOCK_STREAM, 0 ) ) < 0 );

    unlink( serverSocketPath );
    memset( &serverSockAddr, 0, sizeof( struct sockaddr_un ) );

    strncpy( serverSockAddr.sun_path, serverSocketPath, strlen( serverSocketPath ) + 1 );
    serverSockAddr.sun_family = AF_UNIX;

    checkError( bind, bind( serverFd, ( const struct sockaddr* ) &serverSockAddr, sizeof( struct sockaddr_un ) ) != 0 );
    checkError( listen, listen( serverFd, config_Server_Backlog ) != 0 );

    checkError( pthread_create, pthread_create( &serverThreadHandle, NULL, ( void* ) serverThreadProc, NULL ) != 0 );

    return 0;
}

int serverClose( void ) {
    if ( serverThreadHandle ) {
        pthread_mutex_lock( &runFlagMutex );
            runFlag = 0;
        pthread_mutex_unlock( &runFlagMutex );
        pthread_join( serverThreadHandle, NULL );
        
        serverThreadHandle = 0;
    }

    if ( serverFd ) {
        close( serverFd );
        serverFd = -1;
    }

    unlink( serverSocketPath );
    return 0;
}

static void serverThreadProc( void* param ) {
    pthread_t clientThreadHandle = 0;
    struct pollfd fdes;
    int clientFd = -1;
    int run = 0;
    int res = 0;
    int* ptr = NULL;

    printf( "Entered server thread\n" );

    do {
        fdes.events = POLL_IN;
        fdes.revents = 0;
        fdes.fd = serverFd;

        if ( ( res = poll( &fdes, serverFd + 1, config_Server_Poll_Timeout ) ) > 0 ) {
            if ( fdes.revents & POLL_IN ) {
                // New socket is available
                if ( ( clientFd = accept( serverFd, NULL, NULL ) ) >= 0 ) {
                    printf( "Client connected\n" );

                    if ( ( ptr = ( int* ) malloc( sizeof( int ) ) ) != NULL ) {
                        *ptr = clientFd;

                        checkError( pthread_create, pthread_create( &clientThreadHandle, NULL, ( void* ) clientThreadProc, ( void* ) ptr ) != 0 );
                        checkError( pthread_detach, pthread_detach( clientThreadHandle ) != 0 );
                    }else {
                        close( clientFd );
                    }
                }
            }
        }

        pthread_mutex_lock( &runFlagMutex );
            run = runFlag;
        pthread_mutex_unlock( &runFlagMutex );
    } while ( run );

    printf( "Leaving server thread\n" );
}

static void clientThreadProc( void* param ) {
    uint16_t state = 0;
    int clientFd = -1;
    int run = 1;

    printf( "Entered client thread\n" );

    if ( param ) {
        clientFd = *( ( int* ) param );
        free( param );

        // TODO:
        // This still runs when the client quits
        do {
            pthread_mutex_lock( &runFlagMutex );
                run = runFlag;
            pthread_mutex_unlock( &runFlagMutex );

            state = controllerState( );

            run = ( write( clientFd, &state, sizeof( uint16_t ) ) == sizeof( uint16_t ) ) ? run : 0;
        } while ( run );

        close( clientFd );
    }

    printf( "Leaving client thread\n" );
}
