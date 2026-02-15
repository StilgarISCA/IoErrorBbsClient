/*
 * Various utility routines that didn't really belong elsewhere.  Yawn.
 */
#include "defs.h"
#include "ext.h"

/* replyaway routines to reply to X's when you are isAway from keyboard */
/* these globals used only in this file, so let 'em stay here */
/* Please do not change this message; it's used for reply suppression
   * (see below).  If you alter this, you will draw the ire of the ISCA
   * BBS programmers.  Trust me, I know.  :)
   */
char replymsg[5] = "+!R ";

void sendAnX( void )
{
   /* get the ball rolling with the bbs */
   sendingXState = SX_WANT_TO;
#if DEBUG
   stdPrintf( "sendAnX 1 sendingXState is %d, xland is %d\r\n", sendingXState, isXland );
#endif
   netPutChar( 'x' );
   byte++;
   sendingXState = SENDING_X_STATE_SENT_COMMAND_X;
#if DEBUG
   stdPrintf( "sendAnX 2 sendingXState is %d, xland is %d\r\n", sendingXState, isXland );
#endif
}

/* fake getFiveLines for the bbs */
void replyMessage( void )
{
   int lineIndex;
   int charIndex;

   sendBlock();
   for ( lineIndex = 0; replymsg[lineIndex]; lineIndex++ )
   {
      netPutChar( replymsg[lineIndex] );
   }
   byte += lineIndex;
   for ( lineIndex = 0; lineIndex < 5 && *aryAwayMessageLines[lineIndex]; lineIndex++ )
   {
      for ( charIndex = 0; aryAwayMessageLines[lineIndex][charIndex]; charIndex++ )
      {
         netPutChar( aryAwayMessageLines[lineIndex][charIndex] );
      }
      netPutChar( '\n' );
      byte += charIndex + 1;
      stdPrintf( "%s\r\n", aryAwayMessageLines[lineIndex] );
   }
   if ( lineIndex < 5 )
   { /* less than five lines */
      netPutChar( '\n' );
      byte++;
   }
   sendingXState = SX_NOT;
#if DEBUG
   stdPrintf( "replyMessage 1 sendingXState is %d, xland is %d\r\n", sendingXState, isXland );
#endif
}

void fatalPerror( const char *error, const char *heading )
{
   fflush( stdout );
   sPerror( error, heading );
   myExit();
}

void fatalExit( const char *message, const char *heading )
{
   fflush( stdout );
   sError( message, heading );
   myExit();
}

void myExit( void )
{
   fflush( stdout );
   if ( childPid )
   {
      /* Wait for child to terminate */
      sigOff();
      childPid = ( -childPid );
      while ( childPid )
      {
         sigpause( 0 );
      }
   }
   resetTerm();
#ifdef HAVE_OPENSSL
   if ( isSsl )
      killSsl();
#endif
   if ( flagsConfiguration.isLastSave )
   {
      if ( !( tempFile = freopen( aryTempFileName, "w+", tempFile ) ) )
      {
         sPerror( "myExit: reopen temp file before exit", "Shutdown warning" );
      }
   }
   deinitialize();
   exit( 0 );
}

void looper( void )
{
   register int inputChar;
   unsigned int invalid = 0;

   while ( true )
   {
      if ( ( inputChar = inKey() ) < 0 )
      {
         return;
      }
      /* Don't bother sending stuff to the bbs it won't use anyway */
      if ( ( inputChar >= 32 && inputChar <= 127 ) || findChar( "\3\4\5\b\n\r\27\30\32", inputChar ) )
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
      else if ( invalid++ )
      {
         flushInput( invalid );
      }
   }
}

int yesNo( void )
{
   register int inputChar;
   unsigned int invalid = 0;

   while ( !findChar( "nNyY", inputChar = inKey() ) )
   {
      if ( invalid++ )
      {
         flushInput( invalid );
      }
   }
   if ( inputChar == 'y' || inputChar == 'Y' )
   {
      stdPrintf( "Yes\r\n" );
      return ( 1 );
   }
   else
   {
      stdPrintf( "No\r\n" );
      return ( 0 );
   }
}

int yesNoDefault( int defaultAnswer )
{
   register int inputChar;
   unsigned int invalid = 0;

   while ( !findChar( "nNyY\n ", inputChar = inKey() ) )
   {
      if ( invalid++ )
      {
         flushInput( invalid );
      }
   }
   if ( inputChar == '\n' || inputChar == ' ' )
   {
      inputChar = ( defaultAnswer ? 'Y' : 'N' );
   }
   if ( inputChar == 'y' || inputChar == 'Y' )
   {
      stdPrintf( "Yes\r\n" );
      return ( 1 );
   }
   else if ( inputChar == 'n' || inputChar == 'N' )
   {
      stdPrintf( "No\r\n" );
      return ( 0 );
   }
   else
   { /* This should never happen, means bug in findChar() */
      char aryBuffer[160];
      stdPrintf( "\r\n" );
      snprintf( aryBuffer, sizeof( aryBuffer ), "yesNoDefault: 0x%x\r\n"
                                                "Please report this to IO ERROR\r\n",
                inputChar );
      fatalExit( aryBuffer, "Internal error" );
   }
   return 0;
}

void tempFileError( void )
{
   if ( errno == EINTR )
   {
      return;
   }
   fprintf( stderr, "\r\n" );
   sPerror( "writing tempfile", "Local error" );
}

int more( int *line, int percentComplete )
{
   register int inputChar;
   unsigned int invalid = 0;

   if ( percentComplete >= 0 )
   {
      printf( "--MORE--(%d%%)", percentComplete );
   }
   else
   {
      printf( "--MORE--" );
   }
   while ( true )
   {
      inputChar = inKey();
      if ( inputChar == ' ' || inputChar == 'y' || inputChar == 'Y' )
      {
         *line = 1;
      }
      else if ( inputChar == '\n' )
      {
         --*line;
      }
      else if ( findChar( "nNqsS", inputChar ) )
      {
         *line = -1;
      }
      else if ( invalid++ )
      {
         flushInput( invalid );
         continue;
      }
      printf( "\r              \r" );
      break;
   }
   return ( *line < 0 ? -1 : 0 );
}

/*
 * Not all systems have strstr(), so I roll my own...
 */
char *findSubstring( const char *ptrString, const char *ptrSubstring )
{
   const char *ptrSearch;

   for ( ptrSearch = ptrString; *ptrSearch; ptrSearch++ )
   {
      if ( *ptrSearch == *ptrSubstring && !strncmp( ptrSearch, ptrSubstring, strlen( ptrSubstring ) ) )
      {
         break;
      }
   }
   if ( !*ptrSearch )
   {
      return ( (char *)NULL );
   }
   else
   {
#if defined( __clang__ )
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wcast-qual"
#endif
      char *ptrResult = (char *)ptrSearch;
#if defined( __clang__ )
#pragma clang diagnostic pop
#endif
      return ptrResult;
   }
}

/*
 * Not all systems have strchr() either (they usually have index() instead, but
 * I don't want to count on that or check for it)
 */
char *findChar( const char *ptrString, int targetChar )
{
   const char *ptrSearch;

   ptrSearch = ptrString;
   while ( *ptrSearch && targetChar != *ptrSearch )
   {
      ptrSearch++;
   }
   if ( *ptrSearch )
   {
#if defined( __clang__ )
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wcast-qual"
#endif
      char *ptrResult = (char *)ptrSearch;
#if defined( __clang__ )
#pragma clang diagnostic pop
#endif
      return ptrResult;
   }
   else
   {
      return ( (char *)NULL );
   }
}

/* extractName -- get the username out of a post or X message header */
/* returns pointer to username as stored in the array */
char *extractName( const char *header )
{
   char *ptrHeaderName;
   char *ptrExtractedName;
   int isAfterSpace;
   int charIndex;
   int existingIndex = -1;

   ptrHeaderName = findSubstring( header, " from " );
   if ( !ptrHeaderName )
   { /* This isn't an X message or a post */
      return NULL;
   }
   ptrHeaderName += 6;
   if ( *ptrHeaderName == '\033' )
   {
      ptrHeaderName += 5;
   }
   /* Now should be pointing to the aryUser name */
   isAfterSpace = 1;
   ptrExtractedName = duplicateString( ptrHeaderName );
   {
      int nameLength = (int)strlen( ptrExtractedName );
      for ( charIndex = 0; charIndex < nameLength; charIndex++ )
      {
         if ( ptrExtractedName[charIndex] == '\033' )
         {
            break;
         }
         if ( isAfterSpace && !isupper( ptrExtractedName[charIndex] ) )
         {
            break;
         }
         if ( ptrExtractedName[charIndex] == ' ' )
         {
            isAfterSpace = 1;
         }
         else
         {
            isAfterSpace = 0;
         }
      }
   }
   ptrExtractedName[charIndex] = '\0';
   charIndex--;
   /* \r courtesy of Sbum, fixed enemy list in non-ANSI mode 2/9/2000 */
   if ( ptrExtractedName[charIndex] == ' ' || ptrExtractedName[charIndex] == '\r' )
   {
      ptrExtractedName[charIndex] = '\0';
   }
   /* Is the name empty? */
   if ( *ptrExtractedName == 0 )
   {
      return NULL;
   }
   /* check for dupes first */
   for ( charIndex = 0; charIndex < MAX_USER_NAME_HISTORY_COUNT; charIndex++ )
   {
      if ( !strcmp( aryLastName[charIndex], ptrExtractedName ) )
      {
         existingIndex = charIndex;
      }
   }
   /* insert the name */
   if ( existingIndex != 0 )
   {
      for ( charIndex = ( existingIndex > 0 ) ? existingIndex - 1 : MAX_USER_NAME_HISTORY_COUNT - 2; charIndex >= 0; --charIndex )
      {
         snprintf( aryLastName[charIndex + 1], sizeof( aryLastName[charIndex + 1] ), "%s", aryLastName[charIndex] );
      }
      snprintf( aryLastName[0], sizeof( aryLastName[0] ), "%s", ptrExtractedName );
   }
   free( ptrExtractedName );
   return (char *)aryLastName[0];
}

/*
 * extractNumber - extract the X message number from an X message header.
 */
int extractNumber( const char *header )
{
   char *ptrMessageNumber;
   int number = 0;

   ptrMessageNumber = findSubstring( header, "(#" );
   if ( !ptrMessageNumber )
   { /* This isn't an X message */
      return 0;
   }

   for ( ptrMessageNumber += 2; *ptrMessageNumber != ')'; ptrMessageNumber++ )
   {
      number += number * 10 + ( *ptrMessageNumber - '0' );
   }

   return number;
}

char *duplicateString( const char *ptrSource )
{
   size_t length;
   char *ptrCopy;

   length = strlen( ptrSource ) + 2;
   ptrCopy = (char *)calloc( 1, length );
   if ( ptrCopy )
   {
      snprintf( ptrCopy, length, "%s", ptrSource );
   }
   return ptrCopy;
}

#define ifansi if ( flagsConfiguration.useAnsi )

int colorize( const char *str )
{
   const char *ptrText;

   for ( ptrText = str; *ptrText; ptrText++ )
   {
      if ( *ptrText == '@' )
      {
         if ( !*( ptrText + 1 ) )
         {
            ptrText--;
         }
         else
         {
            switch ( *++ptrText )
            {
               case '@':
                  putchar( (int)'@' );
                  break;
               case 'k':
                  ifansi printf( "\033[40m" );
                  break;
               case 'K':
                  ifansi printf( "\033[30m" );
                  break;
               case 'r':
                  ifansi printf( "\033[41m" );
                  break;
               case 'R':
                  ifansi printf( "\033[31m" );
                  break;
               case 'g':
                  ifansi printf( "\033[42m" );
                  break;
               case 'G':
                  ifansi printf( "\033[32m" );
                  break;
               case 'y':
                  ifansi printf( "\033[43m" );
                  break;
               case 'Y':
                  ifansi printf( "\033[33m" );
                  break;
               case 'b':
                  ifansi printf( "\033[44m" );
                  break;
               case 'B':
                  ifansi printf( "\033[34m" );
                  break;
               case 'm':
               case 'p':
                  ifansi printf( "\033[45m" );
                  break;
               case 'M':
               case 'P':
                  ifansi printf( "\033[35m" );
                  break;
               case 'c':
                  ifansi printf( "\033[46m" );
                  break;
               case 'C':
                  ifansi printf( "\033[36m" );
                  break;
               case 'w':
                  ifansi printf( "\033[47m" );
                  break;
               case 'W':
                  ifansi printf( "\033[37m" );
                  break;
               case 'd':
                  ifansi printf( "\033[49m" );
                  break;
               case 'D':
                  ifansi printf( "\033[39m" );
                  break;
               default:
                  break;
            }
         }
      }
      else
      {
         stdPutChar( (int)*ptrText );
      }
   }
   return 1;
}

/*
 * Process command line arguments.  argv[1] is an alternate host, if present,
 * and argv[2] is an alternate port, if present, and argv[1] is also present.
 */
void arguments( int argc, char **argv )
{
   if ( argc > 1 )
   {
      snprintf( aryCommandLineHost, sizeof( aryCommandLineHost ), "%s", argv[1] );
   }
   else
   {
      *aryCommandLineHost = 0;
   }
   if ( argc > 2 )
   {
      cmdLinePort = (unsigned short)atoi( argv[2] );
   }
   else
   {
      cmdLinePort = 0;
   }
   if ( argc > 3 )
   {
      if ( !strncmp( argv[3], "secure", 6 ) || !strncmp( argv[3], "ssl", 6 ) )
      {
         shouldUseSsl = 1;
      }
      else
      {
         shouldUseSsl = 0;
      }
   }
}

/*
 * strcmp() wrapper for friend entries; grabs the correct entry from the
 * struct, which is arg 2.
 */
int fStrCompare( const char *ptrName, const friend *ptrFriend )
{
   return strcmp( ptrName, ptrFriend->name );
}

int fStrCompareVoid( const void *ptrName, const void *ptrFriend )
{
   return fStrCompare( (const char *)ptrName, (const friend *)ptrFriend );
}

/*
 * strcmp() wrapper for char entries.
 */
int sortCompare( char **ptrLeft, char **ptrRight )
{
   return strcmp( *ptrLeft, *ptrRight );
}

int sortCompareVoid( const void *ptrLeft, const void *ptrRight )
{
   const char *const *ptrLeftString = (const char *const *)ptrLeft;
   const char *const *ptrRightString = (const char *const *)ptrRight;
   return strcmp( *ptrLeftString, *ptrRightString );
}

int strCompareVoid( const void *ptrLeft, const void *ptrRight )
{
   return strcmp( (const char *)ptrLeft, (const char *)ptrRight );
}

/*
 * strcmp() wrapper for friend entries; takes two friend * args.
 */
int fSortCompare( const friend *const *ptrLeft, const friend *const *ptrRight )
{
   assert( ( *ptrLeft )->magic == 0x3231 );
   assert( ( *ptrRight )->magic == 0x3231 );

   return strcmp( ( *ptrLeft )->name, ( *ptrRight )->name );
}

int fSortCompareVoid( const void *ptrLeft, const void *ptrRight )
{
   return fSortCompare( (const friend *const *)ptrLeft, (const friend *const *)ptrRight );
}

#ifdef ENABLE_SAVE_PASSWORD
/*
 * Encode/decode password with a simple algorithm.
 * jhp 5Feb95 (Marx Marvelous)
 *
 * This code is horribly insecure.  Don't use it for any passwords
 * you care about!  Also note it's closely tied to ASCII and won't
 * work with a non-ASCII system.  - IO
 */
char *jhpencode( char *ptrDestination, const char *src, size_t seedLength )
{
   char *ptrDestIterator; /* dest iterator */
   char inputChar;        /* a single character */

   ptrDestIterator = ptrDestination;
   while ( ( inputChar = *src++ ) != 0 )
   {
      *ptrDestIterator++ = ( inputChar - 32 - seedLength + 95 ) % 95 + 32;
      seedLength = inputChar - 32;
   }
   *ptrDestIterator = 0;
   return ptrDestination;
}

char *jhpdecode( char *ptrDestination, const char *src, size_t seedLength )
{
   char *ptrDestIterator; /* dest iterator */
   char inputChar;        /* a single character */

   ptrDestIterator = ptrDestination;
   while ( ( inputChar = *src++ ) != 0 )
   {
      seedLength = ( seedLength + inputChar - 32 ) % 95;
      *ptrDestIterator++ = seedLength + 32;
   }
   *ptrDestIterator = 0;
   return ptrDestination;
}
#endif
