/*
 * Copyright (C) 2024-2026 Stilgar
 * Copyright (C) 1995-2003 Michael Hampton
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "defs.h"
#include "ext.h"
#include "telnet.h"

static char thisline[320]; /* Copy of the current aryLine */

static void printBufferedAnsiSequence( const char *ptrAnsiSequence, size_t sequenceLength )
{
   stdPrintf( "%.*s", (int)sequenceLength, ptrAnsiSequence );
}

static void emitTransformedAnsiSequence( const char *ptrAnsiSequence, size_t sequenceLength,
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

void filterPost( register int inputChar )
{
   static int numposts = 0;  /* count of the # of posts received so far */
   static char posthdr[140]; /* store the post header here */
   static char *posthdrp;    /* pointer into posthdr */
   static struct
   {
      unsigned int crlf : 1;    /* need to add a CR/LF */
      unsigned int prochdr : 1; /* process post header */
      unsigned int ignore : 1;  /* kill this post */
      unsigned int secondN : 1; /* send a second n */
   } needs;
   static char aryTempText[160];
   static int isFriend; /* Current post is by a friend */

   if ( inputChar == -1 )
   { /* control: begin/end of post */
      if ( postProgressState )
      { /* beginning of post */
         beginUrlDetectionReport();
         posthdrp = posthdr;
         *posthdr = 0;
         needs.crlf = 0;
         needs.prochdr = 1;
         isFriend = 0;
         needs.ignore = 0;
         needs.secondN = 0;
      }
      else
      { /* end of post */
         if ( needs.secondN )
         {
            netPutChar( 'n' );
            byte++;
         }
         filterUrl( " " );
         numposts++;
         emitUrlDetectionReport();
      }
      return;
   }
   /* If post is killed, do no further processing. */
   if ( needs.ignore )
   { /* ignore our stupid enemies :) */
      return;
   }

   if ( posthdr == posthdrp )
   { /* Check for initial CR/LF pair */
      if ( inputChar == '\r' || inputChar == '\n' )
      {
         needs.crlf = 1;
         return;
      }
   }
   if ( needs.prochdr && posthdrp == NULL )
   {
      posthdrp = posthdr;
      *posthdr = 0;
   }
   /* At this point we should either insert the character into the post
     * buffer, or echo it to the screen.
     */
   if ( !needs.prochdr )
   {
      static char aryAnsiSequence[8];
      static size_t ansiSequenceLength = 0;

      /* Store this aryLine for processing */
      if ( inputChar == '\n' )
      {
         filterUrl( thisline );
         thisline[0] = 0;
      }
      else
      {
         /* This is a whole lot faster than calling strcat() */
         char *ptrCursor = thisline;

         for ( ; *ptrCursor; ptrCursor++ )
         { /* Find end of string */
            ;
         }

         *ptrCursor++ = (char)inputChar; /* Copy character to end of string */
         *ptrCursor = 0;
      }

      /* Process ANSI codes in the middle of a post */
      if ( ansiSequenceLength > 0 )
      {
         if ( ansiSequenceLength < sizeof( aryAnsiSequence ) )
         {
            aryAnsiSequence[ansiSequenceLength++] = (char)inputChar;
         }
         if ( inputChar == 'm' || ansiSequenceLength >= sizeof( aryAnsiSequence ) )
         {
            emitTransformedAnsiSequence( aryAnsiSequence, ansiSequenceLength, 1, isFriend );
            ansiSequenceLength = 0;
         }
         return;
      }
      if ( inputChar == '\033' )
      { /* Escape character */
         ansiSequenceLength = 0;
         aryAnsiSequence[ansiSequenceLength++] = (char)inputChar;
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
      /* Change color for end of more prompt */
      if ( flagsConfiguration.shouldUseAnsi && flagsConfiguration.isMorePromptActive && inputChar == ' ' )
      {
         char aryMorePromptSequence[32];

         lastColor = color.text;
         formatAnsiForegroundSequence( aryMorePromptSequence, sizeof( aryMorePromptSequence ),
                                       lastColor );
         stdPrintf( "%s", aryMorePromptSequence );
      }

      /* Output character */
      stdPutChar( inputChar );
   }
   else
   {
      *posthdrp++ = (char)inputChar; /* store character in buffer */
      *posthdrp = 0;

      /* If reached a \r it's time to do header processing */
      if ( inputChar == '\r' )
      {
         char *ptrSenderNameForChecks;

         needs.prochdr = 0;

         /* Process for enemy list kill file */
         char *ptrSenderName;

         snprintf( aryTempText, sizeof( aryTempText ), "%s", posthdr );
         ptrSenderNameForChecks = extractNameNoHistory( aryTempText );
         if ( ptrSenderNameForChecks &&
              slistFind( enemyList, ptrSenderNameForChecks, strCompareVoid ) != -1 )
         {
            needs.ignore = 1;
            postHeaderActive = -1;
            postProgressState = -1;
            netPrintf( "%c%c%c%c", IAC, POST_K, numposts & 0xFF, 17 );
            netflush();
            if ( !flagsConfiguration.shouldSquelchPost )
            {
               stdPrintf( "%s[Post by %s killed]\r\n",
                          *posthdr == '\n' ? "\r\n" : "", ptrSenderNameForChecks );
            }
            else
            {
               pendingLinesToEat = ( needs.crlf ? 1 : 2 );
               netPutChar( 'n' );
               netflush();
               byte++;
            }
            return;
         }

         ptrSenderName = extractName( aryTempText );
         isFriend = ( ptrSenderName &&
                      slistFind( friendList, ptrSenderName, fStrCompareVoid ) != -1 )
                       ? 1
                       : 0;
         ansiTransformPostHeader( posthdr, sizeof( posthdr ), isFriend );
         snprintf( arySavedHeader, sizeof( arySavedHeader ), "%s\r\n", posthdr );
         if ( needs.crlf )
         {
            stdPrintf( "\r\n" );
         }
         printWithOsc8Links( posthdr );
         stdPrintf( "\r" );
      }
   }
}

void filterData( register int inputChar )
{
   static char aryBufferedAnsiSequence[8];
   static size_t bufferedAnsiSequenceLength = 0;

   /* Copy the current aryLine (or what we have so far) */
   if ( inputChar == '\n' )
   {
      filterUrl( thisline );
      if ( strstr( thisline, "Message received by " ) != NULL )
      {
         emitUrlDetectionReport();
      }
      thisline[0] = 0;
   }
   else
   {
      /* This is faster than calling strcat() */
      char *ptrCursor = thisline;

      while ( *ptrCursor )
      { /* Find end of string */
         ptrCursor++;
      }

      *ptrCursor++ = (char)inputChar; /* Copy character to end of string */
      *ptrCursor = 0;
   }

   /* Auto-answer ANSI question */
   if ( flagsConfiguration.shouldAutoAnswerAnsiPrompt && strstr( thisline, "Are you on an ANSI" ) )
   {
      netPutChar( 'y' );
      byte++;
      *thisline = 0; /* Kill it; we don't need it */
   }

   /* Automatic X reply */
   /*  if (sendingXState == SX_SEND_NEXT && xlandQueue->itemCount && (isAway || isXland))
   sendAnX();
   */
   if ( sendingXState == SX_SEND_NEXT && !*thisline && inputChar == '\r' )
   {
      sendingXState = SX_NOT;
      if ( xlandQueue->itemCount && ( isAway || isXland ) )
      {
         sendAnX();
      }
   }

   /* Change color for end of more prompt */
   if ( flagsConfiguration.shouldUseAnsi && flagsConfiguration.isMorePromptActive && inputChar == ' ' )
   {
      char aryAnsiSequence[32];

      lastColor = color.text;
      formatAnsiForegroundSequence( aryAnsiSequence, sizeof( aryAnsiSequence ),
                                    lastColor );
      stdPrintf( "%s", aryAnsiSequence );
   }

   /* Parse ANSI sequences */
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
   { /* Escape character */
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
      pendingLinesToEat = 0; /* pendingLinesToEat should never be less than 0 */
      stdPutChar( inputChar );
   }
   return;
}

void reprintLine( void )
{
   char aryLine[320];
   char *ptrCursor;

   snprintf( aryLine, sizeof( aryLine ), "%s", thisline );
   stdPutChar( '\r' );
   for ( ptrCursor = aryLine; *ptrCursor; ptrCursor++ )
   {
      filterData( *ptrCursor );
   }
   snprintf( thisline, sizeof( thisline ), "%s", aryLine );
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

void continuedPostHelper( void )
{
   static char aryTempText[] = "\033[32m";
   char *ptrText;

   for ( ptrText = aryTempText; *ptrText; ptrText++ )
   {
      filterPost( *ptrText );
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
