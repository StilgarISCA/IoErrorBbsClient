/*
 * Copyright (C) 2024-2026 Stilgar
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include <stdbool.h>
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

bool tryCreateTempPath( char *aryPath, size_t pathSize, const char *ptrTemplate )
{
   char aryTemplate[128];
   int fileDescriptor;

   if ( aryPath == NULL || pathSize == 0 || ptrTemplate == NULL )
   {
      return false;
   }

   snprintf( aryTemplate, sizeof( aryTemplate ), "%s", ptrTemplate );
   fileDescriptor = mkstemp( aryTemplate );
   if ( fileDescriptor < 0 )
   {
      return false;
   }
   close( fileDescriptor );

   if ( unlink( aryTemplate ) != 0 )
   {
      return false;
   }

   snprintf( aryPath, pathSize, "%s", aryTemplate );
   return true;
}

bool tryDuplicateString( const char *ptrSource, char **ptrOutCopy )
{
   char *ptrCopy;
   size_t sourceLength;

   if ( ptrSource == NULL || ptrOutCopy == NULL )
   {
      return false;
   }

   sourceLength = strlen( ptrSource );
   ptrCopy = calloc( sourceLength + 1, sizeof( char ) );
   if ( ptrCopy == NULL )
   {
      return false;
   }

   memcpy( ptrCopy, ptrSource, sourceLength );
   *ptrOutCopy = ptrCopy;
   return true;
}

bool tryReadFileIntoBuffer( FILE *ptrFile, char *aryBuffer, size_t bufferSize )
{
   size_t bytesRead;

   if ( ptrFile == NULL || aryBuffer == NULL || bufferSize == 0 )
   {
      return false;
   }

   rewind( ptrFile );
   bytesRead = fread( aryBuffer, 1, bufferSize - 1, ptrFile );
   if ( ferror( ptrFile ) != 0 )
   {
      return false;
   }
   aryBuffer[bytesRead] = '\0';
   return true;
}

bool tryWriteFileContents( const char *ptrPath, const char *ptrContents )
{
   FILE *ptrFile;

   if ( ptrPath == NULL || ptrContents == NULL )
   {
      return false;
   }

   ptrFile = fopen( ptrPath, "w" );
   if ( ptrFile == NULL )
   {
      return false;
   }

   if ( fputs( ptrContents, ptrFile ) < 0 )
   {
      fclose( ptrFile );
      return false;
   }

   fclose( ptrFile );
   return true;
}

bool tryWriteRepeatedChar( FILE *ptrFile, char value, int count )
{
   int charIndex;

   if ( ptrFile == NULL )
   {
      return false;
   }

   for ( charIndex = 0; charIndex < count; ++charIndex )
   {
      if ( fputc( value, ptrFile ) == EOF )
      {
         return false;
      }
   }
   return true;
}
