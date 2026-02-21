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
#include "test_helpers.h"

static int aryInputQueue[128];
static size_t inputCount;
static size_t inputIndex;
static unsigned int flushCount;
static unsigned int lastFlushValue;

static void resetState( void )
{
   inputCount = 0;
   inputIndex = 0;
   flushCount = 0;
   lastFlushValue = 0;

   flagsConfiguration.useAnsi = 0;
   lastColor = '0';

   if ( friendList != NULL )
   {
      slistDestroyItems( friendList );
      slistDestroy( friendList );
      friendList = NULL;
   }
}

static void setInputSequence( const int *aryKeys, size_t count )
{
   inputCount = copyIntArray( aryKeys, count, aryInputQueue, sizeof( aryInputQueue ) / sizeof( aryInputQueue[0] ) );
   inputIndex = 0;
}

static void addFriend( const char *ptrName )
{
   friend *ptrFriend;

   if ( friendList == NULL )
   {
      friendList = slistCreate( 0, fSortCompareVoid );
      if ( friendList == NULL )
      {
         fail_msg( "slistCreate failed while preparing friendList for color tests" );
         return;
      }
   }

   ptrFriend = calloc( 1, sizeof( friend ) );
   if ( ptrFriend == NULL )
   {
      fail_msg( "calloc failed while creating friend entry for color tests" );
      return;
   }
   ptrFriend->magic = 0x3231;
   snprintf( ptrFriend->name, sizeof( ptrFriend->name ), "%s", ptrName );
   snprintf( ptrFriend->info, sizeof( ptrFriend->info ), "%s", "Color test friend" );

   if ( !slistAddItem( friendList, ptrFriend, 1 ) )
   {
      free( ptrFriend );
      fail_msg( "slistAddItem failed while creating friendList for color tests" );
   }
}

/* color.c dependencies outside the target behavior under test. */
int colorize( const char *ptrText )
{
   (void)ptrText;
   return 1;
}

int fSortCompareVoid( const void *ptrLeft, const void *ptrRight )
{
   const friend *const *ptrLeftFriend;
   const friend *const *ptrRightFriend;

   ptrLeftFriend = ptrLeft;
   ptrRightFriend = ptrRight;
   return strcmp( ( *ptrLeftFriend )->name, ( *ptrRightFriend )->name );
}

int fStrCompareVoid( const void *ptrName, const void *ptrFriend )
{
   return strcmp( (const char *)ptrName, ( (const friend *)ptrFriend )->name );
}

char *findChar( const char *ptrString, int targetChar )
{
   return (char *)strchr( ptrString, targetChar );
}

char *findSubstring( const char *ptrString, const char *ptrSubstring )
{
   return (char *)strstr( ptrString, ptrSubstring );
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

int stdPrintf( const char *format, ... )
{
   va_list argList;

   va_start( argList, format );
   va_end( argList );
   return 1;
}

int yesNoDefault( int defaultAnswer )
{
   return defaultAnswer;
}

static void defaultColors_WhenClearAllApplied_SetsKnownDefaults( void **state )
{
   // Arrange
   (void)state;

   resetState();
   memset( &color, '0', sizeof( color ) );
   color.background = '7';

   // Act
   defaultColors( 1 );

   // Assert
   if ( color.text != '2' || color.forum != '3' || color.number != '6' || color.errorTextColor != '1' )
   {
      fail_msg( "defaultColors(1) did not set general default colors as expected" );
   }
   if ( color.background != '0' )
   {
      fail_msg( "defaultColors(1) should reset background to '0'; got '%c'", color.background );
   }
   if ( color.postname != '6' || color.postfriendname != '1' || color.expressname != '2' )
   {
      fail_msg( "defaultColors(1) did not set post/express defaults as expected" );
   }
}

static void defaultColors_WhenClearAllDisabled_LeavesBackgroundUnchanged( void **state )
{
   // Arrange
   (void)state;

   resetState();
   memset( &color, '0', sizeof( color ) );
   color.text = '9';
   color.background = '4';

   // Act
   defaultColors( 0 );

   // Assert
   if ( color.text != '2' )
   {
      fail_msg( "defaultColors(0) should repair invalid text color to default '2'; got '%c'", color.text );
   }
   if ( color.background != '4' )
   {
      fail_msg( "defaultColors(0) should not overwrite existing background; got '%c'", color.background );
   }
}

static void ansiTransformExpress_WhenFriendSender_UsesFriendColorCodes( void **state )
{
   // Arrange
   char aryMessage[256];

   (void)state;

   resetState();
   flagsConfiguration.useAnsi = 1;
   color.expressfriendtext = '3';
   color.expressfriendname = '5';
   color.text = '7';
   color.expresstext = '2';
   color.expressname = '6';
   addFriend( "Dr Strange" );
   snprintf( aryMessage, sizeof( aryMessage ), "%s", "*** Message (#1) from Dr Strange at 11:01 ***" );

   // Act
   ansiTransformExpress( aryMessage, sizeof( aryMessage ) );

   // Assert
   if ( strstr( aryMessage, "\033[33m" ) == NULL || strstr( aryMessage, "\033[35m" ) == NULL )
   {
      fail_msg( "friend X message should use friend express color codes; got '%s'", aryMessage );
   }
   if ( lastColor != color.text )
   {
      fail_msg( "ansiTransformExpress should set lastColor to text color; got '%c'", lastColor );
   }
}

static void ansiTransformExpress_WhenAnsiDisabled_LeavesTextUnchanged( void **state )
{
   // Arrange
   char aryMessage[256];
   char aryOriginal[256];

   (void)state;

   resetState();
   flagsConfiguration.useAnsi = 0;
   snprintf( aryMessage, sizeof( aryMessage ), "%s", "*** Message (#2) from Meatball at 11:07 ***" );
   snprintf( aryOriginal, sizeof( aryOriginal ), "%s", aryMessage );

   // Act
   ansiTransformExpress( aryMessage, sizeof( aryMessage ) );

   // Assert
   if ( strcmp( aryMessage, aryOriginal ) != 0 )
   {
      fail_msg( "ansiTransformExpress should leave message unchanged when ANSI is off; got '%s'", aryMessage );
   }
}

static void colorPicker_WhenInvalidThenValidInput_ReturnsMappedColorAndFlushes( void **state )
{
   // Arrange
   const int aryKeys[] = { 'z', 'x', 'R' };
   char result;

   (void)state;

   resetState();
   setInputSequence( aryKeys, sizeof( aryKeys ) / sizeof( aryKeys[0] ) );

   // Act
   result = colorPicker();

   // Assert
   if ( result != '1' )
   {
      fail_msg( "colorPicker should map 'R' to color code '1'; got '%c'", result );
   }
   if ( flushCount != 1 || lastFlushValue != 2 )
   {
      fail_msg( "repeated invalid color picker input should flush once with incremented invalid count; got count=%u last=%u",
                flushCount, lastFlushValue );
   }
}

static void backgroundPicker_WhenDefaultSelected_ReturnsDefaultCode( void **state )
{
   // Arrange
   const int aryKeys[] = { 'x', 'x', 'D' };
   char result;

   (void)state;

   resetState();
   color.background = '2';
   setInputSequence( aryKeys, sizeof( aryKeys ) / sizeof( aryKeys[0] ) );

   // Act
   result = backgroundPicker();

   // Assert
   if ( result != '9' )
   {
      fail_msg( "backgroundPicker should map 'D' to default code '9'; got '%c'", result );
   }
   if ( flushCount != 1 || lastFlushValue != 2 )
   {
      fail_msg( "repeated invalid background input should flush once with incremented invalid count; got count=%u last=%u",
                flushCount, lastFlushValue );
   }
}

int main( void )
{
   const struct CMUnitTest aryTests[] = {
      cmocka_unit_test( defaultColors_WhenClearAllApplied_SetsKnownDefaults ),
      cmocka_unit_test( defaultColors_WhenClearAllDisabled_LeavesBackgroundUnchanged ),
      cmocka_unit_test( ansiTransformExpress_WhenFriendSender_UsesFriendColorCodes ),
      cmocka_unit_test( ansiTransformExpress_WhenAnsiDisabled_LeavesTextUnchanged ),
      cmocka_unit_test( colorPicker_WhenInvalidThenValidInput_ReturnsMappedColorAndFlushes ),
      cmocka_unit_test( backgroundPicker_WhenDefaultSelected_ReturnsDefaultCode ),
   };

   return cmocka_run_group_tests( aryTests, NULL, NULL );
}
