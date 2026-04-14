/*
 * Copyright (C) 2024-2026 Stilgar
 * Copyright (C) 1995-2003 Michael Hampton
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "defs.h"
#include "ext.h"
#include "proto.h"

#define MAX_ALIAS_INPUT_LENGTH 19
#define MAX_USER_NAME_INPUT_LENGTH 40

static void clearSmartCompletion( const char *ptrCursor, int *ptrSmart );
static void moveCursorToBufferEnd( char **ptrCursor, char *ptrBuffer );
static void recallNextLastName( char *ptrBuffer, char **ptrCursor, int *ptrSmart );
static void recallPreviousLastName( char *ptrBuffer, char **ptrCursor,
                                    int *ptrSmart );
static void rewindTypedName( char **ptrCursor, const char *ptrBuffer );

/*
 * Find a unique matching name to the input entered so far by the user.
 */
int smartName( char *ptrBuffer, char *ptrEnd )
{
   int found = -1;
   const char *ptrFriend = NULL;
   char hold = *ptrEnd;
   slist *listToUse;

   *ptrEnd = 0;
   listToUse = whoList;
   {
      size_t bufferLength = strlen( ptrBuffer );
      unsigned int itemIndex;

      for ( itemIndex = 0; itemIndex < listToUse->nitems; itemIndex++ )
      {
         ptrFriend = listToUse->items[itemIndex];
         if ( !strncmp( ptrFriend, ptrBuffer, bufferLength ) )
         {
            if ( itemIndex + 1 >= listToUse->nitems )
            {
               found = (int)itemIndex;
               break;
            }
            else
            {
               const char *ptrNextFriend;

               ptrNextFriend = listToUse->items[itemIndex + 1];
               if ( strncmp( ptrNextFriend, ptrBuffer, bufferLength ) )
               {
                  found = (int)itemIndex;
                  break;
               }
               else
               {
                  break;
               }
            }
         }
      }
   }
   if ( found == -1 )
   {
      *ptrEnd = hold;
      return 0;
   }
   else
   {
      snprintf( ptrBuffer, MAX_USER_NAME_INPUT_LENGTH + 1, "%s", ptrFriend );
   }
   return 1;
}

static void clearSmartCompletion( const char *ptrCursor, int *ptrSmart )
{
   if ( *ptrSmart )
   {
      smartErase( ptrCursor );
      *ptrSmart = 0;
   }
}

static void moveCursorToBufferEnd( char **ptrCursor, char *ptrBuffer )
{
   for ( *ptrCursor = ptrBuffer; **ptrCursor != '\0'; ( *ptrCursor )++ )
   {
      ;
   }
}

static void recallNextLastName( char *ptrBuffer, char **ptrCursor, int *ptrSmart )
{
   clearSmartCompletion( *ptrCursor, ptrSmart );
   rewindTypedName( ptrCursor, ptrBuffer );
   printf( "%s", aryLastName[lastPtr] );
   snprintf( ptrBuffer, MAX_USER_NAME_INPUT_LENGTH + 1, "%s",
             aryLastName[lastPtr] );
   if ( ++lastPtr == 20 || aryLastName[lastPtr][0] == 0 )
   {
      lastPtr = 0;
   }
   moveCursorToBufferEnd( ptrCursor, ptrBuffer );
}

static void recallPreviousLastName( char *ptrBuffer, char **ptrCursor,
                                    int *ptrSmart )
{
   clearSmartCompletion( *ptrCursor, ptrSmart );
   rewindTypedName( ptrCursor, ptrBuffer );
   if ( --lastPtr < 0 )
   {
      for ( lastPtr = 19; lastPtr > 0; --lastPtr )
      {
         if ( aryLastName[lastPtr][0] != 0 )
         {
            break;
         }
      }
   }
   if ( --lastPtr < 0 )
   {
      for ( lastPtr = 19; lastPtr > 0; --lastPtr )
      {
         if ( aryLastName[lastPtr][0] != 0 )
         {
            break;
         }
      }
   }
   printf( "%s", aryLastName[lastPtr] );
   snprintf( ptrBuffer, MAX_USER_NAME_INPUT_LENGTH + 1, "%s",
             aryLastName[lastPtr] );
   if ( ++lastPtr == 20 || aryLastName[lastPtr][0] == 0 )
   {
      lastPtr = 0;
   }
   moveCursorToBufferEnd( ptrCursor, ptrBuffer );
}

static void rewindTypedName( char **ptrCursor, const char *ptrBuffer )
{
   for ( ; *ptrCursor > ptrBuffer; --( *ptrCursor ) )
   {
      printf( "\b \b" );
   }
}

void smartPrint( const char *ptrBuffer, const char *ptrEnd )
{
   const char *ptrScan = ptrEnd;

   for ( ; ptrScan > ptrBuffer; ptrScan-- )
   {
      putchar( '\b' );
   }
   printAnsiForegroundColorValue( color.inputText );
   for ( ; *ptrScan != 0; ptrScan++ )
   {
      if ( ptrScan == ptrEnd && flagsConfiguration.shouldUseAnsi )
      {
         printAnsiForegroundColorValue( color.inputHighlight );
      }
      putchar( *ptrScan );
   }
   for ( ; ptrScan != ptrEnd; ptrScan-- )
   {
      putchar( '\b' );
   }
   printAnsiForegroundColorValue( color.inputText );
}

void smartErase( const char *ptrEnd )
{
   const char *ptrScan = ptrEnd;

   for ( ; *ptrScan != 0; ptrScan++ )
   {
      putchar( ' ' );
   }
   for ( ; ptrScan != ptrEnd; ptrScan-- )
   {
      putchar( '\b' );
   }
}

/*
 * Used for getting names (user names, room names, etc.)  Capitalizes first
 * letter of word automatically)  Does different things depending on the value
 * of quitPriv (that stuff should be left alone)  The name is then returned to
 * the caller.
 */
char *getName( int quitPriv )
{
   char *ptrCursor;
   static char aryNameBuffer[MAX_USER_NAME_INPUT_LENGTH + 1];
   register int inputChar;
   int smart = 0;
   int shouldUppercase;
   int isFirstChar;
   unsigned int invalid = 0;
   static char junk[21];

   lastPtr = 0;
   printAnsiForegroundColorValue( color.inputText );
   if ( quitPriv == 1 && *aryAutoName &&
        strcmp( aryAutoName, "NONE" ) && !isAutoLoggedIn )
   {
      isAutoLoggedIn = 1;
      snprintf( junk, sizeof( junk ), "%s", aryAutoName );
      stdPrintf( "%s\r\n", junk );
      return junk;
   }
   if ( ( isAway || isXland ) && quitPriv == 2 &&
        sendingXState == SENDING_X_STATE_SENT_COMMAND_X )
   {
      sendingXState = SX_SENT_NAME;
      if ( !popQueue( junk, xlandQueue ) )
      {
         stdPrintf( "ACK!  It didn't pop.\r\n" );
      }
      printAnsiForegroundColorValue( lastColor );
      stdPrintf( "\rAutomatic reply to %s                     \r\n", junk );
      return ( junk );
   }
   sendingXState = SX_NOT;
   while ( true )
   {
      shouldUppercase = 1;
      isFirstChar = 1;
      ptrCursor = aryNameBuffer;
      while ( true )
      {
         inputChar = inKey();
         if ( inputChar == '\n' )
         {
            break;
         }
         if ( inputChar == CTRL_D && quitPriv == 1 )
         {
            aryNameBuffer[0] = CTRL_D;
            aryNameBuffer[1] = 0;
            return ( aryNameBuffer );
         }
         if ( inputChar == '_' )
         {
            inputChar = ' ';
         }
         if ( inputChar == 14 )
         {
            recallNextLastName( aryNameBuffer, &ptrCursor, &smart );
            continue;
         }
         if ( inputChar == 16 )
         {
            recallPreviousLastName( aryNameBuffer, &ptrCursor, &smart );
            continue;
         }
         if ( inputChar == ' ' && ( isFirstChar || shouldUppercase ) )
         {
            continue;
         }
         if ( inputChar == '\b' || inputChar == CTRL_X ||
              inputChar == CTRL_W || inputChar == ' ' ||
              isalpha( inputChar ) ||
              ( isdigit( inputChar ) && quitPriv == 3 ) )
         {
            invalid = 0;
         }
         else
         {
            handleInvalidInput( &invalid );
            continue;
         }
         do
         {
            if ( ( inputChar == '\b' || inputChar == CTRL_X ||
                   inputChar == CTRL_W ) &&
                 ptrCursor > aryNameBuffer )
            {
               printf( "\b \b" );
               --ptrCursor;
               if ( smart == 1 )
               {
                  clearSmartCompletion( aryNameBuffer, &smart );
               }
               shouldUppercase = ( ptrCursor == aryNameBuffer || *( ptrCursor - 1 ) == ' ' );
               if ( shouldUppercase && inputChar == CTRL_W )
               {
                  break;
               }
               if ( ptrCursor == aryNameBuffer )
               {
                  isFirstChar = 1;
               }
            }
            else if ( ptrCursor <
                         &aryNameBuffer[!quitPriv || quitPriv == 3 ? MAX_USER_NAME_INPUT_LENGTH : MAX_ALIAS_INPUT_LENGTH] &&
                      ( isalpha( inputChar ) || inputChar == ' ' ||
                        ( isdigit( inputChar ) && quitPriv == 3 ) ) )
            {
               isFirstChar = 0;
               if ( shouldUppercase && isupper( inputChar ) )
               {
                  --shouldUppercase;
               }
               if ( shouldUppercase && islower( inputChar ) )
               {
                  inputChar -= 32;
                  --shouldUppercase;
               }
               if ( inputChar == ' ' )
               {
                  shouldUppercase = 1;
               }
               *ptrCursor++ = (char)inputChar;
               putchar( inputChar );
               if ( flagsConfiguration.shouldEnableNameAutocomplete &&
                    ( quitPriv == 2 || quitPriv == -999 ) )
               {
                  if ( smartName( aryNameBuffer, ptrCursor ) )
                  {
                     smartPrint( aryNameBuffer, ptrCursor );
                     smart = 1;
                  }
                  else if ( smart == 1 )
                  {
                     smartErase( ptrCursor );
                     smart = 0;
                  }
               }
            }
         } while ( ( inputChar == CTRL_X || inputChar == CTRL_W ) && ptrCursor > aryNameBuffer );
      }
      if ( smart == 0 )
      {
         *ptrCursor = 0;
      }
      else
      {
         printAnsiForegroundColorValue( color.inputText );
         for ( ; *ptrCursor != 0; ptrCursor++ )
         {
            putchar( *ptrCursor );
         }
      }
      break;
   }
   capPuts( aryNameBuffer );
   if ( ptrCursor > aryNameBuffer || quitPriv >= 2 )
   {
      stdPrintf( "\r\n" );
   }

   if ( ptrCursor > aryNameBuffer && ptrCursor[-1] == ' ' )
   {
      ptrCursor[-1] = 0;
   }

   if ( quitPriv == 1 && strcmp( aryNameBuffer, "Guest" ) &&
        strcmp( aryAutoName, "NONE" ) )
   {
      snprintf( aryAutoName, sizeof( aryAutoName ), "%s", aryNameBuffer );
      writeBbsRc();
   }
   return ( aryNameBuffer );
}
