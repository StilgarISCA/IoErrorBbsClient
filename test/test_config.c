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

static int aryGetKeyQueue[128];
static size_t getKeyCount;
static size_t getKeyIndex;

static int aryYesNoQueue[32];
static size_t yesNoCount;
static size_t yesNoIndex;

static const char *aryStringQueue[32];
static size_t stringCount;
static size_t stringIndex;
static int getStringCallCount;

static char aryStdPrintfLog[4096];

static void resetState( void )
{
   getKeyCount = 0;
   getKeyIndex = 0;
   yesNoCount = 0;
   yesNoIndex = 0;
   stringCount = 0;
   stringIndex = 0;
   getStringCallCount = 0;
   aryStdPrintfLog[0] = '\0';
}

static void setGetKeySequence( const int *aryValues, size_t valueCount )
{
   getKeyCount = copyIntArray( aryValues,
                               valueCount,
                               aryGetKeyQueue,
                               sizeof( aryGetKeyQueue ) / sizeof( aryGetKeyQueue[0] ) );
   getKeyIndex = 0;
}

static void setYesNoSequence( const int *aryValues, size_t valueCount )
{
   yesNoCount = copyIntArray( aryValues,
                              valueCount,
                              aryYesNoQueue,
                              sizeof( aryYesNoQueue ) / sizeof( aryYesNoQueue[0] ) );
   yesNoIndex = 0;
}

static void setStringSequence( const char **aryValues, size_t valueCount )
{
   stringCount = copyStringPointerArray( aryValues,
                                         valueCount,
                                         aryStringQueue,
                                         sizeof( aryStringQueue ) / sizeof( aryStringQueue[0] ) );
   stringIndex = 0;
}

/* config.c dependencies outside this test target's scope. */
int capPrintf( const char *format, ... )
{
   va_list argList;

   va_start( argList, format );
   va_end( argList );
   return 1;
}

int colorize( const char *ptrText )
{
   (void)ptrText;
   return 1;
}

int colorConfigCalled;
void colorConfig( void )
{
   colorConfigCalled++;
}

int deleteFile( const char *ptrPath )
{
   (void)ptrPath;
   return 0;
}

int fStrCompareVoid( const void *ptrName, const void *ptrFriend )
{
   const friend *ptrEntry;

   ptrEntry = ptrFriend;
   return strcmp( (const char *)ptrName, ptrEntry->name );
}

void fatalExit( const char *message, const char *heading )
{
   fail_msg( "fatalExit invoked unexpectedly: %s (%s)", message, heading );
}

char *findChar( const char *ptrString, int targetChar )
{
   return (char *)strchr( ptrString, targetChar );
}

void flushInput( unsigned int count )
{
   (void)count;
}

void handleInvalidInput( unsigned int *ptrInvalidCount )
{
   if ( ( *ptrInvalidCount )++ )
   {
      flushInput( *ptrInvalidCount );
   }
}

int getKey( void )
{
   if ( getKeyIndex < getKeyCount )
   {
      return aryGetKeyQueue[getKeyIndex++];
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
      inputChar = getKey();
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

char *getName( int quitPriv )
{
   (void)quitPriv;
   return "";
}

void getString( int length, char *result, int line )
{
   const char *ptrSource;

   (void)length;
   (void)line;
   getStringCallCount++;

   ptrSource = "";
   if ( stringIndex < stringCount )
   {
      ptrSource = aryStringQueue[stringIndex++];
   }
   snprintf( result, 80, "%s", ptrSource );
}

int inKey( void )
{
   return '\n';
}

void information( void )
{
}

int more( int *line, int percent )
{
   (void)line;
   (void)percent;
   return 0;
}

void myExit( void )
{
   fail_msg( "myExit invoked unexpectedly during config unit tests" );
}

void resetTerm( void )
{
}

void setTerm( void )
{
}

void sInfo( const char *message, const char *heading )
{
   (void)message;
   (void)heading;
}

int sPrompt( const char *message, const char *heading, int defaultAnswer )
{
   (void)message;
   (void)heading;
   (void)defaultAnswer;
   return 0;
}

int stdPrintf( const char *format, ... )
{
   va_list argList;
   char aryBuffer[512];
   size_t logLength;

   va_start( argList, format );
   vsnprintf( aryBuffer, sizeof( aryBuffer ), format, argList );
   va_end( argList );

   logLength = strlen( aryStdPrintfLog );
   if ( logLength < sizeof( aryStdPrintfLog ) - 1 )
   {
      snprintf( aryStdPrintfLog + logLength, sizeof( aryStdPrintfLog ) - logLength, "%s", aryBuffer );
   }
   return 1;
}

int strCompareVoid( const void *ptrLeft, const void *ptrRight )
{
   return strcmp( (const char *)ptrLeft, (const char *)ptrRight );
}

void truncateBbsRc( long userNameLength )
{
   (void)userNameLength;
}

int yesNo( void )
{
   if ( yesNoIndex < yesNoCount )
   {
      return aryYesNoQueue[yesNoIndex++];
   }
   return 0;
}

int yesNoDefault( int defaultAnswer )
{
   return defaultAnswer;
}

static void strCtrl_WhenControlCharacter_ReturnsCaretNotation( void **state )
{
   // Arrange
   char *ptrResult;

   (void)state;
   resetState();

   // Act
   ptrResult = strCtrl( CTRL_D );

   // Assert
   if ( strcmp( ptrResult, "^D" ) != 0 )
   {
      fail_msg( "strCtrl should map CTRL_D to '^D'; got '%s'", ptrResult );
   }
}

static void strCtrl_WhenPrintableCharacter_ReturnsSameCharacter( void **state )
{
   // Arrange
   char *ptrResult;

   (void)state;
   resetState();

   // Act
   ptrResult = strCtrl( 'Q' );

   // Assert
   if ( strcmp( ptrResult, "Q" ) != 0 )
   {
      fail_msg( "strCtrl should preserve printable characters; got '%s'", ptrResult );
   }
}

static void newKey_WhenConflictingKeyChosen_RetriesUntilAvailable( void **state )
{
   // Arrange
   const int aryKeys[] = { 'k', 'm' };
   int result;

   (void)state;
   resetState();

   commandKey = 'k';
   quitKey = CTRL_D;
   suspKey = CTRL_Z;
   shellKey = '!';
   captureKey = 'c';
   awayKey = 'a';
   browserKey = 'w';

   setGetKeySequence( aryKeys, sizeof( aryKeys ) / sizeof( aryKeys[0] ) );

   // Act
   result = newKey( 'q' );

   // Assert
   if ( result != 'm' )
   {
      fail_msg( "newKey should reject conflicting key and return next valid key; got %d", result );
   }
   if ( strstr( aryStdPrintfLog, "already in use for another hotkey" ) == NULL )
   {
      fail_msg( "newKey should print conflict guidance message when key is already used" );
   }
}

static void newKey_WhenUserEntersSpace_ReturnsOldKey( void **state )
{
   // Arrange
   const int aryKeys[] = { ' ' };
   int result;

   (void)state;
   resetState();
   setGetKeySequence( aryKeys, sizeof( aryKeys ) / sizeof( aryKeys[0] ) );

   // Act
   result = newKey( 'q' );

   // Assert
   if ( result != 'q' )
   {
      fail_msg( "newKey should keep old key on space/newline default; got %d", result );
   }
}

static void newAwayMessage_WhenUserDeclinesChange_PreservesExistingMessage( void **state )
{
   // Arrange
   const int aryAnswers[] = { 0 };

   (void)state;
   resetState();
   snprintf( aryAwayMessageLines[0], sizeof( aryAwayMessageLines[0] ), "%s", "Current away text." );
   aryAwayMessageLines[1][0] = '\0';
   setYesNoSequence( aryAnswers, sizeof( aryAnswers ) / sizeof( aryAnswers[0] ) );

   // Act
   newAwayMessage();

   // Assert
   if ( strcmp( aryAwayMessageLines[0], "Current away text." ) != 0 )
   {
      fail_msg( "newAwayMessage should preserve text when user declines; got '%s'", aryAwayMessageLines[0] );
   }
   if ( getStringCallCount != 0 )
   {
      fail_msg( "newAwayMessage should not prompt for new lines when user declines; getString calls=%d", getStringCallCount );
   }
}

static void newAwayMessage_WhenUserAcceptsChange_ReplacesWithEnteredLines( void **state )
{
   // Arrange
   const int aryAnswers[] = { 1 };
   const char *aryLines[] = { "Gone to lunch.", "Back by 2pm.", "" };

   (void)state;
   resetState();
   snprintf( aryAwayMessageLines[0], sizeof( aryAwayMessageLines[0] ), "%s", "Old away text." );
   aryAwayMessageLines[1][0] = '\0';
   setYesNoSequence( aryAnswers, sizeof( aryAnswers ) / sizeof( aryAnswers[0] ) );
   setStringSequence( aryLines, sizeof( aryLines ) / sizeof( aryLines[0] ) );

   // Act
   newAwayMessage();

   // Assert
   if ( strcmp( aryAwayMessageLines[0], "Gone to lunch." ) != 0 || strcmp( aryAwayMessageLines[1], "Back by 2pm." ) != 0 )
   {
      fail_msg( "newAwayMessage should save entered lines; got ['%s', '%s']", aryAwayMessageLines[0], aryAwayMessageLines[1] );
   }
   if ( aryAwayMessageLines[2][0] != '\0' )
   {
      fail_msg( "newAwayMessage should stop at blank line and leave trailing lines empty" );
   }
   if ( getStringCallCount != 3 )
   {
      fail_msg( "newAwayMessage should call getString until blank line; expected 3 calls, got %d", getStringCallCount );
   }
}

int main( void )
{
   const struct CMUnitTest aryTests[] = {
      cmocka_unit_test( strCtrl_WhenControlCharacter_ReturnsCaretNotation ),
      cmocka_unit_test( strCtrl_WhenPrintableCharacter_ReturnsSameCharacter ),
      cmocka_unit_test( newKey_WhenConflictingKeyChosen_RetriesUntilAvailable ),
      cmocka_unit_test( newKey_WhenUserEntersSpace_ReturnsOldKey ),
      cmocka_unit_test( newAwayMessage_WhenUserDeclinesChange_PreservesExistingMessage ),
      cmocka_unit_test( newAwayMessage_WhenUserAcceptsChange_ReplacesWithEnteredLines ),
   };

   return cmocka_run_group_tests( aryTests, NULL, NULL );
}
