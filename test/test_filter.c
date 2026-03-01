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
   flagsConfiguration.shouldEnableClickableUrls = 1;

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

char *extractNameNoHistory( const char *header )
{
   return extractName( header );
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

void sPerror( const char *message, const char *heading )
{
   (void)message;
   (void)heading;
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

void getString( int length, char *result, int line )
{
   (void)line;
   if ( length > 0 && result != NULL )
   {
      result[0] = '\0';
   }
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
   filterUrl( "Did I leave it here? https://bbs.example.net/threads/42" );
   filterUrl( "Again: https://bbs.example.net/threads/42" );

   // Assert
   if ( urlQueue->itemCount != 1 )
   {
      fail_msg( "duplicate URLs should be queued only once; queue count is %d", urlQueue->itemCount );
   }
   memset( aryUrl, 0, sizeof( aryUrl ) );
   firstPopResult = popQueue( aryUrl, urlQueue );
   secondPopResult = popQueue( aryUrl, urlQueue );
   if ( firstPopResult != 1 || secondPopResult != 0 )
   {
      fail_msg( "URL queue should contain exactly one element; pop results were %d then %d", firstPopResult, secondPopResult );
   }
   if ( strcmp( aryUrl, "https://bbs.example.net/threads/42" ) != 0 )
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
   filterUrl( "One https://example.net/one" );
   filterUrl( "Two https://example.net/two" );
   filterUrl( "Three https://example.net/three" );

   // Assert
   if ( urlQueue->itemCount != 2 )
   {
      fail_msg( "URL queue size should stay capped at 2; got %d", urlQueue->itemCount );
   }
   memset( aryFirst, 0, sizeof( aryFirst ) );
   memset( arySecond, 0, sizeof( arySecond ) );
   popQueue( aryFirst, urlQueue );
   popQueue( arySecond, urlQueue );
   if ( strcmp( aryFirst, "https://example.net/two" ) != 0 || strcmp( arySecond, "https://example.net/three" ) != 0 )
   {
      fail_msg( "URL queue eviction order mismatch; got '%s' then '%s'", aryFirst, arySecond );
   }

   resetLists();
}

static void filterUrl_WhenHttpsAndTrailingPunctuationPresent_ExtractsCleanUrl( void **state )
{
   // Arrange
   char aryUrl[1024];
   int popResult;

   (void)state;

   resetState();
   resetLists();
   urlQueue = newQueue( 1024, 5 );
   if ( urlQueue == NULL )
   {
      fail_msg( "newQueue failed for HTTPS URL punctuation test setup" );
   }

   // Act
   filterUrl( "Read this: https://example.dev/path?q=1#frag)." );

   // Assert
   memset( aryUrl, 0, sizeof( aryUrl ) );
   popResult = popQueue( aryUrl, urlQueue );
   if ( popResult != 1 )
   {
      fail_msg( "HTTPS URL should be queued exactly once; pop returned %d", popResult );
   }
   if ( strcmp( aryUrl, "https://example.dev/path?q=1#frag" ) != 0 )
   {
      fail_msg( "HTTPS URL should trim trailing punctuation; got '%s'", aryUrl );
   }

   resetLists();
}

static void filterUrl_WhenWwwUrlPresent_QueuesUrlWithoutTldList( void **state )
{
   // Arrange
   char aryUrl[1024];
   int popResult;

   (void)state;

   resetState();
   resetLists();
   urlQueue = newQueue( 1024, 5 );
   if ( urlQueue == NULL )
   {
      fail_msg( "newQueue failed for www URL test setup" );
   }

   // Act
   filterUrl( "Mirror is at www.example.photography/section/alpha, check it out." );

   // Assert
   memset( aryUrl, 0, sizeof( aryUrl ) );
   popResult = popQueue( aryUrl, urlQueue );
   if ( popResult != 1 )
   {
      fail_msg( "www URL should be queued exactly once; pop returned %d", popResult );
   }
   if ( strcmp( aryUrl, "www.example.photography/section/alpha" ) != 0 )
   {
      fail_msg( "www URL parsing should support modern TLDs and trim punctuation; got '%s'", aryUrl );
   }

   resetLists();
}

static void filterUrl_WhenHttpOrFtpPresent_DoesNotQueueUnsupportedSchemes( void **state )
{
   // Arrange
   (void)state;

   resetState();
   resetLists();
   urlQueue = newQueue( 1024, 5 );
   if ( urlQueue == NULL )
   {
      fail_msg( "newQueue failed for unsupported scheme test setup" );
   }

   // Act
   filterUrl( "Legacy link http://example.net/legacy should be ignored." );
   filterUrl( "Another legacy link ftp://example.net/pub/file should be ignored." );

   // Assert
   if ( urlQueue->itemCount != 0 )
   {
      fail_msg( "unsupported schemes should not be queued; queue count is %d", urlQueue->itemCount );
   }

   resetLists();
}

static void filterUrl_WhenHttpsWrapsAcrossLines_CombinesIntoSingleUrl( void **state )
{
   // Arrange
   char aryUrl[1024];
   int popResult;

   (void)state;

   resetState();
   resetLists();
   urlQueue = newQueue( 1024, 5 );
   if ( urlQueue == NULL )
   {
      fail_msg( "newQueue failed for wrapped URL test setup" );
   }

   // Act
   filterUrl( "https://arstechnica.com/gadgets/2026/01/inside-nvidias-10-year-effort-to-make-t" );
   filterUrl( "he-shield-tv-the-most-updated-android-device-ever/" );
   filterUrl( "Also still gets software updates!" );

   // Assert
   memset( aryUrl, 0, sizeof( aryUrl ) );
   popResult = popQueue( aryUrl, urlQueue );
   if ( popResult != 1 )
   {
      fail_msg( "wrapped HTTPS URL should be queued once; pop returned %d", popResult );
   }
   if ( strcmp( aryUrl, "https://arstechnica.com/gadgets/2026/01/inside-nvidias-10-year-effort-to-make-the-shield-tv-the-most-updated-android-device-ever/" ) != 0 )
   {
      fail_msg( "wrapped URL should be reconstructed into one URL; got '%s'", aryUrl );
   }

   resetLists();
}

static void printWithOsc8Links_WhenTextContainsHttpsUrl_EmitsHyperlinkEscapes( void **state )
{
   // Arrange
   const char *ptrMessage;

   (void)state;

   resetState();
   ptrMessage = "Read this https://example.dev/path?q=1 now";

   // Act
   printWithOsc8Links( ptrMessage );

   // Assert
   if ( strstr( aryPrintLog, "\033]8;;https://example.dev/path?q=1\033\\" ) == NULL )
   {
      fail_msg( "OSC-8 opening sequence for HTTPS URL was not emitted; log was: %s", aryPrintLog );
   }
   if ( strstr( aryPrintLog, "\033]8;;\033\\" ) == NULL )
   {
      fail_msg( "OSC-8 closing sequence was not emitted; log was: %s", aryPrintLog );
   }
}

static void printWithOsc8Links_WhenTextContainsWwwUrl_UsesHttpsTarget( void **state )
{
   // Arrange
   const char *ptrMessage;

   (void)state;

   resetState();
   ptrMessage = "Mirror: www.example.photography/alpha";

   // Act
   printWithOsc8Links( ptrMessage );

   // Assert
   if ( strstr( aryPrintLog, "\033]8;;https://www.example.photography/alpha\033\\" ) == NULL )
   {
      fail_msg( "OSC-8 target for www URL should be normalized to https://...; log was: %s", aryPrintLog );
   }
}

static void printWithOsc8Links_WhenClickableUrlsDisabled_PrintsPlainText( void **state )
{
   // Arrange
   const char *ptrMessage;

   (void)state;

   resetState();
   flagsConfiguration.shouldEnableClickableUrls = 0;
   ptrMessage = "Read this https://example.dev/path";

   // Act
   printWithOsc8Links( ptrMessage );

   // Assert
   if ( strstr( aryPrintLog, "\033]8;;" ) != NULL )
   {
      fail_msg( "OSC-8 escapes should not be emitted when clickable URLs are disabled; log was: %s", aryPrintLog );
   }
   if ( strstr( aryPrintLog, ptrMessage ) == NULL )
   {
      fail_msg( "plain message text should be printed when clickable URLs are disabled; log was: %s", aryPrintLog );
   }
}

static void emitUrlDetectionReport_WhenUrlsCollected_PrintsClickableSummary( void **state )
{
   // Arrange
   (void)state;

   resetState();
   resetLists();
   urlQueue = newQueue( 1024, 5 );
   if ( urlQueue == NULL )
   {
      fail_msg( "newQueue failed for URL detection report test setup" );
   }

   beginUrlDetectionReport();
   filterUrl( "Read this https://example.dev/alpha" );
   filterUrl( "and this www.example.photography/beta" );

   // Act
   emitUrlDetectionReport();

   // Assert
   if ( strstr( aryPrintLog, "[Clickable URL(s) detected by BBS client]" ) == NULL )
   {
      fail_msg( "URL detection report header was not emitted; log was: %s", aryPrintLog );
   }
   if ( strstr( aryPrintLog, "\033]8;;https://example.dev/alpha\033\\" ) == NULL )
   {
      fail_msg( "URL detection report should include clickable HTTPS URL; log was: %s", aryPrintLog );
   }
   if ( strstr( aryPrintLog, "\033]8;;https://www.example.photography/beta\033\\" ) == NULL )
   {
      fail_msg( "URL detection report should include clickable normalized www URL; log was: %s", aryPrintLog );
   }

   resetLists();
}

static void emitUrlDetectionReport_WhenClickableUrlsDisabled_EmitsNoSummary( void **state )
{
   // Arrange
   (void)state;

   resetState();
   resetLists();
   flagsConfiguration.shouldEnableClickableUrls = 0;
   urlQueue = newQueue( 1024, 5 );
   if ( urlQueue == NULL )
   {
      fail_msg( "newQueue failed for disabled URL detection report test setup" );
   }

   beginUrlDetectionReport();
   filterUrl( "Read this https://example.dev/alpha" );

   // Act
   emitUrlDetectionReport();

   // Assert
   if ( strstr( aryPrintLog, "[Clickable URL(s) detected by BBS client]" ) != NULL )
   {
      fail_msg( "URL detection report should be suppressed when clickable URLs are disabled; log was: %s", aryPrintLog );
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
   if ( xlandQueue->itemCount != 1 || !isQueued( "Dr Strange", xlandQueue ) )
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
      cmocka_unit_test( filterUrl_WhenHttpsAndTrailingPunctuationPresent_ExtractsCleanUrl ),
      cmocka_unit_test( filterUrl_WhenWwwUrlPresent_QueuesUrlWithoutTldList ),
      cmocka_unit_test( filterUrl_WhenHttpOrFtpPresent_DoesNotQueueUnsupportedSchemes ),
      cmocka_unit_test( filterUrl_WhenHttpsWrapsAcrossLines_CombinesIntoSingleUrl ),
      cmocka_unit_test( printWithOsc8Links_WhenTextContainsHttpsUrl_EmitsHyperlinkEscapes ),
      cmocka_unit_test( printWithOsc8Links_WhenTextContainsWwwUrl_UsesHttpsTarget ),
      cmocka_unit_test( printWithOsc8Links_WhenClickableUrlsDisabled_PrintsPlainText ),
      cmocka_unit_test( emitUrlDetectionReport_WhenUrlsCollected_PrintsClickableSummary ),
      cmocka_unit_test( emitUrlDetectionReport_WhenClickableUrlsDisabled_EmitsNoSummary ),
      cmocka_unit_test( filterExpress_WhenAwayAndIncomingNewMessage_QueuesSender ),
   };

   return cmocka_run_group_tests( aryTests, NULL, NULL );
}
