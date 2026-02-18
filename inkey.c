/*
 * Copyright (C) 2024-2026 Stilgar
 * Copyright (C) 1995-2003 Michael Hampton
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

/*
 * Return next letter the aryUser typed.  This function can be indirectly
 * recursive -- inKey may call telReceive(), which calls other functions which can
 * call inKey()...
 *
 * Basically, if there is a keypress waiting in the stdio buffer, it is returned
 * immediately, if not the socket's stdio buffer is checked (this is where the
 * recursion can take place) and if that is also empty, the output buffers are
 * flushed, then the program blocks in select() until either the aryUser types
 * something or something is sent from the remote side.
 */
#include "defs.h"
#include "ext.h"

static int lastCarriageReturn = 0;

/*
 * The functionality of the former inKey() has been renamed to getKey(), so
 * that inKey() could strip a \n after a \r (common terminal program problem
 * with aryUser misconfiguration) and translate certain keypresses into common
 * Unix equivalents (i.e. \r -> \n, DEL -> BS, ctrl-U -> ctrl-X)
 */
int inKey( void )
{
   register int inputChar;

   while ( true )
   {
      inputChar = getKey();
      if ( !lastCarriageReturn || inputChar != '\n' )
      {
         break;
      }
      lastCarriageReturn = 0;
   }
   lastCarriageReturn = 0;
   if ( inputChar == '\r' )
   {
      inputChar = '\n';
      lastCarriageReturn = 1;
   }
   else if ( inputChar == 127 )
   {
      inputChar = '\b';
   }
   else if ( inputChar == CTRL_U )
   {
      inputChar = CTRL_X;
   }
   return ( inputChar );
}

int getKey( void )
{
   static int macroKey = 0;
   static int macroPosition = 0;       /* pointer into the aryMacro array */
   static int isMacroNext = 0;         /* aryMacro key was hit, aryMacro is next */
   static int wasUndefinedCommand = 0; /* to remove the blurb about undefined aryMacro */
   register int inputChar = -1;
   int eventResult; /* eventResult returned from waitNextEvent() */

   /*
    * Throughout this function, if we are currently running with a child
    * process we don't want to do anything with standard input, we are
    * only * concerned with passing along anything that might be coming
    * over the net during this time (connection going down, broadcast
    * message from wizards, etc.)  That's the reason for all the
    * references to '!childPid' The same is true when 'check' is set, it
    * is used when entering the edit menu (abort, arySavedBytes, etc.) to check
    * back with the BBS for any X messages that may have arrived -- it is
    * added purely to make things compatible between the BBS and the
    * client.
    */

   while ( true )
   {

      /*
   	 * If we're busy going to our arySavedBytes buffers, targetByte is
   	 * non-zero.  We continue to take characters from our arySavedBytes
   	 * buffers until bytePosition catches up to targetByte -- then we
   	 * should be synced with the bbs once again.
   	 */
      if ( targetByte && !childPid && !flagsConfiguration.shouldCheckExpress )
      {
         if ( bytePosition == targetByte )
         {
            targetByte = 0;
         }
         else if ( bytePosition > targetByte )
         {
            stdPrintf( "[Internal error: byte synch lost!  %s > %s]\r\n", bytePosition,
                       targetByte );
            exit( 1 );
         }
         else
         {
            lastCarriageReturn = 0;
            {
               size_t index = (size_t)( bytePosition % (long)sizeof arySavedBytes );
               bytePosition++;
               return ( arySavedBytes[index] );
            }
         }
      }
      /*
   	 * Main processing loop for processing a aryMacro or characters
   	 * in the stdin buffers.
   	 */
      while ( ( macroPosition || INPUT_LEFT( stdin ) ) && !childPid && !flagsConfiguration.shouldCheckExpress )
      {
         /* macroPosition > 0 when we are getting out input from a aryMacro key */
         if ( macroPosition > 0 )
         {
            if ( ( inputChar = aryMacro[macroKey][macroPosition++] ) )
            {
               lastCarriageReturn = 0;
               return ( inputChar );
            }
            else
            {
               macroPosition = 0;
               continue;
            }
         }
         if ( !macroPosition )
         {
            inputChar = ptyget() & 0x7f;
         }
         else
         {
            macroPosition = 0;
         }
         if ( inputChar > 0 && isAway == 1 )
         {
            isAway = 0;
            stdPrintf( "\r\n[No longer away]\r\n" );
         }
         /* Did we hit commandKey last?  Then the next key hit is the aryMacro */
         if ( isMacroNext )
         {
            printf( "\b\b\b\b\b\b\b\b\b         \b\b\b\b\b\b\b\b\b" );
            isMacroNext = 0;
            macroPosition = 0;

            if ( inputChar == awayKey )
            {
               isAway ^= 1;
               stdPrintf( "\r\n[%s away]\r\n", ( isAway ) ? "Now" : "No longer" );
               continue;
            }
            if ( inputChar == quitKey )
            {
               stdPrintf( "\r\n[Quitting]\r\n" );
               myExit();
            }
            else if ( inputChar == suspKey )
            {
               if ( !isLoginShell )
               {
                  printf( "\r\n[Suspended]\r\n" );
                  fflush( stdout );
                  suspend();
               }
               continue;
            }
            else if ( inputChar == shellKey )
            {
               if ( !isLoginShell )
               {
                  printf( "\r\n[New shell]\r\n" );
                  run( aryShell, 0 );
                  printf( "\r\n[Continue]\r\n" );
               }
               continue;
            }
            else if ( inputChar == browserKey && !isLoginShell )
            {
               openBrowser();
               continue;
            }
            else if ( inputChar == captureKey )
            {
               if ( capture < 0 || flagsConfiguration.isPosting )
               {
                  printf( "[ Cannot capture! ]" );
                  wasUndefinedCommand = 1;
                  continue;
               }
               capture ^= 1;
               printf( "\r\n[Capture to temp file turned O%s]\r\n", capture ? "N" : "FF" );
               if ( capture )
               {
                  rewind( tempFile );
                  if ( flagsConfiguration.isLastSave )
                  {
                     if ( !( tempFile = freopen( aryTempFileName, "w+", tempFile ) ) )
                     {
                        fatalPerror( "toggle capture: reopen temp file for truncate", "Capture file error" );
                     }
                     flagsConfiguration.isLastSave = 0;
                  }
                  else if ( getc( tempFile ) >= 0 )
                  {
                     printf( "There is text in your edit file.  Do you wish to erase it? (Y/N) -> " );
                     capture = -1;
                     if ( yesNo() )
                     {
                        if ( !( tempFile = freopen( aryTempFileName, "w+", tempFile ) ) )
                        {
                           fatalPerror( "capture prompt: reopen temp file for truncate", "Capture file error" );
                        }
                     }
                     else
                     {
                        fseek( tempFile, 0L, SEEK_END );
                     }
                     capture = 1;
                  }
                  flagsConfiguration.isLastSave = 0;
               }
               else
               {
                  fflush( tempFile );
               }
               continue;
            }
            else if ( inputChar > 127 || !*aryMacro[macroKey = inputChar] )
            {
               printf( "[Undefined command]" );
               wasUndefinedCommand = 1;
               continue;
            }
            else
            {
               return ( aryMacro[macroKey][macroPosition++] );
            }
         }
         /* If we just printed the undefined command blurb, let's erase it */
         else if ( wasUndefinedCommand )
         {
            wasUndefinedCommand = 0;
            printf( "\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b                   \b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b" );
         }
         /* Did the aryUser just hit the command key? */
         if ( inputChar == commandKey && !flagsConfiguration.isConfigMode )
         {
            isMacroNext = 1;
            printf( "[Command]" );
         }
         else
         {
            return ( inputChar ); /* Return the next character to the caller */
         }
      }

      /* Handle any incoming traffic in the network input buffer */
      if ( NET_INPUT_LEFT() )
      {
         while ( NET_INPUT_LEFT() )
         {
            if ( telReceive( netget() ) < 0 )
            {
               return ( -1 );
            }
         }
         continue;
      }
      /* Flush out any output buffers */
      if ( netflush() < 0 )
      {
         stdPrintf( "\r\n" );
         fatalPerror( "send", "Network error" );
      }
      if ( fflush( stdout ) < 0 )
      {
         stdPrintf( "\r\n" );
         fatalPerror( "write", "Local error" );
      }

      /*
   	 * Wait for the next event from either the network or the aryUser,
   	 * note that if we are running with a child process, we'll be
   	 * ignoring the aryUser input entirely -- we're only concerned
   	 * with network traffic until we no longer have a child process
   	 * to swallow up our data for us.
   	 */
      eventResult = waitNextEvent();

      /* The aryUser has input waiting for us to process */
      if ( eventResult & 1 )
      {
         inputChar = ptyget();
         if ( inputChar < 0 )
         {
            stdPrintf( "\r\n" );
            fatalPerror( "read", "Local error" );
         }
         inputChar &= 0x7f;
         macroPosition = -1;
         continue;
      }
      /* The network has input waiting for us to process */
      if ( eventResult & 2 )
      {
         errno = 0;
         if ( ( inputChar = netget() ) < 0 )
         {
            if ( errno )
            {
               stdPrintf( "\r\n" );
               fatalPerror( "recv", "Network error" );
            }
            else
            {
               if ( childPid )
               {
                  stdPrintf( "\r\n\n\007[DISCONNECTED]\r\n\n\007" );
               }
               else
               {
                  stdPrintf( "\r\n[Disconnected]\r\n" );
               }
               myExit();
            }
         }
         /* Handle net traffic */
         if ( telReceive( inputChar ) < 0 )
         {
            return ( -1 );
         }
      }
   }
}
