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

/* replyaway routines to reply to X's when you are isAway from keyboard */
/* these globals used only in this file, so let 'em stay here */
/* Please do not change this message; it's used for reply suppression
   * (see below).  If you alter this, you will draw the ire of the ISCA
   * BBS programmers.  Trust me, I know.  :)
   */
char replymsg[5] = "+!R ";

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

void sendTrackedChar( int inputChar )
{
   netPutChar( inputChar );
   byte++;
}

void sendTrackedBuffer( const char *ptrBuffer, size_t length )
{
   size_t itemIndex;

   for ( itemIndex = 0; itemIndex < length; itemIndex++ )
   {
      netPutChar( ptrBuffer[itemIndex] );
   }
   byte += (long)length;
}

void sendTrackedNewline( void )
{
   sendTrackedChar( '\n' );
}

void sendAnX( void )
{
   /* get the ball rolling with the bbs */
   sendingXState = SX_WANT_TO;
   sendTrackedChar( 'x' );
   sendingXState = SENDING_X_STATE_SENT_COMMAND_X;
}

/* fake getFiveLines for the bbs */
void replyMessage( void )
{
   int lineIndex;
   int charIndex;

   sendBlock();
   lineIndex = (int)strlen( replymsg );
   sendTrackedBuffer( replymsg, (size_t)lineIndex );
   for ( lineIndex = 0; lineIndex < 5 && *aryAwayMessageLines[lineIndex]; lineIndex++ )
   {
      charIndex = (int)strlen( aryAwayMessageLines[lineIndex] );
      sendTrackedBuffer( aryAwayMessageLines[lineIndex], (size_t)charIndex );
      sendTrackedNewline();
      stdPrintf( "%s\r\n", aryAwayMessageLines[lineIndex] );
   }
   if ( lineIndex < 5 )
   { /* less than five lines */
      sendTrackedNewline();
   }
   sendingXState = SX_NOT;
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
