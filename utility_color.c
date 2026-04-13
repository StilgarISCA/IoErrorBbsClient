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

#define ifansi if ( flagsConfiguration.shouldUseAnsi )

int colorize( const char *str )
{
   char aryAnsiSequence[32];
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
            switch ( *++ptrText )
            {
               case '@':
                  putchar( (int)'@' );
                  break;
               case 'k':
                  ifansi
                  {
                     formatAnsiBackgroundSequence( aryAnsiSequence, sizeof( aryAnsiSequence ), 0 );
                     printf( "%s", aryAnsiSequence );
                  }
                  break;
               case 'K':
                  ifansi
                  {
                     formatAnsiForegroundSequence( aryAnsiSequence, sizeof( aryAnsiSequence ), 0 );
                     printf( "%s", aryAnsiSequence );
                  }
                  break;
               case 'r':
                  ifansi
                  {
                     formatAnsiBackgroundSequence( aryAnsiSequence, sizeof( aryAnsiSequence ), 1 );
                     printf( "%s", aryAnsiSequence );
                  }
                  break;
               case 'R':
                  ifansi
                  {
                     formatAnsiForegroundSequence( aryAnsiSequence, sizeof( aryAnsiSequence ), 1 );
                     printf( "%s", aryAnsiSequence );
                  }
                  break;
               case 'g':
                  ifansi
                  {
                     formatAnsiBackgroundSequence( aryAnsiSequence, sizeof( aryAnsiSequence ), 2 );
                     printf( "%s", aryAnsiSequence );
                  }
                  break;
               case 'G':
                  ifansi
                  {
                     formatAnsiForegroundSequence( aryAnsiSequence, sizeof( aryAnsiSequence ), 2 );
                     printf( "%s", aryAnsiSequence );
                  }
                  break;
               case 'y':
                  ifansi
                  {
                     formatAnsiBackgroundSequence( aryAnsiSequence, sizeof( aryAnsiSequence ), 3 );
                     printf( "%s", aryAnsiSequence );
                  }
                  break;
               case 'Y':
                  ifansi
                  {
                     formatAnsiForegroundSequence( aryAnsiSequence, sizeof( aryAnsiSequence ), 3 );
                     printf( "%s", aryAnsiSequence );
                  }
                  break;
               case 'b':
                  ifansi
                  {
                     formatAnsiBackgroundSequence( aryAnsiSequence, sizeof( aryAnsiSequence ), 4 );
                     printf( "%s", aryAnsiSequence );
                  }
                  break;
               case 'B':
                  ifansi
                  {
                     formatAnsiForegroundSequence( aryAnsiSequence, sizeof( aryAnsiSequence ), 4 );
                     printf( "%s", aryAnsiSequence );
                  }
                  break;
               case 'm':
               case 'p':
                  ifansi
                  {
                     formatAnsiBackgroundSequence( aryAnsiSequence, sizeof( aryAnsiSequence ), 5 );
                     printf( "%s", aryAnsiSequence );
                  }
                  break;
               case 'M':
               case 'P':
                  ifansi
                  {
                     formatAnsiForegroundSequence( aryAnsiSequence, sizeof( aryAnsiSequence ), 5 );
                     printf( "%s", aryAnsiSequence );
                  }
                  break;
               case 'c':
                  ifansi
                  {
                     formatAnsiBackgroundSequence( aryAnsiSequence, sizeof( aryAnsiSequence ), 6 );
                     printf( "%s", aryAnsiSequence );
                  }
                  break;
               case 'C':
                  ifansi
                  {
                     formatAnsiForegroundSequence( aryAnsiSequence, sizeof( aryAnsiSequence ), 6 );
                     printf( "%s", aryAnsiSequence );
                  }
                  break;
               case 'w':
                  ifansi
                  {
                     formatAnsiBackgroundSequence( aryAnsiSequence, sizeof( aryAnsiSequence ), 7 );
                     printf( "%s", aryAnsiSequence );
                  }
                  break;
               case 'W':
                  ifansi
                  {
                     formatAnsiForegroundSequence( aryAnsiSequence, sizeof( aryAnsiSequence ), 7 );
                     printf( "%s", aryAnsiSequence );
                  }
                  break;
               case 'd':
                  ifansi
                  {
                     formatAnsiBackgroundSequence( aryAnsiSequence, sizeof( aryAnsiSequence ), 9 );
                     printf( "%s", aryAnsiSequence );
                  }
                  break;
               case 'D':
                  ifansi
                  {
                     formatAnsiForegroundSequence( aryAnsiSequence, sizeof( aryAnsiSequence ), 9 );
                     printf( "%s", aryAnsiSequence );
                  }
                  break;
               default:
                  break;
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

void printAnsiForegroundColorValue( int colorValue )
{
   char aryAnsiSequence[32];

   if ( !flagsConfiguration.shouldUseAnsi )
   {
      return;
   }

   formatAnsiForegroundSequence( aryAnsiSequence, sizeof( aryAnsiSequence ),
                                 colorValue );
   stdPrintf( "%s", aryAnsiSequence );
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
