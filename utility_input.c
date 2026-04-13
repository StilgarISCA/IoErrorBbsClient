/*
 * Copyright (C) 2024-2026 Stilgar
 * Copyright (C) 1995-2003 Michael Hampton
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

/*
 * This file handles input validation and simple menu prompts.
 */
#include "defs.h"
#include "ext.h"

static int printYesNoResult( int inputChar );
static int readValidatedInput( const char *allowedChars, bool shouldFoldInput );

void handleInvalidInput( unsigned int *ptrInvalidCount )
{
   if ( ( *ptrInvalidCount )++ )
   {
      flushInput( *ptrInvalidCount );
   }
}

void looper( void )
{
   unsigned int invalid = 0;

   while ( true )
   {
      register int inputChar;

      if ( ( inputChar = inKey() ) < 0 )
      {
         return;
      }
      /* Don't bother sending stuff to the bbs it won't use anyway */
      if ( ( inputChar >= ASCII_PRINTABLE_MIN &&
             inputChar <= ASCII_PRINTABLE_MAX ) ||
           findChar( ALLOWED_INPUT_CONTROL_CHARS, inputChar ) )
      {
         invalid = 0;
         netPutChar( aryKeyMap[inputChar] );
         if ( byte )
         {
            size_t index = (size_t)( byte % (long)sizeof arySavedBytes );
            arySavedBytes[index] = (unsigned char)inputChar;
            byte++;
         }
      }
      else
      {
         handleInvalidInput( &invalid );
      }
   }
}

int more( int *line, int percentComplete )
{
   char aryPrompt[96];
   unsigned int invalid = 0;

   if ( percentComplete >= 0 )
   {
      snprintf( aryPrompt, sizeof( aryPrompt ),
                "Page break (%d%%): Space next page, Enter next line, Q quit",
                percentComplete );
   }
   else
   {
      snprintf( aryPrompt, sizeof( aryPrompt ),
                "%s",
                "Page break: Space next page, Enter next line, Q quit" );
   }
   printf( "%s", aryPrompt );
   while ( true )
   {
      register int inputChar;

      inputChar = readFoldedKey();
      if ( inputChar == ' ' || inputChar == 'y' )
      {
         *line = 1;
      }
      else if ( inputChar == '\n' )
      {
         --*line;
      }
      else if ( findChar( "nqs", inputChar ) )
      {
         *line = -1;
      }
      else
      {
         handleInvalidInput( &invalid );
         continue;
      }
      printf( "\r%*s\r", (int)strlen( aryPrompt ), "" );
      break;
   }
   return ( *line < 0 ? -1 : 0 );
}

static int printYesNoResult( int inputChar )
{
   if ( inputChar == 'y' || inputChar == 'Y' )
   {
      stdPrintf( "Yes\r\n" );
      return 1;
   }

   stdPrintf( "No\r\n" );
   return 0;
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

static int readValidatedInput( const char *allowedChars, bool shouldFoldInput )
{
   unsigned int invalid = 0;

   while ( true )
   {
      int inputChar;

      if ( shouldFoldInput )
      {
         inputChar = readFoldedKey();
      }
      else
      {
         inputChar = inKey();
      }
      if ( findChar( allowedChars, inputChar ) )
      {
         return inputChar;
      }
      handleInvalidInput( &invalid );
   }
}

int readValidatedKey( const char *allowedChars )
{
   return readValidatedInput( allowedChars, false );
}

int readValidatedMenuKey( const char *allowedCharsLowercase )
{
   return readValidatedInput( allowedCharsLowercase, true );
}

int yesNo( void )
{
   register int inputChar;

   inputChar = readValidatedKey( "nNyY" );
   return printYesNoResult( inputChar );
}

int yesNoDefault( int defaultAnswer )
{
   register int inputChar;

   inputChar = readValidatedKey( "nNyY\n " );
   if ( inputChar == '\n' || inputChar == ' ' )
   {
      inputChar = ( defaultAnswer ? 'Y' : 'N' );
   }
   if ( inputChar == 'y' || inputChar == 'Y' ||
        inputChar == 'n' || inputChar == 'N' )
   {
      return printYesNoResult( inputChar );
   }
   else
   {
      char aryBuffer[160];
      stdPrintf( "\r\n" );
      snprintf( aryBuffer, sizeof( aryBuffer ), "yesNoDefault: 0x%x\r\n"
                                                "Please report this to IO ERROR\r\n",
                inputChar );
      fatalExit( aryBuffer, "Internal error" );
   }
   return 0;
}
