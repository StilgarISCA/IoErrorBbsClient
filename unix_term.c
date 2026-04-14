/*
 * Copyright (C) 2024-2026 Stilgar
 * Copyright (C) 1995-2003 Michael Hampton
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "defs.h"
#include "ext.h"
#include "unix.h"

static bool terminalSupportsTitleBarUpdates( void )
{
   const char *ptrTerm;
   const char *ptrTermProgram;

   ptrTerm = getenv( "TERM" );
   ptrTermProgram = getenv( "TERM_PROGRAM" );

   if ( ptrTermProgram != NULL &&
        ( strcmp( ptrTermProgram, "Apple_Terminal" ) == 0 ||
          strcmp( ptrTermProgram, "iTerm.app" ) == 0 ) )
   {
      return true;
   }
   if ( ptrTerm == NULL )
   {
      return false;
   }
   if ( strncmp( ptrTerm, "xterm", 5 ) == 0 ||
        strncmp( ptrTerm, "screen", 6 ) == 0 ||
        strncmp( ptrTerm, "tmux", 4 ) == 0 )
   {
      return true;
   }

   return false;
}

static bool shouldUpdateTitleBar( void )
{
   if ( !flagsConfiguration.shouldEnableTitleBar ||
        flagsConfiguration.isScreenReaderModeEnabled )
   {
      return false;
   }

   return terminalSupportsTitleBarUpdates();
}

void titleBar( void )
{
   char aryTitle[80];

   if ( !shouldUpdateTitleBar() )
   {
      return;
   }

   snprintf( aryTitle, sizeof( aryTitle ), "%s:%d%s - BBS Client %s (%s)",
             aryCommandLineHost, cmdLinePort, isSsl ? " (Secure)" : "",
             BUILD_VERSION, "Unix" );
   printf( "\033]0;%s\007", aryTitle );
   fflush( stdout );
}

void noTitleBar( void )
{
   if ( !shouldUpdateTitleBar() )
   {
      return;
   }

   printf( "\033]0;\007" );
   fflush( stdout );
}

/*
 * Suspend the client.  Restores terminal to previous state before suspending,
 * puts it back in proper mode when client restarts, and checks if the window
 * size was changed while we were isAway.
 */
void suspend( void )
{
   noTitleBar();
   resetTerm();
   kill( 0, SIGSTOP );
   setTerm();
   titleBar();
   printf( "\r\n[Continue]\r\n" );
   if ( oldRows != getWindowSize() && oldRows != -1 )
   {
      sendNaws();
   }
}

static int isTerminalStateSaved = 0;

#ifdef HAVE_TERMIO_H
static struct termio saveterm;

#else
static struct sgttyb saveterm;
static struct tchars savetchars;
static struct ltchars savedLocalTermChars;
static int savelocalmode;

#endif

/*
 * Set terminal state to proper modes for running the client/bbs
 */
void setTerm( void )
{
#ifdef HAVE_TERMIO_H
   struct termio tmpterm;

#else
   struct sgttyb tmpterm;
   struct tchars temporaryTermChars;
   struct ltchars tmpltchars;
   int tmplocalmode;

#endif

   getWindowSize();

   if ( flagsConfiguration.shouldUseAnsi )
   {
      printAnsiDisplayStateValue( lastColor, color.background );
      fflush( stdout );
   }

   titleBar();
#ifdef HAVE_TERMIO_H
   if ( !isTerminalStateSaved )
   {
      ioctl( 0, TCGETA, &saveterm );
   }
   tmpterm = saveterm;
   tmpterm.c_iflag &= ~( INLCR | IGNCR | ICRNL );
   tmpterm.c_iflag |= IXOFF | IXON | IXANY;
   tmpterm.c_oflag &= ~( ONLCR | OCRNL );
   tmpterm.c_lflag &= ~( ISIG | ICANON | ECHO );
   tmpterm.c_cc[VMIN] = 1;
   tmpterm.c_cc[VTIME] = 0;
   ioctl( 0, TCSETA, &tmpterm );
#else
   if ( !isTerminalStateSaved )
   {
      ioctl( 0, TIOCGETP, (char *)&saveterm );
   }
   tmpterm = saveterm;
   tmpterm.sg_flags &= ~( ECHO | CRMOD );
   tmpterm.sg_flags |= CBREAK | TANDEM;
   ioctl( 0, TIOCSETN, (char *)&tmpterm );
   if ( !isTerminalStateSaved )
   {
      ioctl( 0, TIOCGETC, (char *)&savetchars );
   }
   temporaryTermChars = savetchars;
   temporaryTermChars.t_intrc = -1;
   temporaryTermChars.t_quitc = -1;
   temporaryTermChars.t_eofc = -1;
   temporaryTermChars.t_brkc = -1;
   ioctl( 0, TIOCSETC, (char *)&temporaryTermChars );
   if ( !isTerminalStateSaved )
   {
      ioctl( 0, TIOCGLTC, (char *)&savedLocalTermChars );
   }
   tmpltchars = savedLocalTermChars;
   tmpltchars.t_suspc = -1;
   tmpltchars.t_dsuspc = -1;
   tmpltchars.t_rprntc = -1;
   ioctl( 0, TIOCSLTC, (char *)&tmpltchars );
   if ( !isTerminalStateSaved )
   {
      ioctl( 0, TIOCLGET, (char *)&savelocalmode );
   }
   tmplocalmode = savelocalmode;
   tmplocalmode &= ~( LPRTERA | LCRTERA | LCRTKIL | LCTLECH | LPENDIN | LDECCTQ );
   tmplocalmode |= LCRTBS;
   ioctl( 0, TIOCLSET, (char *)&tmplocalmode );
#endif
   isTerminalStateSaved = 1;
}

/*
 * Reset the terminal to the previous state it was in when we started.
 */
void resetTerm( void )
{
   if ( flagsConfiguration.shouldUseAnsi )
   {
      printAnsiResetValue();
      fflush( stdout );
   }
   if ( !isTerminalStateSaved )
   {
      return;
   }
#ifdef HAVE_TERMIO_H
   ioctl( 0, TCSETA, &saveterm );
#else
   ioctl( 0, TIOCSETN, (char *)&saveterm );
   ioctl( 0, TIOCSETC, (char *)&savetchars );
   ioctl( 0, TIOCSLTC, (char *)&savedLocalTermChars );
   ioctl( 0, TIOCLSET, (char *)&savelocalmode );
#endif
}

/*
 * Get the current window size.
 */
int getWindowSize( void )
{
#ifdef TIOCGWINSZ
   struct winsize ws;

   if ( ioctl( 0, TIOCGWINSZ, (char *)&ws ) < 0 )
   {
      return ( rows = WINDOW_ROWS_DEFAULT );
   }
   else if ( ( rows = ws.ws_row ) < WINDOW_ROWS_MIN || rows > WINDOW_ROWS_MAX )
   {
      return ( rows = WINDOW_ROWS_DEFAULT );
   }
   else
   {
      return ( rows );
   }
#else
   return ( rows = WINDOW_ROWS_DEFAULT );
#endif
}

void mySleep( unsigned int sec )
{
   sleep( sec );
}

/*
 * This function flushes the input buffer in the same manner as the BBS does.
 * By doing it on the client end we arySavedBytes the BBS the trouble of doing it, but
 * in general the same thing will happen on one end or the other, so you won't
 * speed things up at all by changing this, the sleep is there for your
 * protection to insure a cut and paste gone awry or aryLine noise doesn't cause
 * you too much hassle of posting random garbage, changing your profile or
 * configuration or whatever.
 */
void flushInput( unsigned int invalid )
{
#if defined( FIONREAD ) || defined( TCFLSH )
   int pendingInputBytes;
#endif

   if ( invalid / 2 )
   {
      mySleep( invalid / 2 < 3 ? invalid / 2 : 3 );
   }
#ifdef FIONREAD
   while ( isPtyInputAvailable() || ( !ioctl( 0, FIONREAD, &pendingInputBytes ) && pendingInputBytes > 0 ) )
   {
      (void)ptyget();
   }
#else
#ifdef TCFLSH
   pendingInputBytes = 0;
   ioctl( 0, TCFLSH, &pendingInputBytes );
#endif
   while ( isPtyInputAvailable() )
   {
      (void)ptyget();
   }
#endif
}
