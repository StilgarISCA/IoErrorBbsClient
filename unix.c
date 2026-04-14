/*
 * Copyright (C) 2024-2026 Stilgar
 * Copyright (C) 1995-2003 Michael Hampton
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

/*
 * Everything Unix/system specific goes in this file.  If you are looking to
 * port to some system the code currently doesn't work on, most if not all of
 * your problems should restricted to this file.
 *
 * This file covers Unix/macOS builds.
 */
#define _IN_UNIX_C
#include "defs.h"
#include "client_globals.h"
#include "config_globals.h"
#include "network_globals.h"
#include "proto.h"
#include "unix.h"
#include <wordexp.h>

static struct passwd *pw;

/*
 * Find the aryUser's home directory (needed for .bbsrc and .bbstmp)
 */
void findHome( void )
{
   if ( ( pw = getpwuid( getuid() ) ) )
   {
      snprintf( aryUser, sizeof( aryUser ), "%s", pw->pw_name );
   }
   else if ( getenv( "USER" ) )
   {
      snprintf( aryUser, sizeof( aryUser ), "%s", getenv( "USER" ) );
   }
   else
   {
      fatalExit( "findHome: You don't exist, go away.", "Local error" );
   }
   if ( isLoginShell )
   {
      size_t userNameLength = strlen( aryUser );
      if ( userNameLength < sizeof( aryUser ) - 1 )
      {
         snprintf( aryUser + userNameLength, sizeof( aryUser ) - userNameLength, "  (login shell)" );
      }
   }
}

/*
 * Locate the bbsrc file.  Usually is ~/.bbsrc, can be overriden by providing
 * an argument to the command that invokes this client.  If the argument is not
 * provided the BBSRC environment will specify the name of the BBSRC file if it
 * is set.  Returns a pointer to the file via openbbsrc().
 */
FILE *findBbsRc( void )
{
   FILE *ptrFileHandle;

   if ( isLoginShell )
   {
      snprintf( aryBbsRcName, sizeof( aryBbsRcName ), "/tmp/bbsrc.%d", getpid() );
   }
   else
   {
      if ( getenv( "BBSRC" ) )
      {
         snprintf( aryBbsRcName, sizeof( aryBbsRcName ), "%s", getenv( "BBSRC" ) );
      }
      else if ( pw )
      {
         snprintf( aryBbsRcName, sizeof( aryBbsRcName ), "%s/.bbsrc", pw->pw_dir );
      }
      else if ( getenv( "HOME" ) )
      {
         snprintf( aryBbsRcName, sizeof( aryBbsRcName ), "%s/.bbsrc", getenv( "HOME" ) );
      }
      else
      {
         fatalExit( "findbbsrc: You don't exist, go away.", "Local error" );
      }
   }
   if ( ( ptrFileHandle = fopen( aryBbsRcName, "r" ) ) && chmod( aryBbsRcName, 0600 ) < 0 )
   {
      sPerror( "Can't set access on bbsrc file", "Warning" );
   }
   if ( ptrFileHandle )
   {
      fclose( ptrFileHandle );
   }
   return ( openBbsRc() );
}

/* Locate .bbsfriends and keep permissions restricted when the file exists. */
FILE *findBbsFriends( void )
{
   if ( isLoginShell )
   {
      snprintf( aryBbsFriendsName, sizeof( aryBbsFriendsName ), "/tmp/bbsfriends.%d", getpid() );
   }
   else
   {
      if ( getenv( "BBSFRIENDS" ) )
      {
         snprintf( aryBbsFriendsName, sizeof( aryBbsFriendsName ), "%s", getenv( "BBSFRIENDS" ) );
      }
      else if ( pw )
      {
         snprintf( aryBbsFriendsName, sizeof( aryBbsFriendsName ), "%s/.bbsfriends", pw->pw_dir );
      }
      else if ( getenv( "HOME" ) )
      {
         snprintf( aryBbsFriendsName, sizeof( aryBbsFriendsName ), "%s/.bbsfriends", getenv( "HOME" ) );
      }
      else
      {
         fatalExit( "findBbsFriends: You don't exist, go away.", "Local error" );
      }
   }
   chmod( aryBbsFriendsName, 0600 );
   return ( openBbsFriends() );
}

/*
 * Truncates bbsrc file to the specified length.
 */
void truncateBbsRc( long userNameLength )
{
   if ( ftruncate( fileno( ptrBbsRc ), userNameLength ) < 0 )
   {
      fatalExit( "ftruncate", "Local error" );
   }
}

/*
 * Opens the temp file, ~/.bbstmp.  If the BBSTMP environment variable is set,
 * that file is used instead.
 */
void openTmpFile( void )
{
   if ( isLoginShell )
   {
      snprintf( aryTempFileName, sizeof( aryTempFileName ), "/tmp/bbstmp.%d", getpid() );
   }
   else
   {
      if ( getenv( "BBSTMP" ) )
      {
         snprintf( aryTempFileName, sizeof( aryTempFileName ), "%s", getenv( "BBSTMP" ) );
      }
      else if ( pw )
      {
         snprintf( aryTempFileName, sizeof( aryTempFileName ), "%s/.bbstmp", pw->pw_dir );
      }
      else if ( getenv( "HOME" ) )
      {
         snprintf( aryTempFileName, sizeof( aryTempFileName ), "%s/.bbstmp", getenv( "HOME" ) );
      }
      else
      {
         fatalExit( "openTmpFile: You don't exist, go away.", "Local error" );
      }
   }
   if ( !( tempFile = fopen( aryTempFileName, "a+" ) ) )
   {
      fatalPerror( "openTmpFile: fopen", "Local error" );
   }
   if ( chmod( aryTempFileName, 0600 ) < 0 )
   {
      sPerror( "openTmpFile: chmod", "Warning" );
   }
}

/*
 * Quits gracefully when we are given a HUP or STOP signal.
 */
RETSIGTYPE bye( int signalNumber )
{
   (void)signalNumber;
   myExit();
}

/*
 * Handles a WINCH signal given when the window is resized
 */
RETSIGTYPE naws( int signalNumber )
{
   (void)signalNumber;
   if ( oldRows != -1 )
   {
      sendNaws();
   }
#ifdef SIGWINCH
   signal( SIGWINCH, naws );
#endif
}

/*
 * Handles the death of the child by doing a longjmp back to the function that
 * forked it.  We get spurious signals when the child is stopped, and to avoid
 * confusion we don't allow the child to be stopped -- therefore we attempt to
 * send a continue signal to the child here.  If it fails, we assume the child
 * did in fact die, and longjmp back to the function that forked it.  If it
 * doesn't fail, the child is restarted and the aryUser is forced to exit the
 * child cleanly to get back into the main client.
 */
RETSIGTYPE reapChild( int signalNumber )
{
   (void)signalNumber;
   wait( 0 );
   titleBar();
   if ( kill( childPid, SIGCONT ) < 0 )
   {
#ifdef USE_POSIX_SIGSETJMP
      siglongjmp( jumpEnv, 1 );
#else
      longjmp( jumpEnv, 1 );
#endif /* USE_POSIX_SIGSETJMP */
   }
}

/*
 * Initialize necessary signals
 */
void sigInit( void )
{
   oldRows = -1;

   signal( SIGINT, SIG_IGN );
   signal( SIGQUIT, SIG_IGN );
   signal( SIGPIPE, SIG_IGN );
#ifdef SIGTSTP
   signal( SIGTSTP, SIG_IGN );
#endif
#ifdef SIGTTOU
   signal( SIGTTOU, SIG_IGN );
#endif
   signal( SIGHUP, bye );
   signal( SIGTERM, bye );
#ifdef SIGWINCH
   signal( SIGWINCH, naws );
#endif
}

/*
 * Turn off signals now that we are ready to terminate
 */
void sigOff( void )
{
   signal( SIGALRM, SIG_IGN );
#ifdef SIGWINCH
   signal( SIGWINCH, SIG_IGN );
#endif
   signal( SIGHUP, SIG_IGN );
   signal( SIGTERM, SIG_IGN );
}

static void execCommandWithOptionalArg( const char *ptrCommand, const char *ptrArg )
{
   wordexp_t parsedCommand;
   char **aryArguments;
   size_t argumentCount;
   int parseResult;

   parseResult = wordexp( ptrCommand, &parsedCommand, WRDE_NOCMD );
   if ( parseResult != 0 || parsedCommand.we_wordc == 0 )
   {
      fprintf( stderr, "\r\n[Unable to parse command: %s]\r\n", ptrCommand );
      _exit( 1 );
   }

   argumentCount = parsedCommand.we_wordc + ( ptrArg ? 1U : 0U );
   aryArguments = calloc( argumentCount + 1, sizeof( char * ) );
   if ( aryArguments != NULL )
   {
      for ( size_t argumentIndex = 0; argumentIndex < parsedCommand.we_wordc; argumentIndex++ )
      {
         aryArguments[argumentIndex] = parsedCommand.we_wordv[argumentIndex];
      }
      if ( ptrArg )
      {
         aryArguments[parsedCommand.we_wordc] = (char *)ptrArg;
      }

      execvp( aryArguments[0], aryArguments );
      fprintf( stderr, "\r\n" );
      sPerror( "exec", "Local error" );
      free( aryArguments );
      wordfree( &parsedCommand );
      _exit( 1 );
   }

   fprintf( stderr, "\r\n" );
   sPerror( "calloc", "Local error" );
   wordfree( &parsedCommand );
   _exit( 1 );
}

/*
 * Launch aryCommand with an optional trailing argument. Used for the external
 * editor and subshell paths. The child exit path returns here through the
 * existing setjmp/longjmp signal flow.
 */
void run( const char *aryCommand, const char *arg )
{
   fflush( stdout );
#ifdef USE_POSIX_SIGSETJMP
   if ( sigsetjmp( jumpEnv, 1 ) )
   {
#else
   if ( setjmp( jumpEnv ) )
   {
#endif /* USE_POSIX_SIGSETJMP */
      signal( SIGCHLD, SIG_DFL );
      if ( childPid < 0 )
      {
         childPid = 0;
         myExit();
      }
      else
      {
         setTerm();
         childPid = 0;
      }
   }
   else
   {
      signal( SIGCHLD, reapChild );
      noTitleBar();
      resetTerm();

      if ( !( childPid = fork() ) )
      {
         execCommandWithOptionalArg( aryCommand, arg );
      }
      else if ( childPid > 0 )
      {

         /*
        * Flush out anything in our stdio buffer -- it was copied to the
        * child process, we don't want it waiting for us when the child
        * is done.
        */
         flushInput( 0 );
         (void)inKey();
      }
      else
      {
         fatalPerror( "fork", "Local error" );
      }
   }
}

void techInfo( void )
{
   char aryRuntimeInfo[256];
   char aryRuntimeAccessibility[256];

   snprintf( aryRuntimeInfo,
             sizeof( aryRuntimeInfo ),
             "Runtime: SSL %s, keepalive %s, title bar %s, clickable URLs %s\r\n",
             shouldUseSsl ? "on" : "off",
             flagsConfiguration.shouldUseTcpKeepalive ? "on" : "off",
             flagsConfiguration.shouldEnableTitleBar ? "on" : "off",
             flagsConfiguration.shouldEnableClickableUrls ? "on" : "off" );
   snprintf( aryRuntimeAccessibility,
             sizeof( aryRuntimeAccessibility ),
             "Accessibility: screen reader %s, autocomplete %s\r\n",
             flagsConfiguration.isScreenReaderModeEnabled ? "on" : "off",
             flagsConfiguration.shouldEnableNameAutocomplete ? "on" : "off" );

   stdPrintf( "Technical information\r\n\n" );

   feedPager( 3,
              "ISCA BBS Client " BUILD_VERSION " (macOS/Unix)\r\n",
              "Built on: " HOSTTYPE "\r\n",
              "Compiler: " BUILD_COMPILER "\r\n",
              "Build mode: " BUILD_MODE "\r\n",
              "Optimization: " BUILD_OPTIMIZATION_MODE "\r\n",
              "Universal binary: " BUILD_UNIVERSAL_MODE "\r\n",
              "OpenSSL support: " BUILD_SSL_MODE "\r\n",
              "Save password support: " BUILD_SAVE_PASSWORD_MODE "\r\n",
              "Sanitizers: " BUILD_SANITIZER_MODE "\r\n",
              "Native optimizations: " BUILD_NATIVE_OPTIMIZATION_MODE "\r\n",
              "Stack protector: " BUILD_STACK_PROTECTOR_MODE "\r\n",
              aryRuntimeInfo,
              aryRuntimeAccessibility,
              (char *)NULL );
}

void initialize( void )
{
   if ( !isatty( 0 ) || !isatty( 1 ) || !isatty( 2 ) )
   {
      exit( 0 );
   }

   ptrPtyInput = aryPtyInputBuffer;
   ptrNetInput = aryNetInputBuffer;

   isAway = 0;

#ifdef _IOLBF
   setvbuf( stdout, NULL, _IOLBF, 0 );
#endif

   stdPrintf( "\nISCA BBS Client %s (%s)\n", BUILD_VERSION, "macOS/Unix" );
   stdPrintf( "Copyright (C) 2024-2026 Stilgar, 1995-2003 Michael Hampton\n" );
   stdPrintf( "\nhttps://github.com/StilgarISCA/IoErrorBbsClient\n" );
   stdPrintf( "GPL-2.0-or-later (see LICENSE)\n\n" );
   fflush( stdout );
   xlandQueue = newQueue( 21, MAX_USER_NAME_HISTORY_COUNT );
   if ( !xlandQueue )
   {
      isXland = 0;
   }
   if ( isLoginShell )
   {
      snprintf( aryShell, sizeof( aryShell ), "%s", "/bin/true" );
   }
   else
   {
      if ( getenv( "SHELL" ) )
      {
         snprintf( aryShell, sizeof( aryShell ), "%s", getenv( "SHELL" ) );
      }
      else
      {
         snprintf( aryShell, sizeof( aryShell ), "%s", "/bin/sh" );
      }
   }
   if ( isLoginShell )
   {
      snprintf( aryMyEditor, sizeof( aryMyEditor ), "%s", "" );
   }
   else
   {
      if ( getenv( "EDITOR" ) )
      {
         snprintf( aryMyEditor, sizeof( aryMyEditor ), "%s", getenv( "EDITOR" ) );
      }
      else
      {
         snprintf( aryMyEditor, sizeof( aryMyEditor ), "%s", "nano" );
      }
   }
}

void deinitialize( void )
{
   char aryTempFile[PATH_MAX];

   noTitleBar();
   /* Get rid of ~ file emacs always leaves behind */
   snprintf( aryTempFile, sizeof( aryTempFile ), "%s~", aryTempFileName );
   unlink( aryTempFile );
   if ( isLoginShell )
   {
      unlink( aryTempFileName );
      unlink( aryBbsRcName );
      unlink( aryBbsFriendsName );
   }
}

int deleteFile( const char *pathname )
{
   return unlink( pathname );
}

int sPrompt( const char *info, const char *question, int def )
{
   stdPrintf( "\r\n%s\r\n\n", info );
   stdPrintf( "%s (%s) -> ", question, def ? "Yes" : "No" );
   if ( yesNoDefault( def ) )
   {
      return 1;
   }
   return 0;
}

void sInfo( const char *info, const char *heading )
{
   (void)heading;
   /* Heading ignored for Unix */
   stdPrintf( "\r\n%s\r\n\n", info );
   return;
}

void sPerror( const char *message, const char *heading )
{
   char aryErrorBuffer[4096];
   int savedErrno = errno;

   snprintf( aryErrorBuffer, sizeof( aryErrorBuffer ), "%s: %s", heading, message );
   errno = savedErrno;
   perror( aryErrorBuffer );
   fprintf( stderr, "\r" );
   return;
}

void sError( const char *message, const char *heading )
{
   char aryErrorBuffer[4096];
   snprintf( aryErrorBuffer, sizeof( aryErrorBuffer ), "%s: %s", heading, message );
   fflush( stdout );
   fprintf( stderr, "%s\r\n", aryErrorBuffer );
}

/*
 * Move oldpath to newpath if oldpath exists and newpath does not exist.
 * Then delete oldpath, even if newpath already exists.
 */
void moveIfNeeded( const char *oldpath, const char *newpath )
{
   FILE *ptrOldFile;
   FILE *ptrNewFile;
   struct stat targetFileStatus;
   bool shouldCopy;

   ptrOldFile = fopen( oldpath, "r" );
   if ( !ptrOldFile )
   {
      return;
   }

   shouldCopy = ( stat( newpath, &targetFileStatus ) != 0 || targetFileStatus.st_size == 0 );
   if ( !shouldCopy )
   {
      fclose( ptrOldFile );
      unlink( oldpath );
      return;
   }

   ptrNewFile = fopen( newpath, "a" );
   if ( !ptrNewFile )
   {
      fclose( ptrOldFile );
      return;
   }

   {
      char aryCopyBuffer[BUFSIZ];
      size_t bytesRead;

      while ( ( bytesRead = fread( aryCopyBuffer, 1, sizeof( aryCopyBuffer ), ptrOldFile ) ) > 0 )
      {
         if ( fwrite( aryCopyBuffer, 1, bytesRead, ptrNewFile ) != bytesRead )
         {
            break;
         }
      }
   }

   fclose( ptrOldFile );
   fclose( ptrNewFile );
   unlink( oldpath );
   return;
}
