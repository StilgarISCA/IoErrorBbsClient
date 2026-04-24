/*
 * Copyright (C) 2024-2026 Stilgar
 * Copyright (C) 1995-2003 Michael Hampton
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "defs.h"
#include "browser.h"
#include "client_globals.h"
#include "color.h"
#include "filter_globals.h"
#include "filter.h"
#include "utility.h"

typedef struct
{
   unsigned int crlf : 1;      /* Needs initial CR/LF */
   unsigned int prochdr : 1;   /* Needs killfile processing */
   unsigned int ignore : 1;    /* Ignore the remainder of the X */
   unsigned int truncated : 1; /* X message exceeded buffer */
} ExpressFilterFlags;

static void beginExpressMessage( ExpressFilterFlags *ptrFlags );
static void finishExpressMessage( const ExpressFilterFlags *ptrFlags,
                                  const char *ptrSenderName );
static void processExpressHeader( ExpressFilterFlags *ptrFlags, char **ptrSenderName );

static void beginExpressMessage( ExpressFilterFlags *ptrFlags )
{
   beginUrlDetectionReport();
   ptrExpressMessageBuffer = aryExpressMessageBuffer;
   *aryExpressMessageBuffer = 0;
   ptrFlags->ignore = 0;
   ptrFlags->crlf = 0;
   ptrFlags->prochdr = 1;
   ptrFlags->truncated = 0;
}

void filterExpress( register int inputChar )
{
   static char *ptrSenderName; /* comparison pointer */
   static ExpressFilterFlags needs;

   if ( inputChar == -1 )
   { /* signal from IAC to begin/end X */
      if ( isExpressMessageInProgress )
      { /* Need to re-init for new X */
         beginExpressMessage( &needs );
         return;
      }
      else if ( !needs.ignore )
      { /* Finished this X, dump it out */
         finishExpressMessage( &needs, ptrSenderName );
         return;
      }
      emitUrlDetectionReport();
   }
   /* If the message is killed, don't bother doing anything. */
   if ( needs.ignore )
   {
      return;
   }
   if ( needs.truncated )
   {
      return;
   }

   if ( aryExpressMessageBuffer == ptrExpressMessageBuffer )
   { /* Check for initial CR/LF pair */
      if ( inputChar == '\r' || inputChar == '\n' )
      {
         needs.crlf = 1;
         return;
      }
   }
   /* Insert character into the buffer (drop excess to avoid overflow) */
   if ( ptrExpressMessageBuffer >= aryExpressMessageBuffer + sizeof( aryExpressMessageBuffer ) - 1 )
   {
      needs.truncated = 1;
      return;
   }
   *ptrExpressMessageBuffer++ = (char)inputChar;
   *ptrExpressMessageBuffer = 0;

   /* Extract URLs if any */
   if ( !needs.prochdr && inputChar == '\r' )
   {
      filterUrl( ptrExpressMessageBuffer );
   }

   /* If reached a \r it's time to do header processing */
   if ( needs.prochdr && inputChar == '\r' )
   {
      needs.prochdr = 0;
      processExpressHeader( &needs, &ptrSenderName );
   }
}

static void finishExpressMessage( const ExpressFilterFlags *ptrFlags,
                                  const char *ptrSenderName )
{
   int itemIndex;
   int isAutoReply;

   itemIndex = extractNumber( aryExpressMessageBuffer );
   isAutoReply = isAutomaticReply( aryExpressMessageBuffer );

   if ( ( isAway || isXland ) && ptrSenderName &&
        !isQueued( ptrSenderName, xlandQueue ) &&
        itemIndex > highestExpressMessageId && !isAutoReply )
   {
      pushQueue( ptrSenderName, xlandQueue );
      shouldSendExpressMessage = 1;
   }
   else if ( isAutoReply && itemIndex > highestExpressMessageId )
   {
      notReplyingTransformExpress( aryExpressMessageBuffer, sizeof( aryExpressMessageBuffer ) );
   }

   if ( itemIndex > highestExpressMessageId )
   {
      highestExpressMessageId = itemIndex;
   }

   replyCodeTransformExpress( aryExpressMessageBuffer, sizeof( aryExpressMessageBuffer ) );
   ansiTransformExpress( aryExpressMessageBuffer, sizeof( aryExpressMessageBuffer ) );
   if ( ptrFlags->crlf )
   {
      stdPrintf( "\r\n" );
   }
   printWithOsc8Links( aryExpressMessageBuffer );
   if ( ptrFlags->truncated )
   {
      stdPrintf( "\r\n[X message truncated]\r\n" );
   }
   emitUrlDetectionReport();
}

/* Check for an automatic reply message. Return 1 if this is such a message. */
int isAutomaticReply( const char *message )
{
   const char *ptrCursor;

   /* Find first aryLine */
   ptrCursor = findSubstring( message, ">" );

   /* Wasn't a first aryLine? - move past '>' */
   if ( !ptrCursor )
   {
      return 0;
   }
   ptrCursor++;

   /* Check for valid automatic reply messages */
   if ( !strncmp( ptrCursor, "+!R", 3 ) ||
        !strncmp( ptrCursor, "This message was automatically generated", 40 ) ||
        !strncmp( ptrCursor, "*** ISCA Windows Client", 23 ) )
   {
      return 1;
   }

   /* Looks normal to me... */
   return 0;
}

void notReplyingTransformExpress( char *ptrText, size_t size )
{
   char aryTempText[580];
   char *ptrMessageStart;

   /* Verify this is an X message and set up pointers */
   ptrMessageStart = findSubstring( ptrText, " at " );
   if ( !ptrMessageStart )
   {
      return;
   }

   *( ptrMessageStart++ ) = 0;

   snprintf( aryTempText, sizeof( aryTempText ), "%s (not replying) %s", ptrText, ptrMessageStart );
   snprintf( ptrText, size, "%s", aryTempText );
}

static void processExpressHeader( ExpressFilterFlags *ptrFlags, char **ptrSenderName )
{
   *ptrSenderName = extractNameNoHistory( aryExpressMessageBuffer );
   if ( *ptrSenderName &&
        slistFind( enemyList, *ptrSenderName, strCompareVoid ) != -1 )
   {
      if ( !flagsConfiguration.shouldSquelchExpress )
      {
         stdPrintf( "\r\n[X message by %s killed]\r\n", *ptrSenderName );
      }
      ptrFlags->ignore = 1;
      return;
   }
   if ( *ptrSenderName )
   {
      *ptrSenderName = extractName( aryExpressMessageBuffer );
   }
}

void replyCodeTransformExpress( char *ptrText, size_t size )
{
   char aryTempText[580];
   char *ptrMessageStart;

   /* Verify this is an auto reply and set up pointers */
   ptrMessageStart = findSubstring( ptrText, ">" );
   if ( !ptrMessageStart || strncmp( ptrMessageStart, ">+!R ", 5 ) )
   {
      return;
   }

   *( ++ptrMessageStart ) = 0;
   ptrMessageStart += 4;

   snprintf( aryTempText, sizeof( aryTempText ), "%s%s", ptrText, ptrMessageStart );
   snprintf( ptrText, size, "%s", aryTempText );
}
