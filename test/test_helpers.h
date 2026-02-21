/*
 * Copyright (C) 2024-2026 Stilgar
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef TEST_HELPERS_H
#define TEST_HELPERS_H

#include <stddef.h>
#include <stdio.h>

int compareStringItem( const void *ptrNeedle, const void *ptrItem );
int compareStringPointer( const void *ptrLeft, const void *ptrRight );
size_t copyIntArray( const int *arySource, size_t sourceCount, int *aryDestination, size_t destinationCount );
size_t copyStringPointerArray( const char **arySource, size_t sourceCount, const char **aryDestination, size_t destinationCount );
void createTempPathOrFail( char *aryPath, size_t pathSize, const char *ptrTemplate );
char *duplicateStringOrFail( const char *ptrSource, const char *ptrContext );
void readFileIntoBufferOrFail( FILE *ptrFile, char *aryBuffer, size_t bufferSize, const char *ptrContext );
void writeFileContentsOrFail( const char *ptrPath, const char *ptrContents, const char *ptrContext );
void writeRepeatedCharOrFail( FILE *ptrFile, char value, int count, const char *ptrContext );

#endif
