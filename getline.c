/*
 * Copyright (C) 2024-2026 Stilgar
 * Copyright (C) 1995-2003 Michael Hampton
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

/*
 * This handles getting of short strings or groups of strings of information
 * and sending them off to the BBS.  Even though you might think doing this for
 * an 8 character password is a waste, it does cut down network traffic (and
 * the lag to the end user) to not have to echo back each character
 * individually, and it was easier to do them all rather than only some of
 * them.  Again, this is basically code from the actual BBS with slight
 * modification to work here.
 */
#include "defs.h"
#include "client_globals.h"
#include "color.h"
#include "getline_input.h"
#include "telnet.h"
#include "utility.h"

typedef struct
{
   unsigned int invalid;
   int hidden;
   int length;
} GetStringState;

static char aryWrap[80];

static void applyWrapSeed( int length, int line, char *result, char **ptrCursor );
static bool appendPrintableChar( int inputChar, char *result, char **ptrCursor,
                                 const GetStringState *ptrState );
static bool handleBackspaceEdit( int inputChar, char *result, char **ptrCursor );
static bool handleCtrlWEdit( char *result, char **ptrCursor );
static bool handleGetStringOverflow( int inputChar, int line, char *result,
                                     char **ptrCursor );
static bool maybeUseSavedPassword( char *result, GetStringState *ptrState );
static void normalizeGetStringMode( int *ptrLength, GetStringState *ptrState );
static void recordCapturedString( const char *ptrResult, const GetStringState *ptrState );
static void saveHiddenPasswordIfNeeded( const char *ptrResult,
                                        const GetStringState *ptrState );

static bool appendPrintableChar( int inputChar, char *result, char **ptrCursor,
                                 const GetStringState *ptrState );
static void applyWrapSeed( int length, int line, char *result, char **ptrCursor );
static bool handleBackspaceEdit( int inputChar, char *result, char **ptrCursor );
static bool handleCtrlWEdit( char *result, char **ptrCursor );
static bool handleGetStringOverflow( int inputChar, int line, char *result,
                                     char **ptrCursor );
static bool maybeUseSavedPassword( char *result, GetStringState *ptrState );
static void normalizeGetStringMode( int *ptrLength, GetStringState *ptrState );
static void recordCapturedString( const char *ptrResult, const GetStringState *ptrState );
static void saveHiddenPasswordIfNeeded( const char *ptrResult,
                                        const GetStringState *ptrState );


static bool appendPrintableChar( int inputChar, char *result, char **ptrCursor,
                                 const GetStringState *ptrState )
{
   if ( *ptrCursor >= result + ptrState->length || !isprint( inputChar ) )
   {
      return false;
   }

   **ptrCursor = (char)inputChar;
   ( *ptrCursor )++;
   putchar( ptrState->hidden ? '.' : inputChar );
   return true;
}


static void applyWrapSeed( int length, int line, char *result, char **ptrCursor )
{
   if ( line <= 0 )
   {
      *aryWrap = 0;
   }
   else if ( *aryWrap )
   {
      printf( "%s", aryWrap );
      if ( length > 0 )
      {
         snprintf( result, (size_t)length + 1, "%s", aryWrap );
      }
      else
      {
         *result = 0;
      }
      *ptrCursor = result + strlen( result );
      *aryWrap = 0;
   }
}


/*
 * Used for getting X's and profiles.  'which' tells which of those two we are
 * wanting, to allow the special commands for X's, like PING and ABORT. When
 * we've got what we need, we send it immediately over the net.
 */
void getFiveLines( int which )
{
   register int lineIndex;
   register int sendLineIndex;
   register int sendCharIndex;
   char arySendString[21][80];
   int override = 0;
   int local = 0;

   if ( isAway && sendingXState == SX_SENT_NAME )
   {
      sendingXState = SX_REPLYING;
      replyMessage();
      return;
   }
   if ( isXland )
   {
      sendingXState = SX_SEND_NEXT;
   }
   else
   {
      sendingXState = SX_NOT;
   }
   if ( flagsConfiguration.shouldUseAnsi )
   {
      char aryAnsiSequence[32];

      formatAnsiForegroundSequence( aryAnsiSequence, sizeof( aryAnsiSequence ),
                                    color.inputText );
      stdPrintf( "%s", aryAnsiSequence );
   }
   for ( lineIndex = 0; lineIndex < ( 20 + override + local ) &&
                        ( !lineIndex || *arySendString[lineIndex - 1] );
         lineIndex++ )
   {
      stdPrintf( ">" );
      getString( 78, arySendString[lineIndex], lineIndex );
      if ( ( which && !strcmp( arySendString[lineIndex], "ABORT" ) ) ||
           ( !( which & 16 ) && !lineIndex &&
             !strcmp( *arySendString, "PING" ) ) )
      {
         lineIndex++;
         break;
      }
      else if ( ( which & 2 ) && !( which & 8 ) &&
                !strcmp( arySendString[lineIndex], "OVERRIDE" ) )
      {
         stdPrintf( "Override on.\r\n" );
         override++;
      }
      else if ( ( which & 4 ) && !lineIndex &&
                !strcmp( *arySendString, "BEEPS" ) )
      {
         lineIndex++;
         break;
      }
      else if ( ( which & 8 ) &&
                !strcmp( arySendString[lineIndex], "LOCAL" ) )
      {
         stdPrintf( "Only broadcasting to local users.\r\n" );
         local++;
      }
   }
   sendBlock();
   if ( !strcmp( *arySendString, "PING" ) )
   {
      arySendString[0][0] = 0;
   }
   for ( sendLineIndex = 0; sendLineIndex < lineIndex; sendLineIndex++ )
   {
      sendCharIndex = (int)strlen( arySendString[sendLineIndex] );
      sendTrackedBuffer( arySendString[sendLineIndex], (size_t)sendCharIndex );
      sendTrackedNewline();
   }
   if ( flagsConfiguration.shouldUseAnsi )
   {
      char aryAnsiSequence[32];

      lastColor = color.text;
      formatAnsiForegroundSequence( aryAnsiSequence, sizeof( aryAnsiSequence ),
                                    lastColor );
      stdPrintf( "%s", aryAnsiSequence );
   }
}


/*
 * Gets a generic string of length length, puts it in result and returns a a
 * pointer to result.  If the length given is negative, the string is echoed
 * with '.' instead of the character typed (used for passwords)
 */
void getString( int length, char *result, int line )
{
   GetStringState state;
   char *ptrCursor = result;
   register int inputChar;

   normalizeGetStringMode( &length, &state );
   applyWrapSeed( length, line, result, &ptrCursor );
   if ( maybeUseSavedPassword( result, &state ) )
   {
      return;
   }

   while ( true )
   {
      inputChar = inKey();
      if ( inputChar == ' ' && length == 29 && ptrCursor == result )
      {
         break;
      }
      if ( inputChar == '\n' )
      {
         break;
      }
      if ( inputChar < ' ' && inputChar != '\b' &&
           inputChar != CTRL_X && inputChar != CTRL_W )
      {
         handleInvalidInput( &state.invalid );
         continue;
      }
      else
      {
         state.invalid = 0;
      }
      if ( handleBackspaceEdit( inputChar, result, &ptrCursor ) )
      {
         continue;
      }
      if ( inputChar == CTRL_W )
      {
         handleCtrlWEdit( result, &ptrCursor );
         continue;
      }
      if ( appendPrintableChar( inputChar, result, &ptrCursor, &state ) )
      {
         continue;
      }
      if ( line < 0 || line == 19 )
      {
         continue;
      }
      if ( !handleGetStringOverflow( inputChar, line, result, &ptrCursor ) )
      {
         break;
      }
   }
   *ptrCursor = 0;
   recordCapturedString( result, &state );
   saveHiddenPasswordIfNeeded( result, &state );
   stdPrintf( "\r\n" );
}


static bool handleBackspaceEdit( int inputChar, char *result, char **ptrCursor )
{
   if ( inputChar != '\b' && inputChar != CTRL_X )
   {
      return false;
   }
   if ( *ptrCursor == result )
   {
      return true;
   }

   do
   {
      printf( "\b \b" );
      --( *ptrCursor );
   } while ( inputChar == CTRL_X && *ptrCursor > result );
   return true;
}


static bool handleCtrlWEdit( char *result, char **ptrCursor )
{
   char *ptrWordStart;
   int foundWord;

   for ( ptrWordStart = result; ptrWordStart < *ptrCursor; ptrWordStart++ )
   {
      if ( *ptrWordStart != ' ' )
      {
         break;
      }
   }
   foundWord = ( ptrWordStart == *ptrCursor );
   for ( ; *ptrCursor > result &&
           ( !foundWord || ( *ptrCursor )[-1] != ' ' );
         ( *ptrCursor )-- )
   {
      if ( ( *ptrCursor )[-1] != ' ' )
      {
         foundWord = 1;
      }
      printf( "\b \b" );
   }

   return true;
}


static bool handleGetStringOverflow( int inputChar, int line, char *result,
                                     char **ptrCursor )
{
   char *ptrWordStart;
   char *rest;

   if ( line < 0 || line == 19 )
   {
      return true;
   }
   if ( inputChar == ' ' )
   {
      return false;
   }
   for ( ptrWordStart = *ptrCursor - 1;
         ptrWordStart > result && *ptrWordStart != ' ';
         ptrWordStart-- )
   {
      ;
   }
   if ( ptrWordStart > result )
   {
      *ptrWordStart = 0;
      for ( rest = aryWrap, ptrWordStart++; ptrWordStart < *ptrCursor;
            printf( "\b \b" ) )
      {
         *rest++ = *ptrWordStart++;
      }
      *rest++ = (char)inputChar;
      *rest = 0;
   }
   else
   {
      *aryWrap = (char)inputChar;
      *( aryWrap + 1 ) = 0;
   }

   return false;
}


static bool maybeUseSavedPassword( char *result, GetStringState *ptrState )
{
#ifdef ENABLE_SAVE_PASSWORD
   if ( ptrState->hidden != 0 && *aryAutoPassword && !isAutoPasswordSent )
   {
      size_t charIndex;
      size_t resultLength;

      jhpdecode( result, aryAutoPassword, strlen( aryAutoPassword ) );
      resultLength = strlen( result );
      for ( charIndex = 0; charIndex < resultLength; charIndex++ )
      {
         stdPutChar( '.' );
      }
      stdPrintf( "\r\n" );
      isAutoPasswordSent = 1;
      return true;
   }
#else
   (void)result;
   (void)ptrState;
#endif

   return false;
}


static void normalizeGetStringMode( int *ptrLength, GetStringState *ptrState )
{
   ptrState->invalid = 0;
   ptrState->hidden = 0;
   ptrState->length = *ptrLength;

   if ( ptrState->length < 0 )
   {
      ptrState->length = -ptrState->length;
      ptrState->hidden = ptrState->length;
   }
   if ( ptrState->length > 128 )
   {
      ptrState->length = 256 - ptrState->length;
      ptrState->hidden = ptrState->length;
   }

   *ptrLength = ptrState->length;
}


static void recordCapturedString( const char *ptrResult, const GetStringState *ptrState )
{
   size_t charIndex;
   size_t resultLength;

   if ( !ptrState->hidden )
   {
      capPuts( ptrResult );
      return;
   }

   resultLength = strlen( ptrResult );
   for ( charIndex = 0; charIndex < resultLength; charIndex++ )
   {
      capPutChar( '.' );
   }
}


static void saveHiddenPasswordIfNeeded( const char *ptrResult,
                                        const GetStringState *ptrState )
{
#ifdef ENABLE_SAVE_PASSWORD
   if ( ptrState->hidden != 0 )
   {
      jhpencode( aryAutoPassword, ptrResult, strlen( ptrResult ) );
      writeBbsRc();
   }
#else
   (void)ptrResult;
   (void)ptrState;
#endif
}

