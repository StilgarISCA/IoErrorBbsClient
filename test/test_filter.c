/*
 * Copyright (C) 2024-2026 Stilgar
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>

#include "defs.h"
#include "ext.h"
#include "proto.h"

static char aryPrintLog[32768];
static int sendAnXCallCount;

static void resetState( void )
{
   aryPrintLog[0] = '\0';
   sendAnXCallCount = 0;

   isAway = 0;
   isXland = 0;
   isExpressMessageInProgress = 0;
   shouldSendExpressMessage = 0;
   highestExpressMessageId = 0;
   pendingLinesToEat = 0;
   postProgressState = 0;
   whoListProgress = 0;
   savedWhoCount = 0;
   byte = 0;

   flagsConfiguration.shouldSquelchExpress = 0;
   flagsConfiguration.shouldSquelchPost = 0;
   flagsConfiguration.useAnsi = 0;
   flagsConfiguration.isMorePromptActive = 0;
   flagsConfiguration.shouldDisableBold = 0;
   flagsConfiguration.useBold = 0;

   aryExpressMessageBuffer[0] = '\0';
   ptrExpressMessageBuffer = aryExpressMessageBuffer;
}

static void resetLists( void )
{
   if ( urlQueue != NULL )
   {
      deleteQueue( urlQueue );
      urlQueue = NULL;
   }
   if ( xlandQueue != NULL )
   {
      deleteQueue( xlandQueue );
      xlandQueue = NULL;
   }
   if ( enemyList != NULL )
   {
      slistDestroyItems( enemyList );
      slistDestroy( enemyList );
      enemyList = NULL;
   }
   if ( friendList != NULL )
   {
      slistDestroyItems( friendList );
      slistDestroy( friendList );
      friendList = NULL;
   }
   if ( whoList != NULL )
   {
      slistDestroyItems( whoList );
      slistDestroy( whoList );
      whoList = NULL;
   }
}

/* filter.c dependencies not under direct test in this file. */
char ansiTransform( char inputChar )
{
   return inputChar;
}

void ansiTransformExpress( char *ptrText, size_t size )
{
   (void)ptrText;
   (void)size;
}

char ansiTransformPost( char inputChar, int isFriend )
{
   (void)isFriend;
   return inputChar;
}

void ansiTransformPostHeader( char *ptrText, int isFriend )
{
   (void)ptrText;
   (void)isFriend;
}

int colorize( const char *ptrText )
{
   (void)ptrText;
   return 1;
}

char *extractName( const char *ptrHeader )
{
   static char aryName[64];
   const char *ptrStart;
   const char *ptrEnd;
   size_t nameLength;

   ptrStart = strstr( ptrHeader, " from " );
   if ( ptrStart == NULL )
   {
      return NULL;
   }
   ptrStart += 6;

   ptrEnd = strstr( ptrStart, " at " );
   if ( ptrEnd == NULL )
   {
      ptrEnd = ptrStart + strlen( ptrStart );
   }

   nameLength = (size_t)( ptrEnd - ptrStart );
   if ( nameLength >= sizeof( aryName ) )
   {
      nameLength = sizeof( aryName ) - 1;
   }
   memcpy( aryName, ptrStart, nameLength );
   aryName[nameLength] = '\0';
   return aryName;
}

int extractNumber( const char *ptrHeader )
{
   const char *ptrStart;
   int number;

   ptrStart = strstr( ptrHeader, "(#" );
   if ( ptrStart == NULL )
   {
      return 0;
   }
   ptrStart += 2;
   number = 0;
   while ( *ptrStart && isdigit( (unsigned char)*ptrStart ) )
   {
      number = number * 10 + ( *ptrStart - '0' );
      ptrStart++;
   }
   return number;
}

void fatalExit( const char *message, const char *heading )
{
   fail_msg( "fatalExit was called unexpectedly: %s (%s)", message, heading );
}

char *findChar( const char *ptrString, int targetChar )
{
   const char *ptrFound;

   ptrFound = strchr( ptrString, targetChar );
   return (char *)ptrFound;
}

char *findSubstring( const char *ptrString, const char *ptrSubstring )
{
   const char *ptrFound;

   ptrFound = strstr( ptrString, ptrSubstring );
   return (char *)ptrFound;
}

int fStrCompareVoid( const void *ptrName, const void *ptrFriend )
{
   const friend *ptrEntry;

   ptrEntry = ptrFriend;
   return strcmp( (const char *)ptrName, ptrEntry->name );
}

int netPrintf( const char *format, ... )
{
   va_list argList;

   va_start( argList, format );
   va_end( argList );
   return 1;
}

int netPutChar( int inputChar )
{
   return inputChar;
}

void sendAnX( void )
{
   sendAnXCallCount++;
}

int sortCompareVoid( const void *ptrLeft, const void *ptrRight )
{
   const char *const *ptrLeftString;
   const char *const *ptrRightString;

   ptrLeftString = ptrLeft;
   ptrRightString = ptrRight;
   return strcmp( *ptrLeftString, *ptrRightString );
}

int stdPrintf( const char *format, ... )
{
   va_list argList;
   char aryBuffer[1024];
   size_t logLength;

   va_start( argList, format );
   vsnprintf( aryBuffer, sizeof( aryBuffer ), format, argList );
   va_end( argList );

   logLength = strlen( aryPrintLog );
   if ( logLength < sizeof( aryPrintLog ) - 1 )
   {
      snprintf( aryPrintLog + logLength, sizeof( aryPrintLog ) - logLength, "%s", aryBuffer );
   }

   return 1;
}

int stdPutChar( int inputChar )
{
   return inputChar;
}

int strCompareVoid( const void *ptrLeft, const void *ptrRight )
{
   return strcmp( (const char *)ptrLeft, (const char *)ptrRight );
}

static void isAutomaticReply_WhenReplyPrefixPresent_ReturnsTrue( void **state )
{
   // Arrange
   int result;

   (void)state;

   resetState();

   // Act
   result = isAutomaticReply( "*** Message (#4) from Dr Strange at 8:30 PM\r\n>+!R Back soon" );

   // Assert
   if ( result != 1 )
   {
      fail_msg( "isAutomaticReply should detect '+!R' reply marker; got %d", result );
   }
}

static void replyCodeTransformExpress_WhenReplyCodePresent_RemovesReplyCodePrefix( void **state )
{
   // Arrange
   char aryMessage[256];

   (void)state;

   resetState();
   snprintf( aryMessage, sizeof( aryMessage ), "%s", "*** Message (#9) from Meatball at 9:12 PM\r\n>+!R Copy that" );

   // Act
   replyCodeTransformExpress( aryMessage, sizeof( aryMessage ) );

   // Assert
   if ( strstr( aryMessage, ">+!R " ) != NULL )
   {
      fail_msg( "replyCodeTransformExpress should remove '+!R ' marker; got '%s'", aryMessage );
   }
   if ( strstr( aryMessage, ">Copy that" ) == NULL )
   {
      fail_msg( "replyCodeTransformExpress should preserve message body; got '%s'", aryMessage );
   }
}

static void notReplyingTransformExpress_WhenHeaderContainsAt_InsertsNotReplyingLabel( void **state )
{
   // Arrange
   char aryMessage[256];

   (void)state;

   resetState();
   snprintf( aryMessage, sizeof( aryMessage ), "%s", "*** Message (#9) from Dr Strange at 9:12 PM" );

   // Act
   notReplyingTransformExpress( aryMessage, sizeof( aryMessage ) );

   // Assert
   if ( strstr( aryMessage, "(not replying)" ) == NULL )
   {
      fail_msg( "notReplyingTransformExpress should insert '(not replying)'; got '%s'", aryMessage );
   }
}

static void filterUrl_WhenDuplicateSeen_QueuesOnlyOnce( void **state )
{
   // Arrange
   char aryUrl[1024];
   int firstPopResult;
   int secondPopResult;

   (void)state;

   resetState();
   resetLists();
   urlQueue = newQueue( 1024, 5 );
   if ( urlQueue == NULL )
   {
      fail_msg( "newQueue failed for urlQueue setup in duplicate URL test" );
   }

   // Act
   filterUrl( "Did I leave it here? http://bbs.example.net/threads/42" );
   filterUrl( "Again: http://bbs.example.net/threads/42" );

   // Assert
   if ( urlQueue->nobjs != 1 )
   {
      fail_msg( "duplicate URLs should be queued only once; queue count is %d", urlQueue->nobjs );
   }
   memset( aryUrl, 0, sizeof( aryUrl ) );
   firstPopResult = popQueue( aryUrl, urlQueue );
   secondPopResult = popQueue( aryUrl, urlQueue );
   if ( firstPopResult != 1 || secondPopResult != 0 )
   {
      fail_msg( "URL queue should contain exactly one element; pop results were %d then %d", firstPopResult, secondPopResult );
   }
   if ( strcmp( aryUrl, "http://bbs.example.net/threads/42" ) != 0 )
   {
      fail_msg( "queued URL mismatch; got '%s'", aryUrl );
   }

   resetLists();
}

static void filterUrl_WhenQueueIsFull_EvictsOldestUrl( void **state )
{
   // Arrange
   char aryFirst[1024];
   char arySecond[1024];

   (void)state;

   resetState();
   resetLists();
   urlQueue = newQueue( 1024, 2 );
   if ( urlQueue == NULL )
   {
      fail_msg( "newQueue failed for URL overflow test setup" );
   }

   // Act
   filterUrl( "One http://example.net/one" );
   filterUrl( "Two http://example.net/two" );
   filterUrl( "Three http://example.net/three" );

   // Assert
   if ( urlQueue->nobjs != 2 )
   {
      fail_msg( "URL queue size should stay capped at 2; got %d", urlQueue->nobjs );
   }
   memset( aryFirst, 0, sizeof( aryFirst ) );
   memset( arySecond, 0, sizeof( arySecond ) );
   popQueue( aryFirst, urlQueue );
   popQueue( arySecond, urlQueue );
   if ( strcmp( aryFirst, "http://example.net/two" ) != 0 || strcmp( arySecond, "http://example.net/three" ) != 0 )
   {
      fail_msg( "URL queue eviction order mismatch; got '%s' then '%s'", aryFirst, arySecond );
   }

   resetLists();
}

static void filterExpress_WhenAwayAndIncomingNewMessage_QueuesSender( void **state )
{
   // Arrange
   const char *ptrHeader;

   (void)state;

   resetState();
   resetLists();
   isAway = 1;
   xlandQueue = newQueue( 21, 5 );
   enemyList = slistCreate( 0, sortCompareVoid );
   if ( xlandQueue == NULL || enemyList == NULL )
   {
      fail_msg( "failed to allocate xlandQueue/enemyList for filterExpress test" );
   }
   ptrHeader = "*** Message (#5) from Dr Strange at 3:07 PM on Feb 19, 2026 ***\r";

   // Act
   isExpressMessageInProgress = 1;
   filterExpress( -1 );
   for ( ; *ptrHeader != '\0'; ++ptrHeader )
   {
      filterExpress( *ptrHeader );
   }
   isExpressMessageInProgress = 0;
   filterExpress( -1 );

   // Assert
   if ( shouldSendExpressMessage != 1 )
   {
      fail_msg( "new incoming X while away should set shouldSendExpressMessage=1; got %d", shouldSendExpressMessage );
   }
   if ( highestExpressMessageId != 5 )
   {
      fail_msg( "highestExpressMessageId should update to parsed ID 5; got %d", highestExpressMessageId );
   }
   if ( xlandQueue->nobjs != 1 || !isQueued( "Dr Strange", xlandQueue ) )
   {
      fail_msg( "sender should be queued exactly once for auto-reply flow" );
   }

   resetLists();
}

int main( void )
{
   const struct CMUnitTest aryTests[] = {
      cmocka_unit_test( isAutomaticReply_WhenReplyPrefixPresent_ReturnsTrue ),
      cmocka_unit_test( replyCodeTransformExpress_WhenReplyCodePresent_RemovesReplyCodePrefix ),
      cmocka_unit_test( notReplyingTransformExpress_WhenHeaderContainsAt_InsertsNotReplyingLabel ),
      cmocka_unit_test( filterUrl_WhenDuplicateSeen_QueuesOnlyOnce ),
      cmocka_unit_test( filterUrl_WhenQueueIsFull_EvictsOldestUrl ),
      cmocka_unit_test( filterExpress_WhenAwayAndIncomingNewMessage_QueuesSender ),
   };

   return cmocka_run_group_tests( aryTests, NULL, NULL );
}
