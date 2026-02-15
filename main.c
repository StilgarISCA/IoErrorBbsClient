/*
 * Program entry point.
 */
#include "defs.h"
#include "ext.h"

int main( int argc, char *argv[] )
{
   aryEscape[0] = '\033';
   aryEscape[1] = '\0';
   if ( *argv[0] == '-' )
   {
      isLoginShell = 1;
   }
   else
   {
      isLoginShell = 0;
   }
   initialize( "v1.5" );
   findHome();
   readBbsRc();
   openTmpFile();
   arguments( argc, argv );
   connectBbs();
   sigInit();
   telInit();
   setTerm();
   looper();
   exit( 0 );
   return ( 0 );
}
