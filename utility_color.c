/*
 * Copyright (C) 2024-2026 Stilgar
 * Copyright (C) 1995-2003 Michael Hampton
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

/*
 * This file handles ANSI color output helpers.
 */
#include "defs.h"
#include "ext.h"

static void printAnsiColorValue( int colorValue, bool isBackground );
static bool tryPrintLegacyAtColor( int inputChar );

int colorize( const char *str )
{
   const char *ptrText;

   for ( ptrText = str; *ptrText; ptrText++ )
   {
      if ( *ptrText == '@' )
      {
         if ( !*( ptrText + 1 ) )
         {
            ptrText--;
         }
         else
         {
            if ( !tryPrintLegacyAtColor( *++ptrText ) )
            {
               /* Ignore unknown @-tokens for compatibility with the legacy formatter. */
            }
         }
      }
      else
      {
         stdPutChar( (int)*ptrText );
      }
   }
   return 1;
}

static void printAnsiColorValue( int colorValue, bool isBackground )
{
   char aryAnsiSequence[32];

   if ( !flagsConfiguration.shouldUseAnsi )
   {
      return;
   }

   if ( isBackground )
   {
      formatAnsiBackgroundSequence( aryAnsiSequence, sizeof( aryAnsiSequence ), colorValue );
   }
   else
   {
      formatAnsiForegroundSequence( aryAnsiSequence, sizeof( aryAnsiSequence ), colorValue );
   }
   stdPrintf( "%s", aryAnsiSequence );
}

void printAnsiForegroundColorValue( int colorValue )
{
   printAnsiColorValue( colorValue, false );
}

void printThemedMnemonicText( const char *ptrText, int defaultColor )
{
   const char *ptrScan;

   if ( ptrText == NULL )
   {
      return;
   }

   if ( !flagsConfiguration.shouldUseAnsi )
   {
      stdPrintf( "%s", ptrText );
      return;
   }

   printAnsiForegroundColorValue( defaultColor );

   for ( ptrScan = ptrText; *ptrScan; )
   {
      if ( ptrScan[0] == '<' && ptrScan[1] != '\0' && ptrScan[2] == '>' )
      {
         printAnsiForegroundColorValue( color.forum );
         stdPutChar( ptrScan[1] );
         printAnsiForegroundColorValue( defaultColor );
         ptrScan += 3;
         continue;
      }

      stdPutChar( *ptrScan );
      ptrScan++;
   }
}

static bool tryPrintLegacyAtColor( int inputChar )
{
   switch ( inputChar )
   {
      case '@':
         stdPutChar( '@' );
         return true;
      case 'k':
         printAnsiColorValue( 0, true );
         return true;
      case 'K':
         printAnsiColorValue( 0, false );
         return true;
      case 'r':
         printAnsiColorValue( 1, true );
         return true;
      case 'R':
         printAnsiColorValue( 1, false );
         return true;
      case 'g':
         printAnsiColorValue( 2, true );
         return true;
      case 'G':
         printAnsiColorValue( 2, false );
         return true;
      case 'y':
         printAnsiColorValue( 3, true );
         return true;
      case 'Y':
         printAnsiColorValue( 3, false );
         return true;
      case 'b':
         printAnsiColorValue( 4, true );
         return true;
      case 'B':
         printAnsiColorValue( 4, false );
         return true;
      case 'm':
      case 'p':
         printAnsiColorValue( 5, true );
         return true;
      case 'M':
      case 'P':
         printAnsiColorValue( 5, false );
         return true;
      case 'c':
         printAnsiColorValue( 6, true );
         return true;
      case 'C':
         printAnsiColorValue( 6, false );
         return true;
      case 'w':
         printAnsiColorValue( 7, true );
         return true;
      case 'W':
         printAnsiColorValue( 7, false );
         return true;
      case 'd':
         printAnsiColorValue( 9, true );
         return true;
      case 'D':
         printAnsiColorValue( 9, false );
         return true;
      default:
         return false;
   }
}
