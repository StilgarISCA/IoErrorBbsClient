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
         inputChar = 'P';
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

/*
 * This function used to be part of edit(), it was broken out because stupid
 * DEC optimizers found edit() too long to optimize without a warning, and that
 * warning made people think something went wrong in the compilation.  This
 * also might even make this stuff easier for others to understand, but I doubt
 * it.
 */
int prompt( FILE *ptrMessageFile, int *previousChar, int commandChar )
{
   FILE *ptrCopyFile;
   int itemIndex;
   int inputChar = commandChar;
   int lineLength;
   unsigned int invalid = 0;
   long size;
   int lines;
   char aryCurrentLine[80];

   itemIndex = 0;
   while ( true )
   {
      if ( *previousChar != -1 )
      {
         if ( itemIndex != 1 )
         {
            sendBlock();
            sendTrackedChar( CTRL_D );
            sendTrackedChar( 'c' );
            flagsConfiguration.shouldCheckExpress = 1;
            (void)inKey();
            if ( flagsConfiguration.useAnsi )
            {
               colorize( "@YA@Cbort  @YC@Continue  @YE@Cdit  @YP@Crint  @YS@Cave  @YX@Cpress -> @G " );
            }
            else
            {
               printf( "<A>bort <C>ontinue <E>dit <P>rint <S>ave <X>press -> " );
            }
            fflush( stdout );
         }
         itemIndex = 0;
         /* Make 'x' work at this prompt for isXland function */
         if ( !( isXland && xlandQueue->nobjs ) )
         {
            while ( !findChar( " \naAcCeEpPsSQtTx?/", inputChar = inKey() ) )
            {
               handleInvalidInput( &invalid );
            }
            invalid = 0;
         }
         else
         {
            inputChar = 'x';
         }
      }
      switch ( inputChar )
      {
         case ' ':
         case '\n':
            if ( !itemIndex++ )
            {
               continue;
            }
            /* Flush repeated keystrokes before returning to edit mode. */
            flushInput( (unsigned)itemIndex );
            printf( "\r\n" );
            continue;

         case 'a':
         case 'A':
            printf( "Abort: are you sure? " );
            if ( yesNo() )
            {
               sendBlock();
               sendTrackedChar( CTRL_D );
               sendTrackedChar( 'a' );
               flagsConfiguration.isPosting = 0;
               return ( -1 );
            }
            continue;

         case 'c':
         case 'C':
            printf( "Continue...\r\n" );
            if ( flagsConfiguration.useAnsi )
            {
               continuedPostHelper();
            }
            break;

         case 'p':
         case 'P':
            if ( *previousChar == -1 )
            {
               *previousChar = '\n';
            }
            else
            {
               printf( "Print formatted\r\n\n%s", arySavedHeader );
            }
            fseek( ptrMessageFile, 0L, SEEK_END );
            size = ftell( ptrMessageFile );
            rewind( ptrMessageFile );
            lines = 2;
            lineLength = 0;
            itemIndex = 0;
            while ( ( inputChar = getc( ptrMessageFile ) ) > 0 )
            {
               itemIndex++;
               if ( inputChar == TAB )
               {
                  do
                  {
                     putchar( ' ' );
                  } while ( ++lineLength & 7 );
               }
               else
               {
                  if ( inputChar == '\n' )
                  {
                     putchar( '\r' );
                  }
                  putchar( inputChar );
                  lineLength++;
               }
               if ( inputChar == '\n' )
               {
                  lineLength = 0;
                  if ( ++lines == rows && more( &lines, size > 0 ? (int)( itemIndex * 100 / size ) : 0 ) < 0 )
                  {
                     break;
                  }
               }
            }
            fseek( ptrMessageFile, 0L, SEEK_END );
            break;

         case 's':
         case 'S':
            printf( "Save message\r\n" );
            if ( checkFile( ptrMessageFile ) )
            {
               continue;
            }
            rewind( ptrMessageFile );
            sendBlock();
            while ( ( inputChar = getc( ptrMessageFile ) ) > 0 )
            {
               sendTrackedChar( inputChar );
            }
            sendTrackedChar( CTRL_D );
            sendTrackedChar( 's' );
            flagsConfiguration.isLastSave = 1;
            flagsConfiguration.isPosting = 0;
            return ( -1 );

         case 'Q':
         case 't':
         case 'T':
         case 'x':
         case '?':
         case '/':
            sendBlock();
            sendTrackedChar( CTRL_D );
            sendTrackedChar( inputChar );
            looper();
            netPutChar( 'c' );
            continue;

         case 'e':
         case 'E':
            printf( "Edit\r\n" );
            if ( !*aryEditor )
            {
               printf( "[Error:  No editor available]\r\n" );
            }
            else
            {
               if ( inputChar == 'E' )
               {
                  fseek( ptrMessageFile, 0L, SEEK_END );
                  if ( ftell( ptrMessageFile ) )
                  {
                     printf( "\r\nThere is text in your edit file.  Do you wish to erase it? (Y/N) -> " );
                     if ( yesNo() )
                     {
                        if ( !( tempFile = freopen( aryTempFileName, "w+", tempFile ) ) )
                        {
                           fatalPerror( "load file into aryEditor: reopen temp file for truncate", "Edit file error" );
                        }
                        ptrMessageFile = tempFile;
                     }
                     else
                     {
                        continue;
                     }
                  }
                  printf( "\r\nFilename -> " );
                  getString( 67, aryCurrentLine, -999 );
                  if ( !*aryCurrentLine )
                  {
                     continue;
                  }
                  if ( !( ptrCopyFile = fopen( aryCurrentLine, "r" ) ) )
                  {
                     printf( "\r\n[Error:  named file does not exist]\r\n\n" );
                     continue;
                  }
                  else
                  {
                     while ( ( itemIndex = getc( ptrCopyFile ) ) >= 0 )
                     {
                        if ( putc( itemIndex, ptrMessageFile ) < 0 )
                        {
                           tempFileError();
                           break;
                        }
                     }
                     if ( feof( ptrCopyFile ) && fflush( ptrMessageFile ) < 0 )
                     {
                        tempFileError();
                     }
                     fclose( ptrCopyFile );
                  }
               }
               /* We have to close and reopen the tempFile due to locking */
               fclose( tempFile );
               run( aryEditor, aryTempFileName );
               if ( !( tempFile = fopen( aryTempFileName, "a+" ) ) )
               {
                  fatalPerror( "openTmpFile: fopen", "Local error" );
               }
               if ( flagsConfiguration.useAnsi )
               {
                  printf( "\033[%cm\033[3%cm", flagsConfiguration.useBold ? '1' : '0', lastColor );
               }
               printf( "[Editing complete]\r\n" );
               if ( !( tempFile = freopen( aryTempFileName, "r+", tempFile ) ) )
               {
                  fatalPerror( "aryEditor return: freopen(aryTempFileName, \"r+\")", "Edit file error" );
               }
               ptrMessageFile = tempFile;
               if ( checkFile( ptrMessageFile ) )
               {
                  fflush( stdout );
                  mySleep( 1 );
               }
            }
            continue;
      }
      return ( 0 );
   }
}

/*
 * Checks the file for lines longer than 79 characters, unprintable characters,
 * or the file itself being too long.  Returns 1 if the file has problems and
 * cannot be saved as is, 0 otherwise.
 */
int checkFile( FILE *ptrMessageFile )
{
   int itemIndex;
   int count = 0;
   int line = 1;
   int total = 0;

   rewind( ptrMessageFile );
   while ( !feof( ptrMessageFile ) )
   {
      if ( ( itemIndex = getc( ptrMessageFile ) ) != '\r' && itemIndex != '\n' )
      {
         if ( ( itemIndex >= 0 && itemIndex < 32 &&
                itemIndex != TAB ) ||
              itemIndex >= DEL )
         {
            printf( "\r\n[Warning:  illegal character in line %d, edit file before saving]\r\n\n", line );
            return ( 1 );
         }
         else
         {
            if ( itemIndex == TAB )
            {
               count = ( count + 8 ) & 0xf8;
            }
            else
            {
               count++;
            }
            if ( count > 79 )
            {
               printf( "\r\n[Warning:  line %d too long, edit file before saving]\r\n\n", line );
               return ( 1 );
            }
         }
      }
      else
      {
         total += count;
         count = 0;
         line++;
      }
   }
   if ( total > 48800 )
   {
      printf( "\r\n[Warning:  message too long, edit file before saving]\r\n\n" );
      return ( 1 );
   }
   return ( 0 );
}
