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
#include "network_globals.h"
#include "proto.h"

static const char replymsg[] = "+!R ";
static void sendTrackedCString( const char *ptrText );

/* fake getFiveLines for the bbs */
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

void sendAnX( void )
{
   sendingXState = SX_WANT_TO;
   sendTrackedChar( 'x' );
   sendingXState = SENDING_X_STATE_SENT_COMMAND_X;
}

void sendTrackedBuffer( const char *ptrBuffer, size_t length )
{
   size_t itemIndex;

   for ( itemIndex = 0; itemIndex < length; itemIndex++ )
   {
      netPutChar( ptrBuffer[itemIndex] );
   }
   byte += (long)length;
}

void sendTrackedChar( int inputChar )
{
   netPutChar( inputChar );
   byte++;
}

static void sendTrackedCString( const char *ptrText )
{
   sendTrackedBuffer( ptrText, strlen( ptrText ) );
}

void sendTrackedNewline( void )
{
   sendTrackedChar( '\n' );
}
