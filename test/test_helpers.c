/*
 * Copyright (C) 2024-2026 Stilgar
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>

#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "test_helpers.h"

int compareStringItem( const void *ptrNeedle, const void *ptrItem )
{
   const char *ptrSearch;
   const char *ptrValue;

   ptrSearch = ptrNeedle;
   ptrValue = ptrItem;
   return strcmp( ptrSearch, ptrValue );
}

int compareStringPointer( const void *ptrLeft, const void *ptrRight )
{
   const char *const *ptrLeftString;
   const char *const *ptrRightString;

   ptrLeftString = ptrLeft;
   ptrRightString = ptrRight;
   return strcmp( *ptrLeftString, *ptrRightString );
}

size_t copyIntArray( const int *arySource, size_t sourceCount, int *aryDestination, size_t destinationCount )
{
   size_t copiedCount;
   size_t valueIndex;

   copiedCount = sourceCount;
   if ( copiedCount > destinationCount )
   {
      copiedCount = destinationCount;
   }

   for ( valueIndex = 0; valueIndex < copiedCount; ++valueIndex )
   {
      aryDestination[valueIndex] = arySource[valueIndex];
   }

   return copiedCount;
}

size_t copyStringPointerArray(
   const char **arySource, size_t sourceCount, const char **aryDestination, size_t destinationCount )
{
   size_t copiedCount;
   size_t valueIndex;

   copiedCount = sourceCount;
   if ( copiedCount > destinationCount )
   {
      copiedCount = destinationCount;
   }

   for ( valueIndex = 0; valueIndex < copiedCount; ++valueIndex )
   {
      aryDestination[valueIndex] = arySource[valueIndex];
   }

   return copiedCount;
}

void createTempPathOrFail( char *aryPath, size_t pathSize, const char *ptrTemplate )
{
   char aryTemplate[128];
   int fileDescriptor;

   snprintf( aryTemplate, sizeof( aryTemplate ), "%s", ptrTemplate );
   fileDescriptor = mkstemp( aryTemplate );
   if ( fileDescriptor < 0 )
   {
      fail_msg( "mkstemp failed while creating temporary path template '%s'", ptrTemplate );
      return;
   }
   close( fileDescriptor );

   if ( unlink( aryTemplate ) != 0 )
   {
      fail_msg( "unlink failed for temporary path '%s'", aryTemplate );
      return;
   }

   snprintf( aryPath, pathSize, "%s", aryTemplate );
}

char *duplicateStringOrFail( const char *ptrSource, const char *ptrContext )
{
   char *ptrCopy;
   size_t sourceLength;

   sourceLength = strlen( ptrSource );
   ptrCopy = calloc( sourceLength + 1, sizeof( char ) );
   if ( ptrCopy == NULL )
   {
      fail_msg( "calloc failed while duplicating '%s' in %s", ptrSource, ptrContext );
      return NULL;
   }

   memcpy( ptrCopy, ptrSource, sourceLength );
   return ptrCopy;
}

void readFileIntoBufferOrFail( FILE *ptrFile, char *aryBuffer, size_t bufferSize, const char *ptrContext )
{
   size_t bytesRead;

   if ( ptrFile == NULL )
   {
      fail_msg( "readFileIntoBufferOrFail called with NULL file in %s", ptrContext );
      return;
   }
   if ( bufferSize == 0 )
   {
      fail_msg( "readFileIntoBufferOrFail called with empty destination buffer in %s", ptrContext );
      return;
   }

   rewind( ptrFile );
   bytesRead = fread( aryBuffer, 1, bufferSize - 1, ptrFile );
   aryBuffer[bytesRead] = '\0';
}

void writeFileContentsOrFail( const char *ptrPath, const char *ptrContents, const char *ptrContext )
{
   FILE *ptrFile;

   ptrFile = fopen( ptrPath, "w" );
   if ( ptrFile == NULL )
   {
      fail_msg( "fopen failed while writing '%s' in %s", ptrPath, ptrContext );
      return;
   }

   if ( fputs( ptrContents, ptrFile ) < 0 )
   {
      fclose( ptrFile );
      fail_msg( "fputs failed while writing '%s' in %s", ptrPath, ptrContext );
      return;
   }

   fclose( ptrFile );
}

void writeRepeatedCharOrFail( FILE *ptrFile, char value, int count, const char *ptrContext )
{
   int charIndex;

   if ( ptrFile == NULL )
   {
      fail_msg( "writeRepeatedCharOrFail called with NULL file in %s", ptrContext );
      return;
   }

   for ( charIndex = 0; charIndex < count; ++charIndex )
   {
      fputc( value, ptrFile );
   }
}
