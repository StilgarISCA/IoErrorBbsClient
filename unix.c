/*
 * Everything Unix/system specific goes in this file.  If you are looking to
 * port to some system the code currently doesn't work on, most if not all of
 * your problems should restricted to this file.
 *
 * This file covers Unix/macOS builds.
 */
#define _IN_UNIX_C
#include "defs.h"
#include "ext.h"
#include "unix.h"

static struct passwd *pw;

#ifdef HAVE_OPENSSL
SSL_CTX *ctx;

void killSsl( void )
{
   SSL_shutdown( ssl );
   SSL_free( ssl );
}

int initSSL( void )
{
   SSL_METHOD *meth;

   SSL_load_error_strings();
   SSLeay_add_ssl_algorithms();
   meth = SSLv23_client_method();
   ctx = SSL_CTX_new( meth );

   if ( !ctx )
   {
      printf( "%s\n", ERR_reason_error_string( ERR_get_error() ) );
      exit( 1 );
   }

   ssl = SSL_new( ctx );
   if ( !ssl )
   {
      printf( "SSL_new failed\n" );
      return 0;
   }

#if DEBUG
   printf( "SSL initialized\n" );
#endif
   return 1;
}
#endif /* HAVE_OPENSSL */

/*
 * Wait for the next activity from either the aryUser or the network -- we ignore
 * the aryUser while we have a child process running.  Returns 1 for aryUser input
 * pending, 2 for network input pending, 3 for both.
 */
int waitNextEvent( void )
{
   fd_set fdr;
   int result;

   while ( true )
   {
      FD_ZERO( &fdr );
      if ( !childPid && !flagsConfiguration.shouldCheckExpress )
      {
         FD_SET( 0, &fdr );
      }
      if ( !shouldIgnoreNetwork )
      {
         FD_SET( net, &fdr );
      }

      if ( select( shouldIgnoreNetwork ? 1 : net + 1, &fdr, 0, 0, 0 ) < 0 )
      {
         if ( errno == EINTR )
         {
            continue;
         }
         else
         {
            stdPrintf( "\r\n" );
            fatalPerror( "select", "Local error" );
         }
      }

      if ( ( result = ( ( FD_ISSET( net, &fdr ) != 0 ) << 1 | ( FD_ISSET( 0, &fdr ) != 0 ) ) ) )
      {
         return ( result );
      }
   }
}

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
         snprintf( aryUser + userNameLength, sizeof( aryUser ) - userNameLength, "  (login aryShell)" );
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

/* Added by Dave (Isoroku).  Finds .bbsfriends for friends list */
/* Edited by IO ERROR.  We read-only the .bbsfriends now, if it exists. */
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

void titleBar( void )
{
#ifdef ENABLE_TITLEBAR
   char aryTitle[80];

   snprintf( aryTitle, sizeof( aryTitle ), "%s:%d%s - BBS Client %s (%s)",
             aryCommandLineHost, cmdLinePort, isSsl ? " (Secure)" : "",
             VERSION, "Unix" );
   /* xterm */
   if ( !strcmp( getenv( "TERM" ), "xterm" ) )
   {
      printf( "\033]0;%s\007", aryTitle );
   }
   /* NeXT */
   if ( getenv( "STUART" ) )
   {
      printf( "\033]1;%s\\", aryTitle );
      printf( "\033]2;%s\\", aryTitle );
   }
#endif
   return;
}

void noTitleBar( void )
{
#ifdef ENABLE_TITLEBAR
   /* xterm */
   if ( !strcmp( getenv( "TERM" ), "xterm" ) )
   {
      printf( "\033]0;xterm\007" );
   }
   /* NeXT */
   if ( getenv( "STUART" ) )
   {
      struct winsize ws;

      ioctl( 0, TIOCGWINSZ, (char *)&ws );
      printf( "\033]1; csh (%s)\033\\", rindex( (char *)ttyname( 0 ), '/' ) + 1 );
      printf( "\033]2; (%s) %dx%d\033\\", rindex( (char *)ttyname( 0 ), '/' ) + 1, ws.ws_col, ws.ws_row );
   }
   fflush( stdout );
#endif
   return;
}

/*
 * Open a socket connection to the bbs.  Defaults to BBS_HOSTNAME with port BBS_PORT_NUMBER
 * (by default a standard telnet to bbs.isca.uiowa.edu) but can be overridden
 * in the bbsrc file if/when the source to the ISCA BBS is released and others
 * start their own on different machines and/or ports.
 */
void connectBbs( void )
{
   register struct hostent *host;
   register int connectResult;
   struct sockaddr_in socketAddress;

   if ( !*aryBbsHost )
   {
      snprintf( aryBbsHost, sizeof( aryBbsHost ), "%s", BBS_HOSTNAME );
   }
   if ( !bbsPort )
   {
      bbsPort = BBS_PORT_NUMBER;
   }
   if ( !*aryCommandLineHost )
   {
      snprintf( aryCommandLineHost, sizeof( aryCommandLineHost ), "%s", aryBbsHost );
   }
   if ( !cmdLinePort )
   {
      cmdLinePort = bbsPort;
   }
   strncpy( (char *)&socketAddress, "", sizeof socketAddress );
   socketAddress.sin_family = AF_INET;
   socketAddress.sin_port = htons( cmdLinePort ); /* Spurious gcc warning */
   if ( isdigit( *aryCommandLineHost ) )
   {
      socketAddress.sin_addr.s_addr = inet_addr( aryCommandLineHost );
   }
   else if ( !( host = gethostbyname( aryCommandLineHost ) ) )
   {
      socketAddress.sin_addr.s_addr = inet_addr( BBS_IP_ADDRESS );
   }
   else
   {
      strncpy( (char *)&socketAddress.sin_addr, host->h_addr, sizeof socketAddress.sin_addr );
   }

   net = socket( AF_INET, SOCK_STREAM, 0 );
   if ( net < 0 )
   {
      fatalPerror( "socket", "Local error" );
   }
   connectResult = connect( net, (struct sockaddr *)&socketAddress, sizeof socketAddress );
   if ( connectResult < 0 )
   {
#define BBSREFUSED "The BBS has refused connection, try again later.\r\n"
#define BBSNETDOWN "Network problems prevent connection with the BBS, try again later.\r\n"
#define BBSHOSTDOWN "The BBS is down or there are network problems, try again later.\r\n"

#ifdef ECONNREFUSED
      if ( errno == ECONNREFUSED )
      {
         stdPrintf( BBSREFUSED );
      }
#endif
#ifdef ENETDOWN
      if ( errno == ENETDOWN )
      {
         stdPrintf( BBSNETDOWN );
      }
#endif
#ifdef ENETUNREACH
      if ( errno == ENETUNREACH )
      {
         stdPrintf( BBSNETDOWN );
      }
#endif
#ifdef ETIMEDOUT
      if ( errno == ETIMEDOUT )
      {
         stdPrintf( BBSHOSTDOWN );
      }
#endif
#ifdef EHOSTDOWN
      if ( errno == EHOSTDOWN )
      {
         stdPrintf( BBSHOSTDOWN );
      }
#endif
#ifdef EHOSTUNREACH
      if ( errno == EHOSTUNREACH )
      {
         stdPrintf( BBSNETDOWN );
      }
#endif
      fatalPerror( "connect", "Network error" );
   }
#ifdef HAVE_OPENSSL
   if ( shouldUseSsl )
   {
      initSSL();
      if ( SSL_set_fd( ssl, net ) != 1 )
      {
         printf( "%s\n", ERR_reason_error_string( ERR_get_error() ) );
         shutdown( net, 2 );
         exit( 1 );
      }
      if ( ( connectResult = SSL_connect( ssl ) ) != 1 )
      {
         printf( "%s\n", ERR_reason_error_string( ERR_get_error() ) );
         shutdown( net, 2 );
         exit( 1 );
      }
      isSsl = 1;
   }
#endif
   stdPrintf( "[%ssecure connection established]\n", ( shouldUseSsl ) ? "S" : "Ins" );
   titleBar();
   fflush( stdout );

   /*
     * We let the stdio libraries handle buffering issues for us.  Only for
     * output, there are portability problems with what is needed for input.
     */
   if ( !( netOutputFile = fdopen( net, "w" ) ) )
   {
      fatalPerror( "fdopen w", "Local error" );
   }
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

   if ( flagsConfiguration.useAnsi )
   {
      printf( "\033[%cm\033[3%c;4%cm", flagsConfiguration.useBold ? '1' : '0', lastColor,
              color.background );
   }
   fflush( stdout );

   titleBar();
#ifdef HAVE_TERMIO_H
   if ( !isTerminalStateSaved )
      ioctl( 0, TCGETA, &saveterm );
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
   if ( flagsConfiguration.useAnsi )
   {
      /*	printf("\033[0m\033[1;37;49m"); */
      printf( "\033[0;39;49m" );
   }
   fflush( stdout );
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
   int pendingInputBytes;

   if ( invalid / 2 )
   {
      mySleep( invalid / 2 < 3 ? invalid / 2 : 3 );
   }
#ifdef FIONREAD
   while ( INPUT_LEFT( stdin ) || ( !ioctl( 0, FIONREAD, &pendingInputBytes ) && pendingInputBytes > 0 ) )
   {
#else
#ifdef TCFLSH
   pendingInputBytes = 0;
   ioctl( 0, TCFLSH, &pendingInputBytes );
#endif
   while ( INPUT_LEFT( stdin ) )
#endif
      (void)ptyget();
   }
}

/*
 * Run the command 'aryCommand' with argument 'arg'.  Used only for running the aryEditor
 * right now.  In order to work properly with all the versions of Unix I've
 * tried to port this to so far without be overly complicated, I have to use a
 * setjmp to arySavedBytes the local stack context in this function, then longjmp back
 * here once I receive a signal from the child that it has terminated. So I
 * guess there actually IS a use for setjmp/longjmp after all! :-)
 */
void run( char *aryCommand, char *arg )
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
         execlp( aryCommand, aryCommand, arg, 0 );
         fprintf( stderr, "\r\n" );
         sPerror( "exec", "Local error" );
         _exit( 0 );
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
   stdPrintf( "Technical information\r\n\n" );

   feedPager( 3,
              "ISCA BBS Client " VERSION " (Unix)\r\n",
              "Compiled on: " HOSTTYPE "\r\n",
              "With: "
#ifdef __STDC__
              "ANSI "
#endif
#ifdef __cplusplus
              "C++ "
#endif
#ifdef __GNUC__
              "gcc "
#endif
#ifdef _POSIX_SOURCE
              "POSIX "
#endif
#ifdef ENABLE_SAVE_PASSWORD
              "arySavedBytes-password "
#endif
#ifdef USE_POSIX_SIGSETJMP
              "sigsetjmp "
#endif
              "\r\n",
              (char *)NULL );
}

void initialize( const char *protocol )
{
   (void)protocol;
   if ( !isatty( 0 ) || !isatty( 1 ) || !isatty( 2 ) )
   {
      exit( 0 );
   }

   ptrPtyInput = aryPtyInputBuffer;
   ptrNetInput = aryNetInputBuffer;

   isAway = 0;

#ifdef _IOFBF
   setvbuf( stdout, NULL, _IOFBF, 4096 );
#endif

   stdPrintf( "\nISCA BBS Client %s (%s)\n", VERSION,
              "Unix" );
   stdPrintf( "\nCopyright (C) 1995-2003 Michael Hampton.\n" );
   stdPrintf( "OSI Certified Open Source Software.  GNU General Public License version 2.\n" );
   stdPrintf( "For information about this client visit http://www.ioerror.us/client/\n\n" );
#if DEBUG
   stdPrintf( "DEBUGGING VERSION - DEBUGGING CODE IS ENABLED!  DO NOT USE THIS CLIENT!\r\n\n" );
#endif
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
   if ( !isLoginShell )
   {
      snprintf( aryBrowser, sizeof( aryBrowser ), "%s", "netscape -remote" );
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
         snprintf( aryMyEditor, sizeof( aryMyEditor ), "%s", "vi" );
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
   snprintf( aryErrorBuffer, sizeof( aryErrorBuffer ), "%s: %s", heading, message );
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

/* TODO: system() is kind of cheating */
/* TODO: Peeking into the queue object itself is REALLY cheating */
void openBrowser( void )
{
   int inputIndex;
   int originalCaptureState;
   char aryLine[4];
   char aryCommand[4096];
   char *ptrUrlEntry;

   if ( urlQueue->nobjs < 1 )
   {
      return;
   }
   if ( urlQueue->nobjs == 1 )
   {
      snprintf( aryCommand, sizeof( aryCommand ), "%s \"%s\"%s", aryBrowser,
                urlQueue->start + ( urlQueue->objsize * urlQueue->head ),
                flagsConfiguration.shouldRunBrowserInBackground ? " &" : "" );
      system( aryCommand );
      if ( !flagsConfiguration.shouldRunBrowserInBackground )
      {
         reprintLine();
      }
      return;
   }

   originalCaptureState = capture;
   capture = 0;
   shouldIgnoreNetwork = 1;
   printf( "\r\n\n" );
   ptrUrlEntry = urlQueue->start + ( urlQueue->objsize * urlQueue->head );
   for ( inputIndex = 0; inputIndex < urlQueue->nobjs; inputIndex++ )
   {
      if ( strlen( ptrUrlEntry ) > 72 )
      {
         char junk[71];

         strncpy( junk, ptrUrlEntry, 70 );
         junk[70] = 0;
         printf( "%d. %-70s...\r\n", inputIndex + 1, junk );
      }
      else
      {
         printf( "%d. %s\r\n", inputIndex + 1, ptrUrlEntry );
      }
      ptrUrlEntry += urlQueue->objsize;
      if ( ptrUrlEntry >= (char *)( urlQueue->start + ( urlQueue->objsize * urlQueue->size ) ) )
      {
         ptrUrlEntry = urlQueue->start;
      }
   }

   printf( "\r\nChoose the URL you want to view: " );
   getString( 3, aryLine, 1 ); /* No more than 999 URLs in a post? */
   printf( "\r\n" );
   inputIndex = atoi( aryLine );
   ptrUrlEntry = urlQueue->start + ( urlQueue->objsize * urlQueue->head );
   if ( inputIndex > 0 && inputIndex <= urlQueue->nobjs )
   {
      int urlIndex;

      inputIndex -= 1;
      for ( urlIndex = 0; urlIndex < inputIndex; urlIndex++ )
      {
         ptrUrlEntry += urlQueue->objsize;
         if ( ptrUrlEntry >= (char *)( urlQueue->start + ( urlQueue->objsize * urlQueue->size ) ) )
         {
            ptrUrlEntry = urlQueue->start;
         }
      }
      snprintf( aryCommand, sizeof( aryCommand ), "%s \"%s\"%s", aryBrowser, ptrUrlEntry,
                flagsConfiguration.shouldRunBrowserInBackground ? " &" : "" );
      system( aryCommand );
   }
   shouldIgnoreNetwork = 0;
   reprintLine();
   capture = originalCaptureState;
   return;
}

/*
 * Move oldpath to newpath if oldpath exists and newpath does not exist.
 * Then delete oldpath, even if newpath already exists.
 */
void moveIfNeeded( const char *oldpath, const char *newpath )
{
   FILE *ptrOldFile;
   FILE *ptrNewFile;
   char aryCopyBuffer[BUFSIZ];
   size_t bytesRead;
   long targetSize;

   ptrOldFile = fopen( oldpath, "r" );
   if ( !ptrOldFile )
   {
      return;
   }

   ptrNewFile = fopen( newpath, "a" );
   if ( !ptrNewFile )
   {
      return;
   }

   targetSize = ftell( ptrNewFile );
   if ( targetSize == 0 )
   {
      /* Args 2 and 3 intentionally reversed */
      while ( ( bytesRead = fread( aryCopyBuffer, 1, BUFSIZ, ptrOldFile ) ) > 0 )
      {
         bytesRead = fwrite( aryCopyBuffer, 1, BUFSIZ, ptrNewFile );
      }
   }

   fclose( ptrOldFile );
   fclose( ptrNewFile );
   unlink( oldpath );
   return;
}
