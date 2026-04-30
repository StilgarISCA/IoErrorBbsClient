/*
 * Copyright (C) 2024-2026 Stilgar
 * Copyright (C) 1995-2003 Michael Hampton
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

/*
 * This file handles tracked output to the BBS and automatic away replies.
 */
#include "client_globals.h"
#include "config_globals.h"
#include "defs.h"
#include "getline_input.h"
#include "network_globals.h"
#include "telnet.h"
#include "utility.h"
static const char replymsg[] = "+!R ";
static void sendTrackedCString( const char *ptrText );
static void trackSentChar( int inputChar, bool canReplay );

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
      sendTrackedChar( ptrBuffer[itemIndex] );
   }
}

/// @brief Send one character to the BBS while updating the tracked byte count.
///
/// @param inputChar Character to send.
///
/// @return This function does not return a value.
void sendTrackedChar( int inputChar )
{
   netPutChar( inputChar );
   trackReplayableSentChar( inputChar );
}

/// @brief Send one character to the BBS without making it eligible for replay.
///
/// @param inputChar Character to send.
///
/// @return This function does not return a value.
void sendTrackedCharWithoutReplay( int inputChar )
{
   netPutChar( inputChar );
   trackUnreplayableSentChar( inputChar );
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

/// @brief Save one sent byte for protocol replay and advance the byte counter.
///
/// @param inputChar Character to save in the replay buffer.
///
/// @return This function does not return a value.
void trackReplayableSentChar( int inputChar )
{
   trackSentChar( inputChar, true );
}

/// @brief Save one sent byte and advance the byte counter.
///
/// @param inputChar Character to save in the replay buffer.
/// @param canReplay `true` if the byte can be replayed after a BBS sync request.
///
/// @return This helper does not return a value.
static void trackSentChar( int inputChar, bool canReplay )
{
   if ( byte )
   {
      size_t index = (size_t)( byte % (long)sizeof arySavedBytes );

      arySavedBytes[index] = (unsigned char)inputChar;
      arySavedByteCanReplay[index] = canReplay;
   }
   byte++;
}

/// @brief Save one sent byte without allowing protocol replay and advance the byte counter.
///
/// @param inputChar Character to save in the replay buffer.
///
/// @return This function does not return a value.
void trackUnreplayableSentChar( int inputChar )
{
   trackSentChar( inputChar, false );
}
