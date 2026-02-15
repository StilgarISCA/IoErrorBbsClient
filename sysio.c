/* System I/O routines.
 */

#include "defs.h"
#include "ext.h"
#include <stdarg.h>

char swork[BUFSIZ]; /* temp buffer for color stripping */

/* stdPutChar() and capPutChar() write a single character to stdout and the
 * capture file, respectively.  On error, they terminate the client.
 */
int stdPutChar( int inputChar )
{
   if ( putchar( inputChar ) < 0 )
   {
      fatalPerror( "stdPutChar", "Local error" );
   }
   capPutChar( inputChar );
   return inputChar;
}

int capPutChar( int inputChar )
{
   static int skipansi = 0; /* Counter for avoidance of capturing ANSI */

   if ( skipansi )
   {
      skipansi--;
      if ( skipansi == 1 )
      {
         if ( flagsConfiguration.shouldDisableBold && inputChar == 109 )
         { /* Damned weird kludge */
            printf( "\033[0m" );
            skipansi--;
         }
         else
         {
            lastColor = (char)inputChar;
         }
      }
   }
   else if ( inputChar == '\033' )
   {
      skipansi = 4;
   }
   else if ( capture > 0 && !flagsConfiguration.isPosting &&
             !flagsConfiguration.isMorePromptActive &&
             inputChar != '\r' )
   {
      if ( putc( inputChar, tempFile ) < 0 )
      {
         tempFileError();
      }
   }
   return inputChar;
}

int netPutChar( int inputChar )
{
   return ( netput( inputChar ) );
}

/* stripAnsi removes ANSI aryEscape sequences from a aryString.  Limits: aryString
 * buffer space is BUFSIZ bytes, should not overflow this!!
 */
char *stripAnsi( char *ptrText, size_t bufferSize )
{
   const char *ptrRead;
   char *ptrWrite;
   ptrWrite = swork;
   for ( ptrRead = ptrText; *ptrRead != '\0'; ptrRead++ )
   {
      if ( *ptrRead != '\033' )
      {
         *ptrWrite++ = *ptrRead;
      }
      else
      {
         for ( ; *ptrRead != '\0' && !isalpha( *ptrRead ); ptrRead++ )
         {
            ;
         }
      }
   }
   if ( *ptrRead == '\r' )
   { /* strip ^M too while we're here */
      ptrWrite--;
   }
   *ptrWrite = '\0';
   if ( bufferSize > 0 )
   {
      snprintf( ptrText, bufferSize, "%s", swork );
   }
   return ptrText;
}

/* stdPuts and capPuts write a aryString to stdout.  They differ from libc *puts
 * in that they do NOT write a trailing \n to the stream.  On error, they
 * terminate the client.
 */
int stdPuts( const char *ptrText )
{
   printf( "%s", ptrText );
   fflush( stdout );
   capPuts( ptrText );
   return 1;
}

int capPuts( const char *ptrText )
{
   if ( capture > 0 && !flagsConfiguration.isPosting && !flagsConfiguration.isMorePromptActive )
   {
      char aryBuffer[BUFSIZ];
      snprintf( aryBuffer, sizeof( aryBuffer ), "%s", ptrText );
      stripAnsi( aryBuffer, sizeof( aryBuffer ) );
      fprintf( tempFile, "%s", aryBuffer );
      fflush( tempFile );
   }
   return 1;
}

int netPuts( const char *ptrText )
{
   const char *ptrRead;

   for ( ptrRead = ptrText; *ptrRead; ptrRead++ )
   {
      netput( *ptrRead );
   }
   return 1;
}

/* stdPrintf and capPrintf print a formatted aryString to stdout, exactly as
 * libc *printf.
 */
int stdPrintf( const char *format, ... )
{
   /* Know what sucks?  I can't really call capPrintf directly... */
   char aryString[BUFSIZ];
   va_list ap;

   va_start( ap, format );
#if defined( __clang__ )
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wformat-nonliteral"
#endif
   (void)vsnprintf( aryString, sizeof( aryString ), format, ap );
#if defined( __clang__ )
#pragma clang diagnostic pop
#endif
   va_end( ap );
   return stdPuts( aryString );
}

int capPrintf( const char *format, ... )
{
   char aryString[BUFSIZ];
   va_list ap;

   if ( capture )
   {
      va_start( ap, format );
#if defined( __clang__ )
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wformat-nonliteral"
#endif
      (void)vsnprintf( aryString, sizeof( aryString ), format, ap );
#if defined( __clang__ )
#pragma clang diagnostic pop
#endif
      va_end( ap );
      return capPuts( aryString );
   }
   return 1;
}

int netPrintf( const char *format, ... )
{
   va_list ap;
   static char work[BUFSIZ];

   va_start( ap, format );
#if defined( __clang__ )
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wformat-nonliteral"
#endif
   (void)vsnprintf( work, sizeof( work ), format, ap );
#if defined( __clang__ )
#pragma clang diagnostic pop
#endif
   va_end( ap );
   netPuts( work );
   return 1;
}
