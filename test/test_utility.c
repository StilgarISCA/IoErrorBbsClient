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

/* Stubs for utility.c dependencies that are outside this test target's scope. */
void deinitialize( void ) {}

void flushInput( unsigned int count )
{
   (void)count;
}

int inKey( void )
{
   return '\n';
}

int netPutChar( int inputChar )
{
   return inputChar;
}

void resetTerm( void ) {}

void sError( const char *message, const char *heading )
{
   (void)message;
   (void)heading;
}

void sPerror( const char *message, const char *heading )
{
   (void)message;
   (void)heading;
}

void sendBlock( void ) {}

void sigOff( void ) {}

int stdPrintf( const char *format, ... )
{
   va_list argList;

   va_start( argList, format );
   va_end( argList );
   return 0;
}

int stdPutChar( int inputChar )
{
   return inputChar;
}

static void findSubstring_WhenNeedleExists_ReturnsPointerToFirstMatch( void **state )
{
   // Arrange
   const char *ptrHaystack;
   const char *ptrResult;

   (void)state;

   ptrHaystack = "Did I leave my needle in this haystack?";

   // Act
   ptrResult = findSubstring( ptrHaystack, "needle" );

   // Assert
   if ( ptrResult == NULL )
   {
      fail_msg( "findSubstring should return a match pointer when substring exists" );
   }
   if ( strcmp( ptrResult, "needle in this haystack?" ) != 0 )
   {
      fail_msg( "findSubstring should return pointer at first match; got '%s'", ptrResult );
   }
}

static void findSubstring_WhenNeedleMissing_ReturnsNull( void **state )
{
   // Arrange
   const char *ptrResult;

   (void)state;

   // Act
   ptrResult = findSubstring( "Watson. Come here. I need you.", "Moriarty" );

   // Assert
   if ( ptrResult != NULL )
   {
      fail_msg( "findSubstring should return NULL when substring is missing; got '%s'", ptrResult );
   }
}

static void findChar_WhenTargetExists_ReturnsPointerToCharacter( void **state )
{
   // Arrange
   const char *ptrInput;
   const char *ptrResult;

   (void)state;

   ptrInput = "Elementary, my dear Watson.";

   // Act
   ptrResult = findChar( ptrInput, 'W' );

   // Assert
   if ( ptrResult == NULL )
   {
      fail_msg( "findChar should return a pointer when target char exists" );
   }
   if ( *ptrResult != 'W' )
   {
      fail_msg( "findChar should point to the matched char 'W'; got '%c'", *ptrResult );
   }
}

static void findChar_WhenTargetMissing_ReturnsNull( void **state )
{
   // Arrange
   const char *ptrResult;

   (void)state;

   // Act
   ptrResult = findChar( "The game is afoot.", 'z' );

   // Assert
   if ( ptrResult != NULL )
   {
      fail_msg( "findChar should return NULL when target char is absent; got '%s'", ptrResult );
   }
}

static void duplicateString_WhenSourceProvided_ReturnsIndependentCopy( void **state )
{
   // Arrange
   char arySource[] = "Watson. Come here. I need you.";
   char *ptrCopy;

   (void)state;

   // Act
   ptrCopy = duplicateString( arySource );

   // Assert
   if ( ptrCopy == NULL )
   {
      fail_msg( "duplicateString should allocate and return a copy for non-NULL input" );
   }
   if ( strcmp( ptrCopy, arySource ) != 0 )
   {
      fail_msg( "duplicateString result mismatch: expected '%s', got '%s'", arySource, ptrCopy );
   }

   ptrCopy[0] = 'w';
   if ( strcmp( arySource, "Watson. Come here. I need you." ) != 0 )
   {
      fail_msg( "duplicateString should create an independent copy, not alias source" );
   }

   free( ptrCopy );
}

static void extractName_WhenHeaderContainsSender_ReturnsSenderName( void **state )
{
   // Arrange
   const char *ptrName;

   (void)state;

   memset( aryLastName, 0, sizeof( aryLastName ) );

   // Act
   ptrName = extractName( "*** Message (old) from Dr Strange at 3:07 PM on Feb 19, 2026 ***" );

   // Assert
   if ( ptrName == NULL )
   {
      fail_msg( "extractName should return a name pointer for valid header input" );
   }
   if ( strcmp( ptrName, "Dr Strange" ) != 0 )
   {
      fail_msg( "extractName should parse 'Dr Strange'; got '%s'", ptrName );
   }
   if ( strcmp( aryLastName[0], "Dr Strange" ) != 0 )
   {
      fail_msg( "extractName should store parsed name at history slot 0; got '%s'", aryLastName[0] );
   }
}

static void extractName_WhenDuplicateSeen_MovesExistingEntryToFront( void **state )
{
   // Arrange
   const char *ptrName;

   (void)state;

   memset( aryLastName, 0, sizeof( aryLastName ) );
   extractName( "Jan 29, 2026 10:17 AM from Dr Strange to Stilgar" );
   extractName( "Jan 29, 2026 10:17 AM from Meatball to Stilgar" );

   // Act
   ptrName = extractName( "Jan 29, 2026 11:07 AM from Dr Strange to Stilgar" );

   // Assert
   if ( ptrName == NULL )
   {
      fail_msg( "extractName should return non-NULL for duplicate sender headers" );
   }
   if ( strcmp( aryLastName[0], "Dr Strange" ) != 0 || strcmp( aryLastName[1], "Meatball" ) != 0 )
   {
      fail_msg( "duplicate sender should move to front; expected [Dr Strange, Meatball], got ['%s', '%s']",
                aryLastName[0], aryLastName[1] );
   }
}

static void extractName_WhenHeaderDoesNotContainFrom_ReturnsNull( void **state )
{
   // Arrange
   const char *ptrName;

   (void)state;

   // Act
   ptrName = extractName( "No sender marker here" );

   // Assert
   if ( ptrName != NULL )
   {
      fail_msg( "extractName should return NULL when header lacks ' from '; got '%s'", ptrName );
   }
}

static void extractNumber_WhenHeaderContainsSingleDigit_ReturnsParsedNumber( void **state )
{
   // Arrange
   int number;

   (void)state;

   // Act
   number = extractNumber( "*** Message (new) (#7) from Dr Strange at 3:07 PM on Feb 19, 2026 ***" );

   // Assert
   if ( number != 7 )
   {
      fail_msg( "extractNumber should parse a single-digit number; expected 7, got %d", number );
   }
}

static void extractNumber_WhenHeaderHasNoMarker_ReturnsZero( void **state )
{
   // Arrange
   int number;

   (void)state;

   // Act
   number = extractNumber( "no id in this header" );

   // Assert
   if ( number != 0 )
   {
      fail_msg( "extractNumber should return 0 when '(#' marker is missing; got %d", number );
   }
}

int main( void )
{
   const struct CMUnitTest aryTests[] = {
      cmocka_unit_test( findSubstring_WhenNeedleExists_ReturnsPointerToFirstMatch ),
      cmocka_unit_test( findSubstring_WhenNeedleMissing_ReturnsNull ),
      cmocka_unit_test( findChar_WhenTargetExists_ReturnsPointerToCharacter ),
      cmocka_unit_test( findChar_WhenTargetMissing_ReturnsNull ),
      cmocka_unit_test( duplicateString_WhenSourceProvided_ReturnsIndependentCopy ),
      cmocka_unit_test( extractName_WhenHeaderContainsSender_ReturnsSenderName ),
      cmocka_unit_test( extractName_WhenDuplicateSeen_MovesExistingEntryToFront ),
      cmocka_unit_test( extractName_WhenHeaderDoesNotContainFrom_ReturnsNull ),
      cmocka_unit_test( extractNumber_WhenHeaderContainsSingleDigit_ReturnsParsedNumber ),
      cmocka_unit_test( extractNumber_WhenHeaderHasNoMarker_ReturnsZero ),
   };

   return cmocka_run_group_tests( aryTests, NULL, NULL );
}
