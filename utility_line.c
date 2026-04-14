/*
 * Copyright (C) 2024-2026 Stilgar
 * Copyright (C) 1995-2003 Michael Hampton
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

/*
 * This file handles line cleanup and normalized line reading.
 */
#include "defs.h"
#include "utility.h"

int readNormalizedLine( FILE *ptrFileHandle, char *ptrLine, size_t lineSize,
                        int *ptrLineNumber, int *ptrReadCount,
                        const char *ptrSourceName )
{
   size_t maxLineLength = lineSize - 1;

   while ( ptrFileHandle && fgets( ptrLine, (int)lineSize, ptrFileHandle ) )
   {
      ( *ptrReadCount )++;
      ( *ptrLineNumber )++;
      if ( strlen( ptrLine ) >= maxLineLength )
      {
         stdPrintf( "Line %d in %s too long, ignored.\n", *ptrLineNumber, ptrSourceName );
         while ( strlen( ptrLine ) >= maxLineLength &&
                 ptrLine[maxLineLength - 1] != '\n' )
         {
            if ( !fgets( ptrLine, (int)lineSize, ptrFileHandle ) )
            {
               break;
            }
         }
         continue;
      }
      trimTrailingWhitespace( ptrLine );
      return 1;
   }
   return 0;
}

void trimTrailingWhitespace( char *ptrLine )
{
   size_t lineLength = strlen( ptrLine );

   while ( lineLength > 0 )
   {
      size_t index = lineLength - 1;
      if ( ptrLine[index] == ' ' || ptrLine[index] == '\t' ||
           ptrLine[index] == '\n' || ptrLine[index] == '\r' )
      {
         ptrLine[index] = 0;
      }
      else
      {
         break;
      }
      lineLength = index;
   }
}
