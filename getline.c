/*
 * Copyright (C) 2024-2026 Stilgar
 * Copyright (C) 1995-2003 Michael Hampton
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

/*
 * This handles getting of short strings or groups of strings of information
 * and sending them off to the BBS.  Even though you might think doing this for
 * an 8 character password is a waste, it does cut down network traffic (and
 * the lag to the end user) to not have to echo back each character
 * individually, and it was easier to do them all rather than only some of
 * them.  Again, this is basically code from the actual BBS with slight
 * modification to work here.
 */
#include "defs.h"
#include "client_globals.h"
#include "color.h"
#include "getline_input.h"
#include "telnet.h"
#include "utility.h"

/*
 * Used for getting X's and profiles.  'which' tells which of those two we are
 * wanting, to allow the special commands for X's, like PING and ABORT. When
 * we've got what we need, we send it immediately over the net.
 */
void getFiveLines( int which )
{
   register int lineIndex;
   register int sendLineIndex;
   register int sendCharIndex;
   char arySendString[21][80];
   int override = 0;
   int local = 0;

   if ( isAway && sendingXState == SX_SENT_NAME )
   {
      sendingXState = SX_REPLYING;
      replyMessage();
      return;
   }
   if ( isXland )
   {
      sendingXState = SX_SEND_NEXT;
   }
   else
   {
      sendingXState = SX_NOT;
   }
   if ( flagsConfiguration.shouldUseAnsi )
   {
      char aryAnsiSequence[32];

      formatAnsiForegroundSequence( aryAnsiSequence, sizeof( aryAnsiSequence ),
                                    color.inputText );
      stdPrintf( "%s", aryAnsiSequence );
   }
   for ( lineIndex = 0; lineIndex < ( 20 + override + local ) &&
                        ( !lineIndex || *arySendString[lineIndex - 1] );
         lineIndex++ )
   {
      stdPrintf( ">" );
      getString( 78, arySendString[lineIndex], lineIndex );
      if ( ( which && !strcmp( arySendString[lineIndex], "ABORT" ) ) ||
           ( !( which & 16 ) && !lineIndex &&
             !strcmp( *arySendString, "PING" ) ) )
      {
         lineIndex++;
         break;
      }
      else if ( ( which & 2 ) && !( which & 8 ) &&
                !strcmp( arySendString[lineIndex], "OVERRIDE" ) )
      {
         stdPrintf( "Override on.\r\n" );
         override++;
      }
      else if ( ( which & 4 ) && !lineIndex &&
                !strcmp( *arySendString, "BEEPS" ) )
      {
         lineIndex++;
         break;
      }
      else if ( ( which & 8 ) &&
                !strcmp( arySendString[lineIndex], "LOCAL" ) )
      {
         stdPrintf( "Only broadcasting to local users.\r\n" );
         local++;
      }
   }
   sendBlock();
   if ( !strcmp( *arySendString, "PING" ) )
   {
      arySendString[0][0] = 0;
   }
   for ( sendLineIndex = 0; sendLineIndex < lineIndex; sendLineIndex++ )
   {
      sendCharIndex = (int)strlen( arySendString[sendLineIndex] );
      sendTrackedBuffer( arySendString[sendLineIndex], (size_t)sendCharIndex );
      sendTrackedNewline();
   }
   if ( flagsConfiguration.shouldUseAnsi )
   {
      char aryAnsiSequence[32];

      lastColor = color.text;
      formatAnsiForegroundSequence( aryAnsiSequence, sizeof( aryAnsiSequence ),
                                    lastColor );
      stdPrintf( "%s", aryAnsiSequence );
   }
}

/*
 * Gets a generic string of length length, puts it in result and returns a a
 * pointer to result.  If the length given is negative, the string is echoed
 * with '.' instead of the character typed (used for passwords)
 */
void getString( int length, char *result, int line )
{
   static char wrap[80];
   char *rest;
   register char *ptrCursor = result;
   register char *ptrWordStart;
   register int inputChar;
   int hidden;
   unsigned int invalid = 0;

   if ( line <= 0 )
   {
      *wrap = 0;
   }
   else if ( *wrap )
   {
      printf( "%s", wrap );
      {
         int maxlen = length;
         if ( maxlen < 0 )
         {
            maxlen = -maxlen;
         }
         if ( maxlen > 0 )
         {
            snprintf( result, (size_t)maxlen + 1, "%s", wrap );
         }
         else
         {
            *result = 0;
         }
      }
      ptrCursor = result + strlen( result );
      *wrap = 0;
   }
   hidden = 0;
   if ( length < 0 )
   {
      length = 0 - length;
      hidden = length;
   }
   if ( length > 128 )
   {
      length = 256 - length;
      hidden = length;
   }
#ifdef ENABLE_SAVE_PASSWORD
   if ( hidden != 0 && *aryAutoPassword )
   {
      if ( !isAutoPasswordSent )
      {
         jhpdecode( result, aryAutoPassword, strlen( aryAutoPassword ) );
         {
            size_t rlen = strlen( result );
            for ( size_t charIndex = 0; charIndex < rlen; charIndex++ )
            {
               stdPutChar( '.' );
            }
         }
         stdPrintf( "\r\n" );
         isAutoPasswordSent = 1;
         return;
      }
   }
#endif
   while ( true )
   {
      inputChar = inKey();
      if ( inputChar == ' ' && length == 29 && ptrCursor == result )
      {
         break;
      }
      if ( inputChar == '\n' )
      {
         break;
      }
      if ( inputChar < ' ' && inputChar != '\b' &&
           inputChar != CTRL_X && inputChar != CTRL_W )
      {
         handleInvalidInput( &invalid );
         continue;
      }
      else
      {
         invalid = 0;
      }
      if ( inputChar == '\b' || inputChar == CTRL_X )
      {
         if ( ptrCursor == result )
         {
            continue;
         }
         else
         {
            do
            {
               printf( "\b \b" );
               --ptrCursor;
            } while ( inputChar == CTRL_X && ptrCursor > result );
         }
      }
      else if ( inputChar == CTRL_W )
      {
         for ( ptrWordStart = result; ptrWordStart < ptrCursor; ptrWordStart++ )
         {
            if ( *ptrWordStart != ' ' )
            {
               break;
            }
         }
         inputChar = ( ptrWordStart == ptrCursor );
         for ( ; ptrCursor > result &&
                 ( !inputChar || ptrCursor[-1] != ' ' );
               ptrCursor-- )
         {
            if ( ptrCursor[-1] != ' ' )
            {
               inputChar = 1;
            }
            printf( "\b \b" );
         }
      }
      else if ( ptrCursor < result + length && isprint( inputChar ) )
      {
         *ptrCursor++ = (char)inputChar;
         if ( !hidden )
         {
            putchar( inputChar );
         }
         else
         {
            putchar( '.' );
         }
      }
      else if ( line < 0 || line == 19 )
      {
         continue;
      }
      else
      {
         if ( inputChar == ' ' )
         {
            break;
         }
         for ( ptrWordStart = ptrCursor - 1;
               ptrWordStart > result && *ptrWordStart != ' ';
               ptrWordStart-- )
         {
            ;
         }
         if ( ptrWordStart > result )
         {
            *ptrWordStart = 0;
            for ( rest = wrap, ptrWordStart++; ptrWordStart < ptrCursor;
                  printf( "\b \b" ) )
            {
               *rest++ = *ptrWordStart++;
            }
            *rest++ = (char)inputChar;
            *rest = 0;
         }
         else
         {
            *wrap = (char)inputChar;
            *( wrap + 1 ) = 0;
         }
         break;
      }
   }
   *ptrCursor = 0;
   if ( !hidden )
   {
      capPuts( result );
   }
   else
   {
      size_t rlen = strlen( result );
      for ( size_t charIndex = 0; charIndex < rlen; charIndex++ )
      {
         capPutChar( '.' );
      }
   }
#ifdef ENABLE_SAVE_PASSWORD
   if ( hidden != 0 )
   {
      jhpencode( aryAutoPassword, result, strlen( result ) );
      writeBbsRc();
   }
#endif
   stdPrintf( "\r\n" );
}
