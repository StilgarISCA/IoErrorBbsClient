/*
 * Copyright (C) 2024-2026 Stilgar
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>

#include <sys/stat.h>
#include <unistd.h>

#include "defs.h"
#include "ext.h"
#include "proto.h"
#include "test_helpers.h"

static int setupCallCount;
static int setupVersionArg;
static int perrorCallCount;
static char aryLastPerrorMessage[128];
static char aryLastPerrorHeading[64];
static char aryStdPrintfLog[16384];

static void resetTracking( void )
{
   setupCallCount = 0;
   setupVersionArg = 0;
   perrorCallCount = 0;
   aryLastPerrorMessage[0] = '\0';
   aryLastPerrorHeading[0] = '\0';
   aryStdPrintfLog[0] = '\0';
}

static void cleanupReadState( void )
{
   if ( ptrBbsRc != NULL )
   {
      fclose( ptrBbsRc );
      ptrBbsRc = NULL;
   }
   if ( bbsFriends != NULL )
   {
      fclose( bbsFriends );
      bbsFriends = NULL;
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
   if ( whoList != NULL )
   {
      slistDestroyItems( whoList );
      slistDestroy( whoList );
      whoList = NULL;
   }
   if ( xlandQueue != NULL )
   {
      deleteQueue( xlandQueue );
      xlandQueue = NULL;
   }
   if ( urlQueue != NULL )
   {
      deleteQueue( urlQueue );
      urlQueue = NULL;
   }
}

/* bbsrc.c dependencies that are not under test here. */
void configBbsRc( void )
{
}

void defaultColors( int setall )
{
   (void)setall;
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

void fatalExit( const char *message, const char *heading )
{
   fail_msg( "fatalExit invoked unexpectedly: %s (%s)", message, heading );
   abort();
}

char *findChar( const char *ptrString, int targetChar )
{
   const char *ptrSearch;

   for ( ptrSearch = ptrString; *ptrSearch && *ptrSearch != targetChar; ++ptrSearch )
   {
      ;
   }
   if ( *ptrSearch == targetChar )
   {
      return (char *)ptrSearch;
   }
   return NULL;
}

FILE *findBbsRc( void )
{
   return openBbsRc();
}

FILE *findBbsFriends( void )
{
   return NULL;
}

void resetTerm( void )
{
}

void setTerm( void )
{
}

void setup( int oldversion )
{
   setupCallCount++;
   setupVersionArg = oldversion;
}

void sPerror( const char *message, const char *heading )
{
   perrorCallCount++;
   snprintf( aryLastPerrorMessage, sizeof( aryLastPerrorMessage ), "%s", message );
   snprintf( aryLastPerrorHeading, sizeof( aryLastPerrorHeading ), "%s", heading );
}

int sortCompareVoid( const void *ptrLeft, const void *ptrRight )
{
   const char *const *ptrLeftString;
   const char *const *ptrRightString;

   ptrLeftString = ptrLeft;
   ptrRightString = ptrRight;
   return strcmp( *ptrLeftString, *ptrRightString );
}

int strCompareVoid( const void *ptrLeft, const void *ptrRight )
{
   return strcmp( (const char *)ptrLeft, (const char *)ptrRight );
}

int stdPrintf( const char *format, ... )
{
   va_list argList;
   char aryBuffer[512];
   size_t currentLength;

   va_start( argList, format );
   vsnprintf( aryBuffer, sizeof( aryBuffer ), format, argList );
   va_end( argList );

   currentLength = strlen( aryStdPrintfLog );
   if ( currentLength < sizeof( aryStdPrintfLog ) - 1 )
   {
      snprintf( aryStdPrintfLog + currentLength,
                sizeof( aryStdPrintfLog ) - currentLength,
                "%s",
                aryBuffer );
   }
   return 0;
}

static void openBbsRc_WhenPathMissing_CreatesWritableConfigurationFile( void **state )
{
   // Arrange
   char aryPath[PATH_MAX];
   FILE *ptrFile;
   struct stat fileStats;

   (void)state;

   resetTracking();
   isBbsRcReadOnly = 0;
   if ( !tryCreateTempPath( aryPath, sizeof( aryPath ), "/tmp/iobbsrc_test_XXXXXX" ) )
   {
      fail_msg( "Arrange failed: unable to create temporary path for openBbsRc missing-path test" );
      return;
   }
   snprintf( aryBbsRcName, sizeof( aryBbsRcName ), "%s", aryPath );

   // Act
   ptrFile = openBbsRc();

   // Assert
   if ( ptrFile == NULL )
   {
      fail_msg( "openBbsRc should create and open a missing configuration file" );
   }
   if ( isBbsRcReadOnly != 0 )
   {
      fail_msg( "openBbsRc should not mark a newly created file as read-only" );
   }
   if ( perrorCallCount != 0 )
   {
      fail_msg( "openBbsRc should not call sPerror on successful create/open; got %d calls", perrorCallCount );
   }
   if ( stat( aryPath, &fileStats ) != 0 )
   {
      fail_msg( "openBbsRc should create the configuration file on disk" );
   }

   fclose( ptrFile );
   unlink( aryPath );
}

static void openBbsRc_WhenPathIsReadOnly_SetsReadOnlyAndWarns( void **state )
{
   // Arrange
   char aryPath[PATH_MAX];
   FILE *ptrFile;

   (void)state;

   resetTracking();
   isBbsRcReadOnly = 0;
   if ( !tryCreateTempPath( aryPath, sizeof( aryPath ), "/tmp/iobbsrc_test_XXXXXX" ) )
   {
      fail_msg( "Arrange failed: unable to create temporary path for openBbsRc read-only test" );
      return;
   }
   if ( !tryWriteFileContents( aryPath, "version 2310\n" ) )
   {
      unlink( aryPath );
      fail_msg( "Arrange failed: unable to write configuration file for openBbsRc read-only test" );
      return;
   }
   chmod( aryPath, S_IRUSR | S_IRGRP | S_IROTH );
   snprintf( aryBbsRcName, sizeof( aryBbsRcName ), "%s", aryPath );

   // Act
   ptrFile = openBbsRc();

   // Assert
   if ( ptrFile == NULL )
   {
      fail_msg( "openBbsRc should fall back to read mode when file is read-only" );
   }
   if ( isBbsRcReadOnly != 1 )
   {
      fail_msg( "openBbsRc should set isBbsRcReadOnly when only read access is available" );
   }
   if ( perrorCallCount != 1 )
   {
      fail_msg( "openBbsRc should issue one warning for read-only configuration; got %d", perrorCallCount );
   }
   if ( strcmp( aryLastPerrorMessage, "Configuration is read-only" ) != 0 )
   {
      fail_msg( "openBbsRc warning text mismatch; got '%s'", aryLastPerrorMessage );
   }

   fclose( ptrFile );
   chmod( aryPath, S_IRUSR | S_IWUSR );
   unlink( aryPath );
}

static void readBbsRc_WhenConfigIsEmpty_AppliesDefaults( void **state )
{
   // Arrange
   char aryPath[PATH_MAX];

   (void)state;

   cleanupReadState();
   resetTracking();
   if ( !tryCreateTempPath( aryPath, sizeof( aryPath ), "/tmp/iobbsrc_test_XXXXXX" ) )
   {
      fail_msg( "Arrange failed: unable to create temporary path for readBbsRc empty-config test" );
      return;
   }
   if ( !tryWriteFileContents( aryPath, "" ) )
   {
      unlink( aryPath );
      fail_msg( "Arrange failed: unable to write empty configuration for readBbsRc empty-config test" );
      return;
   }
   snprintf( aryBbsRcName, sizeof( aryBbsRcName ), "%s", aryPath );
   snprintf( aryMyEditor, sizeof( aryMyEditor ), "%s", "nano" );
   isLoginShell = 0;
   isBbsRcReadOnly = 0;

   // Act
   readBbsRc();

   // Assert
   if ( commandKey != ESC )
   {
      fail_msg( "default commandKey should be ESC; got %d", commandKey );
   }
   if ( awayKey != 'a' || quitKey != CTRL_D || suspKey != CTRL_Z || captureKey != 'c' || shellKey != '!' || browserKey != 'w' )
   {
      fail_msg( "default key bindings were not applied as expected" );
   }
   if ( strcmp( aryBbsHost, BBS_HOSTNAME ) != 0 || bbsPort != BBS_PORT_NUMBER )
   {
      fail_msg( "default site should be %s:%d, got %s:%u", BBS_HOSTNAME, BBS_PORT_NUMBER, aryBbsHost, bbsPort );
   }
   if ( strcmp( aryEditor, "nano" ) != 0 )
   {
      fail_msg( "default editor should come from aryMyEditor; got '%s'", aryEditor );
   }
   if ( strcmp( aryAwayMessageLines[0], "I'm away from my keyboard right now." ) != 0 )
   {
      fail_msg( "default away message mismatch; got '%s'", aryAwayMessageLines[0] );
   }
   if ( setupCallCount != 1 || setupVersionArg != -1 )
   {
      fail_msg( "empty config should trigger setup(-1); got count=%d arg=%d", setupCallCount, setupVersionArg );
   }

   cleanupReadState();
   unlink( aryPath );
}

static void readBbsRc_WhenConfigHasEntries_ParsesValuesAndIgnoresDuplicateEnemy( void **state )
{
   // Arrange
   char aryPath[PATH_MAX];
   const friend *ptrFriendEntry;

   (void)state;

   cleanupReadState();
   resetTracking();
   if ( !tryCreateTempPath( aryPath, sizeof( aryPath ), "/tmp/iobbsrc_test_XXXXXX" ) )
   {
      fail_msg( "Arrange failed: unable to create temporary path for readBbsRc parse-config test" );
      return;
   }
   if ( !tryWriteFileContents(
           aryPath,
           "site bbs.example.net 2323\n"
           "commandkey ^[\n"
           "quit ^D\n"
           "susp ^Z\n"
           "capture c\n"
           "awaykey a\n"
           "aryShell !\n"
           "aryBrowser 0 lynx\n"
           "aryEditor nano\n"
           "a1 Back in five.\n"
           "enemy Meatball\n"
           "enemy Meatball\n"
           "friend Dr Strange           Time traveler\n"
           "version 2310\n"
           "xland\n" ) )
   {
      unlink( aryPath );
      fail_msg( "Arrange failed: unable to write configuration content for readBbsRc parse-config test" );
      return;
   }
   snprintf( aryBbsRcName, sizeof( aryBbsRcName ), "%s", aryPath );
   snprintf( aryMyEditor, sizeof( aryMyEditor ), "%s", "vi" );
   isLoginShell = 0;
   isBbsRcReadOnly = 0;

   // Act
   readBbsRc();

   // Assert
   if ( strcmp( aryBbsHost, "bbs.example.net" ) != 0 || bbsPort != 2323 )
   {
      fail_msg( "site parsing failed; expected bbs.example.net:2323, got %s:%u", aryBbsHost, bbsPort );
   }
   if ( commandKey != ESC || quitKey != CTRL_D || suspKey != CTRL_Z || captureKey != 'c' || awayKey != 'a' || shellKey != '!' )
   {
      fail_msg( "configured key bindings were not parsed correctly" );
   }
   if ( isXland != 0 )
   {
      fail_msg( "xland directive should disable xland mode; got isXland=%d", isXland );
   }
   if ( strcmp( aryEditor, "nano" ) != 0 )
   {
      fail_msg( "aryEditor parsing failed; expected 'nano', got '%s'", aryEditor );
   }
   if ( strcmp( aryBrowser, "lynx" ) != 0 || flagsConfiguration.shouldRunBrowserInBackground != 0 )
   {
      fail_msg( "aryBrowser parsing failed; expected foreground browser 'lynx'" );
   }
   if ( strcmp( aryAwayMessageLines[0], "Back in five." ) != 0 )
   {
      fail_msg( "away message parsing failed; got '%s'", aryAwayMessageLines[0] );
   }
   if ( enemyList == NULL || enemyList->nitems != 1 )
   {
      fail_msg( "duplicate enemy entries should be ignored; expected 1 enemy, got %u",
                enemyList == NULL ? 0u : enemyList->nitems );
   }
   if ( friendList == NULL || friendList->nitems != 1 || whoList == NULL || whoList->nitems != 1 )
   {
      fail_msg( "friend parsing/copy failed; expected 1 friend and 1 who entry" );
   }
   ptrFriendEntry = friendList->items[0];
   if ( strcmp( ptrFriendEntry->name, "Dr Strange" ) != 0 )
   {
      fail_msg( "friend name parsing failed; expected 'Dr Strange', got '%s'", ptrFriendEntry->name );
   }
   if ( setupCallCount != 0 )
   {
      fail_msg( "version-matched config should not call setup(); got %d calls", setupCallCount );
   }

   cleanupReadState();
   unlink( aryPath );
}

static void readBbsRc_WhenConfigContainsInvalidDirective_PrintsSyntaxError( void **state )
{
   // Arrange
   char aryPath[PATH_MAX];

   (void)state;

   cleanupReadState();
   resetTracking();
   if ( !tryCreateTempPath( aryPath, sizeof( aryPath ), "/tmp/iobbsrc_test_XXXXXX" ) )
   {
      fail_msg( "Arrange failed: unable to create temporary path for invalid-directive test" );
      return;
   }
   if ( !tryWriteFileContents(
           aryPath,
           "site bbs.example.net 2323\n"
           "mysterysetting 42\n"
           "quit ^D\n"
           "version 2310\n" ) )
   {
      unlink( aryPath );
      fail_msg( "Arrange failed: unable to write configuration content for invalid-directive test" );
      return;
   }
   snprintf( aryBbsRcName, sizeof( aryBbsRcName ), "%s", aryPath );
   snprintf( aryMyEditor, sizeof( aryMyEditor ), "%s", "nano" );
   isLoginShell = 0;
   isBbsRcReadOnly = 0;

   // Act
   readBbsRc();

   // Assert
   if ( strstr( aryStdPrintfLog, "Syntax error in .bbsrc file in line 2." ) == NULL )
   {
      fail_msg( "invalid directive should emit syntax error with correct line number; log was: %s", aryStdPrintfLog );
   }
   if ( quitKey != CTRL_D )
   {
      fail_msg( "parser should continue after bad line; expected quit key Ctrl-D, got %d", quitKey );
   }

   cleanupReadState();
   unlink( aryPath );
}

static void readBbsRc_WhenConfigContainsInvalidColor_PrintsWarning( void **state )
{
   // Arrange
   char aryPath[PATH_MAX];

   (void)state;

   cleanupReadState();
   resetTracking();
   if ( !tryCreateTempPath( aryPath, sizeof( aryPath ), "/tmp/iobbsrc_test_XXXXXX" ) )
   {
      fail_msg( "Arrange failed: unable to create temporary path for invalid-color test" );
      return;
   }
   if ( !tryWriteFileContents(
           aryPath,
           "color 12345\n"
           "version 2310\n" ) )
   {
      unlink( aryPath );
      fail_msg( "Arrange failed: unable to write configuration content for invalid-color test" );
      return;
   }
   snprintf( aryBbsRcName, sizeof( aryBbsRcName ), "%s", aryPath );
   snprintf( aryMyEditor, sizeof( aryMyEditor ), "%s", "nano" );
   isLoginShell = 0;
   isBbsRcReadOnly = 0;

   // Act
   readBbsRc();

   // Assert
   if ( strstr( aryStdPrintfLog, "Invalid 'color' scheme on line 1, ignored." ) == NULL )
   {
      fail_msg( "invalid color definition should emit warning; log was: %s", aryStdPrintfLog );
   }

   cleanupReadState();
   unlink( aryPath );
}

static void readBbsRc_WhenConfigFileMissing_CreatesFileAndUsesDefaults( void **state )
{
   // Arrange
   char aryPath[PATH_MAX];
   struct stat fileStats;

   (void)state;

   cleanupReadState();
   resetTracking();
   if ( !tryCreateTempPath( aryPath, sizeof( aryPath ), "/tmp/iobbsrc_test_XXXXXX" ) )
   {
      fail_msg( "Arrange failed: unable to create temporary path for missing-config test" );
      return;
   }
   snprintf( aryBbsRcName, sizeof( aryBbsRcName ), "%s", aryPath );
   snprintf( aryMyEditor, sizeof( aryMyEditor ), "%s", "nano" );
   isLoginShell = 0;
   isBbsRcReadOnly = 0;

   // Act
   readBbsRc();

   // Assert
   if ( stat( aryPath, &fileStats ) != 0 )
   {
      fail_msg( "readBbsRc should create configuration file when it does not exist" );
   }
   if ( setupCallCount != 1 || setupVersionArg != -1 )
   {
      fail_msg( "missing config should trigger setup(-1); got count=%d arg=%d", setupCallCount, setupVersionArg );
   }
   if ( strcmp( aryBbsHost, BBS_HOSTNAME ) != 0 || bbsPort != BBS_PORT_NUMBER )
   {
      fail_msg( "missing config should still apply default host/port" );
   }

   cleanupReadState();
   unlink( aryPath );
}

int main( void )
{
   const struct CMUnitTest aryTests[] = {
      cmocka_unit_test( openBbsRc_WhenPathMissing_CreatesWritableConfigurationFile ),
      cmocka_unit_test( openBbsRc_WhenPathIsReadOnly_SetsReadOnlyAndWarns ),
      cmocka_unit_test( readBbsRc_WhenConfigIsEmpty_AppliesDefaults ),
      cmocka_unit_test( readBbsRc_WhenConfigHasEntries_ParsesValuesAndIgnoresDuplicateEnemy ),
      cmocka_unit_test( readBbsRc_WhenConfigContainsInvalidDirective_PrintsSyntaxError ),
      cmocka_unit_test( readBbsRc_WhenConfigContainsInvalidColor_PrintsWarning ),
      cmocka_unit_test( readBbsRc_WhenConfigFileMissing_CreatesFileAndUsesDefaults ),
   };

   return cmocka_run_group_tests( aryTests, NULL, NULL );
}
