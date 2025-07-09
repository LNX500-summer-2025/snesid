#ifndef _MAIN_H_
#define _MAIN_H_

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include "config.h"

#define checkError( fname, fncall ) do { if ( ( fncall ) ) { perror( #fname ); exit( 1 ); } } while ( 0 )

int serverInit( void );
int serverClose( void );

int controllerInit( void );
void controllerClose( void );
uint16_t controllerState( void );

extern pthread_mutex_t runFlagMutex;
extern int runFlag;

#endif
