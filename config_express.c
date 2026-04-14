/*
 * Copyright (C) 2024-2026 Stilgar
 * Copyright (C) 1995-2003 Michael Hampton
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

/*
 * This file handles express and away-message configuration from the client
 * configuration menu.
 */
#include "defs.h"
#include "client_globals.h"
#include "color.h"
#include "config_globals.h"
#include "config_menu.h"
#include "getline_input.h"
#include "utility.h"

static const char *CONFIG_EXPRESS_MENU_KEYS = "axq \n";

static void clearAwayMessageLines( void )
{
   int itemIndex;

   for ( itemIndex = 0; itemIndex < 5; itemIndex++ )
   {
      *aryAwayMessageLines[itemIndex] = 0;
   }
}

void expressConfig( void )
{
   stdPrintf( "Express\r\n" );

   while ( true )
   {
      int inputChar;

      printThemedMnemonicText( "\r\n<A>way  <X>Land  <Q>uit", color.number );
      printThemedMnemonicText( "\r\nExpress config -> ", color.forum );
      printAnsiForegroundColorValue( color.text );

      inputChar = readValidatedMenuKey( CONFIG_EXPRESS_MENU_KEYS );

      switch ( inputChar )
      {
         case 'a':
            stdPrintf( "Away from keyboard\r\n\n" );
            newAwayMessage();
            break;

         case 'x':
            stdPrintf( "XLand\r\n\nAutomatically reply to X messages you receive? (%s) -> ",
                       isXland ? "Yes" : "No" );
            isXland = yesNoDefault( isXland );
            break;

         case 'q':
         case ' ':
         case '\n':
            stdPrintf( "Quit\r\n" );
            return;
            /* NOTREACHED */

         default:
            break;
      }
   }
}

void newAwayMessage( void )
{
   int itemIndex;

   if ( **aryAwayMessageLines )
   {
      stdPrintf( "Current away from keyboard message is:\r\n\n" );
      for ( itemIndex = 0; itemIndex < 5 && *aryAwayMessageLines[itemIndex];
            itemIndex++ )
      {
         stdPrintf( " %s\r\n", aryAwayMessageLines[itemIndex] );
      }
      stdPrintf( "\r\nDo you wish to change this? -> " );
      if ( !yesNo() )
      {
         return;
      }
      stdPrintf( "\r\nOk, you have five lines to do something creative.\r\n\n" );
   }
   else
   {
      stdPrintf( "Enter a message, up to 5 lines\r\n\n" );
   }

   clearAwayMessageLines();
   for ( itemIndex = 0;
         itemIndex < 5 &&
         ( !itemIndex || *aryAwayMessageLines[itemIndex - 1] );
         itemIndex++ )
   {
      stdPrintf( ">" );
      getString( itemIndex ? 78 : 74, aryAwayMessageLines[itemIndex], itemIndex );
   }
}
