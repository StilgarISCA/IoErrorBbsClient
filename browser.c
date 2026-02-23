/*
 * Copyright (C) 2024-2026 Stilgar
 * Copyright (C) 1995-2003 Michael Hampton
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "defs.h"
#include "ext.h"
#include "proto.h"

#include "browser.h"

#include <errno.h>
#include <spawn.h>
#include <sys/wait.h>

extern char **environ;

static bool isUrlTerminator( int inputChar )
{
   if ( inputChar == 0 || isspace( inputChar ) )
   {
      return true;
   }
   return false;
}

static bool isUrlBodyChar( int inputChar )
{
   static const char *ptrAllowedPunctuation = "-._~:/?#[]@!$&'()*+,;=%";

   if ( isalnum( inputChar ) )
   {
      return true;
   }
   return findChar( ptrAllowedPunctuation, inputChar ) != NULL;
}

static char *findUrlStart( char *ptrText )
{
   char *ptrHttps;
   char *ptrWww;
   char *ptrEarliest;

   ptrHttps = strstr( ptrText, "https://" );
   ptrWww = strstr( ptrText, "www." );

   ptrEarliest = ptrHttps;
   if ( ptrWww != NULL && ( ptrEarliest == NULL || ptrWww < ptrEarliest ) )
   {
      ptrEarliest = ptrWww;
   }

   if ( ptrWww != NULL && ptrEarliest == ptrWww )
   {
      if ( ptrWww != ptrText && !isspace( (unsigned char)ptrWww[-1] ) && ptrWww[-1] != '(' &&
           ptrWww[-1] != '<' && ptrWww[-1] != '"' && ptrWww[-1] != '\'' )
      {
         ptrEarliest = NULL;
      }
   }

   return ptrEarliest;
}

static void trimUrlTailPunctuation( char *ptrUrlStart )
{
   size_t urlLength;

   urlLength = strlen( ptrUrlStart );
   while ( urlLength > 0 )
   {
      char tailCharacter;

      tailCharacter = ptrUrlStart[urlLength - 1];
      if ( tailCharacter == '.' || tailCharacter == ',' || tailCharacter == ';' ||
           tailCharacter == ':' || tailCharacter == '!' || tailCharacter == '?' ||
           tailCharacter == ')' || tailCharacter == ']' || tailCharacter == '}' ||
           tailCharacter == '>' || tailCharacter == '"' || tailCharacter == '\'' )
      {
         ptrUrlStart[urlLength - 1] = '\0';
         urlLength--;
      }
      else
      {
         break;
      }
   }
}

static bool parseBrowserCommand( const char *ptrBrowserCommand,
                                 char *aryCommandBuffer,
                                 size_t commandBufferSize,
                                 char **aryArguments,
                                 size_t maxArguments,
                                 size_t *ptrArgumentCount )
{
   const char *ptrRead;
   char *ptrWrite;

   if ( ptrBrowserCommand == NULL || aryCommandBuffer == NULL || commandBufferSize == 0 ||
        aryArguments == NULL || maxArguments < 2 || ptrArgumentCount == NULL )
   {
      return false;
   }

   snprintf( aryCommandBuffer, commandBufferSize, "%s", ptrBrowserCommand );
   ptrRead = aryCommandBuffer;
   ptrWrite = aryCommandBuffer;
   *ptrArgumentCount = 0;

   while ( *ptrRead != '\0' )
   {
      char quoteCharacter;

      while ( *ptrRead != '\0' && isspace( (unsigned char)*ptrRead ) )
      {
         ptrRead++;
      }
      if ( *ptrRead == '\0' )
      {
         break;
      }
      if ( *ptrArgumentCount >= maxArguments - 1 )
      {
         return false;
      }

      aryArguments[*ptrArgumentCount] = ptrWrite;
      ( *ptrArgumentCount )++;
      quoteCharacter = 0;

      if ( *ptrRead == '"' || *ptrRead == '\'' )
      {
         quoteCharacter = *ptrRead;
         ptrRead++;
      }

      while ( *ptrRead != '\0' )
      {
         if ( quoteCharacter != 0 )
         {
            if ( *ptrRead == quoteCharacter )
            {
               ptrRead++;
               break;
            }
         }
         else if ( isspace( (unsigned char)*ptrRead ) )
         {
            break;
         }

         *ptrWrite++ = *ptrRead++;
      }

      *ptrWrite++ = '\0';
      while ( *ptrRead != '\0' && isspace( (unsigned char)*ptrRead ) )
      {
         ptrRead++;
      }
   }

   if ( *ptrArgumentCount == 0 )
   {
      return false;
   }

   return true;
}

static bool launchBrowserUrl( const char *ptrBrowserCommand, const char *ptrUrl, bool shouldRunBackground )
{
   char aryCommandBuffer[4096];
   char *aryArguments[64];
   size_t argumentCount;
   pid_t browserProcessId;
   int spawnResult;

   if ( ptrUrl == NULL || !*ptrUrl )
   {
      return false;
   }
   if ( !parseBrowserCommand( ptrBrowserCommand,
                              aryCommandBuffer,
                              sizeof( aryCommandBuffer ),
                              aryArguments,
                              sizeof( aryArguments ) / sizeof( aryArguments[0] ),
                              &argumentCount ) )
   {
      return false;
   }
   if ( argumentCount >= ( sizeof( aryArguments ) / sizeof( aryArguments[0] ) ) - 1 )
   {
      return false;
   }

   aryArguments[argumentCount++] = (char *)ptrUrl;
   aryArguments[argumentCount] = NULL;

   spawnResult = posix_spawnp( &browserProcessId, aryArguments[0], NULL, NULL, aryArguments, environ );
   if ( spawnResult != 0 )
   {
      errno = spawnResult;
      sPerror( "Unable to launch web browser command", "Warning" );
      return false;
   }

   if ( !shouldRunBackground )
   {
      if ( waitpid( browserProcessId, NULL, 0 ) < 0 )
      {
         sPerror( "Browser process wait failed", "Warning" );
      }
   }

   return true;
}

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

   ptrCursor = findUrlStart( aryUrlBuffer );
   if ( ptrCursor == NULL )
   {
      aryUrlBuffer[0] = 0;
      multiline = 0;
      return;
   }

   for ( ptrNext = ptrCursor; *ptrNext; ptrNext++ )
   {
      if ( !isUrlTerminator( *ptrNext ) && isUrlBodyChar( *ptrNext ) )
      {
         continue;
      }
      *ptrNext = 0;
      trimUrlTailPunctuation( ptrCursor );
      if ( *ptrCursor == '\0' )
      {
         multiline = 0;
         return;
      }

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
   const char *ptrBrowserCommand;
   bool shouldBackgroundOverride;
   char *ptrUrlEntry;

   ptrBrowserCommand = *aryBrowser ? aryBrowser : aryDefaultBrowser;
   if ( ptrBrowserCommand == NULL || !*ptrBrowserCommand )
   {
      ptrBrowserCommand = "open";
   }
   shouldBackgroundOverride = ( *aryBrowser && flagsConfiguration.shouldRunBrowserInBackground );

   if ( urlQueue->nobjs < 1 )
   {
      return;
   }
   if ( urlQueue->nobjs == 1 )
   {
      launchBrowserUrl( ptrBrowserCommand,
                        urlQueue->start + ( urlQueue->objsize * urlQueue->head ),
                        shouldBackgroundOverride );
      if ( !shouldBackgroundOverride )
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
      launchBrowserUrl( ptrBrowserCommand, ptrUrlEntry, shouldBackgroundOverride );
   }
   shouldIgnoreNetwork = 0;
   reprintLine();
   capture = originalCaptureState;
}
