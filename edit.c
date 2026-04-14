/*
 * Copyright (C) 2024-2026 Stilgar
 * Copyright (C) 1995-2003 Michael Hampton
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

/*
 * This is just a hacked-up version of the editor from the BBS...ugly, isn't
 * it?  You don't need to mess with this unless you have a lot of time to waste
 * on a lost cause.  Use a real editor, that is what '.edit' is for!
 */
#include "defs.h"
#include "ext.h"

void makeMessage( int upload ) /* 0 = normal, 1 = upload (end w/^D) */
{
   int inputChar;
   FILE *ptrMessageFile = tempFile;
   int itemIndex;
   int lineLength;          /* line length */
   int lastSpacePosition;   /* position of last space encountered */
   int cancelspace;         /* true when last character is space */
   char aryCurrentLine[81]; /* array to arySavedBytes current line */
   int previousChar = '\n';
   unsigned int invalid = 0;
   int tabcount = 0;

   flagsConfiguration.isPosting = 1;
   if ( isAway )
   {
      printf( "[No longer away]\r\n" );
      isAway = 0;
   }
   if ( capture )
   {
      printf( "[Capture to temp file turned OFF]\r\n" );
      capture = 0;
      fflush( tempFile );
   }
   rewind( ptrMessageFile );
   if ( flagsConfiguration.isLastSave )
   {
      if ( !( tempFile = freopen( aryTempFileName, "w+", tempFile ) ) )
      {
         fatalPerror( "makeMessage: freopen(aryTempFileName, \"w+\")", "Edit file error" );
      }
      ptrMessageFile = tempFile;
      flagsConfiguration.isLastSave = 0;
   }
   if ( getc( ptrMessageFile ) >= 0 )
   {
      rewind( ptrMessageFile );
      printf( "There is text in your edit file.  Do you wish to erase it? (Y/N) -> " );
      if ( yesNo() )
      {
         if ( !( tempFile = freopen( aryTempFileName, "w+", tempFile ) ) )
         {
            fatalPerror( "makeMessage: reopen temp file for truncate", "Edit file error" );
         }
         ptrMessageFile = tempFile;
      }
      else
      {
         (void)checkFile( ptrMessageFile );
         previousChar = -1;
      }
   }
   lineLength = 0;
   lastSpacePosition = 0;
   cancelspace = 0;

   while ( true )
   {
      if ( ftell( ptrMessageFile ) > 48700 )
      {
         if ( previousChar != -1 )
         {
            printf( "\r\nMessage too long, must Abort or Save\r\n\n" );
            fflush( stdout );
            mySleep( 1 );
            flushInput( 0 );
            tabcount = 0;
         }
         inputChar = CTRL_D;
         previousChar = '\n';
      }
      else if ( previousChar < 0 )
      {
         inputChar = 'p';
      }
      else if ( tabcount )
      {
         tabcount--;
         inputChar = ' ';
      }
      else
      {
         inputChar = inKey();
         if ( iscntrl( inputChar ) && inputChar == CTRL_D && !upload )
         {
            inputChar = 1; /* Just a random invalid character */
         }

         if ( inputChar != CTRL_D && inputChar != TAB &&
              inputChar != '\b' && inputChar != '\n' &&
              inputChar != CTRL_X && inputChar != CTRL_W &&
              inputChar != CTRL_R && !isprint( inputChar ) )
         {
            handleInvalidInput( &invalid );
            continue;
         }
         invalid = 0;
      }

      if ( inputChar == CTRL_R )
      {
         aryCurrentLine[lineLength + 1] = 0;
         printf( "\r\n%s", aryCurrentLine + 1 );
         continue;
      }
      if ( inputChar == TAB )
      {
         tabcount = 7 - ( lineLength & 7 );
         inputChar = ' ';
      }
      if ( inputChar == '\b' )
      {
         if ( lineLength )
         {
            putchar( '\b' );
            putchar( ' ' );
            putchar( '\b' );
            if ( lineLength-- == lastSpacePosition )
            {
               for ( ; --lastSpacePosition && aryCurrentLine[lastSpacePosition] != ' '; )
               {
                  ;
               }
            }
            if ( !lineLength )
            {
               previousChar = '\n';
            }
         }
         continue;
      }
      if ( inputChar == CTRL_W )
      { /* ctrl-W is 'word erase' from Unix */
         for ( itemIndex = 0; itemIndex < lineLength; itemIndex++ )
         {
            if ( aryCurrentLine[itemIndex + 1] != ' ' )
            {
               break;
            }
         }
         itemIndex = ( itemIndex == lineLength );
         for ( ; lineLength && ( !itemIndex || aryCurrentLine[lineLength] != ' ' ); lineLength-- )
         {
            if ( aryCurrentLine[lineLength] != ' ' )
            {
               itemIndex = 1;
            }
            putchar( '\b' );
            putchar( ' ' );
            putchar( '\b' );
         }
         if ( !lineLength || aryCurrentLine[lineLength] == ' ' )
         {
            lastSpacePosition = lineLength;
         }
         if ( !lineLength )
         {
            previousChar = '\n';
         }
         continue;
      }
      if ( inputChar == CTRL_X )
      { /* ctrl-X works like in normal Unix */
         for ( ; lineLength; lineLength-- )
         {
            putchar( '\b' );
            putchar( ' ' );
            putchar( '\b' );
         }
         lastSpacePosition = 0;
         previousChar = '\n';
         continue;
      }
      /*
    * Ignore space when the last character typed on a the previous line
    * and the first character typed on this line are both spaces.  This
    * makes free flow typing easier and properly formatted.
    */
      if ( cancelspace )
      {
         cancelspace = 0;
         if ( inputChar == ' ' )
         {
            continue;
         }
      }

      if ( inputChar != '\n' && inputChar != CTRL_D && previousChar != -1 )
      {
         lineLength++;
      }

      if ( inputChar == ' ' && ( lastSpacePosition = lineLength ) == 80 )
      {
         cancelspace = 1;
         inputChar = '\n';
         for ( ; --lineLength && aryCurrentLine[lineLength] == ' '; )
         {
            ;
         }
      }
      if ( lineLength == 80 )
      {
         if ( lastSpacePosition > ( 80 / 2 ) )
         { /* don't autowrap past 40th column */
            for ( itemIndex = 80 - 1; itemIndex && ( itemIndex > lastSpacePosition || aryCurrentLine[itemIndex] == ' ' ); itemIndex-- )
            {
               if ( itemIndex > lastSpacePosition )
               {
                  putchar( '\b' );
                  putchar( ' ' );
                  putchar( '\b' );
               }
            }
            for ( lineLength = 1; lineLength <= itemIndex; lineLength++ )
            {
               if ( putc( aryCurrentLine[lineLength], ptrMessageFile ) < 0 )
               {
                  tempFileError();
               }
            }
            if ( putc( '\n', ptrMessageFile ) < 0 )
            {
               tempFileError();
            }
            printf( "\r\n" );
            for ( lineLength = 1; ( lineLength + lastSpacePosition ) < 80; lineLength++ )
            {
               previousChar = aryCurrentLine[lineLength + lastSpacePosition];
               putchar( previousChar );
               aryCurrentLine[lineLength] = (char)previousChar;
            }
            lastSpacePosition = 0;
         }
         else
         {
            for ( itemIndex = 1; itemIndex < 80; itemIndex++ )
            {
               if ( putc( aryCurrentLine[itemIndex], ptrMessageFile ) < 0 )
               {
                  tempFileError();
               }
            }
            if ( putc( '\n', ptrMessageFile ) < 0 )
            {
               tempFileError();
            }
            printf( "\r\n" );
            previousChar = '\n';
            lastSpacePosition = 0;
            lineLength = 1;
         }
      }
      if ( inputChar != CTRL_D && inputChar != '\n' && previousChar != -1 )
      {
         putchar( inputChar ); /* echo aryUser's input to screen */
         aryCurrentLine[lineLength] = (char)inputChar;
      }
      else if ( lineLength && inputChar == CTRL_D )
      { /* simulate LF */
         for ( itemIndex = 1; itemIndex <= lineLength; itemIndex++ )
         {
            if ( putc( aryCurrentLine[itemIndex], ptrMessageFile ) < 0 )
            {
               tempFileError();
            }
         }
         if ( putc( '\n', ptrMessageFile ) < 0 )
         {
            tempFileError();
         }
         printf( "\r\n" );
         lastSpacePosition = 0;
         lineLength = 0;
      }
      if ( ( previousChar != '\n' || inputChar != '\n' || upload ) &&
           inputChar != CTRL_D && previousChar != -1 )
      {
         previousChar = inputChar;
         if ( inputChar == '\n' )
         {
            for ( itemIndex = 1; itemIndex <= lineLength; itemIndex++ )
            {
               if ( putc( aryCurrentLine[itemIndex], ptrMessageFile ) < 0 )
               {
                  tempFileError();
               }
            }
            if ( putc( '\n', ptrMessageFile ) < 0 )
            {
               tempFileError();
            }
            printf( "\r\n" );
            lastSpacePosition = 0;
            lineLength = 0;
         }
         continue; /* go back and get next character */
      }
      else
      { /* 2 LFs in a rows (or a ctrl-D) */
         if ( fflush( ptrMessageFile ) < 0 )
         { /* make sure we've written it all */
            tempFileError();
         }

         if ( prompt( ptrMessageFile, &previousChar, inputChar ) < 0 )
         {
            return;
         }
      }
   }
}
