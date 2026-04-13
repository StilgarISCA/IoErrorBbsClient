/*
 * Copyright (C) 2024-2026 Stilgar
 * Copyright (C) 1995-2003 Michael Hampton
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

/*
 * Various utility routines that didn't really belong elsewhere.  Yawn.
 */
#include "defs.h"
#include "ext.h"

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

/*
 * Process command line arguments.  argv[1] is an alternate host, if present,
 * and argv[2] is an alternate port, if present, and argv[1] is also present.
 */
void arguments( int argc, char **argv )
{
   if ( argc > 1 )
   {
      snprintf( aryCommandLineHost, sizeof( aryCommandLineHost ), "%s", argv[1] );
   }
   else
   {
      *aryCommandLineHost = 0;
   }
   if ( argc > 2 )
   {
      cmdLinePort = (unsigned short)atoi( argv[2] );
   }
   else
   {
      cmdLinePort = 0;
   }
   if ( argc > 3 )
   {
      if ( !strncmp( argv[3], "secure", 6 ) || !strncmp( argv[3], "ssl", 6 ) )
      {
         shouldUseSsl = 1;
      }
      else
      {
         shouldUseSsl = 0;
      }
   }
}

/*
 * strcmp() wrapper for friend entries; grabs the correct entry from the
 * struct, which is arg 2.
 */
#ifdef ENABLE_SAVE_PASSWORD
/*
 * Encode/decode password with a simple algorithm.
 * jhp 5Feb95 (Marx Marvelous)
 *
 * This code is horribly insecure.  Don't use it for any passwords
 * you care about!  Also note it's closely tied to ASCII and won't
 * work with a non-ASCII system.  - IO
 */
char *jhpencode( char *ptrDestination, const char *src, size_t seedLength )
{
   char *ptrDestIterator; /* dest iterator */
   char inputChar;        /* a single character */

   ptrDestIterator = ptrDestination;
   while ( ( inputChar = *src++ ) != 0 )
   {
      *ptrDestIterator++ = ( inputChar - 32 - seedLength + 95 ) % 95 + 32;
      seedLength = inputChar - 32;
   }
   *ptrDestIterator = 0;
   return ptrDestination;
}

char *jhpdecode( char *ptrDestination, const char *src, size_t seedLength )
{
   char *ptrDestIterator; /* dest iterator */
   char inputChar;        /* a single character */

   ptrDestIterator = ptrDestination;
   while ( ( inputChar = *src++ ) != 0 )
   {
      seedLength = ( seedLength + inputChar - 32 ) % 95;
      *ptrDestIterator++ = seedLength + 32;
   }
   *ptrDestIterator = 0;
   return ptrDestination;
}
#endif
