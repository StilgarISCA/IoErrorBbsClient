/*
 * Copyright (C) 2024-2026 Stilgar
 * Copyright (C) 1995-2003 Michael Hampton
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "defs.h"
#include "proto.h"

typedef struct
{
   size_t lineStartIndex;
   size_t lastWrapIndex;
   int lineWidth;
   int lineNumber;
   int totalWidth;
   bool didWrapLongLine;
} CheckFileState;

static bool appendCheckedChar( CheckFileState *ptrState, char *ptrNormalizedText,
                               size_t *ptrNormalizedLength, int inputChar );
static int calculateDisplayWidth( const char *ptrText, size_t startIndex,
                                  size_t endIndex );
static size_t findLastWrapIndex( const char *ptrText, size_t startIndex,
                                 size_t endIndex );
static int nextLineWidth( int currentWidth, int inputChar );
static int reportIllegalCharacter( char *ptrNormalizedText, int lineNumber );
static int reportLineTooLong( char *ptrNormalizedText, int lineNumber );
static int reportMessageTooLong( char *ptrNormalizedText );
static bool tryNormalizeSupportedUtf8Sequence( FILE *ptrMessageFile, int inputChar,
                                               char *ptrReplacementText,
                                               size_t *ptrReplacementLength );
static int rewriteNormalizedFile( FILE *ptrMessageFile,
                                  const char *ptrNormalizedText,
                                  size_t normalizedLength );

static bool appendCheckedChar( CheckFileState *ptrState, char *ptrNormalizedText,
                               size_t *ptrNormalizedLength, int inputChar )
{
   int updatedWidth;

   if ( inputChar == '\r' || inputChar == '\n' )
   {
      ptrNormalizedText[( *ptrNormalizedLength )++] = (char)inputChar;
      ptrState->lineStartIndex = *ptrNormalizedLength;
      ptrState->lastWrapIndex = SIZE_MAX;
      ptrState->lineWidth = 0;
      ptrState->lineNumber++;
      return true;
   }

   ptrNormalizedText[( *ptrNormalizedLength )++] = (char)inputChar;
   updatedWidth = nextLineWidth( ptrState->lineWidth, inputChar );
   ptrState->totalWidth += updatedWidth - ptrState->lineWidth;
   ptrState->lineWidth = updatedWidth;

   if ( inputChar == ' ' )
   {
      ptrState->lastWrapIndex = *ptrNormalizedLength - 1;
   }

   if ( ptrState->lineWidth <= 79 )
   {
      return true;
   }

   if ( ptrState->lastWrapIndex == SIZE_MAX ||
        ptrState->lastWrapIndex < ptrState->lineStartIndex )
   {
      return false;
   }

   ptrNormalizedText[ptrState->lastWrapIndex] = '\n';
   ptrState->didWrapLongLine = true;
   ptrState->lineNumber++;
   ptrState->lineStartIndex = ptrState->lastWrapIndex + 1;
   ptrState->lineWidth = calculateDisplayWidth( ptrNormalizedText,
                                                ptrState->lineStartIndex,
                                                *ptrNormalizedLength );
   ptrState->lastWrapIndex = findLastWrapIndex( ptrNormalizedText,
                                                ptrState->lineStartIndex,
                                                *ptrNormalizedLength );

   return ptrState->lineWidth <= 79;
}

static int calculateDisplayWidth( const char *ptrText, size_t startIndex,
                                  size_t endIndex )
{
   int lineWidth = 0;
   size_t itemIndex;

   for ( itemIndex = startIndex; itemIndex < endIndex; itemIndex++ )
   {
      lineWidth = nextLineWidth( lineWidth, ptrText[itemIndex] );
   }

   return lineWidth;
}

static int reportIllegalCharacter( char *ptrNormalizedText, int lineNumber )
{
   free( ptrNormalizedText );
   printf( "\r\n[Warning:  illegal character in line %d, edit file before saving]\r\n\n",
           lineNumber );
   return 1;
}

static int reportLineTooLong( char *ptrNormalizedText, int lineNumber )
{
   free( ptrNormalizedText );
   printf( "\r\n[Warning:  line %d too long, edit file before saving]\r\n\n",
           lineNumber );
   return 1;
}

static int reportMessageTooLong( char *ptrNormalizedText )
{
   free( ptrNormalizedText );
   printf( "\r\n[Warning:  message too long, edit file before saving]\r\n\n" );
   return 1;
}

/*
 * Checks the file for lines longer than 79 characters, unprintable characters,
 * or the file itself being too long. Supported UTF-8 typographic punctuation
 * is normalized to ASCII in place. Long lines are wrapped automatically at
 * spaces when that can be done safely. Returns 1 if the file still has
 * problems and cannot be saved as is, 0 otherwise.
 */
int checkFile( FILE *ptrMessageFile )
{
   CheckFileState fileState;
   char *ptrNormalizedText;
   long fileSize;
   size_t normalizedLength;
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
   fileState.lineStartIndex = 0;
   fileState.lastWrapIndex = SIZE_MAX;
   fileState.lineWidth = 0;
   fileState.lineNumber = 1;
   fileState.totalWidth = 0;
   fileState.didWrapLongLine = false;

   rewind( ptrMessageFile );
   while ( !feof( ptrMessageFile ) )
   {
      int inputChar = getc( ptrMessageFile );

      if ( inputChar == EOF )
      {
         break;
      }

      if ( inputChar != '\r' && inputChar != '\n' )
      {
         char aryReplacementText[3];
         size_t replacementLength;

         if ( tryNormalizeSupportedUtf8Sequence( ptrMessageFile, inputChar,
                                                 aryReplacementText,
                                                 &replacementLength ) )
         {
            size_t replacementIndex;

            shouldRewriteFile = true;
            for ( replacementIndex = 0; replacementIndex < replacementLength; replacementIndex++ )
            {
               if ( !appendCheckedChar( &fileState,
                                        ptrNormalizedText,
                                        &normalizedLength,
                                        aryReplacementText[replacementIndex] ) )
               {
                  return reportLineTooLong( ptrNormalizedText,
                                            fileState.lineNumber );
               }
            }
            continue;
         }
         else if ( ( inputChar >= 0 && inputChar < 32 && inputChar != TAB ) ||
                   inputChar >= DEL )
         {
            return reportIllegalCharacter( ptrNormalizedText,
                                           fileState.lineNumber );
         }

         if ( !appendCheckedChar( &fileState,
                                  ptrNormalizedText,
                                  &normalizedLength,
                                  inputChar ) )
         {
            return reportLineTooLong( ptrNormalizedText,
                                      fileState.lineNumber );
         }
      }
      else
      {
         appendCheckedChar( &fileState, ptrNormalizedText, &normalizedLength,
                            inputChar );
      }
   }
   if ( fileState.totalWidth > 48800 )
   {
      return reportMessageTooLong( ptrNormalizedText );
   }
   if ( fileState.didWrapLongLine )
   {
      shouldRewriteFile = true;
   }
   if ( shouldRewriteFile )
   {
      if ( rewriteNormalizedFile( ptrMessageFile, ptrNormalizedText,
                                  normalizedLength ) )
      {
         free( ptrNormalizedText );
         return 1;
      }
      if ( fileState.didWrapLongLine )
      {
         printf( "\r\n[Wrapped long lines while saving]\r\n\n" );
      }
   }
   free( ptrNormalizedText );
   return 0;
}

static size_t findLastWrapIndex( const char *ptrText, size_t startIndex,
                                 size_t endIndex )
{
   size_t itemIndex;

   for ( itemIndex = endIndex; itemIndex > startIndex; itemIndex-- )
   {
      if ( ptrText[itemIndex - 1] == ' ' )
      {
         return itemIndex - 1;
      }
   }

   return SIZE_MAX;
}

static int nextLineWidth( int currentWidth, int inputChar )
{
   if ( inputChar == TAB )
   {
      return ( currentWidth + 8 ) & 0xf8;
   }

   return currentWidth + 1;
}

static int rewriteNormalizedFile( FILE *ptrMessageFile,
                                  const char *ptrNormalizedText,
                                  size_t normalizedLength )
{
   rewind( ptrMessageFile );
   if ( ftruncate( fileno( ptrMessageFile ), 0 ) != 0 )
   {
      fatalPerror( "ftruncate", "Edit file error" );
   }
   if ( normalizedLength > 0 &&
        fwrite( ptrNormalizedText, sizeof( char ), normalizedLength,
                ptrMessageFile ) != normalizedLength )
   {
      tempFileError();
      return 1;
   }
   fflush( ptrMessageFile );
   rewind( ptrMessageFile );

   return 0;
}

static bool tryNormalizeSupportedUtf8Sequence( FILE *ptrMessageFile, int inputChar,
                                               char *ptrReplacementText,
                                               size_t *ptrReplacementLength )
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
