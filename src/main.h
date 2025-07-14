#ifndef _MAIN_H_
#define _MAIN_H_

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <pthread.h>
#include "config.h"

#define checkError( fname, fncall ) do { if ( ( fncall ) ) { printf( "%s::%s::%d: %s\n", __FILE__, __FUNCTION__, __LINE__, #fncall ); perror( #fname ); exit( 1 ); } } while ( 0 )

int serverInit( void );
int serverTick( void );
int serverClose( void );

int controllerInit( void );
void controllerClose( void );
uint16_t controllerState( void );

extern volatile sig_atomic_t runFlag;

#endif
