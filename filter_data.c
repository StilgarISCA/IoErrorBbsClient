/*
 * Copyright (C) 2024-2026 Stilgar
 * Copyright (C) 1995-2003 Michael Hampton
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "defs.h"
#include "ext.h"
#include "proto.h"
#include "telnet.h"

static void printBufferedAnsiSequence( const char *ptrAnsiSequence, size_t sequenceLength )
{
   stdPrintf( "%.*s", (int)sequenceLength, ptrAnsiSequence );
}

void emitTransformedAnsiSequence( const char *ptrAnsiSequence, size_t sequenceLength,
                                  int isPostContext, int isFriend )
{
   if ( sequenceLength == 4 &&
        memcmp( ptrAnsiSequence, "\033[1m", sequenceLength ) == 0 )
   {
      if ( flagsConfiguration.shouldDisableBold )
      {
         printBufferedAnsiSequence( ptrAnsiSequence, sequenceLength );
         stdPrintf( "\033[0m" );
         return;
      }
   }
   if ( sequenceLength == 5 && ptrAnsiSequence[0] == '\033' &&
        ptrAnsiSequence[1] == '[' && ptrAnsiSequence[2] == '3' &&
        ptrAnsiSequence[4] == 'm' )
   {
      char aryAnsiSequence[32];

      formatTransformedAnsiForegroundSequence( aryAnsiSequence, sizeof( aryAnsiSequence ),
                                               ptrAnsiSequence[3], isPostContext,
                                               isFriend );
      stdPrintf( "%s", aryAnsiSequence );
      return;
   }

   printBufferedAnsiSequence( ptrAnsiSequence, sequenceLength );
}

void filterData( register int inputChar )
{
   static char aryBufferedAnsiSequence[8];
   static size_t bufferedAnsiSequenceLength = 0;

   /* Copy the current line (or what we have so far) */
   if ( inputChar == '\n' )
   {
      filterUrl( aryFilterLine );
      if ( strstr( aryFilterLine, "Message received by " ) != NULL )
      {
         emitUrlDetectionReport();
      }
      aryFilterLine[0] = 0;
   }
   else
   {
      /* This is faster than calling strcat() */
      char *ptrCursor = aryFilterLine;

      while ( *ptrCursor )
      {
         ptrCursor++;
      }

      *ptrCursor++ = (char)inputChar;
      *ptrCursor = 0;
   }

   if ( flagsConfiguration.shouldAutoAnswerAnsiPrompt &&
        strstr( aryFilterLine, "Are you on an ANSI" ) )
   {
      netPutChar( 'y' );
      byte++;
      *aryFilterLine = 0;
   }

   if ( sendingXState == SX_SEND_NEXT && !*aryFilterLine && inputChar == '\r' )
   {
      sendingXState = SX_NOT;
      if ( xlandQueue->itemCount && ( isAway || isXland ) )
      {
         sendAnX();
      }
   }

   if ( flagsConfiguration.shouldUseAnsi &&
        flagsConfiguration.isMorePromptActive && inputChar == ' ' )
   {
      char aryAnsiSequence[32];

      lastColor = color.text;
      formatAnsiForegroundSequence( aryAnsiSequence, sizeof( aryAnsiSequence ),
                                    lastColor );
      stdPrintf( "%s", aryAnsiSequence );
   }

   if ( bufferedAnsiSequenceLength > 0 )
   {
      if ( bufferedAnsiSequenceLength < sizeof( aryBufferedAnsiSequence ) )
      {
         aryBufferedAnsiSequence[bufferedAnsiSequenceLength++] = (char)inputChar;
      }
      if ( inputChar == 'm' || bufferedAnsiSequenceLength >= sizeof( aryBufferedAnsiSequence ) )
      {
         emitTransformedAnsiSequence( aryBufferedAnsiSequence,
                                      bufferedAnsiSequenceLength,
                                      0, 0 );
         bufferedAnsiSequenceLength = 0;
      }
      return;
   }
   if ( inputChar == '\033' )
   {
      char aryAnsiSequence[32];

      formatAnsiBackgroundSequence( aryAnsiSequence, sizeof( aryAnsiSequence ),
                                    color.background );
      stdPrintf( "%s", aryAnsiSequence );
      bufferedAnsiSequenceLength = 0;
      aryBufferedAnsiSequence[bufferedAnsiSequenceLength++] = (char)inputChar;
      if ( !flagsConfiguration.shouldUseAnsi )
      {
         flagsConfiguration.shouldUseAnsi = 1;
         if ( !flagsConfiguration.shouldUseBold )
         {
            flagsConfiguration.shouldDisableBold = 1;
         }
      }
      return;
   }
   if ( pendingLinesToEat > 0 )
   {
      if ( inputChar == '\n' )
      {
         pendingLinesToEat--;
      }
   }
   else
   {
      pendingLinesToEat = 0;
      stdPutChar( inputChar );
   }
}

void reprintLine( void )
{
   char aryLine[320];
   char *ptrCursor;

   snprintf( aryLine, sizeof( aryLine ), "%s", aryFilterLine );
   stdPutChar( '\r' );
   for ( ptrCursor = aryLine; *ptrCursor; ptrCursor++ )
   {
      filterData( *ptrCursor );
   }
   snprintf( aryFilterLine, sizeof( aryFilterLine ), "%s", aryLine );
}

void morePromptHelper( void )
{
   if ( !flagsConfiguration.shouldUseAnsi )
   {
      return;
   }

   if ( postProgressState )
   {
      continuedPostHelper();
   }
   else
   {
      continuedDataHelper();
   }
}

void continuedDataHelper( void )
{
   static char aryTempText[] = "\033[32m";
   char *ptrText;

   for ( ptrText = aryTempText; *ptrText; ptrText++ )
   {
      filterData( *ptrText );
   }
}
