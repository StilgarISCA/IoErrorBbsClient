/*
 * Copyright (C) 2024-2026 Stilgar
 * Copyright (C) 1995-2003 Michael Hampton
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

/*
 * This file handles tracked output to the BBS and automatic away replies.
 */
#include "defs.h"
#include "ext.h"

static const char replymsg[] = "+!R ";

void replyMessage( void );
void sendAnX( void );
void sendTrackedBuffer( const char *ptrBuffer, size_t length );
void sendTrackedChar( int inputChar );
void sendTrackedNewline( void );

/* fake getFiveLines for the bbs */
void replyMessage( void )
{
   int lineIndex;
   int charIndex;

   sendBlock();
   lineIndex = (int)strlen( replymsg );
   sendTrackedBuffer( replymsg, (size_t)lineIndex );
   for ( lineIndex = 0; lineIndex < 5 && *aryAwayMessageLines[lineIndex]; lineIndex++ )
   {
      charIndex = (int)strlen( aryAwayMessageLines[lineIndex] );
      sendTrackedBuffer( aryAwayMessageLines[lineIndex], (size_t)charIndex );
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

void sendTrackedNewline( void )
{
   sendTrackedChar( '\n' );
}
