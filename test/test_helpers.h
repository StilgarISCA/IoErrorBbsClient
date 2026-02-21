/*
 * Copyright (C) 2024-2026 Stilgar
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef TEST_HELPERS_H
#define TEST_HELPERS_H

#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>

int compareStringItem( const void *ptrNeedle, const void *ptrItem );
int compareStringPointer( const void *ptrLeft, const void *ptrRight );
size_t copyIntArray( const int *arySource, size_t sourceCount, int *aryDestination, size_t destinationCount );
size_t copyStringPointerArray( const char **arySource, size_t sourceCount, const char **aryDestination, size_t destinationCount );
bool tryCreateTempPath( char *aryPath, size_t pathSize, const char *ptrTemplate );
bool tryDuplicateString( const char *ptrSource, char **ptrOutCopy );
bool tryReadFileIntoBuffer( FILE *ptrFile, char *aryBuffer, size_t bufferSize );
bool tryWriteFileContents( const char *ptrPath, const char *ptrContents );
bool tryWriteRepeatedChar( FILE *ptrFile, char value, int count );

#endif
