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

static int aryInputQueue[256];
static size_t inputCount;
static size_t inputIndex;
static unsigned int flushCount;
static unsigned int lastFlushValue;
static char aryCapturedString[256];
static int capPutsCallCount;
static char aryCapturedDots[256];
static size_t capturedDotCount;

static void setInputSequence( const int *aryKeys, size_t count )
{
   size_t keyIndex;

   for ( keyIndex = 0; keyIndex < count && keyIndex < sizeof( aryInputQueue ) / sizeof( aryInputQueue[0] ); ++keyIndex )
   {
      aryInputQueue[keyIndex] = aryKeys[keyIndex];
   }
   inputCount = count;
   inputIndex = 0;
}

static void resetTracking( void )
{
   inputCount = 0;
   inputIndex = 0;
   flushCount = 0;
   lastFlushValue = 0;
   aryCapturedString[0] = '\0';
   capPutsCallCount = 0;
   aryCapturedDots[0] = '\0';
   capturedDotCount = 0;
}

static int compareStringsVoid( const void *ptrLeft, const void *ptrRight )
{
   const char *const *ptrLeftString;
   const char *const *ptrRightString;

   ptrLeftString = ptrLeft;
   ptrRightString = ptrRight;
   return strcmp( *ptrLeftString, *ptrRightString );
}

static char *allocateString( const char *ptrSource )
{
   char *ptrCopy;
   size_t sourceLength;

   sourceLength = strlen( ptrSource );
   ptrCopy = calloc( sourceLength + 1, sizeof( char ) );
   if ( ptrCopy == NULL )
   {
      fail_msg( "calloc failed while allocating test string '%s'", ptrSource );
      return NULL;
   }
   memcpy( ptrCopy, ptrSource, sourceLength );
   return ptrCopy;
}

static void setupWhoList( const char *ptrFirst, const char *ptrSecond )
{
   whoList = slistCreate( 0, compareStringsVoid );
   if ( whoList == NULL )
   {
      fail_msg( "slistCreate failed for whoList setup" );
      return;
   }

   if ( !slistAddItem( whoList, allocateString( ptrFirst ), 1 ) ||
        !slistAddItem( whoList, allocateString( ptrSecond ), 1 ) )
   {
      fail_msg( "slistAddItem failed while preparing whoList for getline tests" );
      return;
   }
   slistSort( whoList );
}

static void teardownWhoList( void )
{
   if ( whoList != NULL )
   {
      slistDestroyItems( whoList );
      slistDestroy( whoList );
      whoList = NULL;
   }
}

/* getline.c dependencies not under test here. */
int capPutChar( int inputChar )
{
   if ( capturedDotCount < sizeof( aryCapturedDots ) - 1 )
   {
      aryCapturedDots[capturedDotCount++] = (char)inputChar;
      aryCapturedDots[capturedDotCount] = '\0';
   }
   return inputChar;
}

int capPuts( const char *ptrText )
{
   capPutsCallCount++;
   snprintf( aryCapturedString, sizeof( aryCapturedString ), "%s", ptrText );
   return 1;
}

void flushInput( unsigned int count )
{
   flushCount++;
   lastFlushValue = count;
}

int inKey( void )
{
   if ( inputIndex < inputCount )
   {
      return aryInputQueue[inputIndex++];
   }
   return '\n';
}

int netPutChar( int inputChar )
{
   return inputChar;
}

int popQueue( char *ptrObject, queue *ptrQueue )
{
   (void)ptrObject;
   (void)ptrQueue;
   return 0;
}

void replyMessage( void ) {}

void sendBlock( void ) {}

int stdPrintf( const char *format, ... )
{
   va_list argList;

   va_start( argList, format );
   va_end( argList );
   return 0;
}

void writeBbsRc( void ) {}

static void smartName_WhenUniquePrefix_ExpandsToFullName( void **state )
{
   // Arrange
   char aryBuffer[41];
   int found;

   (void)state;

   resetTracking();
   teardownWhoList();
   setupWhoList( "Dr Strange", "Meatball" );
   snprintf( aryBuffer, sizeof( aryBuffer ), "%s", "Dr S" );

   // Act
   found = smartName( aryBuffer, aryBuffer + strlen( aryBuffer ) );

   // Assert
   if ( found != 1 )
   {
      fail_msg( "smartName should return 1 for a unique prefix match; got %d", found );
   }
   if ( strcmp( aryBuffer, "Dr Strange" ) != 0 )
   {
      fail_msg( "smartName should expand to 'Dr Strange'; got '%s'", aryBuffer );
   }

   teardownWhoList();
}

static void smartName_WhenPrefixIsAmbiguous_ReturnsNoMatchAndRestoresTail( void **state )
{
   // Arrange
   char aryBuffer[41];
   int found;

   (void)state;

   resetTracking();
   teardownWhoList();
   setupWhoList( "Meatball", "Merlin" );
   snprintf( aryBuffer, sizeof( aryBuffer ), "%s", "MeX" );

   // Act
   found = smartName( aryBuffer, aryBuffer + 2 );

   // Assert
   if ( found != 0 )
   {
      fail_msg( "smartName should return 0 for ambiguous prefix; got %d", found );
   }
   if ( aryBuffer[2] != 'X' )
   {
      fail_msg( "smartName should restore tail character when no unique match; got '%c'", aryBuffer[2] );
   }

   teardownWhoList();
}

static void getString_WhenSimpleInputProvided_ReturnsTypedString( void **state )
{
   // Arrange
   char aryResult[64];
   const int aryKeys[] = { 'H', 'i', '\n' };

   (void)state;

   resetTracking();
   setInputSequence( aryKeys, sizeof( aryKeys ) / sizeof( aryKeys[0] ) );

   // Act
   getString( 20, aryResult, 0 );

   // Assert
   if ( strcmp( aryResult, "Hi" ) != 0 )
   {
      fail_msg( "getString should return typed string 'Hi'; got '%s'", aryResult );
   }
   if ( capPutsCallCount != 1 || strcmp( aryCapturedString, "Hi" ) != 0 )
   {
      fail_msg( "getString should capture plain input via capPuts; got calls=%d text='%s'",
                capPutsCallCount, aryCapturedString );
   }
}

static void getString_WhenCtrlWUsed_RemovesPreviousWord( void **state )
{
   // Arrange
   char aryResult[64];
   const int aryKeys[] = { 'D', 'r', ' ', 'S', 't', 'r', 'a', 'n', 'g', 'e', CTRL_W, 'W', 'h', 'o', '\n' };

   (void)state;

   resetTracking();
   setInputSequence( aryKeys, sizeof( aryKeys ) / sizeof( aryKeys[0] ) );

   // Act
   getString( 30, aryResult, 0 );

   // Assert
   if ( strcmp( aryResult, "Dr Who" ) != 0 )
   {
      fail_msg( "CTRL_W should erase previous word; expected 'Dr Who', got '%s'", aryResult );
   }
}

static void getString_WhenHiddenInputUsed_CapturesDotsInsteadOfPlainText( void **state )
{
   // Arrange
   char aryResult[64];
   const int aryKeys[] = { 's', 'e', 'c', 'r', 'e', 't', '\n' };

   (void)state;

   resetTracking();
   setInputSequence( aryKeys, sizeof( aryKeys ) / sizeof( aryKeys[0] ) );

   // Act
   getString( -20, aryResult, 0 );

   // Assert
   if ( strcmp( aryResult, "secret" ) != 0 )
   {
      fail_msg( "hidden getString should still store clear text in result; got '%s'", aryResult );
   }
   if ( capPutsCallCount != 0 )
   {
      fail_msg( "hidden getString should not call capPuts directly; got %d calls", capPutsCallCount );
   }
   if ( strcmp( aryCapturedDots, "......" ) != 0 )
   {
      fail_msg( "hidden getString should capture one dot per character; got '%s'", aryCapturedDots );
   }
}

static void getString_WhenRepeatedInvalidControlInputReceived_FlushesInput( void **state )
{
   // Arrange
   char aryResult[64];
   const int aryKeys[] = { 1, 2, 'A', '\n' };

   (void)state;

   resetTracking();
   setInputSequence( aryKeys, sizeof( aryKeys ) / sizeof( aryKeys[0] ) );

   // Act
   getString( 20, aryResult, 0 );

   // Assert
   if ( strcmp( aryResult, "A" ) != 0 )
   {
      fail_msg( "getString should ignore invalid controls and keep valid chars; got '%s'", aryResult );
   }
   if ( flushCount == 0 || lastFlushValue != 2 )
   {
      fail_msg( "repeated invalid controls should trigger flushInput with incremented invalid counter; got count=%u last=%u",
                flushCount, lastFlushValue );
   }
}

int main( void )
{
   const struct CMUnitTest aryTests[] = {
      cmocka_unit_test( smartName_WhenUniquePrefix_ExpandsToFullName ),
      cmocka_unit_test( smartName_WhenPrefixIsAmbiguous_ReturnsNoMatchAndRestoresTail ),
      cmocka_unit_test( getString_WhenSimpleInputProvided_ReturnsTypedString ),
      cmocka_unit_test( getString_WhenCtrlWUsed_RemovesPreviousWord ),
      cmocka_unit_test( getString_WhenHiddenInputUsed_CapturesDotsInsteadOfPlainText ),
      cmocka_unit_test( getString_WhenRepeatedInvalidControlInputReceived_FlushesInput ),
   };

   return cmocka_run_group_tests( aryTests, NULL, NULL );
}
