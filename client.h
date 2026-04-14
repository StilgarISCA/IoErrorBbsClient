/*
 * Copyright (C) 2024-2026 Stilgar
 * Copyright (C) 1995-2003 Michael Hampton
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef CLIENT_H_INCLUDED
#define CLIENT_H_INCLUDED

#include "defs.h"
#include <stdnoreturn.h>

noreturn void fatalExit( const char *message, const char *heading );
noreturn void fatalPerror( const char *error, const char *heading );
noreturn void myExit( void );

void arguments( int argc, char **argv );
void connectBbs( void );
void copyright( void );
void deinitialize( void );
void feedPager( int startrow, ... );
void findHome( void );
void flushInput( unsigned int invalid );
void information( void );
void initialize( void );
void killSsl( void );
void license( void );
void moveIfNeeded( const char *oldpath, const char *newpath );
void mySleep( unsigned int sec );
void noTitleBar( void );
void openTmpFile( void );
void resetTerm( void );
void run( const char *aryCommand, const char *arg );
void setTerm( void );
void sError( const char *message, const char *heading );
void sigInit( void );
void sigOff( void );
void sInfo( const char *info, const char *heading );
void suspend( void );
void techInfo( void );
void telInit( void );
void titleBar( void );
void warranty( void );

int getWindowSize( void );
int sPrompt( const char *info, const char *question, int def );
int waitNextEvent( void );

RETSIGTYPE bye( int signalNumber );
RETSIGTYPE naws( int signalNumber );
RETSIGTYPE reapChild( int signalNumber );

#endif /* CLIENT_H_INCLUDED */
