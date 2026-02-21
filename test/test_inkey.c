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

static int waitNextEventCallCount;
static int myExitCallCount;
static int fatalPerrorCallCount;

static void resetState( void )
{
   int keyIndex;

   waitNextEventCallCount = 0;
   myExitCallCount = 0;
   fatalPerrorCallCount = 0;

   targetByte = 0;
   bytePosition = 0;
   childPid = 0;
   isAway = 0;
   isLoginShell = 0;
   capture = 0;

   flagsConfiguration.isConfigMode = 0;
   flagsConfiguration.isPosting = 0;
   flagsConfiguration.shouldCheckExpress = 0;
   flagsConfiguration.isLastSave = 0;

   commandKey = -1;
   awayKey = 'a';
   quitKey = CTRL_D;
   suspKey = CTRL_Z;
   shellKey = '!';
   browserKey = 'w';
   captureKey = 'c';

   for ( keyIndex = 0; keyIndex < 128; ++keyIndex )
   {
      aryMacro[keyIndex][0] = '\0';
   }

   ptyInputLength = 0;
   ptrPtyInput = aryPtyInputBuffer;
   netInputLength = 0;
   ptrNetInput = aryNetInputBuffer;
}

static void setPtyInput( const int *aryInput, size_t inputCount )
{
   int aryCopiedInput[sizeof( aryPtyInputBuffer )];
   size_t copiedCount;
   size_t inputIndex;

   copiedCount = copyIntArray( aryInput,
                               inputCount,
                               aryCopiedInput,
                               sizeof( aryCopiedInput ) / sizeof( aryCopiedInput[0] ) );
   for ( inputIndex = 0; inputIndex < copiedCount; ++inputIndex )
   {
      aryPtyInputBuffer[inputIndex] = (unsigned char)aryCopiedInput[inputIndex];
   }
   ptyInputLength = (ssize_t)copiedCount;
   ptrPtyInput = aryPtyInputBuffer;
}

/* inkey.c dependencies outside this test scope. */
void fatalPerror( const char *message, const char *heading )
{
   (void)message;
   (void)heading;
   fatalPerrorCallCount++;
}

void myExit( void )
{
   myExitCallCount++;
}

void openBrowser( void )
{
}

void run( char *ptrCommand, char *ptrArg )
{
   (void)ptrCommand;
   (void)ptrArg;
}

int stdPrintf( const char *format, ... )
{
   va_list argList;

   va_start( argList, format );
   va_end( argList );
   return 0;
}

void suspend( void )
{
}

int telReceive( int inputChar )
{
   (void)inputChar;
   return 0;
}

int waitNextEvent( void )
{
   waitNextEventCallCount++;
   return 0;
}

int yesNo( void )
{
   return 0;
}

static void inKey_WhenCarriageReturnThenLineFeed_SkipsSecondNewline( void **state )
{
   // Arrange
   const int aryInput[] = { '\r', '\n', 'M' };
   int firstResult;
   int secondResult;

   (void)state;

   resetState();
   setPtyInput( aryInput, sizeof( aryInput ) / sizeof( aryInput[0] ) );

   // Act
   firstResult = inKey();
   secondResult = inKey();

   // Assert
   if ( firstResult != '\n' )
   {
      fail_msg( "first result should normalize CR to newline; got %d", firstResult );
   }
   if ( secondResult != 'M' )
   {
      fail_msg( "second result should skip LF after CR and return next char; got %d", secondResult );
   }
   if ( waitNextEventCallCount != 0 )
   {
      fail_msg( "inKey should not call waitNextEvent when PTY input already exists; got %d calls", waitNextEventCallCount );
   }
}

static void inKey_WhenDeleteAndCtrlU_AppliesKeyTranslations( void **state )
{
   // Arrange
   const int aryInput[] = { 127, CTRL_U };
   int deleteResult;
   int ctrlUResult;

   (void)state;

   resetState();
   setPtyInput( aryInput, sizeof( aryInput ) / sizeof( aryInput[0] ) );

   // Act
   deleteResult = inKey();
   ctrlUResult = inKey();

   // Assert
   if ( deleteResult != '\b' )
   {
      fail_msg( "delete key should map to backspace; got %d", deleteResult );
   }
   if ( ctrlUResult != CTRL_X )
   {
      fail_msg( "CTRL_U should map to CTRL_X; got %d", ctrlUResult );
   }
}

static void getKey_WhenTargetByteActive_ReturnsSavedByteAndAdvancesPosition( void **state )
{
   // Arrange
   int result;

   (void)state;

   resetState();
   arySavedBytes[0] = (unsigned char)'Q';
   bytePosition = 0;
   targetByte = 1;

   // Act
   result = getKey();

   // Assert
   if ( result != 'Q' )
   {
      fail_msg( "getKey should return byte from arySavedBytes when targetByte is active; got %d", result );
   }
   if ( bytePosition != 1 )
   {
      fail_msg( "getKey should advance bytePosition when replaying saved byte; got %ld", bytePosition );
   }
}

static void getKey_WhenCommandMacroTriggered_ReturnsMacroText( void **state )
{
   // Arrange
   const int aryInput[] = { ';', 'm' };
   int firstResult;
   int secondResult;

   (void)state;

   resetState();
   commandKey = ';';
   snprintf( aryMacro['m'], sizeof( aryMacro['m'] ), "%s", "Hi" );
   setPtyInput( aryInput, sizeof( aryInput ) / sizeof( aryInput[0] ) );

   // Act
   firstResult = getKey();
   secondResult = getKey();

   // Assert
   if ( firstResult != 'H' || secondResult != 'i' )
   {
      fail_msg( "macro expansion should return 'H' then 'i'; got %d then %d", firstResult, secondResult );
   }
   if ( fatalPerrorCallCount != 0 || myExitCallCount != 0 )
   {
      fail_msg( "macro expansion should not hit fatal paths; fatalPerror=%d myExit=%d", fatalPerrorCallCount, myExitCallCount );
   }
}

int main( void )
{
   const struct CMUnitTest aryTests[] = {
      cmocka_unit_test( inKey_WhenCarriageReturnThenLineFeed_SkipsSecondNewline ),
      cmocka_unit_test( inKey_WhenDeleteAndCtrlU_AppliesKeyTranslations ),
      cmocka_unit_test( getKey_WhenTargetByteActive_ReturnsSavedByteAndAdvancesPosition ),
      cmocka_unit_test( getKey_WhenCommandMacroTriggered_ReturnsMacroText ),
   };

   return cmocka_run_group_tests( aryTests, NULL, NULL );
}
