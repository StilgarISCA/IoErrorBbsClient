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

static int aryInputQueue[16];
static size_t inputCount;
static size_t inputIndex;
static int arySentChars[512];
static size_t sentCharCount;

static void resetState( void )
{
   inputCount = 0;
   inputIndex = 0;
   sentCharCount = 0;
   flagsConfiguration.isLastSave = 0;
   flagsConfiguration.isPosting = 0;
}

static void setInputSequence( const int *aryKeys, size_t count )
{
   inputCount = copyIntArray( aryKeys, count, aryInputQueue,
                              sizeof( aryInputQueue ) / sizeof( aryInputQueue[0] ) );
   inputIndex = 0;
}

/* edit.c dependencies outside checkFile() scope for these tests. */
int colorize( const char *ptrText )
{
   (void)ptrText;
   return 1;
}

void continuedPostHelper( void )
{
}

void fatalPerror( const char *error, const char *heading )
{
   (void)error;
   (void)heading;
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

void getString( int length, char *result, int line )
{
   (void)length;
   (void)line;
   result[0] = '\0';
}

int inKey( void )
{
   if ( inputIndex < inputCount )
   {
      return aryInputQueue[inputIndex++];
   }
   return '\n';
}

int readFoldedKey( void )
{
   int inputChar;

   inputChar = inKey();
   if ( isalpha( inputChar ) )
   {
      inputChar = tolower( inputChar );
   }
   return inputChar;
}

void looper( void )
{
}

int more( int *line, int percentComplete )
{
   (void)line;
   (void)percentComplete;
   return 0;
}

void mySleep( unsigned int seconds )
{
   (void)seconds;
}

int netPutChar( int inputChar )
{
   return inputChar;
}

void sendTrackedChar( int inputChar )
{
   if ( sentCharCount < sizeof( arySentChars ) / sizeof( arySentChars[0] ) )
   {
      arySentChars[sentCharCount++] = inputChar;
   }
   netPutChar( inputChar );
   byte++;
}

void run( char *ptrCommand, char *ptrArg )
{
   (void)ptrCommand;
   (void)ptrArg;
}

void sendBlock( void )
{
}

void tempFileError( void )
{
}

int yesNo( void )
{
   return 0;
}

static void checkFile_WhenMessageIsValid_ReturnsZero( void **state )
{
   // Arrange
   FILE *ptrMessageFile;
   int result;

   (void)state;

   resetState();

   ptrMessageFile = tmpfile();
   if ( ptrMessageFile == NULL )
   {
      fail_msg( "tmpfile failed in valid-message test setup" );
      return;
   }
   fprintf( ptrMessageFile, "Hello world.\nThis line is fine.\n" );
   fflush( ptrMessageFile );

   // Act
   result = checkFile( ptrMessageFile );

   // Assert
   if ( result != 0 )
   {
      fail_msg( "checkFile should return 0 for valid message content; got %d", result );
   }

   fclose( ptrMessageFile );
}

static void checkFile_WhenLineExceeds79Chars_ReturnsOne( void **state )
{
   // Arrange
   FILE *ptrMessageFile;
   int result;

   (void)state;

   resetState();

   ptrMessageFile = tmpfile();
   if ( ptrMessageFile == NULL )
   {
      fail_msg( "tmpfile failed in line-length test setup" );
      return;
   }
   if ( !tryWriteRepeatedChar( ptrMessageFile, 'A', 80 ) )
   {
      fclose( ptrMessageFile );
      fail_msg( "Arrange failed: unable to write long line content for line-length test" );
      return;
   }
   fputc( '\n', ptrMessageFile );
   fflush( ptrMessageFile );

   // Act
   result = checkFile( ptrMessageFile );

   // Assert
   if ( result != 1 )
   {
      fail_msg( "checkFile should return 1 when line exceeds 79 chars; got %d", result );
   }

   fclose( ptrMessageFile );
}

static void checkFile_WhenIllegalControlCharacterPresent_ReturnsOne( void **state )
{
   // Arrange
   FILE *ptrMessageFile;
   int result;

   (void)state;

   resetState();

   ptrMessageFile = tmpfile();
   if ( ptrMessageFile == NULL )
   {
      fail_msg( "tmpfile failed in illegal-control-char test setup" );
      return;
   }
   fputc( 'O', ptrMessageFile );
   fputc( 1, ptrMessageFile );
   fputc( 'K', ptrMessageFile );
   fputc( '\n', ptrMessageFile );
   fflush( ptrMessageFile );

   // Act
   result = checkFile( ptrMessageFile );

   // Assert
   if ( result != 1 )
   {
      fail_msg( "checkFile should return 1 for illegal control chars; got %d", result );
   }

   fclose( ptrMessageFile );
}

static void checkFile_WhenTabExpansionPushesPast79_ReturnsOne( void **state )
{
   // Arrange
   FILE *ptrMessageFile;
   int result;

   (void)state;

   resetState();

   ptrMessageFile = tmpfile();
   if ( ptrMessageFile == NULL )
   {
      fail_msg( "tmpfile failed in tab-expansion test setup" );
      return;
   }
   if ( !tryWriteRepeatedChar( ptrMessageFile, 'A', 73 ) )
   {
      fclose( ptrMessageFile );
      fail_msg( "Arrange failed: unable to write message content for tab-expansion test" );
      return;
   }
   fputc( '\t', ptrMessageFile );
   fputc( '\n', ptrMessageFile );
   fflush( ptrMessageFile );

   // Act
   result = checkFile( ptrMessageFile );

   // Assert
   if ( result != 1 )
   {
      fail_msg( "checkFile should return 1 when tab expansion exceeds limit; got %d", result );
   }

   fclose( ptrMessageFile );
}

static void checkFile_WhenTotalMessageSizeExceedsLimit_ReturnsOne( void **state )
{
   // Arrange
   FILE *ptrMessageFile;
   int result;
   int lineIndex;

   (void)state;

   resetState();

   ptrMessageFile = tmpfile();
   if ( ptrMessageFile == NULL )
   {
      fail_msg( "tmpfile failed in total-size test setup" );
      return;
   }
   for ( lineIndex = 0; lineIndex < 620; ++lineIndex )
   {
      if ( !tryWriteRepeatedChar( ptrMessageFile, 'A', 79 ) )
      {
         fclose( ptrMessageFile );
         fail_msg( "Arrange failed: unable to write message content for size-limit test" );
         return;
      }
      fputc( '\n', ptrMessageFile );
   }
   fflush( ptrMessageFile );

   // Act
   result = checkFile( ptrMessageFile );

   // Assert
   if ( result != 1 )
   {
      fail_msg( "checkFile should return 1 when total message size exceeds hard cap; got %d", result );
   }

   fclose( ptrMessageFile );
}

static void prompt_WhenSaveSelected_SavesMessageAndReturnsMinusOne( void **state )
{
   // Arrange
   FILE *ptrMessageFile;
   int result;
   int previousChar;
   int aryKeys[] = { '\n', 's' };

   (void)state;

   resetState();
   setInputSequence( aryKeys, sizeof( aryKeys ) / sizeof( aryKeys[0] ) );
   flagsConfiguration.isPosting = 1;
   previousChar = 0;
   ptrMessageFile = tmpfile();
   if ( ptrMessageFile == NULL )
   {
      fail_msg( "tmpfile failed in prompt save test setup" );
      return;
   }
   fprintf( ptrMessageFile, "Breaking News\n" );
   fflush( ptrMessageFile );

   // Act
   result = prompt( ptrMessageFile, &previousChar, '\n' );

   // Assert
   if ( result != -1 )
   {
      fclose( ptrMessageFile );
      fail_msg( "prompt should return -1 after saving a valid message; got %d", result );
      return;
   }
   if ( !flagsConfiguration.isLastSave || flagsConfiguration.isPosting )
   {
      fclose( ptrMessageFile );
      fail_msg( "prompt should mark the post saved and clear posting state; got isLastSave=%u isPosting=%u",
                flagsConfiguration.isLastSave, flagsConfiguration.isPosting );
      return;
   }
   if ( sentCharCount < 3 ||
        arySentChars[sentCharCount - 2] != CTRL_D ||
        arySentChars[sentCharCount - 1] != 's' )
   {
      fclose( ptrMessageFile );
      fail_msg( "prompt save path should send CTRL_D followed by 's'; sent count=%zu last=%d second_last=%d",
                sentCharCount,
                sentCharCount > 0 ? arySentChars[sentCharCount - 1] : -1,
                sentCharCount > 1 ? arySentChars[sentCharCount - 2] : -1 );
      return;
   }

   fclose( ptrMessageFile );
}

static void prompt_WhenInvokedWithPrintCommand_LoadsExistingMessage( void **state )
{
   // Arrange
   FILE *ptrMessageFile;
   int result;
   int previousChar;

   (void)state;

   resetState();
   previousChar = -1;
   ptrMessageFile = tmpfile();
   if ( ptrMessageFile == NULL )
   {
      fail_msg( "tmpfile failed in prompt print test setup" );
      return;
   }
   fprintf( ptrMessageFile, "Existing draft line\n" );
   fflush( ptrMessageFile );

   // Act
   result = prompt( ptrMessageFile, &previousChar, 'p' );

   // Assert
   if ( result != 0 )
   {
      fclose( ptrMessageFile );
      fail_msg( "prompt should return 0 when reloading an existing draft; got %d", result );
      return;
   }
   if ( previousChar != '\n' )
   {
      fclose( ptrMessageFile );
      fail_msg( "prompt should change previousChar from -1 to newline after reloading an existing draft; got %d", previousChar );
      return;
   }
   if ( fseek( ptrMessageFile, 0L, SEEK_END ) != 0 )
   {
      fclose( ptrMessageFile );
      fail_msg( "Arrange failed: unable to seek to end of message file after print path" );
      return;
   }
   if ( ftell( ptrMessageFile ) <= 0 )
   {
      fclose( ptrMessageFile );
      fail_msg( "prompt print path should preserve existing draft contents" );
      return;
   }

   fclose( ptrMessageFile );
}

int main( void )
{
   const struct CMUnitTest aryTests[] = {
      cmocka_unit_test( checkFile_WhenMessageIsValid_ReturnsZero ),
      cmocka_unit_test( checkFile_WhenLineExceeds79Chars_ReturnsOne ),
      cmocka_unit_test( checkFile_WhenIllegalControlCharacterPresent_ReturnsOne ),
      cmocka_unit_test( checkFile_WhenTabExpansionPushesPast79_ReturnsOne ),
      cmocka_unit_test( checkFile_WhenTotalMessageSizeExceedsLimit_ReturnsOne ),
      cmocka_unit_test( prompt_WhenSaveSelected_SavesMessageAndReturnsMinusOne ),
      cmocka_unit_test( prompt_WhenInvokedWithPrintCommand_LoadsExistingMessage ),
   };

   return cmocka_run_group_tests( aryTests, NULL, NULL );
}
