/*
 * Copyright (C) 2024-2026 Stilgar
 * Copyright (C) 1995-2003 Michael Hampton
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "defs.h"
#include "ext.h"
#include "telnet.h"

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

      /* Store this line for processing */
      if ( inputChar == '\n' )
      {
         filterUrl( aryFilterLine );
         aryFilterLine[0] = 0;
      }
      else
      {
         /* This is a whole lot faster than calling strcat() */
         char *ptrCursor = aryFilterLine;

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

void continuedPostHelper( void )
{
   static char aryTempText[] = "\033[32m";
   char *ptrText;

   for ( ptrText = aryTempText; *ptrText; ptrText++ )
   {
      filterPost( *ptrText );
   }
}
