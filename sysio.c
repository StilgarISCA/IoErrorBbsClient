/* System I/O routines.
 */

#include "defs.h"
#include "ext.h"
#include <stdarg.h>

char swork[BUFSIZ]; /* temp buffer for color stripping */

/* std_putchar() and cap_putchar() write a single character to stdout and the
 * capture file, respectively.  On error, they terminate the client.
 */
int std_putchar( int c )
{
   if ( putchar( c ) < 0 )
   {
      fatalperror( "std_putchar", "Local error" );
   }
   cap_putchar( c );
   return c;
}

int cap_putchar( int c )
{
   static int skipansi = 0; /* Counter for avoidance of capturing ANSI */

   if ( skipansi )
   {
      skipansi--;
      if ( skipansi == 1 )
      {
         if ( flags.offbold && c == 109 )
         { /* Damned weird kludge */
            printf( "\033[0m" );
            skipansi--;
         }
         else
         {
            lastcolor = (char)c;
         }
      }
   }
   else if ( c == '\033' )
   {
      skipansi = 4;
   }
   else if ( capture > 0 && !flags.posting && !flags.moreflag && c != '\r' )
   {
      if ( putc( c, tempfile ) < 0 )
      {
         tempfileerror();
      }
   }
   return c;
}

int net_putchar( int c )
{
   return ( netput( c ) );
}

/* stripansi removes ANSI escape sequences from a string.  Limits: string
 * buffer space is BUFSIZ bytes, should not overflow this!!
 */
char *stripansi( char *c, size_t csize )
{
   const char *p;
   char *q;
   q = swork;
   for ( p = c; *p != '\0'; p++ )
   {
      if ( *p != '\033' )
      {
         *q++ = *p;
      }
      else
      {
         for ( ; *p != '\0' && !isalpha( *p ); p++ )
         {
            ;
         }
      }
   }
   if ( *p == '\r' )
   { /* strip ^M too while we're here */
      q--;
   }
   *q = '\0';
   if ( csize > 0 )
   {
      snprintf( c, csize, "%s", swork );
   }
   return c;
}

/* std_puts and cap_puts write a string to stdout.  They differ from libc *puts
 * in that they do NOT write a trailing \n to the stream.  On error, they
 * terminate the client.
 */
int std_puts( const char *c )
{
   printf( "%s", c );
   fflush( stdout );
   cap_puts( c );
   return 1;
}

int cap_puts( const char *c )
{
   if ( capture > 0 && !flags.posting && !flags.moreflag )
   {
      char buf[BUFSIZ];
      snprintf( buf, sizeof( buf ), "%s", c );
      stripansi( buf, sizeof( buf ) );
      fprintf( tempfile, "%s", buf );
      fflush( tempfile );
   }
   return 1;
}

int net_puts( const char *c )
{
   const char *i;

   for ( i = c; *i; i++ )
   {
      netput( *i );
   }
   return 1;
}

/* std_printf and cap_printf print a formatted string to stdout, exactly as
 * libc *printf.
 */
int std_printf( const char *format, ... )
{
   /* Know what sucks?  I can't really call cap_printf directly... */
   char string[BUFSIZ];
   va_list ap;

   va_start( ap, format );
#if defined( __clang__ )
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wformat-nonliteral"
#endif
   (void)vsnprintf( string, sizeof( string ), format, ap );
#if defined( __clang__ )
#pragma clang diagnostic pop
#endif
   va_end( ap );
   return std_puts( string );
}

int cap_printf( const char *format, ... )
{
   char string[BUFSIZ];
   va_list ap;

   if ( capture )
   {
      va_start( ap, format );
#if defined( __clang__ )
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wformat-nonliteral"
#endif
      (void)vsnprintf( string, sizeof( string ), format, ap );
#if defined( __clang__ )
#pragma clang diagnostic pop
#endif
      va_end( ap );
      return cap_puts( string );
   }
   return 1;
}

int net_printf( const char *format, ... )
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
   net_puts( work );
   return 1;
}
