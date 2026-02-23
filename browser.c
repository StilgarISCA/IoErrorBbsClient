/*
 * Copyright (C) 2024-2026 Stilgar
 * Copyright (C) 1995-2003 Michael Hampton
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "defs.h"
#include "ext.h"
#include "proto.h"

#include "browser.h"

void filterUrl( const char *ptrLine )
{
   static int multiline = 0;
   static char aryUrlBuffer[1024];
   char *ptrCursor;
   char *ptrNext;

   if ( !urlQueue )
   {
      return;
   }

   if ( !multiline )
   {
      snprintf( aryUrlBuffer, sizeof( aryUrlBuffer ), "%s", ptrLine );
   }
   else
   {
      size_t existingLength;

      existingLength = strlen( aryUrlBuffer );
      if ( existingLength < sizeof( aryUrlBuffer ) - 1 )
      {
         snprintf( aryUrlBuffer + existingLength, sizeof( aryUrlBuffer ) - existingLength, "%s", ptrLine );
      }
   }

   {
      size_t urlLength;

      urlLength = strlen( aryUrlBuffer );
      while ( urlLength > 0 )
      {
         size_t index;

         index = urlLength - 1;
         if ( aryUrlBuffer[index] == ' ' || aryUrlBuffer[index] == '\t' || aryUrlBuffer[index] == '\r' )
         {
            aryUrlBuffer[index] = 0;
         }
         else
         {
            break;
         }
         urlLength = index;
      }
   }

   {
      size_t existingLength;

      existingLength = strlen( aryUrlBuffer );
      if ( existingLength < sizeof( aryUrlBuffer ) - 1 )
      {
         snprintf( aryUrlBuffer + existingLength, sizeof( aryUrlBuffer ) - existingLength, " " );
      }
   }

   ptrCursor = strstr( aryUrlBuffer, "http://" );
   if ( ptrCursor == NULL )
   {
      ptrCursor = strstr( aryUrlBuffer, "ftp://" );
   }
   if ( ptrCursor == NULL )
   {
      aryUrlBuffer[0] = 0;
      multiline = 0;
      return;
   }

   for ( ptrNext = ptrCursor; *ptrNext; ptrNext++ )
   {
      if ( findChar( ":/abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789$-_@.&+,=;?%~{|}", *ptrNext ) )
      {
         continue;
      }
      *ptrNext = 0;

      if ( ( !multiline && ptrCursor == ptrLine && ptrNext > ptrCursor + 77 ) ||
           ( multiline && strlen( ptrLine ) > 77 ) )
      {
         if ( strlen( ptrLine ) > 77 )
         {
            break;
         }
      }

      if ( !isQueued( ptrCursor, urlQueue ) )
      {
         char aryTempText[1024];

         while ( !pushQueue( ptrCursor, urlQueue ) )
         {
            popQueue( aryTempText, urlQueue );
         }
      }
      multiline = 0;
      return;
   }

   multiline = 1;
}

void openBrowser( void )
{
   int inputIndex;
   int originalCaptureState;
   char aryLine[4];
   char aryCommand[4096];
   char *ptrUrlEntry;

   if ( urlQueue->nobjs < 1 )
   {
      return;
   }
   if ( urlQueue->nobjs == 1 )
   {
      snprintf( aryCommand, sizeof( aryCommand ), "%s \"%s\"%s", aryBrowser,
                urlQueue->start + ( urlQueue->objsize * urlQueue->head ),
                flagsConfiguration.shouldRunBrowserInBackground ? " &" : "" );
      system( aryCommand );
      if ( !flagsConfiguration.shouldRunBrowserInBackground )
      {
         reprintLine();
      }
      return;
   }

   originalCaptureState = capture;
   capture = 0;
   shouldIgnoreNetwork = 1;
   printf( "\r\n\n" );
   ptrUrlEntry = urlQueue->start + ( urlQueue->objsize * urlQueue->head );
   for ( inputIndex = 0; inputIndex < urlQueue->nobjs; inputIndex++ )
   {
      if ( strlen( ptrUrlEntry ) > 72 )
      {
         char aryShortUrl[71];

         strncpy( aryShortUrl, ptrUrlEntry, 70 );
         aryShortUrl[70] = 0;
         printf( "%d. %-70s...\r\n", inputIndex + 1, aryShortUrl );
      }
      else
      {
         printf( "%d. %s\r\n", inputIndex + 1, ptrUrlEntry );
      }
      ptrUrlEntry += urlQueue->objsize;
      if ( ptrUrlEntry >= (char *)( urlQueue->start + ( urlQueue->objsize * urlQueue->size ) ) )
      {
         ptrUrlEntry = urlQueue->start;
      }
   }

   printf( "\r\nChoose the URL you want to view: " );
   getString( 3, aryLine, 1 );
   printf( "\r\n" );
   inputIndex = atoi( aryLine );
   ptrUrlEntry = urlQueue->start + ( urlQueue->objsize * urlQueue->head );
   if ( inputIndex > 0 && inputIndex <= urlQueue->nobjs )
   {
      int urlIndex;

      inputIndex -= 1;
      for ( urlIndex = 0; urlIndex < inputIndex; urlIndex++ )
      {
         ptrUrlEntry += urlQueue->objsize;
         if ( ptrUrlEntry >= (char *)( urlQueue->start + ( urlQueue->objsize * urlQueue->size ) ) )
         {
            ptrUrlEntry = urlQueue->start;
         }
      }
      snprintf( aryCommand, sizeof( aryCommand ), "%s \"%s\"%s", aryBrowser, ptrUrlEntry,
                flagsConfiguration.shouldRunBrowserInBackground ? " &" : "" );
      system( aryCommand );
   }
   shouldIgnoreNetwork = 0;
   reprintLine();
   capture = originalCaptureState;
}
