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
/// @brief Read the next non-overlong line and trim trailing whitespace.
///
/// @param ptrFileHandle Source file stream.
/// @param ptrLine Destination buffer.
/// @param lineSize Size of `ptrLine`.
/// @param ptrLineNumber Running line counter updated in place.
/// @param ptrReadCount Running read counter updated in place.
/// @param ptrSourceName Name of the source file for warnings.
///
/// @return `1` when a normalized line was read, or `0` on end of file.
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

/// @brief Trim trailing spaces, tabs, carriage returns, and newlines from a line.
///
/// @param ptrLine Line buffer to modify in place.
///
/// @return This function does not return a value.
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
