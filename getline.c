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
#include "ext.h"

#define MAX_ALIAS_INPUT_LENGTH 19
#define MAX_USER_NAME_INPUT_LENGTH 40

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

#if DEBUG
   stdPrintf( " %d SX == %d}\r\n", which, sendingXState );
#endif
   if ( isAway && sendingXState == SX_SENT_NAME )
   {
      sendingXState = SX_REPLYING;
#if DEBUG
      stdPrintf( "getFiveLines 1 sendingXState is %d, xland is %d\r\n", sendingXState, isXland );
#endif
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
#if DEBUG
   stdPrintf( "getFiveLines 2 sendingXState is %d, xland is %d\r\n", sendingXState, isXland );
#endif
   if ( flagsConfiguration.useAnsi )
   {
      stdPrintf( "\033[3%cm", color.input1 );
   }
   for ( lineIndex = 0; lineIndex < ( 20 + override + local ) && ( !lineIndex || *arySendString[lineIndex - 1] ); lineIndex++ )
   {
      stdPrintf( ">" );
      getString( 78, arySendString[lineIndex], lineIndex );
      if ( ( which && !strcmp( arySendString[lineIndex], "ABORT" ) ) || ( !( which & 16 ) && !lineIndex && !strcmp( *arySendString, "PING" ) ) )
      {
         lineIndex++;
         break;
      }
      else if ( ( which & 2 ) && !( which & 8 ) && !strcmp( arySendString[lineIndex], "OVERRIDE" ) )
      {
         stdPrintf( "Override on.\r\n" );
         override++;
      }
      else if ( ( which & 4 ) && !lineIndex && !strcmp( *arySendString, "BEEPS" ) )
      {
         lineIndex++;
         break;
      }
      else if ( ( which & 8 ) && !strcmp( arySendString[lineIndex], "LOCAL" ) )
      {
         stdPrintf( "Only broadcasting to local users.\r\n" );
         local++;
      }
   }
   sendBlock();
   if ( !strcmp( *arySendString, "PING" ) )
   {
      arySendString[0][0] = 0; /* please let this go isAway soon */
   }
   for ( sendLineIndex = 0; sendLineIndex < lineIndex; sendLineIndex++ )
   {
      for ( sendCharIndex = 0; arySendString[sendLineIndex][sendCharIndex]; sendCharIndex++ )
      {
         netPutChar( arySendString[sendLineIndex][sendCharIndex] );
      }
      netPutChar( '\n' );
      byte += sendCharIndex + 1;
   }
   if ( flagsConfiguration.useAnsi )
   {
      stdPrintf( "\033[3%cm", lastColor = color.text ); /* assignment */
   }
}

/*
 * Find a unique matching name to the input entered so far by the user.
 */
int smartName( char *ptrBuffer, char *ptrEnd )
{
   unsigned int itemIndex;
   int found = -1;
   const char *ptrFriend = NULL;
   const char *ptrNextFriend;
   char hold = *ptrEnd;
   slist *listToUse;

   *ptrEnd = 0;
   listToUse = whoList;
   {
      size_t bufferLength = strlen( ptrBuffer );
      for ( itemIndex = 0; itemIndex < listToUse->nitems; itemIndex++ )
      {
         ptrFriend = listToUse->items[itemIndex];
         if ( !strncmp( ptrFriend, ptrBuffer, bufferLength ) )
         { /* Partial match? */
            /* Partial match unique? */
            if ( itemIndex + 1 >= listToUse->nitems )
            {
               found = (int)itemIndex;
               break;
            }
            else
            {
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

void smartPrint( const char *ptrBuffer, const char *ptrEnd )
{
   const char *ptrScan = ptrEnd;

   for ( ; ptrScan > ptrBuffer; ptrScan-- )
   {
      putchar( '\b' );
   }
   if ( flagsConfiguration.useAnsi )
   {
      stdPrintf( "\033[3%cm", color.input1 );
   }
   for ( ; *ptrScan != 0; ptrScan++ )
   {
      if ( ptrScan == ptrEnd && flagsConfiguration.useAnsi )
      {
         stdPrintf( "\033[3%cm", color.input2 );
      }
      putchar( *ptrScan );
   }
   for ( ; ptrScan != ptrEnd; ptrScan-- )
   {
      putchar( '\b' );
   }
   if ( flagsConfiguration.useAnsi )
   {
      stdPrintf( "\033[3%cm", color.input1 );
   }
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
   register char *ptrCursor;
   static char aryNameBuffer[MAX_USER_NAME_INPUT_LENGTH + 1];
   register int inputChar;
   int smart = 0;
   int shouldUppercase;
   int isFirstChar;
   unsigned int invalid = 0;
   static char junk[21];

#if DEBUG
   stdPrintf( " %d SX = %d} ", quitPriv, sendingXState );
#endif
   lastPtr = 0;
   if ( flagsConfiguration.useAnsi )
   {
      stdPrintf( "\033[3%cm", color.input1 );
   }
   if ( quitPriv == 1 && *aryAutoName && strcmp( aryAutoName, "NONE" ) && !isAutoLoggedIn )
   {
      isAutoLoggedIn = 1;
      snprintf( junk, sizeof( junk ), "%s", aryAutoName );
      stdPrintf( "%s\r\n", junk );
      return junk;
   }
   if ( ( isAway || isXland ) && quitPriv == 2 && sendingXState == SENDING_X_STATE_SENT_COMMAND_X )
   {
      sendingXState = SX_SENT_NAME;
#if DEBUG
      stdPrintf( "getName 1 sendingXState is %d, xland is %d\r\n", sendingXState, isXland );
#endif
      if ( !popQueue( junk, xlandQueue ) )
      {
         stdPrintf( "ACK!  It didn't pop.\r\n" );
      }
      if ( flagsConfiguration.useAnsi )
      {
         stdPrintf( "\033[3%cm", lastColor );
      }
      stdPrintf( "\rAutomatic reply to %s                     \r\n", junk );
      return ( junk );
   }
   sendingXState = SX_NOT;
#if DEBUG
   stdPrintf( "getName 2 sendingXState is %d, xland is %d\r\n", sendingXState, isXland );
#endif
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
         if ( inputChar == 14 /* CTRL_N */ )
         {
            if ( smart )
            {
               smartErase( ptrCursor );
               smart = 0;
            }
            for ( ; ptrCursor > aryNameBuffer; --ptrCursor )
            {
               printf( "\b \b" );
            }
            printf( "%s", aryLastName[lastPtr] );
            snprintf( aryNameBuffer, sizeof( aryNameBuffer ), "%s", aryLastName[lastPtr] );
            if ( ++lastPtr == 20 || aryLastName[lastPtr][0] == 0 )
            {
               lastPtr = 0;
            }
            for ( ptrCursor = aryNameBuffer; *ptrCursor != '\0'; ptrCursor++ )
            {
               ;
            }
            continue;
         }
         if ( inputChar == 16 /* CTRL_P */ )
         {
            if ( smart )
            {
               smartErase( ptrCursor );
               smart = 0;
            }
            for ( ; ptrCursor > aryNameBuffer; --ptrCursor )
            {
               printf( "\b \b" );
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
            snprintf( aryNameBuffer, sizeof( aryNameBuffer ), "%s", aryLastName[lastPtr] );
            if ( ++lastPtr == 20 || aryLastName[lastPtr][0] == 0 )
            {
               lastPtr = 0;
            }
            for ( ptrCursor = aryNameBuffer; *ptrCursor != 0; ptrCursor++ )
            {
               ;
            }
            continue;
         }
         if ( inputChar == ' ' && ( isFirstChar || shouldUppercase ) )
         {
            continue;
         }
         if ( inputChar == '\b' || inputChar == CTRL_X || inputChar == CTRL_W || inputChar == CTRL_R || inputChar == ' ' || isalpha( inputChar ) || ( isdigit( inputChar ) && quitPriv == 3 ) )
         {
            invalid = 0;
         }
         else
         {
            if ( invalid++ )
            {
               flushInput( invalid );
            }
            continue;
         }
         if ( inputChar == CTRL_R )
         {
            *ptrCursor = 0;
            printf( "\r\n%s", aryNameBuffer );
            continue;
         }
         do
         {
            if ( ( inputChar == '\b' || inputChar == CTRL_X || inputChar == CTRL_W ) && ptrCursor > aryNameBuffer )
            {
               printf( "\b \b" );
               --ptrCursor;
               if ( smart == 1 )
               {
                  smartErase( aryNameBuffer );
                  smart = 0;
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
            else if ( ptrCursor < &aryNameBuffer[!quitPriv || quitPriv == 3 ? MAX_USER_NAME_INPUT_LENGTH : MAX_ALIAS_INPUT_LENGTH] && ( isalpha( inputChar ) || inputChar == ' ' || ( isdigit( inputChar ) && quitPriv == 3 ) ) )
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
               if ( quitPriv == 2 || quitPriv == -999 )
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
         if ( flagsConfiguration.useAnsi )
         {
            stdPrintf( "\033[3%cm", color.input1 );
         }
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

   if ( quitPriv == 1 && strcmp( aryNameBuffer, "Guest" ) && strcmp( aryAutoName, "NONE" ) )
   {
      snprintf( aryAutoName, sizeof( aryAutoName ), "%s", aryNameBuffer );
      writeBbsRc();
   }
   return ( aryNameBuffer );
}

/*
 * Gets a generic string of length length, puts it in result and returns a a
 * pointer to result.  If the length given is negative, the string is echoed
 * with '.' instead of the character typed (used for passwords)
 */
void getString( int length, char *result, int line )
{
   static char wrap[80];
   char *rest;
   register char *ptrCursor = result;
   register char *ptrWordStart;
   register int inputChar;
   int hidden;
   unsigned int invalid = 0;

   if ( line <= 0 )
   {
      *wrap = 0;
   }
   else if ( *wrap )
   {
      printf( "%s", wrap );
      {
         int maxlen = length;
         if ( maxlen < 0 )
         {
            maxlen = -maxlen;
         }
         if ( maxlen > 0 )
         {
            snprintf( result, (size_t)maxlen + 1, "%s", wrap );
         }
         else
         {
            *result = 0;
         }
      }
      ptrCursor = result + strlen( result );
      *wrap = 0;
   }
   hidden = 0;
   if ( length < 0 )
   {
      length = 0 - length;
      hidden = length;
   }
   /* Kludge here, since some C compilers too stupid to understand 'signed' */
   if ( length > 128 )
   {
      length = 256 - length;
      hidden = length;
   }
#ifdef ENABLE_SAVE_PASSWORD
   if ( hidden != 0 && *aryAutoPassword )
   {
      if ( !isAutoPasswordSent )
      {
         jhpdecode( result, aryAutoPassword, strlen( aryAutoPassword ) );
         {
            size_t rlen = strlen( result );
            for ( size_t charIndex = 0; charIndex < rlen; charIndex++ )
               stdPutChar( '.' );
         }
         stdPrintf( "\r\n" );
         isAutoPasswordSent = 1;
         return;
      }
   }
#endif
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
      if ( inputChar < ' ' && inputChar != '\b' && inputChar != CTRL_X && inputChar != CTRL_W && inputChar != CTRL_R )
      {
         if ( invalid++ )
         {
            flushInput( invalid );
         }
         continue;
      }
      else
      {
         invalid = 0;
      }
      if ( inputChar == CTRL_R )
      {
         *ptrCursor = 0;
         if ( !hidden )
         {
            printf( "\r\n%s", result );
         }
         continue;
      }
      if ( inputChar == '\b' || inputChar == CTRL_X )
      {
         if ( ptrCursor == result )
         {
            continue;
         }
         else
         {
            do
            {
               printf( "\b \b" );
               --ptrCursor;
            } while ( inputChar == CTRL_X && ptrCursor > result );
         }
      }
      else if ( inputChar == CTRL_W )
      {
         for ( ptrWordStart = result; ptrWordStart < ptrCursor; ptrWordStart++ )
         {
            if ( *ptrWordStart != ' ' )
            {
               break;
            }
         }
         inputChar = ( ptrWordStart == ptrCursor );
         for ( ; ptrCursor > result && ( !inputChar || ptrCursor[-1] != ' ' ); ptrCursor-- )
         {
            if ( ptrCursor[-1] != ' ' )
            {
               inputChar = 1;
            }
            printf( "\b \b" );
         }
      }
      else if ( ptrCursor < result + length && isprint( inputChar ) )
      {
         *ptrCursor++ = (char)inputChar;
         if ( !hidden )
         {
            putchar( inputChar );
         }
         else
         {
            putchar( '.' );
         }
      }
      else if ( line < 0 || line == 19 )
      {
         continue;
      }
      else
      {
         if ( inputChar == ' ' )
         {
            break;
         }
         for ( ptrWordStart = ptrCursor - 1; *ptrWordStart != ' ' && ptrWordStart > result; ptrWordStart-- )
         {
            ;
         }
         if ( ptrWordStart > result )
         {
            *ptrWordStart = 0;
            for ( rest = wrap, ptrWordStart++; ptrWordStart < ptrCursor; printf( "\b \b" ) )
            {
               *rest++ = *ptrWordStart++;
            }
            *rest++ = (char)inputChar;
            *rest = 0;
         }
         else
         {
            *wrap = (char)inputChar;
            *( wrap + 1 ) = 0;
         }
         break;
      }
   }
   *ptrCursor = 0;
   if ( !hidden )
   {
      capPuts( result );
   }
   else
   {
      size_t rlen = strlen( result );
      for ( size_t charIndex = 0; charIndex < rlen; charIndex++ )
      {
         capPutChar( '.' );
      }
   }
#ifdef ENABLE_SAVE_PASSWORD
   if ( hidden != 0 )
   {
      jhpencode( aryAutoPassword, result, strlen( result ) );
      writeBbsRc();
   }
#endif
   stdPrintf( "\r\n" );
}
