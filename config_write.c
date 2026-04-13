/*
 * Copyright (C) 2024-2026 Stilgar
 * Copyright (C) 1995-2003 Michael Hampton
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

/*
 * This file writes the client configuration back to .bbsrc.
 */
#include "defs.h"
#include "ext.h"

void writeBbsRc( void )
{
   int itemIndex, innerIndex;
   const char *ptrColorName;
   const friend *ptrFriend;
   const int *ptrColorValues;

   deleteFile( aryBbsFriendsName );
   rewind( ptrBbsRc );
   fprintf( ptrBbsRc, "aryEditor %s\n", aryEditor );
   /* Change:  site line will always be written */
   fprintf( ptrBbsRc, "site %s %d%s\n", aryBbsHost, bbsPort,
            shouldUseSsl ? " secure" : "" );
   fprintf( ptrBbsRc, "commandkey %s\n", strCtrl( commandKey ) );
   fprintf( ptrBbsRc, "quit %s\n", strCtrl( quitKey ) );
   fprintf( ptrBbsRc, "susp %s\n", strCtrl( suspKey ) );
   fprintf( ptrBbsRc, "shellkey %s\n", strCtrl( shellKey ) );
   fprintf( ptrBbsRc, "capture %s\n", strCtrl( captureKey ) );
   fprintf( ptrBbsRc, "awaykey %s\n", strCtrl( awayKey ) );
   fprintf( ptrBbsRc, "squelch %d\n",
            ( flagsConfiguration.shouldSquelchPost ? 2 : 0 ) +
               ( flagsConfiguration.shouldSquelchExpress ? 1 : 0 ) );
   fprintf( ptrBbsRc, "keepalive %d\n",
            flagsConfiguration.shouldUseTcpKeepalive ? 1 : 0 );
   fprintf( ptrBbsRc, "clickableurls %d\n",
            flagsConfiguration.shouldEnableClickableUrls ? 1 : 0 );
   fprintf( ptrBbsRc, "titlebar %d\n", flagsConfiguration.shouldEnableTitleBar ? 1 : 0 );
   fprintf( ptrBbsRc, "screenreader %d\n",
            flagsConfiguration.isScreenReaderModeEnabled ? 1 : 0 );
   fprintf( ptrBbsRc, "autocomplete %d\n",
            flagsConfiguration.shouldEnableNameAutocomplete ? 1 : 0 );
   if ( *aryAutoName )
   {
      fprintf( ptrBbsRc, "aryAutoName %s\n", aryAutoName );
   }
#ifdef ENABLE_SAVE_PASSWORD
   if ( *aryAutoPassword )
   {
      fprintf( ptrBbsRc, "autopass %s\n", aryAutoPassword );
   }
#endif
   ptrColorValues = (const int *)&color;
   fprintf( ptrBbsRc, "color" );
   for ( itemIndex = 0; itemIndex < COLOR_FIELD_COUNT; itemIndex++ )
   {
      ptrColorName = colorNameFromValue( ptrColorValues[itemIndex] );
      if ( ptrColorName != NULL )
      {
         fprintf( ptrBbsRc, " %s", ptrColorName );
      }
      else
      {
         fprintf( ptrBbsRc, " %d", ptrColorValues[itemIndex] );
      }
   }
   fprintf( ptrBbsRc, "\n" );
   if ( flagsConfiguration.shouldAutoAnswerAnsiPrompt )
   {
      fprintf( ptrBbsRc, "autoansi\n" );
   }
   if ( **aryAwayMessageLines )
   {
      for ( itemIndex = 0; itemIndex < 5 && *aryAwayMessageLines[itemIndex];
            itemIndex++ )
      {
         fprintf( ptrBbsRc, "a%d %s\n", itemIndex + 1, aryAwayMessageLines[itemIndex] );
      }
   }
   fprintf( ptrBbsRc, "version %d\n", version );
   if ( flagsConfiguration.shouldUseBold )
   {
      fprintf( ptrBbsRc, "bold\n" );
   }
   if ( !isXland )
   {
      fprintf( ptrBbsRc, "xland\n" );
   }
   for ( itemIndex = 0; itemIndex < (int)friendList->nitems; itemIndex++ )
   {
      ptrFriend = (friend *)friendList->items[itemIndex];
      fprintf( ptrBbsRc, "friend %-20s   %s\n", ptrFriend->name, ptrFriend->info );
   }
   for ( itemIndex = 0; itemIndex < (int)enemyList->nitems; itemIndex++ )
   {
      fprintf( ptrBbsRc, "enemy %s\n", (char *)enemyList->items[itemIndex] );
   }
   for ( itemIndex = 0; itemIndex < 128; itemIndex++ )
   {
      if ( *aryMacro[itemIndex] )
      {
         fprintf( ptrBbsRc, "aryMacro %s ", strCtrl( itemIndex ) );
         for ( innerIndex = 0; aryMacro[itemIndex][innerIndex]; innerIndex++ )
         {
            fprintf( ptrBbsRc, "%s", strCtrl( aryMacro[itemIndex][innerIndex] ) );
         }
         fprintf( ptrBbsRc, "\n" );
      }
   }
   for ( itemIndex = 33; itemIndex < 128; itemIndex++ )
   {
      if ( aryKeyMap[itemIndex] != itemIndex )
      {
         fprintf( ptrBbsRc, "aryKeyMap %c %c\n", itemIndex, aryKeyMap[itemIndex] );
      }
   }
   fflush( ptrBbsRc );
   truncateBbsRc( ftell( ptrBbsRc ) );
}
