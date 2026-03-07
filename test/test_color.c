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
   lastColor = 0;

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

void printAnsiForegroundColorValue( int colorValue )
{
   (void)colorValue;
}

void printThemedMnemonicText( const char *ptrText, int defaultColor )
{
   (void)ptrText;
   (void)defaultColor;
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

void handleInvalidInput( unsigned int *ptrInvalidCount )
{
   if ( ( *ptrInvalidCount )++ )
   {
      flushInput( *ptrInvalidCount );
   }
}

int inKey( void )
{
   if ( inputIndex < inputCount )
   {
      return aryInputQueue[inputIndex++];
   }
   return '\n';
}

int readValidatedMenuKey( const char *allowedCharsLowercase )
{
   int inputChar;
   unsigned int invalid;

   invalid = 0;
   for ( ;; )
   {
      inputChar = inKey();
      if ( isalpha( inputChar ) )
      {
         inputChar = tolower( inputChar );
      }
      if ( findChar( allowedCharsLowercase, inputChar ) )
      {
         return inputChar;
      }
      handleInvalidInput( &invalid );
   }
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
   memset( &color, 0, sizeof( color ) );
   color.background = 7;

   // Act
   defaultColors( 1 );

   // Assert
   if ( color.text != 2 || color.forum != 3 || color.number != 6 || color.errorTextColor != 1 )
   {
      fail_msg( "defaultColors(1) did not set general default colors as expected" );
   }
   if ( color.background != 0 )
   {
      fail_msg( "defaultColors(1) should reset background to 0; got %d", color.background );
   }
   if ( color.postname != 6 || color.postfriendname != 1 || color.expressname != 2 )
   {
      fail_msg( "defaultColors(1) did not set post/express defaults as expected" );
   }
}

static void defaultColors_WhenClearAllDisabled_LeavesBackgroundUnchanged( void **state )
{
   // Arrange
   (void)state;

   resetState();
   memset( &color, 0, sizeof( color ) );
   color.text = -1;
   color.background = 4;

   // Act
   defaultColors( 0 );

   // Assert
   if ( color.text != 2 )
   {
      fail_msg( "defaultColors(0) should repair negative text color to default 2; got %d", color.text );
   }
   if ( color.background != 4 )
   {
      fail_msg( "defaultColors(0) should not overwrite existing background; got %d", color.background );
   }
}

static void colorValueFromName_WhenCanonicalNameProvided_ReturnsNamedPaletteValue( void **state )
{
   int colorValue;

   // Arrange
   (void)state;

   resetState();

   // Act
   colorValue = colorValueFromName( "green" );

   // Assert
   if ( colorValue != 34 )
   {
      fail_msg( "colorValueFromName should map green to palette value 34; got %d", colorValue );
   }
}

static void colorValueFromName_WhenAliasProvided_ReturnsCanonicalPaletteValue( void **state )
{
   int colorValue;

   // Arrange
   (void)state;

   resetState();

   // Act
   colorValue = colorValueFromName( "Purple" );

   // Assert
   if ( colorValue != 91 )
   {
      fail_msg( "colorValueFromName should map Purple to palette value 91; got %d", colorValue );
   }
}

static void colorValueFromName_WhenBrightAnsiNameProvided_ReturnsBrightAnsiValue( void **state )
{
   int colorValue;

   // Arrange
   (void)state;

   resetState();

   // Act
   colorValue = colorValueFromName( "BrightBlue" );

   // Assert
   if ( colorValue != 12 )
   {
      fail_msg( "colorValueFromName should map BrightBlue to ANSI 16 value 12; got %d", colorValue );
   }
}

static void colorValueFromName_WhenNameUnknown_ReturnsInvalidSentinel( void **state )
{
   int colorValue;

   // Arrange
   (void)state;

   resetState();

   // Act
   colorValue = colorValueFromName( "chartreuse" );

   // Assert
   if ( colorValue != -1 )
   {
      fail_msg( "colorValueFromName should reject unknown names with -1; got %d", colorValue );
   }
}

static void colorNameFromValue_WhenPaletteValueMatchesAlias_ReturnsCanonicalName( void **state )
{
   const char *ptrColorName;

   // Arrange
   (void)state;

   resetState();

   // Act
   ptrColorName = colorNameFromValue( 91 );

   // Assert
   if ( ptrColorName == NULL || strcmp( ptrColorName, "magenta" ) != 0 )
   {
      fail_msg( "colorNameFromValue should return canonical name 'magenta' for value 91; got '%s'",
                ptrColorName == NULL ? "(null)" : ptrColorName );
   }
}

static void colorNameFromValue_WhenBrightAnsiValueProvided_ReturnsBrightCanonicalName( void **state )
{
   const char *ptrColorName;

   // Arrange
   (void)state;

   resetState();

   // Act
   ptrColorName = colorNameFromValue( 13 );

   // Assert
   if ( ptrColorName == NULL || strcmp( ptrColorName, "brightmagenta" ) != 0 )
   {
      fail_msg( "colorNameFromValue should return canonical name 'brightmagenta' for value 13; got '%s'",
                ptrColorName == NULL ? "(null)" : ptrColorName );
   }
}

static void colorNameFromValue_WhenPaletteValueUnknown_ReturnsNull( void **state )
{
   const char *ptrColorName;

   // Arrange
   (void)state;

   resetState();

   // Act
   ptrColorName = colorNameFromValue( 999 );

   // Assert
   if ( ptrColorName != NULL )
   {
      fail_msg( "colorNameFromValue should return NULL for unknown values; got '%s'", ptrColorName );
   }
}

static void formatAnsiForegroundSequence_WhenClassicColorRequested_UsesClassicAnsiCode( void **state )
{
   char arySequence[32];

   // Arrange
   (void)state;

   resetState();

   // Act
   formatAnsiForegroundSequence( arySequence, sizeof( arySequence ), 2 );

   // Assert
   if ( strcmp( arySequence, "\033[32m" ) != 0 )
   {
      fail_msg( "formatAnsiForegroundSequence should encode classic green as '\\033[32m'; got '%s'",
                arySequence );
   }
}

static void formatAnsiForegroundSequence_WhenBrightColorRequested_UsesBrightAnsiCode( void **state )
{
   char arySequence[32];

   // Arrange
   (void)state;

   resetState();

   // Act
   formatAnsiForegroundSequence( arySequence, sizeof( arySequence ), 10 );

   // Assert
   if ( strcmp( arySequence, "\033[92m" ) != 0 )
   {
      fail_msg( "formatAnsiForegroundSequence should encode bright green as '\\033[92m'; got '%s'",
                arySequence );
   }
}

static void formatAnsiForegroundSequence_WhenExtendedColorRequested_Uses256ColorCode( void **state )
{
   char arySequence[32];

   // Arrange
   (void)state;

   resetState();

   // Act
   formatAnsiForegroundSequence( arySequence, sizeof( arySequence ), 34 );

   // Assert
   if ( strcmp( arySequence, "\033[38;5;34m" ) != 0 )
   {
      fail_msg( "formatAnsiForegroundSequence should encode extended palette value 34 as '\\033[38;5;34m'; got '%s'",
                arySequence );
   }
}

static void formatAnsiDisplayStateSequence_WhenDefaultBackgroundRequested_UsesCombinedSelectors( void **state )
{
   char arySequence[32];

   // Arrange
   (void)state;

   resetState();

   // Act
   formatAnsiDisplayStateSequence( arySequence, sizeof( arySequence ), 7,
                                   COLOR_VALUE_DEFAULT, false );

   // Assert
   if ( strcmp( arySequence, "\033[0;37;49m" ) != 0 )
   {
      fail_msg( "formatAnsiDisplayStateSequence should encode white-on-default without bold as '\\033[0;37;49m'; got '%s'",
                arySequence );
   }
}

static void colorblindColors_WhenApplied_SetsAccessiblePalette( void **state )
{
   // Arrange
   (void)state;

   resetState();
   memset( &color, 0, sizeof( color ) );

   // Act
   colorblindColors();

   // Assert
   if ( color.text != 231 || color.forum != 75 || color.number != 214 ||
        color.errorTextColor != 166 )
   {
      fail_msg( "colorblindColors should set general accessible colors; got text=%d forum=%d number=%d error=%d",
                color.text, color.forum, color.number, color.errorTextColor );
   }
   if ( color.background != 16 )
   {
      fail_msg( "colorblindColors should keep a dark background; got %d", color.background );
   }
   if ( color.postdate != 75 || color.postfrienddate != 25 ||
        color.postname != 214 || color.postfriendname != 175 ||
        color.input2 != 214 || color.moreprompt != 221 ||
        color.expressname != 214 || color.expressfriendname != 175 )
   {
      fail_msg( "colorblindColors should map post, input, and express roles onto the preset palette; got postdate=%d frienddate=%d postname=%d friendname=%d input2=%d moreprompt=%d expressname=%d expressfriendname=%d",
                color.postdate, color.postfrienddate, color.postname,
                color.postfriendname, color.input2, color.moreprompt,
                color.expressname, color.expressfriendname );
   }
}

static void brilliantColors_WhenApplied_SetsBrightDefaultPalette( void **state )
{
   // Arrange
   (void)state;

   resetState();
   memset( &color, 0, sizeof( color ) );

   // Act
   brilliantColors();

   // Assert
   if ( color.text != 10 || color.forum != 11 || color.number != 14 ||
        color.errorTextColor != 9 )
   {
      fail_msg( "brilliantColors should set bright general colors; got text=%d forum=%d number=%d error=%d",
                color.text, color.forum, color.number, color.errorTextColor );
   }
   if ( color.background != 0 )
   {
      fail_msg( "brilliantColors should keep a black background; got %d", color.background );
   }
   if ( color.postdate != 13 || color.postfrienddate != 13 ||
        color.postname != 14 || color.postfriendname != 9 ||
        color.posttext != 10 || color.postfriendtext != 10 ||
        color.anonymous != 11 || color.moreprompt != 11 ||
        color.input1 != 10 || color.input2 != 14 ||
        color.expresstext != 10 || color.expressname != 10 ||
        color.expressfriendname != 10 || color.expressfriendtext != 10 )
   {
      fail_msg( "brilliantColors should map the default roles onto bright ANSI values; got postdate=%d frienddate=%d postname=%d friendname=%d posttext=%d friendposttext=%d anonymous=%d moreprompt=%d input1=%d input2=%d expresstext=%d expressname=%d expressfriendname=%d expressfriendtext=%d",
                color.postdate, color.postfrienddate, color.postname,
                color.postfriendname, color.posttext, color.postfriendtext,
                color.anonymous, color.moreprompt, color.input1, color.input2,
                color.expresstext, color.expressname,
                color.expressfriendname, color.expressfriendtext );
   }
}

static void hotDogColors_WhenApplied_SetsClassicHotDogPalette( void **state )
{
   // Arrange
   (void)state;

   resetState();
   memset( &color, 0, sizeof( color ) );

   // Act
   hotDogColors();

   // Assert
   if ( color.text != 220 || color.forum != 196 || color.number != 220 ||
        color.errorTextColor != 231 )
   {
      fail_msg( "hotDogColors should set general hot dog colors; got text=%d forum=%d number=%d error=%d",
                color.text, color.forum, color.number, color.errorTextColor );
   }
   if ( color.background != 16 )
   {
      fail_msg( "hotDogColors should keep a black background; got %d", color.background );
   }
   if ( color.posttext != 214 || color.postfriendtext != 214 ||
        color.expresstext != 214 || color.expressfriendtext != 214 )
   {
      fail_msg( "hotDogColors should keep only post and eXpress bodies orange; got posttext=%d friendposttext=%d expresstext=%d friendexpresstext=%d",
                color.posttext, color.postfriendtext, color.expresstext,
                color.expressfriendtext );
   }

   if ( color.postdate != 226 || color.postfrienddate != 226 ||
        color.postname != 226 || color.postfriendname != 226 ||
        color.anonymous != 226 || color.moreprompt != 220 ||
        color.input1 != 220 || color.expressname != 226 ||
        color.expressfriendname != 226 )
   {
      fail_msg( "hotDogColors should keep date and name headers yellow while leaving only bodies orange; got postdate=%d frienddate=%d postname=%d friendname=%d anonymous=%d moreprompt=%d input1=%d expressname=%d expressfriendname=%d",
                color.postdate, color.postfrienddate, color.postname,
                color.postfriendname, color.anonymous, color.moreprompt,
                color.input1, color.expressname, color.expressfriendname );
   }
}

static void ansiTransformExpress_WhenFriendSender_UsesFriendColorCodes( void **state )
{
   // Arrange
   char aryMessage[256];

   (void)state;

   resetState();
   flagsConfiguration.useAnsi = 1;
   color.expressfriendtext = 3;
   color.expressfriendname = 5;
   color.text = 7;
   color.expresstext = 2;
   color.expressname = 6;
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
      fail_msg( "ansiTransformExpress should set lastColor to text color; got %d", lastColor );
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

static void ansiTransformPostHeader_WhenFriendPost_RewritesHeaderDigitsAndTracksColor( void **state )
{
   // Arrange
   char aryHeader[256];

   (void)state;

   resetState();
   color.postfrienddate = 4;
   color.postfriendname = 13;
   color.postfriendtext = 2;
   snprintf( aryHeader, sizeof( aryHeader ), "%s",
             "\033[35mMar 1, 2026 3:34 PM\033[32m from \033[36mSkankhunt Four Two\033[32m" );

   // Act
   ansiTransformPostHeader( aryHeader, sizeof( aryHeader ), 1 );

   // Assert
   if ( strstr( aryHeader, "\033[34mMar 1, 2026 3:34 PM" ) == NULL )
   {
      fail_msg( "ansiTransformPostHeader should remap friend post date color; got '%s'", aryHeader );
   }
   if ( strstr( aryHeader, "\033[95mSkankhunt Four Two" ) == NULL )
   {
      fail_msg( "ansiTransformPostHeader should remap friend post name color; got '%s'", aryHeader );
   }
   if ( lastColor != color.postfriendtext )
   {
      fail_msg( "ansiTransformPostHeader should set lastColor to friend post text color; got %d", lastColor );
   }
}

static void colorPicker_WhenInvalidThenValidInput_ReturnsMappedColorAndFlushes( void **state )
{
   // Arrange
   const int aryKeys[] = { 'z', 'x', 'R' };
   int result;

   (void)state;

   resetState();
   setInputSequence( aryKeys, sizeof( aryKeys ) / sizeof( aryKeys[0] ) );

   // Act
   result = colorPicker();

   // Assert
   if ( result != 1 )
   {
      fail_msg( "colorPicker should map 'R' to color code 1; got %d", result );
   }
   if ( flushCount != 1 || lastFlushValue != 2 )
   {
      fail_msg( "repeated invalid color picker input should flush once with incremented invalid count; got count=%u last=%u",
                flushCount, lastFlushValue );
   }
}

static void colorPicker_WhenBrightAnsiDigitSelected_ReturnsBrightAnsiValue( void **state )
{
   // Arrange
   const int aryKeys[] = { '6' };
   int result;

   (void)state;

   resetState();
   setInputSequence( aryKeys, sizeof( aryKeys ) / sizeof( aryKeys[0] ) );

   // Act
   result = colorPicker();

   // Assert
   if ( result != 13 )
   {
      fail_msg( "colorPicker should map '6' to bright magenta value 13; got %d", result );
   }
}

static void backgroundPicker_WhenDefaultSelected_ReturnsDefaultCode( void **state )
{
   // Arrange
   const int aryKeys[] = { 'x', 'x', 'D' };
   int result;

   (void)state;

   resetState();
   color.background = 2;
   setInputSequence( aryKeys, sizeof( aryKeys ) / sizeof( aryKeys[0] ) );

   // Act
   result = backgroundPicker();

   // Assert
   if ( result != COLOR_VALUE_DEFAULT )
   {
      fail_msg( "backgroundPicker should map 'D' to the default color sentinel; got %d", result );
   }
   if ( flushCount != 1 || lastFlushValue != 2 )
   {
      fail_msg( "repeated invalid background input should flush once with incremented invalid count; got count=%u last=%u",
                flushCount, lastFlushValue );
   }
}

static void backgroundPicker_WhenBrightAnsiDigitSelected_ReturnsBrightAnsiValue( void **state )
{
   // Arrange
   const int aryKeys[] = { '8' };
   int result;

   (void)state;

   resetState();
   color.background = 2;
   setInputSequence( aryKeys, sizeof( aryKeys ) / sizeof( aryKeys[0] ) );

   // Act
   result = backgroundPicker();

   // Assert
   if ( result != 15 )
   {
      fail_msg( "backgroundPicker should map '8' to bright white value 15; got %d", result );
   }
}

int main( void )
{
   const struct CMUnitTest aryTests[] = {
      cmocka_unit_test( defaultColors_WhenClearAllApplied_SetsKnownDefaults ),
      cmocka_unit_test( defaultColors_WhenClearAllDisabled_LeavesBackgroundUnchanged ),
      cmocka_unit_test( colorValueFromName_WhenCanonicalNameProvided_ReturnsNamedPaletteValue ),
      cmocka_unit_test( colorValueFromName_WhenAliasProvided_ReturnsCanonicalPaletteValue ),
      cmocka_unit_test( colorValueFromName_WhenBrightAnsiNameProvided_ReturnsBrightAnsiValue ),
      cmocka_unit_test( colorValueFromName_WhenNameUnknown_ReturnsInvalidSentinel ),
      cmocka_unit_test( colorNameFromValue_WhenPaletteValueMatchesAlias_ReturnsCanonicalName ),
      cmocka_unit_test( colorNameFromValue_WhenBrightAnsiValueProvided_ReturnsBrightCanonicalName ),
      cmocka_unit_test( colorNameFromValue_WhenPaletteValueUnknown_ReturnsNull ),
      cmocka_unit_test( formatAnsiForegroundSequence_WhenClassicColorRequested_UsesClassicAnsiCode ),
      cmocka_unit_test( formatAnsiForegroundSequence_WhenBrightColorRequested_UsesBrightAnsiCode ),
      cmocka_unit_test( formatAnsiForegroundSequence_WhenExtendedColorRequested_Uses256ColorCode ),
      cmocka_unit_test( formatAnsiDisplayStateSequence_WhenDefaultBackgroundRequested_UsesCombinedSelectors ),
      cmocka_unit_test( brilliantColors_WhenApplied_SetsBrightDefaultPalette ),
      cmocka_unit_test( colorblindColors_WhenApplied_SetsAccessiblePalette ),
      cmocka_unit_test( hotDogColors_WhenApplied_SetsClassicHotDogPalette ),
      cmocka_unit_test( ansiTransformExpress_WhenFriendSender_UsesFriendColorCodes ),
      cmocka_unit_test( ansiTransformExpress_WhenAnsiDisabled_LeavesTextUnchanged ),
      cmocka_unit_test( ansiTransformPostHeader_WhenFriendPost_RewritesHeaderDigitsAndTracksColor ),
      cmocka_unit_test( colorPicker_WhenInvalidThenValidInput_ReturnsMappedColorAndFlushes ),
      cmocka_unit_test( colorPicker_WhenBrightAnsiDigitSelected_ReturnsBrightAnsiValue ),
      cmocka_unit_test( backgroundPicker_WhenDefaultSelected_ReturnsDefaultCode ),
      cmocka_unit_test( backgroundPicker_WhenBrightAnsiDigitSelected_ReturnsBrightAnsiValue ),
   };

   return cmocka_run_group_tests( aryTests, NULL, NULL );
}
