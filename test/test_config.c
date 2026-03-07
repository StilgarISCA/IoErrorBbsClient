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

static int aryPromptQueue[32];
static size_t promptCount;
static size_t promptIndex;

static const char *aryStringQueue[32];
static size_t stringCount;
static size_t stringIndex;
static int getStringCallCount;
static int sPromptCallCount;

static char aryStdPrintfLog[4096];

static void cleanupWriteBbsRcFixture( void )
{
   if ( ptrBbsRc != NULL )
   {
      fclose( ptrBbsRc );
      ptrBbsRc = NULL;
   }
   if ( friendList != NULL )
   {
      slistDestroyItems( friendList );
      slistDestroy( friendList );
      friendList = NULL;
   }
   if ( enemyList != NULL )
   {
      slistDestroyItems( enemyList );
      slistDestroy( enemyList );
      enemyList = NULL;
   }
}

static void resetState( void )
{
   getKeyCount = 0;
   getKeyIndex = 0;
   yesNoCount = 0;
   yesNoIndex = 0;
   promptCount = 0;
   promptIndex = 0;
   stringCount = 0;
   stringIndex = 0;
   getStringCallCount = 0;
   sPromptCallCount = 0;
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

static void setPromptSequence( const int *aryValues, size_t valueCount )
{
   promptCount = copyIntArray( aryValues,
                               valueCount,
                               aryPromptQueue,
                               sizeof( aryPromptQueue ) / sizeof( aryPromptQueue[0] ) );
   promptIndex = 0;
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

void printAnsiForegroundColorValue( int colorValue )
{
   (void)colorValue;
}

void printThemedMnemonicText( const char *ptrText, int defaultColor )
{
   (void)ptrText;
   (void)defaultColor;
}

int colorValueToLegacyDigit( int colorValue )
{
   return colorValue + '0';
}

const char *colorNameFromValue( int colorValue )
{
   switch ( colorValue )
   {
      case 8:
         return "brightblack";
      case 9:
         return "brightred";
      case 10:
         return "brightgreen";
      case 11:
         return "brightyellow";
      case 12:
         return "brightblue";
      case 13:
         return "brightmagenta";
      case 14:
         return "brightcyan";
      case 15:
         return "brightwhite";
      case 16:
         return "black";
      case 160:
         return "red";
      case 34:
         return "green";
      case 220:
         return "yellow";
      case 26:
         return "blue";
      case 91:
         return "magenta";
      case 44:
         return "cyan";
      case 231:
         return "white";
      case COLOR_VALUE_DEFAULT:
         return "default";
      default:
         return NULL;
   }
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

int fSortCompareVoid( const void *ptrLeft, const void *ptrRight )
{
   const friend *const *ptrLeftFriend;
   const friend *const *ptrRightFriend;

   ptrLeftFriend = ptrLeft;
   ptrRightFriend = ptrRight;
   return strcmp( ( *ptrLeftFriend )->name, ( *ptrRightFriend )->name );
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
   sPromptCallCount++;
   if ( promptIndex < promptCount )
   {
      return aryPromptQueue[promptIndex++];
   }
   return defaultAnswer;
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

int sortCompareVoid( const void *ptrLeft, const void *ptrRight )
{
   const char *const *ptrLeftString;
   const char *const *ptrRightString;

   ptrLeftString = ptrLeft;
   ptrRightString = ptrRight;
   return strcmp( *ptrLeftString, *ptrRightString );
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
   if ( yesNoIndex < yesNoCount )
   {
      return aryYesNoQueue[yesNoIndex++];
   }
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

static void setup_WhenScreenReaderModeIsUnset_PromptsAndStoresAnswer( void **state )
{
   // Arrange
   const int aryPromptAnswers[] = { 1, 0 };

   (void)state;
   resetState();

   flagsConfiguration.hasScreenReaderModeSetting = 0;
   flagsConfiguration.isScreenReaderModeEnabled = 0;
   setPromptSequence( aryPromptAnswers, sizeof( aryPromptAnswers ) / sizeof( aryPromptAnswers[0] ) );

   cleanupWriteBbsRcFixture();
   ptrBbsRc = tmpfile();
   friendList = slistCreate( 0, fSortCompareVoid );
   enemyList = slistCreate( 0, sortCompareVoid );
   if ( ptrBbsRc == NULL || friendList == NULL || enemyList == NULL )
   {
      cleanupWriteBbsRcFixture();
      fail_msg( "Arrange failed: unable to initialize setup fixture" );
      return;
   }

   snprintf( aryEditor, sizeof( aryEditor ), "%s", "nano" );
   snprintf( aryBbsHost, sizeof( aryBbsHost ), "%s", "bbs.example.net" );
   bbsPort = 23;
   commandKey = ESC;
   quitKey = CTRL_D;
   suspKey = CTRL_Z;
   shellKey = '!';
   captureKey = 'c';
   awayKey = 'a';

   // Act
   setup( INT_VERSION );

   // Assert
   if ( !flagsConfiguration.hasScreenReaderModeSetting )
   {
      cleanupWriteBbsRcFixture();
      fail_msg( "setup should mark the screen reader mode setting as present after prompting" );
      return;
   }
   if ( !flagsConfiguration.isScreenReaderModeEnabled )
   {
      cleanupWriteBbsRcFixture();
      fail_msg( "setup should store the user's screen reader mode choice when they answer yes" );
      return;
   }
   if ( !flagsConfiguration.hasNameAutocompleteSetting )
   {
      cleanupWriteBbsRcFixture();
      fail_msg( "setup should mark the autocomplete setting as present after choosing a screen reader mode" );
      return;
   }
   if ( flagsConfiguration.shouldEnableNameAutocomplete )
   {
      cleanupWriteBbsRcFixture();
      fail_msg( "setup should default autocomplete off when screen reader mode is enabled" );
      return;
   }
   if ( sPromptCallCount < 2 )
   {
      cleanupWriteBbsRcFixture();
      fail_msg( "setup should prompt for screen reader mode before asking about advanced options; got %d prompts",
                sPromptCallCount );
      return;
   }

   cleanupWriteBbsRcFixture();
}

static void configBbsRc_WhenOptionsToggleScreenReaderMode_UpdatesFlags( void **state )
{
   // Arrange
   const int aryMenuKeys[] = { 'o', 'q' };
   const int aryYesNoAnswers[] = { 1, 0, 1, 0 };

   (void)state;
   resetState();

   cleanupWriteBbsRcFixture();
   ptrBbsRc = tmpfile();
   friendList = slistCreate( 0, fSortCompareVoid );
   enemyList = slistCreate( 0, sortCompareVoid );
   if ( ptrBbsRc == NULL || friendList == NULL || enemyList == NULL )
   {
      cleanupWriteBbsRcFixture();
      fail_msg( "Arrange failed: unable to initialize configBbsRc fixture" );
      return;
   }

   snprintf( aryEditor, sizeof( aryEditor ), "%s", "nano" );
   snprintf( aryBbsHost, sizeof( aryBbsHost ), "%s", "bbs.example.net" );
   bbsPort = 23;
   commandKey = ESC;
   quitKey = CTRL_D;
   suspKey = CTRL_Z;
   shellKey = '!';
   captureKey = 'c';
   awayKey = 'a';
   browserKey = 'w';
   rows = 24;
   isLoginShell = 0;
   isBbsRcReadOnly = 0;
   flagsConfiguration.useAnsi = 0;
   flagsConfiguration.shouldUseTcpKeepalive = 1;
   flagsConfiguration.shouldEnableClickableUrls = 1;
   flagsConfiguration.isScreenReaderModeEnabled = 0;
   flagsConfiguration.hasScreenReaderModeSetting = 0;
   flagsConfiguration.shouldEnableNameAutocomplete = 1;
   flagsConfiguration.hasNameAutocompleteSetting = 0;

   setGetKeySequence( aryMenuKeys, sizeof( aryMenuKeys ) / sizeof( aryMenuKeys[0] ) );
   setYesNoSequence( aryYesNoAnswers, sizeof( aryYesNoAnswers ) / sizeof( aryYesNoAnswers[0] ) );

   // Act
   configBbsRc();

   // Assert
   if ( !flagsConfiguration.isScreenReaderModeEnabled )
   {
      cleanupWriteBbsRcFixture();
      fail_msg( "configBbsRc should enable screen reader mode when the option is answered yes" );
      return;
   }
   if ( !flagsConfiguration.hasScreenReaderModeSetting )
   {
      cleanupWriteBbsRcFixture();
      fail_msg( "configBbsRc should mark screen reader mode as configured after toggling it" );
      return;
   }
   if ( flagsConfiguration.shouldEnableClickableUrls )
   {
      cleanupWriteBbsRcFixture();
      fail_msg( "configBbsRc should seed clickable URL summaries to no when screen reader mode is enabled and the user accepts the default" );
      return;
   }
   if ( flagsConfiguration.shouldEnableNameAutocomplete )
   {
      cleanupWriteBbsRcFixture();
      fail_msg( "configBbsRc should seed autocomplete to no when screen reader mode is enabled and the user accepts the default" );
      return;
   }
   if ( !flagsConfiguration.hasNameAutocompleteSetting )
   {
      cleanupWriteBbsRcFixture();
      fail_msg( "configBbsRc should mark autocomplete as configured after toggling it" );
      return;
   }
   if ( strstr( aryStdPrintfLog, "Use screen reader friendly mode?" ) == NULL )
   {
      cleanupWriteBbsRcFixture();
      fail_msg( "configBbsRc should display the screen reader mode option in the Options menu" );
      return;
   }
   if ( strstr( aryStdPrintfLog,
                "Append OSC 8 URL summaries to posts & mail? (No) -> " ) == NULL )
   {
      cleanupWriteBbsRcFixture();
      fail_msg( "configBbsRc should show the screen reader default of No for clickable URL summaries after enabling screen reader mode" );
      return;
   }
   if ( strstr( aryStdPrintfLog, "Autocomplete username in recipient prompts?" ) == NULL )
   {
      cleanupWriteBbsRcFixture();
      fail_msg( "configBbsRc should display the autocomplete option in the Options menu" );
      return;
   }
   if ( strstr( aryStdPrintfLog,
                "Autocomplete username in recipient prompts? (No) -> " ) == NULL )
   {
      cleanupWriteBbsRcFixture();
      fail_msg( "configBbsRc should show the screen reader default of No for autocomplete after enabling screen reader mode" );
      return;
   }

   cleanupWriteBbsRcFixture();
}

static void writeBbsRc_WhenTcpKeepaliveEnabled_WritesKeepaliveOne( void **state )
{
   // Arrange
   char aryOutput[4096];

   (void)state;
   resetState();

   cleanupWriteBbsRcFixture();
   ptrBbsRc = tmpfile();
   friendList = slistCreate( 0, fSortCompareVoid );
   enemyList = slistCreate( 0, sortCompareVoid );
   if ( ptrBbsRc == NULL || friendList == NULL || enemyList == NULL )
   {
      cleanupWriteBbsRcFixture();
      fail_msg( "Arrange failed: unable to initialize writeBbsRc fixture" );
      return;
   }

   snprintf( aryEditor, sizeof( aryEditor ), "%s", "nano" );
   snprintf( aryBbsHost, sizeof( aryBbsHost ), "%s", "bbs.example.net" );
   bbsPort = 23;
   commandKey = ESC;
   quitKey = CTRL_D;
   suspKey = CTRL_Z;
   shellKey = '!';
   captureKey = 'c';
   awayKey = 'a';
   color.text = 10;
   color.forum = 11;
   color.number = 14;
   color.errorTextColor = 9;
   color.reserved1 = 8;
   color.reserved2 = 8;
   color.reserved3 = 8;
   color.postdate = 13;
   color.postname = 12;
   color.posttext = 15;
   color.postfrienddate = 9;
   color.postfriendname = 10;
   color.postfriendtext = 11;
   color.anonymous = 12;
   color.moreprompt = 14;
   color.reserved4 = 8;
   color.reserved5 = 8;
   color.background = COLOR_VALUE_DEFAULT;
   color.input1 = 15;
   color.input2 = 10;
   color.expresstext = 11;
   color.expressname = 13;
   color.expressfriendtext = 14;
   color.expressfriendname = 13;
   flagsConfiguration.shouldUseTcpKeepalive = true;
   flagsConfiguration.shouldEnableClickableUrls = true;
   flagsConfiguration.isScreenReaderModeEnabled = true;
   flagsConfiguration.shouldEnableNameAutocomplete = false;

   // Act
   writeBbsRc();

   // Assert
   if ( !tryReadFileIntoBuffer( ptrBbsRc, aryOutput, sizeof( aryOutput ) ) )
   {
      cleanupWriteBbsRcFixture();
      fail_msg( "Assert failed: unable to read generated .bbsrc output" );
      return;
   }
   if ( strstr( aryOutput, "\nkeepalive 1\n" ) == NULL )
   {
      cleanupWriteBbsRcFixture();
      fail_msg( "writeBbsRc should emit 'keepalive 1' when keepalive is enabled; output was:\n%s", aryOutput );
      return;
   }
   if ( strstr( aryOutput, "\nclickableurls 1\n" ) == NULL )
   {
      cleanupWriteBbsRcFixture();
      fail_msg( "writeBbsRc should emit 'clickableurls 1' when clickable URLs are enabled; output was:\n%s", aryOutput );
      return;
   }
   if ( strstr( aryOutput, "\nscreenreader 1\n" ) == NULL )
   {
      cleanupWriteBbsRcFixture();
      fail_msg( "writeBbsRc should emit 'screenreader 1' when screen reader mode is enabled; output was:\n%s", aryOutput );
      return;
   }
   if ( strstr( aryOutput, "\nautocomplete 0\n" ) == NULL )
   {
      cleanupWriteBbsRcFixture();
      fail_msg( "writeBbsRc should emit 'autocomplete 0' when autocomplete is disabled; output was:\n%s", aryOutput );
      return;
   }
   if ( strstr( aryOutput, "\ncolor brightgreen brightyellow brightcyan brightred brightblack brightblack brightblack brightmagenta brightblue brightwhite brightred brightgreen brightyellow brightblue brightcyan brightblack brightblack default brightwhite brightgreen brightyellow brightmagenta brightcyan brightmagenta\n" ) == NULL )
   {
      cleanupWriteBbsRcFixture();
      fail_msg( "writeBbsRc should emit bright ANSI color names when palette values have ANSI 16 names; output was:\n%s", aryOutput );
      return;
   }
   if ( strstr( aryOutput, "\naryBrowser " ) != NULL )
   {
      cleanupWriteBbsRcFixture();
      fail_msg( "writeBbsRc should not emit obsolete browser configuration; output was:\n%s", aryOutput );
      return;
   }

   cleanupWriteBbsRcFixture();
}

static void writeBbsRc_WhenTcpKeepaliveDisabled_WritesKeepaliveZero( void **state )
{
   // Arrange
   char aryOutput[4096];

   (void)state;
   resetState();

   cleanupWriteBbsRcFixture();
   ptrBbsRc = tmpfile();
   friendList = slistCreate( 0, fSortCompareVoid );
   enemyList = slistCreate( 0, sortCompareVoid );
   if ( ptrBbsRc == NULL || friendList == NULL || enemyList == NULL )
   {
      cleanupWriteBbsRcFixture();
      fail_msg( "Arrange failed: unable to initialize writeBbsRc fixture" );
      return;
   }

   snprintf( aryEditor, sizeof( aryEditor ), "%s", "nano" );
   snprintf( aryBbsHost, sizeof( aryBbsHost ), "%s", "bbs.example.net" );
   bbsPort = 23;
   commandKey = ESC;
   quitKey = CTRL_D;
   suspKey = CTRL_Z;
   shellKey = '!';
   captureKey = 'c';
   awayKey = 'a';
   color.text = 10;
   color.forum = 123;
   color.number = 14;
   color.errorTextColor = 9;
   color.reserved1 = 8;
   color.reserved2 = 8;
   color.reserved3 = 8;
   color.postdate = 13;
   color.postname = 12;
   color.posttext = 15;
   color.postfrienddate = 9;
   color.postfriendname = 10;
   color.postfriendtext = 11;
   color.anonymous = 12;
   color.moreprompt = 14;
   color.reserved4 = 8;
   color.reserved5 = 8;
   color.background = COLOR_VALUE_DEFAULT;
   color.input1 = 15;
   color.input2 = 10;
   color.expresstext = 11;
   color.expressname = 13;
   color.expressfriendtext = 14;
   color.expressfriendname = 13;
   flagsConfiguration.shouldUseTcpKeepalive = false;
   flagsConfiguration.shouldEnableClickableUrls = false;
   flagsConfiguration.isScreenReaderModeEnabled = false;
   flagsConfiguration.shouldEnableNameAutocomplete = true;

   // Act
   writeBbsRc();

   // Assert
   if ( !tryReadFileIntoBuffer( ptrBbsRc, aryOutput, sizeof( aryOutput ) ) )
   {
      cleanupWriteBbsRcFixture();
      fail_msg( "Assert failed: unable to read generated .bbsrc output" );
      return;
   }
   if ( strstr( aryOutput, "\nkeepalive 0\n" ) == NULL )
   {
      cleanupWriteBbsRcFixture();
      fail_msg( "writeBbsRc should emit 'keepalive 0' when keepalive is disabled; output was:\n%s", aryOutput );
      return;
   }
   if ( strstr( aryOutput, "\nclickableurls 0\n" ) == NULL )
   {
      cleanupWriteBbsRcFixture();
      fail_msg( "writeBbsRc should emit 'clickableurls 0' when clickable URLs are disabled; output was:\n%s", aryOutput );
      return;
   }
   if ( strstr( aryOutput, "\nscreenreader 0\n" ) == NULL )
   {
      cleanupWriteBbsRcFixture();
      fail_msg( "writeBbsRc should emit 'screenreader 0' when screen reader mode is disabled; output was:\n%s", aryOutput );
      return;
   }
   if ( strstr( aryOutput, "\nautocomplete 1\n" ) == NULL )
   {
      cleanupWriteBbsRcFixture();
      fail_msg( "writeBbsRc should emit 'autocomplete 1' when autocomplete is enabled; output was:\n%s", aryOutput );
      return;
   }
   if ( strstr( aryOutput, "\ncolor brightgreen 123 brightcyan brightred brightblack brightblack brightblack brightmagenta brightblue brightwhite brightred brightgreen brightyellow brightblue brightcyan brightblack brightblack default brightwhite brightgreen brightyellow brightmagenta brightcyan brightmagenta\n" ) == NULL )
   {
      cleanupWriteBbsRcFixture();
      fail_msg( "writeBbsRc should fall back to numeric palette values when no named color exists; output was:\n%s", aryOutput );
      return;
   }
   if ( strstr( aryOutput, "\naryBrowser " ) != NULL )
   {
      cleanupWriteBbsRcFixture();
      fail_msg( "writeBbsRc should not emit obsolete browser configuration; output was:\n%s", aryOutput );
      return;
   }

   cleanupWriteBbsRcFixture();
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
      cmocka_unit_test( setup_WhenScreenReaderModeIsUnset_PromptsAndStoresAnswer ),
      cmocka_unit_test( configBbsRc_WhenOptionsToggleScreenReaderMode_UpdatesFlags ),
      cmocka_unit_test( writeBbsRc_WhenTcpKeepaliveEnabled_WritesKeepaliveOne ),
      cmocka_unit_test( writeBbsRc_WhenTcpKeepaliveDisabled_WritesKeepaliveZero ),
   };

   return cmocka_run_group_tests( aryTests, NULL, NULL );
}
