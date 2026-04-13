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

void printAnsiForegroundColorValue( int colorValue )
{
   char aryAnsiSequence[32];

   if ( !flagsConfiguration.shouldUseAnsi )
   {
      return;
   }

   formatAnsiForegroundSequence( aryAnsiSequence, sizeof( aryAnsiSequence ),
                                 colorValue );
   stdPrintf( "%s", aryAnsiSequence );
}

void printThemedMnemonicText( const char *ptrText, int defaultColor )
{
   const char *ptrScan;

   if ( ptrText == NULL )
   {
      return;
   }

   if ( !flagsConfiguration.shouldUseAnsi )
   {
      stdPrintf( "%s", ptrText );
      return;
   }

   printAnsiForegroundColorValue( defaultColor );

   for ( ptrScan = ptrText; *ptrScan; )
   {
      if ( ptrScan[0] == '<' && ptrScan[1] != '\0' && ptrScan[2] == '>' )
      {
         printAnsiForegroundColorValue( color.forum );
         stdPutChar( ptrScan[1] );
         printAnsiForegroundColorValue( defaultColor );
         ptrScan += 3;
         continue;
      }

      stdPutChar( *ptrScan );
      ptrScan++;
   }
}

#define ifansi if ( flagsConfiguration.shouldUseAnsi )

int colorize( const char *str )
{
   char aryAnsiSequence[32];
   const char *ptrText;

   for ( ptrText = str; *ptrText; ptrText++ )
   {
      if ( *ptrText == '@' )
      {
         if ( !*( ptrText + 1 ) )
         {
            ptrText--;
         }
         else
         {
            switch ( *++ptrText )
            {
               case '@':
                  putchar( (int)'@' );
                  break;
               case 'k':
                  ifansi
                  {
                     formatAnsiBackgroundSequence( aryAnsiSequence, sizeof( aryAnsiSequence ), 0 );
                     printf( "%s", aryAnsiSequence );
                  }
                  break;
               case 'K':
                  ifansi
                  {
                     formatAnsiForegroundSequence( aryAnsiSequence, sizeof( aryAnsiSequence ), 0 );
                     printf( "%s", aryAnsiSequence );
                  }
                  break;
               case 'r':
                  ifansi
                  {
                     formatAnsiBackgroundSequence( aryAnsiSequence, sizeof( aryAnsiSequence ), 1 );
                     printf( "%s", aryAnsiSequence );
                  }
                  break;
               case 'R':
                  ifansi
                  {
                     formatAnsiForegroundSequence( aryAnsiSequence, sizeof( aryAnsiSequence ), 1 );
                     printf( "%s", aryAnsiSequence );
                  }
                  break;
               case 'g':
                  ifansi
                  {
                     formatAnsiBackgroundSequence( aryAnsiSequence, sizeof( aryAnsiSequence ), 2 );
                     printf( "%s", aryAnsiSequence );
                  }
                  break;
               case 'G':
                  ifansi
                  {
                     formatAnsiForegroundSequence( aryAnsiSequence, sizeof( aryAnsiSequence ), 2 );
                     printf( "%s", aryAnsiSequence );
                  }
                  break;
               case 'y':
                  ifansi
                  {
                     formatAnsiBackgroundSequence( aryAnsiSequence, sizeof( aryAnsiSequence ), 3 );
                     printf( "%s", aryAnsiSequence );
                  }
                  break;
               case 'Y':
                  ifansi
                  {
                     formatAnsiForegroundSequence( aryAnsiSequence, sizeof( aryAnsiSequence ), 3 );
                     printf( "%s", aryAnsiSequence );
                  }
                  break;
               case 'b':
                  ifansi
                  {
                     formatAnsiBackgroundSequence( aryAnsiSequence, sizeof( aryAnsiSequence ), 4 );
                     printf( "%s", aryAnsiSequence );
                  }
                  break;
               case 'B':
                  ifansi
                  {
                     formatAnsiForegroundSequence( aryAnsiSequence, sizeof( aryAnsiSequence ), 4 );
                     printf( "%s", aryAnsiSequence );
                  }
                  break;
               case 'm':
               case 'p':
                  ifansi
                  {
                     formatAnsiBackgroundSequence( aryAnsiSequence, sizeof( aryAnsiSequence ), 5 );
                     printf( "%s", aryAnsiSequence );
                  }
                  break;
               case 'M':
               case 'P':
                  ifansi
                  {
                     formatAnsiForegroundSequence( aryAnsiSequence, sizeof( aryAnsiSequence ), 5 );
                     printf( "%s", aryAnsiSequence );
                  }
                  break;
               case 'c':
                  ifansi
                  {
                     formatAnsiBackgroundSequence( aryAnsiSequence, sizeof( aryAnsiSequence ), 6 );
                     printf( "%s", aryAnsiSequence );
                  }
                  break;
               case 'C':
                  ifansi
                  {
                     formatAnsiForegroundSequence( aryAnsiSequence, sizeof( aryAnsiSequence ), 6 );
                     printf( "%s", aryAnsiSequence );
                  }
                  break;
               case 'w':
                  ifansi
                  {
                     formatAnsiBackgroundSequence( aryAnsiSequence, sizeof( aryAnsiSequence ), 7 );
                     printf( "%s", aryAnsiSequence );
                  }
                  break;
               case 'W':
                  ifansi
                  {
                     formatAnsiForegroundSequence( aryAnsiSequence, sizeof( aryAnsiSequence ), 7 );
                     printf( "%s", aryAnsiSequence );
                  }
                  break;
               case 'd':
                  ifansi
                  {
                     formatAnsiBackgroundSequence( aryAnsiSequence, sizeof( aryAnsiSequence ), 9 );
                     printf( "%s", aryAnsiSequence );
                  }
                  break;
               case 'D':
                  ifansi
                  {
                     formatAnsiForegroundSequence( aryAnsiSequence, sizeof( aryAnsiSequence ), 9 );
                     printf( "%s", aryAnsiSequence );
                  }
                  break;
               default:
                  break;
            }
         }
      }
      else
      {
         stdPutChar( (int)*ptrText );
      }
   }
   return 1;
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
int fStrCompare( const char *ptrName, const friend *ptrFriend )
{
   return strcmp( ptrName, ptrFriend->name );
}

int fStrCompareVoid( const void *ptrName, const void *ptrFriend )
{
   return fStrCompare( (const char *)ptrName, (const friend *)ptrFriend );
}

/*
 * strcmp() wrapper for char entries.
 */
int sortCompare( char **ptrLeft, char **ptrRight )
{
   return strcmp( *ptrLeft, *ptrRight );
}

int sortCompareVoid( const void *ptrLeft, const void *ptrRight )
{
   const char *const *ptrLeftString = (const char *const *)ptrLeft;
   const char *const *ptrRightString = (const char *const *)ptrRight;
   return strcmp( *ptrLeftString, *ptrRightString );
}

int strCompareVoid( const void *ptrLeft, const void *ptrRight )
{
   return strcmp( (const char *)ptrLeft, (const char *)ptrRight );
}

/*
 * strcmp() wrapper for friend entries; takes two friend * args.
 */
int fSortCompare( const friend *const *ptrLeft, const friend *const *ptrRight )
{
   assert( ( *ptrLeft )->magic == 0x3231 );
   assert( ( *ptrRight )->magic == 0x3231 );

   return strcmp( ( *ptrLeft )->name, ( *ptrRight )->name );
}

int fSortCompareVoid( const void *ptrLeft, const void *ptrRight )
{
   return fSortCompare( (const friend *const *)ptrLeft, (const friend *const *)ptrRight );
}

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
