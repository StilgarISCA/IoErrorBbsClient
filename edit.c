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

static void printEditorCommandPrompt( void )
{
   char aryAnsiSequence[32];
   static const char *aryCommandLabels[] =
      {
         "Abort",
         "Continue",
         "Edit",
         "Print",
         "Save",
         "Xpress" };
   size_t itemIndex;

   if ( !flagsConfiguration.shouldUseAnsi )
   {
      printf( "<A>bort <C>ontinue <E>dit <P>rint <S>ave <X>press -> " );
      return;
   }

   for ( itemIndex = 0; itemIndex < sizeof( aryCommandLabels ) / sizeof( aryCommandLabels[0] ); itemIndex++ )
   {
      formatAnsiForegroundSequence( aryAnsiSequence, sizeof( aryAnsiSequence ),
                                    color.forum );
      printf( "%s", aryAnsiSequence );
      printf( "%c", aryCommandLabels[itemIndex][0] );

      formatAnsiForegroundSequence( aryAnsiSequence, sizeof( aryAnsiSequence ),
                                    color.number );
      printf( "%s", aryAnsiSequence );
      printf( "%s", aryCommandLabels[itemIndex] + 1 );

      if ( itemIndex + 1 < sizeof( aryCommandLabels ) / sizeof( aryCommandLabels[0] ) )
      {
         printf( "  " );
      }
   }

   printf( " -> " );
   formatAnsiForegroundSequence( aryAnsiSequence, sizeof( aryAnsiSequence ),
                                 color.text );
   printf( "%s", aryAnsiSequence );
}

static const char *resolveEditorCommand( void )
{
   if ( !*aryEditor )
   {
      return NULL;
   }
   if ( strcmp( aryEditor, DEFAULT_EDITOR_CONFIG_VALUE ) == 0 )
   {
      return *aryMyEditor ? aryMyEditor : NULL;
   }
   return aryEditor;
}

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
            if ( flagsConfiguration.shouldUseAnsi )
            {
               printEditorCommandPrompt();
            }
            else
            {
               printf( "<A>bort <C>ontinue <E>dit <P>rint <S>ave <X>press -> " );
            }
            fflush( stdout );
         }
         itemIndex = 0;
         /* Make 'x' work at this prompt for isXland function */
         if ( !( isXland && xlandQueue->itemCount ) )
         {
            while ( true )
            {
               inputChar = readFoldedKey();
               if ( findChar( " \nacepsqtx?/", inputChar ) )
               {
                  break;
               }
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
            printf( "Continue...\r\n" );
            if ( flagsConfiguration.shouldUseAnsi )
            {
               continuedPostHelper();
            }
            break;

         case 'p':
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

         case 'q':
         case 't':
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
            {
               const char *ptrEditorCommand;

               printf( "Edit\r\n" );
               ptrEditorCommand = resolveEditorCommand();
               if ( ptrEditorCommand == NULL )
               {
                  printf( "[Error:  No editor available]\r\n" );
               }
               else
               {
                  if ( isupper( commandChar ) )
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
                  run( ptrEditorCommand, aryTempFileName );
                  if ( !( tempFile = fopen( aryTempFileName, "a+" ) ) )
                  {
                     fatalPerror( "openTmpFile: fopen", "Local error" );
                  }
                  if ( flagsConfiguration.shouldUseAnsi )
                  {
                     char aryAnsiSequence[32];

                     formatAnsiDisplayStateSequence( aryAnsiSequence, sizeof( aryAnsiSequence ),
                                                     lastColor, color.background,
                                                     flagsConfiguration.shouldUseBold );
                     printf( "%s", aryAnsiSequence );
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
      }
      return ( 0 );
   }
}

static bool tryNormalizeSupportedUtf8Sequence( FILE *ptrMessageFile, int inputChar,
                                               char *ptrReplacementText, size_t *ptrReplacementLength )
{
   int secondByte;
   int thirdByte;

   if ( inputChar == 0xc2 )
   {
      secondByte = getc( ptrMessageFile );
      if ( secondByte == EOF )
      {
         return false;
      }

      switch ( secondByte )
      {
         case 0xa0: /* non-breaking space */
            ptrReplacementText[0] = ' ';
            *ptrReplacementLength = 1;
            return true;

         case 0xad: /* soft hyphen */
            *ptrReplacementLength = 0;
            return true;

         case 0xab: /* left-pointing double angle quotation mark */
         case 0xbb: /* right-pointing double angle quotation mark */
            ptrReplacementText[0] = '"';
            *ptrReplacementLength = 1;
            return true;
      }

      return false;
   }

   if ( inputChar == 0xef )
   {
      secondByte = getc( ptrMessageFile );
      thirdByte = getc( ptrMessageFile );
      if ( secondByte == 0xbb && thirdByte == 0xbf ) /* byte order mark */
      {
         *ptrReplacementLength = 0;
         return true;
      }
      return false;
   }

   if ( inputChar != 0xe2 )
   {
      return false;
   }

   secondByte = getc( ptrMessageFile );
   thirdByte = getc( ptrMessageFile );
   if ( secondByte == EOF || thirdByte == EOF )
   {
      return false;
   }

   if ( secondByte == 0x80 )
   {
      switch ( thirdByte )
      {
         case 0x98: /* left single quotation mark */
         case 0x99: /* right single quotation mark */
            ptrReplacementText[0] = '\'';
            *ptrReplacementLength = 1;
            return true;

         case 0x9c: /* left double quotation mark */
         case 0x9d: /* right double quotation mark */
            ptrReplacementText[0] = '"';
            *ptrReplacementLength = 1;
            return true;

         case 0x92: /* figure dash */
         case 0x93: /* en dash */
         case 0x94: /* em dash */
         case 0x95: /* horizontal bar */
            ptrReplacementText[0] = '-';
            *ptrReplacementLength = 1;
            return true;

         case 0x8b: /* zero width space */
         case 0x8c: /* zero width non-joiner */
         case 0x8d: /* zero width joiner */
            *ptrReplacementLength = 0;
            return true;

         case 0xa6: /* ellipsis */
            ptrReplacementText[0] = '.';
            ptrReplacementText[1] = '.';
            ptrReplacementText[2] = '.';
            *ptrReplacementLength = 3;
            return true;
      }
   }
   else if ( secondByte == 0x88 && thirdByte == 0x92 ) /* minus sign */
   {
      ptrReplacementText[0] = '-';
      *ptrReplacementLength = 1;
      return true;
   }

   return false;
}

/*
 * Checks the file for lines longer than 79 characters, unprintable characters,
 * or the file itself being too long. Supported UTF-8 typographic punctuation
 * is normalized to ASCII in place. Returns 1 if the file still has problems
 * and cannot be saved as is, 0 otherwise.
 */
int checkFile( FILE *ptrMessageFile )
{
   char *ptrNormalizedText;
   long fileSize;
   size_t normalizedLength;
   int count = 0;
   int line = 1;
   int total = 0;
   bool shouldRewriteFile = false;

   if ( fseek( ptrMessageFile, 0L, SEEK_END ) != 0 )
   {
      return 1;
   }
   fileSize = ftell( ptrMessageFile );
   if ( fileSize < 0 )
   {
      return 1;
   }
   normalizedLength = 0;
   ptrNormalizedText = calloc( (size_t)fileSize + 1, sizeof( char ) );
   if ( ptrNormalizedText == NULL )
   {
      fatalExit( "Out of memory validating edit file", "Edit file error" );
      return 1;
   }

   rewind( ptrMessageFile );
   while ( !feof( ptrMessageFile ) )
   {
      int inputChar = getc( ptrMessageFile );
      char aryReplacementText[3];
      size_t replacementLength;

      if ( inputChar == EOF )
      {
         break;
      }

      if ( inputChar != '\r' && inputChar != '\n' )
      {
         if ( tryNormalizeSupportedUtf8Sequence( ptrMessageFile, inputChar, aryReplacementText,
                                                 &replacementLength ) )
         {
            size_t replacementIndex;

            shouldRewriteFile = true;
            for ( replacementIndex = 0; replacementIndex < replacementLength; replacementIndex++ )
            {
               ptrNormalizedText[normalizedLength++] = aryReplacementText[replacementIndex];
               if ( aryReplacementText[replacementIndex] == TAB )
               {
                  count = ( count + 8 ) & 0xf8;
               }
               else
               {
                  count++;
               }
               if ( count > 79 )
               {
                  free( ptrNormalizedText );
                  printf( "\r\n[Warning:  line %d too long, edit file before saving]\r\n\n", line );
                  return ( 1 );
               }
            }
            continue;
         }
         else if ( ( inputChar >= 0 && inputChar < 32 && inputChar != TAB ) || inputChar >= DEL )
         {
            free( ptrNormalizedText );
            printf( "\r\n[Warning:  illegal character in line %d, edit file before saving]\r\n\n", line );
            return ( 1 );
         }

         if ( inputChar == TAB )
         {
            count = ( count + 8 ) & 0xf8;
         }
         else
         {
            count++;
         }
         if ( count > 79 )
         {
            free( ptrNormalizedText );
            printf( "\r\n[Warning:  line %d too long, edit file before saving]\r\n\n", line );
            return ( 1 );
         }
      }
      else
      {
         total += count;
         count = 0;
         line++;
      }

      ptrNormalizedText[normalizedLength++] = (char)inputChar;
   }
   if ( total > 48800 )
   {
      free( ptrNormalizedText );
      printf( "\r\n[Warning:  message too long, edit file before saving]\r\n\n" );
      return ( 1 );
   }
   if ( shouldRewriteFile )
   {
      rewind( ptrMessageFile );
      if ( ftruncate( fileno( ptrMessageFile ), 0 ) != 0 )
      {
         free( ptrNormalizedText );
         fatalPerror( "ftruncate", "Edit file error" );
      }
      if ( normalizedLength > 0 &&
           fwrite( ptrNormalizedText, sizeof( char ), normalizedLength, ptrMessageFile ) != normalizedLength )
      {
         free( ptrNormalizedText );
         tempFileError();
         return 1;
      }
      fflush( ptrMessageFile );
      rewind( ptrMessageFile );
   }
   free( ptrNormalizedText );
   return ( 0 );
}
