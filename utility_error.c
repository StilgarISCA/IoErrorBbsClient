/*
 * Copyright (C) 2024-2026 Stilgar
 * Copyright (C) 1995-2003 Michael Hampton
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

/*
 * This file handles fatal errors and shutdown cleanup.
 */
#include "defs.h"
#include "client.h"
#include "client_globals.h"
#include "config_globals.h"
#include "utility.h"

static void reopenTempFileForShutdown( void );
static void waitForChildShutdown( void );

static void reopenTempFileForShutdown( void );
static void waitForChildShutdown( void );


noreturn void fatalExit( const char *message, const char *heading )
{
   fflush( stdout );
   sError( message, heading );
   myExit();
}


noreturn void fatalPerror( const char *error, const char *heading )
{
   int savedErrno = errno;

   fflush( stdout );
   errno = savedErrno;
   sPerror( error, heading );
   myExit();
}


noreturn void myExit( void )
{
   fflush( stdout );
   waitForChildShutdown();
   resetTerm();
#ifdef HAVE_OPENSSL
   if ( isSsl )
   {
      killSsl();
   }
#endif
   reopenTempFileForShutdown();
   deinitialize();
   exit( 0 );
}


static void reopenTempFileForShutdown( void )
{
   if ( flagsConfiguration.isLastSave )
   {
      if ( !( tempFile = freopen( aryTempFileName, "w+", tempFile ) ) )
      {
         sPerror( "myExit: reopen temp file before exit", "Shutdown warning" );
      }
   }
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


static void waitForChildShutdown( void )
{
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
}

