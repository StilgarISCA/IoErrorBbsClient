/*
 * Copyright (C) 2024-2026 Stilgar
 * Copyright (C) 1995-2003 Michael Hampton
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "defs.h"
#include "client_globals.h"
#include "filter_globals.h"
#include "proto.h"

static void printThemedWhoListEntry( const char *ptrName, char statusMarker,
                                     const char *ptrTimeText, const char *ptrInfo );

static char *duplicateWhoNameOrDie( const char *ptrName, const char *ptrErrorText )
{
   char *ptrWhoCopy;

   ptrWhoCopy = (char *)calloc( 1, strlen( ptrName ) + 1 );
   if ( !ptrWhoCopy )
   {
      fatalExit( ptrErrorText, "Fatal error" );
      return NULL;
   }
   snprintf( ptrWhoCopy, strlen( ptrName ) + 1, "%s", ptrName );
   return ptrWhoCopy;
}

static void formatElapsedWhoHeader( char *ptrBuffer, size_t bufferSize,
                                    const char *ptrPrefix, long elapsedSeconds )
{
   snprintf( ptrBuffer, bufferSize, "%s (%d:%02d old)%s",
             ptrPrefix, (int)( elapsedSeconds / 60 ),
             (int)( elapsedSeconds % 60 ),
             strstr( ptrPrefix, "Your friends online" ) ? "\r\n\n" : "" );
}

static void formatWhoTimeText( char *ptrBuffer, size_t bufferSize, long elapsedMinutes )
{
   if ( elapsedMinutes >= 1440 )
   {
      snprintf( ptrBuffer, bufferSize, "%2ldd%02ld:%02ld",
                elapsedMinutes / 1440,
                ( elapsedMinutes % 1440 ) / 60,
                ( elapsedMinutes % 1440 ) % 60 );
   }
   else
   {
      snprintf( ptrBuffer, bufferSize, "   %2ld:%02ld",
                elapsedMinutes / 60,
                elapsedMinutes % 60 );
   }
}

static void printSavedWhoList( long elapsedSeconds, int *ptrFriendColumn )
{
   char aryTempText[80];

   for ( ; ( *ptrFriendColumn )++ < savedWhoCount; )
   {
      char aryTimeText[16];

      snprintf( aryTempText, sizeof( aryTempText ), "%s",
                (char *)arySavedWhoNames[*ptrFriendColumn - 1] + 1 );
      snprintf( aryTimeText, sizeof( aryTimeText ), "   %2d:%02d",
                ( *arySavedWhoNames[*ptrFriendColumn - 1] +
                  (int)( elapsedSeconds / 60 ) ) /
                   60,
                ( *arySavedWhoNames[*ptrFriendColumn - 1] +
                  (int)( elapsedSeconds / 60 ) ) %
                   60 );
      printThemedWhoListEntry( aryTempText + 1,
                               *aryTempText & 0x80 ? '*' : ' ',
                               aryTimeText,
                               (char *)arySavedWhoInfo[*ptrFriendColumn - 1] );
   }
   ( *ptrFriendColumn )--;
}

static void printThemedWhoListEntry( const char *ptrName, char statusMarker,
                                     const char *ptrTimeText, const char *ptrInfo )
{
   if ( flagsConfiguration.shouldUseAnsi )
   {
      char aryAnsiSequence[32];

      formatAnsiForegroundSequence( aryAnsiSequence, sizeof( aryAnsiSequence ),
                                    color.postFriendName );
      stdPrintf( "%s", aryAnsiSequence );
      stdPrintf( "%-19s%c ", ptrName, statusMarker );
      formatAnsiForegroundSequence( aryAnsiSequence, sizeof( aryAnsiSequence ),
                                    color.postFriendDate );
      stdPrintf( "%s", aryAnsiSequence );
      stdPrintf( "%s", ptrTimeText );
      formatAnsiForegroundSequence( aryAnsiSequence, sizeof( aryAnsiSequence ),
                                    color.postFriendText );
      stdPrintf( "%s", aryAnsiSequence );
      stdPrintf( "  %s\r\n", ptrInfo );
      formatAnsiForegroundSequence( aryAnsiSequence, sizeof( aryAnsiSequence ),
                                    color.text );
      stdPrintf( "%s", aryAnsiSequence );
      return;
   }

   stdPrintf( "%-19s%c %s  %s\r\n", ptrName, statusMarker, ptrTimeText, ptrInfo );
}

static void printThemedWhoListHeader( const char *ptrText )
{
   if ( flagsConfiguration.shouldUseAnsi )
   {
      char aryAnsiSequence[32];

      formatAnsiForegroundSequence( aryAnsiSequence, sizeof( aryAnsiSequence ),
                                    color.forum );
      stdPrintf( "%s", aryAnsiSequence );
      stdPrintf( "%s", ptrText );
      formatAnsiForegroundSequence( aryAnsiSequence, sizeof( aryAnsiSequence ),
                                    color.text );
      stdPrintf( "%s", aryAnsiSequence );
      return;
   }

   stdPrintf( "%s", ptrText );
}

void filterWhoList( register int inputChar )
{
   static char new;
   static int friendColumn;
   static unsigned char aryWhoEntry[21]; /* Buffer for current name in aryWhoEntry list */
   static unsigned char *ptrWhoEntryWrite = NULL;
   static long timestamp = 0; /* Friend list timestamp */
   static long extime = 0;    /* Extended time decoder */
   char aryTempText[80];

   if ( !ptrWhoEntryWrite )
   { /* First time through */
      ptrWhoEntryWrite = aryWhoEntry;
      *aryWhoEntry = 0;
      new = 0;
      friendColumn = 0;
   }
   if ( inputChar )
   { /* received a character */
      new = 1;
      *ptrWhoEntryWrite++ = (unsigned char)inputChar;
      *ptrWhoEntryWrite = 0;
   }
   else
   { /* received a null */
      /*
    * DOC sends two NULs after S_WHO when we are supposed to display the
    * saved aryWhoEntry list.  We will eat the first character and only exit when
    * the second NUL comes in.  Since it sends a single NUL to signal
    * termination of the aryWhoEntry list, this lets us distinguish.
    * Follow carefully, lest you get lost in here.
    */
      if ( aryWhoEntry == ptrWhoEntryWrite )
      { /* Time to end it all */
         if ( new == 1 )
         {
            if ( !savedWhoCount )
            {
               stdPrintf( "No friends online (new)" );
            }
            ptrWhoEntryWrite = NULL;
            new = 0;
            whoListProgress = 0;
         }
         else if ( new == 0 )
         {
            static long timer = 0;          /* Friend list timestamp */
            static long elapsedSeconds = 0; /* Current time */

            elapsedSeconds = time( NULL ) - timestamp;
            if ( elapsedSeconds - 66 == timer )
            {
               timer = -1;
            }
            else
            {
               timer = elapsedSeconds;
            }
            if ( savedWhoCount )
            { /* we've stored an old whoListProgress */
               if ( timer == -1 )
               {
                  printThemedWhoListHeader( "Die die die fornicate (666)\r\n\n" );
               }
               else
               {
                  formatElapsedWhoHeader( aryTempText, sizeof( aryTempText ),
                                          "Your friends online",
                                          elapsedSeconds );
                  printThemedWhoListHeader( aryTempText );
               }
               printSavedWhoList( elapsedSeconds, &friendColumn );
            }
            else
            {
               if ( timer == -1 )
               {
                  printThemedWhoListHeader( "Die die die (666)" );
               }
               else
               {
                  formatElapsedWhoHeader( aryTempText, sizeof( aryTempText ),
                                          "No friends online",
                                          elapsedSeconds );
                  printThemedWhoListHeader( aryTempText );
               }
            }
            whoListProgress = 0;
            ptrWhoEntryWrite = NULL;
         }
      }
      else
      { /* Received a friend */
         ptrWhoEntryWrite = aryWhoEntry;
         if ( whoListProgress++ == 1 )
         { /* List copy is OK */
            savedWhoCount = 0;
            slistDestroyItems( whoList );
            slistDestroy( whoList );
            if ( !( whoList = slistCreate( 0, sortCompareVoid ) ) )
            {
               fatalExit( "Can't re-create saved aryWhoEntry list!\r\n", "Fatal error" );
            }
            for ( unsigned int ui = 0; ui < friendList->nitems; ui++ )
            {
               const friend *ptrFriend;

               ptrFriend = friendList->items[ui];
               char *ptrWhoCopy = duplicateWhoNameOrDie(
                  ptrFriend->name, "Out of memory for list copy!\r\n" );
               if ( !( slistAddItem( whoList, ptrWhoCopy, 0 ) ) )
               {
                  fatalExit( "Out of memory adding item in list copy!\r\n", "Fatal error" );
               }
            }
         }
         /* Handle extended time information */
         if ( *aryWhoEntry == 0xfe )
         {
            /* Decode BCD */
            {
               unsigned char *ptrEncodedTime;
               for ( ptrEncodedTime = aryWhoEntry + 1; *ptrEncodedTime; ptrEncodedTime++ )
               {
                  extime = 10 * extime + *ptrEncodedTime - 1;
               }
            }
         }
         else
         {
            /* output name and info if aryUser is on our 'friend' list */
            snprintf( aryTempText, sizeof( aryTempText ), "%s", (char *)aryWhoEntry + 1 );
            *aryTempText &= 0x7f;
            {
               char *ptrWhoCopy = duplicateWhoNameOrDie(
                  aryTempText, "Out of memory adding to saved aryWhoEntry list!\r\n" );
               if ( slistFind( whoList, ptrWhoCopy, strCompareVoid ) == -1 )
               {
                  if ( !( slistAddItem( whoList, ptrWhoCopy, 0 ) ) )
                  {
                     fatalExit( "Can't add item to saved aryWhoEntry list!\r\n", "Fatal error" );
                  }
               }
            }
            timestamp = time( NULL );
            if ( ( inputChar = slistFind( friendList, aryTempText, fStrCompareVoid ) ) != -1 )
            {
               if ( !friendColumn++ )
               {
                  stdPrintf( "Your friends online (new)\r\n\n" );
               }
               --*aryWhoEntry;
               if ( friendColumn <= 60 )
               {
                  const friend *ptrFriend;

                  ptrFriend = friendList->items[inputChar];
                  if ( extime == 0 )
                  {
                     extime = (long)( *aryWhoEntry );
                  }
                  {
                     char aryTimeText[16];

                     formatWhoTimeText( aryTimeText, sizeof( aryTimeText ), extime );
                     printThemedWhoListEntry( aryTempText,
                                              aryWhoEntry[1] & 0x80 ? '*' : ' ',
                                              aryTimeText,
                                              ptrFriend->info );
                  }
                  *arySavedWhoNames[savedWhoCount] = *aryWhoEntry;
                  snprintf( (char *)arySavedWhoNames[savedWhoCount] + 1, sizeof( arySavedWhoNames[savedWhoCount] ) - 1, "%s", (char *)aryWhoEntry + 1 );
                  snprintf( (char *)arySavedWhoInfo[savedWhoCount], sizeof( arySavedWhoInfo[savedWhoCount] ), "%s", ptrFriend->info );
                  savedWhoCount++;
               }
            }
            extime = 0;
         }
      }
   }
}
