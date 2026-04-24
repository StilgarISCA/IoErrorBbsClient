/*
 * Copyright (C) 2024-2026 Stilgar
 * Copyright (C) 1995-2003 Michael Hampton
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

/*
 * This file handles tracked output to the BBS and automatic away replies.
 */
#include "defs.h"
#include "client_globals.h"
#include "config_globals.h"
#include "getline_input.h"
#include "network_globals.h"
#include "telnet.h"
#include "utility.h"

static const char replymsg[] = "+!R ";
static void sendTrackedCString( const char *ptrText );

/// @brief Send the configured away message as an automatic reply.
///
/// @return This function does not return a value.
void replyMessage( void )
{
   int lineIndex;

   sendBlock();
   sendTrackedCString( replymsg );
   for ( lineIndex = 0; lineIndex < 5 && *aryAwayMessageLines[lineIndex]; lineIndex++ )
   {
      sendTrackedCString( aryAwayMessageLines[lineIndex] );
      sendTrackedNewline();
      stdPrintf( "%s\r\n", aryAwayMessageLines[lineIndex] );
   }
   if ( lineIndex < 5 )
   {
      sendTrackedNewline();
   }
   sendingXState = SX_NOT;
}

/// @brief Start the client-side express-message send flow.
///
/// @return This function does not return a value.
void sendAnX( void )
{
   sendingXState = SX_WANT_TO;
   sendTrackedChar( 'x' );
   sendingXState = SENDING_X_STATE_SENT_COMMAND_X;
}

/// @brief Send a buffer to the BBS while updating the tracked byte count.
///
/// @param ptrBuffer Bytes to send.
/// @param length Number of bytes to send.
///
/// @return This function does not return a value.
void sendTrackedBuffer( const char *ptrBuffer, size_t length )
{
   size_t itemIndex;

   for ( itemIndex = 0; itemIndex < length; itemIndex++ )
   {
      netPutChar( ptrBuffer[itemIndex] );
   }
   byte += (long)length;
}

/// @brief Send one character to the BBS while updating the tracked byte count.
///
/// @param inputChar Character to send.
///
/// @return This function does not return a value.
void sendTrackedChar( int inputChar )
{
   netPutChar( inputChar );
   byte++;
}

/// @brief Send a NUL-terminated string to the BBS while updating the byte count.
///
/// @param ptrText String to send.
///
/// @return This function does not return a value.
static void sendTrackedCString( const char *ptrText )
{
   sendTrackedBuffer( ptrText, strlen( ptrText ) );
}

/// @brief Send a newline byte to the BBS while updating the tracked byte count.
///
/// @return This function does not return a value.
void sendTrackedNewline( void )
{
   sendTrackedChar( '\n' );
}
