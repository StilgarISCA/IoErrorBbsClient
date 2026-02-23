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

static const char *findUrlStartConst( const char *ptrText )
{
   const char *ptrHttps;
   const char *ptrWww;
   const char *ptrEarliest;

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

void printWithOsc8Links( const char *ptrText )
{
   const char *ptrCursor;

   if ( ptrText == NULL )
   {
      return;
   }

   ptrCursor = ptrText;
   while ( *ptrCursor != '\0' )
   {
      const char *ptrUrlStart;
      const char *ptrUrlEnd;
      const char *ptrTrimEnd;
      const char *ptrEndPunctuation;
      char aryUrlText[1024];
      char aryUrlTarget[1152];
      size_t urlLength;

      ptrUrlStart = findUrlStartConst( ptrCursor );
      if ( ptrUrlStart == NULL )
      {
         stdPrintf( "%s", ptrCursor );
         return;
      }

      if ( ptrUrlStart > ptrCursor )
      {
         stdPrintf( "%.*s", (int)( ptrUrlStart - ptrCursor ), ptrCursor );
      }

      for ( ptrUrlEnd = ptrUrlStart; *ptrUrlEnd != '\0'; ptrUrlEnd++ )
      {
         if ( isUrlTerminator( (unsigned char)*ptrUrlEnd ) ||
              !isUrlBodyChar( (unsigned char)*ptrUrlEnd ) )
         {
            break;
         }
      }

      ptrTrimEnd = ptrUrlEnd;
      while ( ptrTrimEnd > ptrUrlStart )
      {
         char tailCharacter;

         tailCharacter = ptrTrimEnd[-1];
         if ( tailCharacter == '.' || tailCharacter == ',' || tailCharacter == ';' ||
              tailCharacter == ':' || tailCharacter == '!' || tailCharacter == '?' ||
              tailCharacter == ')' || tailCharacter == ']' || tailCharacter == '}' ||
              tailCharacter == '>' || tailCharacter == '"' || tailCharacter == '\'' )
         {
            ptrTrimEnd--;
         }
         else
         {
            break;
         }
      }

      if ( ptrTrimEnd == ptrUrlStart )
      {
         stdPrintf( "%.*s", (int)( ptrUrlEnd - ptrUrlStart ), ptrUrlStart );
         ptrCursor = ptrUrlEnd;
         continue;
      }

      urlLength = (size_t)( ptrTrimEnd - ptrUrlStart );
      if ( urlLength >= sizeof( aryUrlText ) )
      {
         urlLength = sizeof( aryUrlText ) - 1;
      }
      memcpy( aryUrlText, ptrUrlStart, urlLength );
      aryUrlText[urlLength] = '\0';

      if ( strncmp( aryUrlText, "www.", 4 ) == 0 )
      {
         snprintf( aryUrlTarget, sizeof( aryUrlTarget ), "https://%s", aryUrlText );
      }
      else
      {
         snprintf( aryUrlTarget, sizeof( aryUrlTarget ), "%s", aryUrlText );
      }

      stdPrintf( "\033]8;;%s\033\\", aryUrlTarget );
      stdPrintf( "%s", aryUrlText );
      stdPrintf( "\033]8;;\033\\" );

      ptrEndPunctuation = ptrTrimEnd;
      if ( ptrUrlEnd > ptrEndPunctuation )
      {
         stdPrintf( "%.*s", (int)( ptrUrlEnd - ptrEndPunctuation ), ptrEndPunctuation );
      }

      ptrCursor = ptrUrlEnd;
   }
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

static void queueUrlIfNew( const char *ptrUrl )
{
   char aryTempText[1024];

   if ( ptrUrl == NULL || !*ptrUrl )
   {
      return;
   }
   if ( isQueued( ptrUrl, urlQueue ) )
   {
      return;
   }
   while ( !pushQueue( ptrUrl, urlQueue ) )
   {
      popQueue( aryTempText, urlQueue );
   }
}

static void finalizePendingUrl( char *aryPendingUrl, size_t pendingUrlSize, bool *ptrHasPendingUrl )
{
   (void)pendingUrlSize;
   trimUrlTailPunctuation( aryPendingUrl );
   if ( *aryPendingUrl )
   {
      queueUrlIfNew( aryPendingUrl );
   }
   aryPendingUrl[0] = '\0';
   *ptrHasPendingUrl = false;
}

void filterUrl( const char *ptrLine )
{
   static bool hasPendingUrl = false;
   static char aryPendingUrl[2048];
   static const size_t WRAP_GUESS_MIN_LENGTH = 70;
   char aryLineBuffer[1024];
   char *ptrCursor;
   char *ptrNext;

   if ( !urlQueue )
   {
      return;
   }
   if ( ptrLine == NULL )
   {
      return;
   }

   snprintf( aryLineBuffer, sizeof( aryLineBuffer ), "%s", ptrLine );
   {
      size_t lineLength;

      lineLength = strlen( aryLineBuffer );
      while ( lineLength > 0 &&
              ( aryLineBuffer[lineLength - 1] == ' ' || aryLineBuffer[lineLength - 1] == '\t' ||
                aryLineBuffer[lineLength - 1] == '\r' ) )
      {
         aryLineBuffer[lineLength - 1] = '\0';
         lineLength--;
      }
   }

   ptrCursor = aryLineBuffer;
   if ( hasPendingUrl )
   {
      size_t pendingLength;
      size_t appendedCount;

      while ( *ptrCursor != '\0' && isspace( (unsigned char)*ptrCursor ) )
      {
         ptrCursor++;
      }
      pendingLength = strlen( aryPendingUrl );
      appendedCount = 0;
      while ( *ptrCursor != '\0' && isUrlBodyChar( (unsigned char)*ptrCursor ) )
      {
         if ( pendingLength + 1 < sizeof( aryPendingUrl ) )
         {
            aryPendingUrl[pendingLength++] = *ptrCursor;
            aryPendingUrl[pendingLength] = '\0';
         }
         appendedCount++;
         ptrCursor++;
      }
      if ( appendedCount > 0 )
      {
         if ( *ptrCursor == '\0' )
         {
            size_t continuationVisibleLength;

            continuationVisibleLength = strlen( ptrCursor - appendedCount );
            if ( continuationVisibleLength < WRAP_GUESS_MIN_LENGTH )
            {
               finalizePendingUrl( aryPendingUrl, sizeof( aryPendingUrl ), &hasPendingUrl );
            }
         }
         else
         {
            finalizePendingUrl( aryPendingUrl, sizeof( aryPendingUrl ), &hasPendingUrl );
         }
      }
      else
      {
         finalizePendingUrl( aryPendingUrl, sizeof( aryPendingUrl ), &hasPendingUrl );
      }
   }

   for ( ptrCursor = findUrlStart( aryLineBuffer ); ptrCursor != NULL; ptrCursor = findUrlStart( ptrNext ) )
   {
      for ( ptrNext = ptrCursor; *ptrNext; ptrNext++ )
      {
         if ( !isUrlTerminator( *ptrNext ) && isUrlBodyChar( (unsigned char)*ptrNext ) )
         {
            continue;
         }
         break;
      }

      if ( *ptrNext == '\0' )
      {
         size_t fragmentLength;

         fragmentLength = strlen( ptrCursor );
         if ( fragmentLength >= WRAP_GUESS_MIN_LENGTH )
         {
            snprintf( aryPendingUrl, sizeof( aryPendingUrl ), "%s", ptrCursor );
            hasPendingUrl = true;
         }
         else
         {
            trimUrlTailPunctuation( ptrCursor );
            queueUrlIfNew( ptrCursor );
         }
         return;
      }

      *ptrNext = '\0';
      trimUrlTailPunctuation( ptrCursor );
      queueUrlIfNew( ptrCursor );

      ptrNext++;
   }
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
