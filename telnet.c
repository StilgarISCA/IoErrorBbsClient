/*
 * Copyright (C) 2024-2026 Stilgar
 * Copyright (C) 1995-2003 Michael Hampton
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

/*
 * This handles the information flowing into the client from the BBS.  It
 * basically understands a very limited subset of the telnet protocol (enough
 * to "fake it" with the BBS) with some extensions to allow the extra features
 * the client provides.  The telnet stuff is unimportant, but it'll confuse the
 * BBS if you alter it.
 *
 * The client tells the BBS it is a client rather a telnet program when it first
 * connects, after that the BBS acts differently, letting the client know when
 * something in particular is being done (entering an X message, posting, etc.)
 * to allow the client to handle that however it wants, as well as providing
 * extra protocol for the start/end of messages/X'aryString and the special who list.
 *
 * This is made more complex by the fact that the client doesn't know it should
 * handle (for example) an X message differently until it receives word from
 * the BBS that an X message should be entered -- but by this time the client
 * may have already sent some of the X message over the network instead of
 * gathering it up locally.  So when the BBS tells the client to go into the
 * local X message mode it also tells the client how many bytes have been
 * passed to it (they count them in the same manner) and throws isAway the excess
 * on its side.  The client has buffered this excess on its side and therefore
 * make it available locally instead of having it lost forever.  Boy, that was
 * a pisser when I realized I'd have to do that!
 */
#include "defs.h"
#include "browser.h"
#include "client.h"
#include "client_globals.h"
#include "config_menu.h"
#include "edit.h"
#include "filter.h"
#include "filter_globals.h"
#include "getline_input.h"
#include "network_globals.h"
#include "utility.h"
#include "telnet.h"

static int handleTelnetDataState( int inputByte, int *ptrState );
static int handleTelnetGetCommand( unsigned char *aryTelnetBuffer );
static int handleTelnetGetState( int inputByte, int *ptrState,
                                 unsigned char *aryTelnetBuffer,
                                 int *ptrTelnetBufferPos );
static int handleTelnetIacState( int inputByte, int *ptrState,
                                 unsigned char *aryTelnetBuffer,
                                 int *ptrTelnetBufferPos );
static int handleTelnetVoidState( int inputByte, int *ptrState );

static int handleTelnetDataState( int inputByte, int *ptrState );
static int handleTelnetGetCommand( unsigned char *aryTelnetBuffer );
static int handleTelnetGetState( int inputByte, int *ptrState,
                                 unsigned char *aryTelnetBuffer,
                                 int *ptrTelnetBufferPos );
static int handleTelnetIacState( int inputByte, int *ptrState,
                                 unsigned char *aryTelnetBuffer,
                                 int *ptrTelnetBufferPos );
static int handleTelnetVoidState( int inputByte, int *ptrState );


static int handleTelnetDataState( int inputByte, int *ptrState )
{
   if ( inputByte == IAC )
   {
      *ptrState = TS_IAC;
      return 0;
   }
   if ( whoListProgress )
   {
      filterWhoList( inputByte );
   }
   else if ( isExpressMessageInProgress )
   {
      filterExpress( inputByte );
   }
   else if ( postProgressState )
   {
      filterPost( inputByte );
   }
   else
   {
      filterData( inputByte );
   }

   return 0;
}


static int handleTelnetGetCommand( unsigned char *aryTelnetBuffer )
{
   int outputIndex;
   const char *ptrInputString;

   switch ( *aryTelnetBuffer )
   {
      case G_POST:
         if ( flagsConfiguration.isPosting )
         {
            flagsConfiguration.shouldCheckExpress = 0;
            return -1;
         }
         makeMessage( aryTelnetBuffer[1] );
         break;

      case G_FIVE:
         getFiveLines( aryTelnetBuffer[1] );
         if ( aryTelnetBuffer[1] == 1 && xlandQueue->itemCount > 0 )
         {
            sendAnX();
         }
         break;

      case G_NAME:
         sendBlock();
         ptrInputString = getName( aryTelnetBuffer[1] );
         outputIndex = (int)strlen( ptrInputString );
         sendTrackedBuffer( ptrInputString, (size_t)outputIndex );
         if ( *ptrInputString != CTRL_D )
         {
            sendTrackedNewline();
         }
         break;

      case G_STR:
         sendBlock();
         getString( aryTelnetBuffer[1], (char *)aryTelnetBuffer, -1 );
         outputIndex = (int)strlen( (char *)aryTelnetBuffer );
         sendTrackedBuffer( (char *)aryTelnetBuffer, (size_t)outputIndex );
         sendTrackedNewline();
         break;

      case CONFIG:
         sendBlock();
         configBbsRc();
         sendTrackedNewline();
         break;
   }

   return 0;
}


static int handleTelnetGetState( int inputByte, int *ptrState,
                                 unsigned char *aryTelnetBuffer,
                                 int *ptrTelnetBufferPos )
{
   aryTelnetBuffer[( *ptrTelnetBufferPos )++] = (unsigned char)inputByte;
   if ( *ptrTelnetBufferPos != 5 )
   {
      return 0;
   }

   targetByte = byte;
   bytePosition = ( (long)aryTelnetBuffer[2] << 16 ) +
                  ( (long)aryTelnetBuffer[3] << 8 ) +
                  aryTelnetBuffer[4];
   byte = bytePosition;

   if ( byte < targetByte - (int)( sizeof arySavedBytes ) - 1 )
   {
      stdPrintf( "\r\n[Error:  characters lost during transmission]\r\n" );
   }

   *ptrState = TS_DATA;
   *ptrTelnetBufferPos = 0;
   return handleTelnetGetCommand( aryTelnetBuffer );
}


static int handleTelnetIacState( int inputByte, int *ptrState,
                                 unsigned char *aryTelnetBuffer,
                                 int *ptrTelnetBufferPos )
{
   switch ( inputByte )
   {
      case CLIENT:
         *ptrState = TS_DATA;
         netPutChar( IAC );
         netPutChar( CLIENT );
         break;

      case S_WHO:
         *ptrState = TS_DATA;
         whoListProgress = 1;
         break;

      case G_POST:
      case G_FIVE:
      case G_NAME:
      case G_STR:
      case CONFIG:
         *ptrState = TS_GET;
         aryTelnetBuffer[( *ptrTelnetBufferPos )++] = (unsigned char)inputByte;
         break;

      case START:
         *ptrState = TS_DATA;
         byte = 1;
         netPutChar( IAC );
         netPutChar( START3 );
         break;

      case POST_S:
         *ptrState = TS_DATA;
         postHeaderActive = 1;
         postProgressState = 1;
         filterPost( -1 );
         break;

      case POST_E:
         *ptrState = TS_DATA;
         postHeaderActive = 0;
         ptrPostBuffer = 0;
         postProgressState = 0;
         isPostJustEnded = 1;
         filterPost( -1 );
         break;

      case MORE_M:
         *ptrState = TS_DATA;
         flagsConfiguration.isMorePromptActive ^= 1;
         if ( !flagsConfiguration.isMorePromptActive &&
              flagsConfiguration.shouldUseAnsi )
         {
            morePromptHelper();
         }
         break;

      case XMSG_S:
         *ptrState = TS_DATA;
         *aryExpressParsing = 0;
         isExpressMessageHeaderActive = 1;
         isExpressMessageInProgress = 1;
         filterExpress( -1 );
         break;

      case XMSG_E:
         *ptrState = TS_DATA;
         *aryExpressParsing = 0;
         isExpressMessageHeaderActive = 0;
         isExpressMessageInProgress = 0;
         ptrExpressMessageBuffer = aryExpressMessageBuffer;
         filterExpress( -1 );
         if ( shouldSendExpressMessage )
         {
            sendAnX();
            shouldSendExpressMessage = 0;
         }
         break;

      case DO:
      case DONT:
      case WILL:
      case WONT:
         *ptrState = TS_VOID;
         break;

      default:
         *ptrState = TS_DATA;
         break;
   }

   return 0;
}


static int handleTelnetVoidState( int inputByte, int *ptrState )
{
   netPutChar( IAC );
   netPutChar( WONT );
   netPutChar( inputByte );
   *ptrState = TS_DATA;
   return 0;
}


/*
 * Send signal that block of data follows -- this is a signal to the bbs that
 * it should stop ignoring what we send it, since it begins ignoring and
 * throwing isAway everything it receives from the time it sends an IAC G_*
 * command until the time it receives an IAC BLOCK command.
 */
void sendBlock( void )
{
   netPutChar( IAC );
   netPutChar( BLOCK );
}


/*
 * Send a NAWS command to the bbs to tell it what our window size is.
 */
void sendNaws( void )
{
   if ( oldRows != getWindowSize() )
   {
      char aryString[10];
      register int outputIndex;

      /* Old window max was 70 */
      if ( rows > NAWS_ROWS_MAX || rows < NAWS_ROWS_MIN )
      {
         rows = WINDOW_ROWS_DEFAULT;
      }
      else
      {
         oldRows = rows;
      }
      snprintf( aryString, sizeof( aryString ), "%c%c%c%c%c%c%c%c%c", IAC, SB, TELOPT_NAWS, 0, 0, 0, rows, IAC, SE );
      for ( outputIndex = 0; outputIndex < 9; outputIndex++ )
      {
         netPutChar( aryString[outputIndex] );
      }
   }
}


/*
 * Initialize telnet negotations with the bbs -- we don't really do the
 * negotations, we just tell the bbs what it needs to hear, since we don't need
 * to negotiate because we know the correct state to put the terminal in. The
 * BBS queue daemon also expects the IAC CLIENT command.
 */
void telInit( void )
{
   netPutChar( IAC );
   netPutChar( CLIENT2 );
   netPutChar( IAC );
   netPutChar( SB );
   netPutChar( TELOPT_ENVIRON );
   netPutChar( 0 );
   netPutChar( 1 );
   netPutChar( 0 );
   netPuts( "USER" );
   netPutChar( 0 );
   netPuts( aryUser );
   netPutChar( IAC );
   netPutChar( SE );
   sendNaws();
}


int telReceive( int inputByte )
{
   static int state = TS_DATA;               /* Current state of telnet state machine */
   static unsigned char aryTelnetBuffer[80]; /* Generic buffer */
   static int telnetBufferPos = 0;           /* Pointer into generic buffer */

   switch ( state )
   {
      case TS_DATA:
         return handleTelnetDataState( inputByte, &state );

      case TS_IAC:
         return handleTelnetIacState( inputByte, &state, aryTelnetBuffer,
                                      &telnetBufferPos );

      case TS_GET:
         return handleTelnetGetState( inputByte, &state, aryTelnetBuffer,
                                      &telnetBufferPos );

      case TS_VOID:
         return handleTelnetVoidState( inputByte, &state );

      default:
         state = TS_DATA;
         break;
   }
   return ( 0 );
}

