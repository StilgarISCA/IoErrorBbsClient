/*
 * Copyright (C) 2024-2026 Stilgar
 * Copyright (C) 1995-2003 Michael Hampton
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef UTILITY_H_INCLUDED
#define UTILITY_H_INCLUDED

#include "defs.h"

int capPrintf( const char *format, ... );
int capPutChar( int inputChar );
int capPuts( const char *ptrText );
int deleteFile( const char *pathname );
int deleteQueue( queue *ptrQueue );
int extractNumber( const char *header );
int fSortCompare( const friend *const *ptrLeft, const friend *const *ptrRight );
int fSortCompareVoid( const void *ptrLeft, const void *ptrRight );
int fStrCompare( const char *ptrName, const friend *ptrFriend );
int fStrCompareVoid( const void *ptrName, const void *ptrFriend );
int getKey( void );
void handleInvalidInput( unsigned int *ptrInvalidCount );
int inKey( void );
int isQueued( const char *ptrObject, queue *ptrQueue );
void looper( void );
int more( int *line, int percentComplete );
int netPrintf( const char *format, ... );
int netPutChar( int inputChar );
int netPuts( const char *ptrText );
int popQueue( char *ptrObject, queue *ptrQueue );
int pushQueue( const char *ptrObject, queue *ptrQueue );
int readFoldedKey( void );
int readValidatedKey( const char *allowedChars );
int readValidatedMenuKey( const char *allowedCharsLowercase );
int safeDeleteQueue( queue *ptrQueue );
int slistAddItem( slist *list, void *item, int deferSort );
int slistFind( slist *list, void *toFind,
               int ( *findfn )( const void *, const void * ) );
int slistRemoveItem( slist *list, int item );
int sortCompare( char **ptrLeft, char **ptrRight );
int sortCompareVoid( const void *ptrLeft, const void *ptrRight );
int strCompareVoid( const void *ptrLeft, const void *ptrRight );
int stdPrintf( const char *format, ... );
int stdPutChar( int inputChar );
int stdPuts( const char *ptrText );
int yesNo( void );
int yesNoDefault( int defaultAnswer );

void replyMessage( void );
void sendAnX( void );
void sendTrackedBuffer( const char *ptrBuffer, size_t length );
void sendTrackedChar( int inputChar );
void sendTrackedNewline( void );
void slistDestroy( slist *list );
void slistDestroyItems( slist *list );
void slistSort( slist *list );
void sPerror( const char *message, const char *heading );
void tempFileError( void );
void trimTrailingWhitespace( char *ptrLine );

char *duplicateString( const char *ptrSource );
char *extractName( const char *header );
char *extractNameNoHistory( const char *header );
char *findChar( const char *ptrString, int targetChar );
char *findSubstring( const char *ptrString, const char *ptrSubstring );
char *stripAnsi( char *ptrText, size_t bufferSize );

int readNormalizedLine( FILE *ptrFileHandle, char *ptrLine, size_t lineSize,
                        int *ptrLineNumber, int *ptrReadCount,
                        const char *ptrSourceName );

queue *newQueue( int size, int itemCount );

slist *slistCreate( int nitems, int ( *sortfn )( const void *, const void * ), ... );
slist *slistIntersection( const slist *list1, const slist *list2 );

#endif /* UTILITY_H_INCLUDED */
