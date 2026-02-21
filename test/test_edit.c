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
   return '\n';
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

int main( void )
{
   const struct CMUnitTest aryTests[] = {
      cmocka_unit_test( checkFile_WhenMessageIsValid_ReturnsZero ),
      cmocka_unit_test( checkFile_WhenLineExceeds79Chars_ReturnsOne ),
      cmocka_unit_test( checkFile_WhenIllegalControlCharacterPresent_ReturnsOne ),
      cmocka_unit_test( checkFile_WhenTabExpansionPushesPast79_ReturnsOne ),
      cmocka_unit_test( checkFile_WhenTotalMessageSizeExceedsLimit_ReturnsOne ),
   };

   return cmocka_run_group_tests( aryTests, NULL, NULL );
}
