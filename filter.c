/*
 * Copyright (C) 2024-2026 Stilgar
 * Copyright (C) 1995-2003 Michael Hampton
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "defs.h"
#include "ext.h"
#include "telnet.h"

static char thisline[320]; /* Copy of the current aryLine */

void filterWhoList( register int inputChar )
{
   static char new;
   static int friendColumn;
   static unsigned char aryWhoEntry[21]; /* Buffer for current name in aryWhoEntry list */
   static unsigned char *ptrWhoEntryWrite = NULL;
   static long timestamp = 0;      /* Friend list timestamp */
   static long timer = 0;          /* Friend list timestamp */
   static long elapsedSeconds = 0; /* Current time */
   static long extime = 0;         /* Extended time decoder */
   char aryTempText[80], aryDisplayLine[100];
   char *ptrWhoCopy;
   const friend *ptrFriend;

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
               stdPrintf( timer == -1 ? "Die die die fornicate (666)\r\n\n" : "Your friends online (%d:%02d old)\r\n\n",
                          (int)( elapsedSeconds / 60 ),
                          (int)( elapsedSeconds % 60 ) );
               for ( ; friendColumn++ < savedWhoCount; )
               {
                  snprintf( aryTempText, sizeof( aryTempText ), "%s", (char *)arySavedWhoNames[friendColumn - 1] + 1 );
                  snprintf( aryDisplayLine, sizeof( aryDisplayLine ), flagsConfiguration.useAnsi ? "@Y%c%-18s%c @R   %2d:%02d@G  @C%s\r\n" : "%c%-18s%c    %2d:%02d  %s\r\n",
                            *aryTempText & 0x7f, aryTempText + 1, *aryTempText & 0x80 ? '*' : ' ',
                            ( *arySavedWhoNames[friendColumn - 1] + (int)( elapsedSeconds / 60 ) ) / 60,
                            ( *arySavedWhoNames[friendColumn - 1] + (int)( elapsedSeconds / 60 ) ) % 60,
                            arySavedWhoInfo[friendColumn - 1] );
                  colorize( aryDisplayLine );
               }
               friendColumn--;
            }
            else
            {
               stdPrintf( timer == -1 ? "Die die die (666)" : "No friends online (%d:%02d old)",
                          (int)( elapsedSeconds / 60 ), (int)( elapsedSeconds % 60 ) );
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
               ptrFriend = friendList->items[ui];
               if ( !( ptrWhoCopy = (char *)calloc( 1, strlen( ptrFriend->name ) + 1 ) ) )
               {
                  fatalExit( "Out of memory for list copy!\r\n", "Fatal error" );
               }
               snprintf( ptrWhoCopy, strlen( ptrFriend->name ) + 1, "%s", ptrFriend->name );
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
            if ( !( ptrWhoCopy = (char *)calloc( 1, strlen( aryTempText ) + 1 ) ) )
            {
               fatalExit( "Out of memory adding to saved aryWhoEntry list!\r\n", "Fatal error" );
            }
            snprintf( ptrWhoCopy, strlen( aryTempText ) + 1, "%s", aryTempText );
            if ( slistFind( whoList, ptrWhoCopy, strCompareVoid ) == -1 )
            {
               if ( !( slistAddItem( whoList, ptrWhoCopy, 0 ) ) )
               {
                  fatalExit( "Can't add item to saved aryWhoEntry list!\r\n", "Fatal error" );
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
                  ptrFriend = friendList->items[inputChar];
                  if ( extime == 0 )
                  {
                     extime = (long)( *aryWhoEntry );
                  }
                  if ( extime >= 1440 )
                  {
                     snprintf( aryDisplayLine, sizeof( aryDisplayLine ), flagsConfiguration.useAnsi ? "@Y%-19s%c @R%2ldd%02ld:%02ld@G  @C%s\r\n" : "%-19s%c %2ldd%02ld:%02ld  %s\r\n",
                               aryTempText, aryWhoEntry[1] & 0x80 ? '*' : ' ',
                               extime / 1440,
                               ( extime % 1440 ) / 60, ( extime % 1440 ) % 60,
                               ptrFriend->info );
                  }
                  else
                  {
                     snprintf( aryDisplayLine, sizeof( aryDisplayLine ), flagsConfiguration.useAnsi ? "@Y%-19s%c @R   %2ld:%02ld@G  @C%s\r\n" : "%-19s%c    %2ld:%02ld  %s\r\n",
                               aryTempText, aryWhoEntry[1] & 0x80 ? '*' : ' ',
                               extime / 60, extime % 60, ptrFriend->info );
                  }
                  colorize( aryDisplayLine );
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

void filterExpress( register int inputChar )
{
   register int itemIndex;     /* generic counter */
   static char *ptrSenderName; /* comparison pointer */
   static struct
   {
      unsigned int crlf : 1;      /* Needs initial CR/LF */
      unsigned int prochdr : 1;   /* Needs killfile processing */
      unsigned int ignore : 1;    /* Ignore the remainder of the X */
      unsigned int truncated : 1; /* X message exceeded buffer */
   } needs;

   if ( inputChar == -1 )
   { /* signal from IAC to begin/end X */
      if ( isExpressMessageInProgress )
      { /* Need to re-init for new X */
         ptrExpressMessageBuffer = aryExpressMessageBuffer;
         *aryExpressMessageBuffer = 0;
         needs.ignore = 0;
         needs.crlf = 0;
         needs.prochdr = 1;
         needs.truncated = 0;
         return;
      }
      else if ( !needs.ignore )
      { /* Finished this X, dump it out */
         /* Process for automatic reply */
         itemIndex = extractNumber( aryExpressMessageBuffer );
         /* Don't queue if it's an outgoing X */
         /* Only send 'x' if it's a new incoming X */
         if ( ( isAway || isXland ) && !isQueued( ptrSenderName, xlandQueue ) &&
              itemIndex > highestExpressMessageId && !isAutomaticReply( aryExpressMessageBuffer ) && ptrSenderName )
         {
            pushQueue( ptrSenderName, xlandQueue );
            shouldSendExpressMessage = 1;
         }
         else if ( isAutomaticReply( aryExpressMessageBuffer ) && itemIndex > highestExpressMessageId )
         {
            notReplyingTransformExpress( aryExpressMessageBuffer, sizeof( aryExpressMessageBuffer ) );
         }
         /* Update highestExpressMessageId with greatest number seen */
         if ( itemIndex > highestExpressMessageId )
         {
            highestExpressMessageId = itemIndex;
         }
         replyCodeTransformExpress( aryExpressMessageBuffer, sizeof( aryExpressMessageBuffer ) );
         ansiTransformExpress( aryExpressMessageBuffer, sizeof( aryExpressMessageBuffer ) );
         stdPrintf( "%s%s", ( needs.crlf ) ? "\r\n" : "", aryExpressMessageBuffer );
         if ( needs.truncated )
         {
            stdPrintf( "\r\n[X message truncated]\r\n" );
         }
         return;
      }
   }
   /* If the message is killed, don't bother doing anything. */
   if ( needs.ignore )
   {
      return;
   }
   if ( needs.truncated )
   {
      return;
   }

   if ( aryExpressMessageBuffer == ptrExpressMessageBuffer )
   { /* Check for initial CR/LF pair */
      if ( inputChar == '\r' || inputChar == '\n' )
      {
         needs.crlf = 1;
         return;
      }
   }
   /* Insert character into the buffer (drop excess to avoid overflow) */
   if ( ptrExpressMessageBuffer >= aryExpressMessageBuffer + sizeof( aryExpressMessageBuffer ) - 1 )
   {
      needs.truncated = 1;
      return;
   }
   *ptrExpressMessageBuffer++ = (char)inputChar;
   *ptrExpressMessageBuffer = 0;

   /* Extract URLs if any */
   if ( !needs.prochdr && inputChar == '\r' )
   {
      filterUrl( ptrExpressMessageBuffer );
   }

   /* If reached a \r it's time to do header processing */
   if ( needs.prochdr && inputChar == '\r' )
   {
      needs.prochdr = 0;
      /* Process for kill file */
      if ( !findSubstring( aryExpressMessageBuffer, "*** Message " ) &&
           !findSubstring( aryExpressMessageBuffer, "%%% Question " ) )
      {
         /* not an X header we care about */
      }
      ptrSenderName = extractNameNoHistory( aryExpressMessageBuffer );
      if ( ptrSenderName &&
           slistFind( enemyList, ptrSenderName, strCompareVoid ) != -1 )
      {
         if ( !flagsConfiguration.shouldSquelchExpress )
         {
            stdPrintf( "\r\n[X message by %s killed]\r\n", ptrSenderName );
         }
         needs.ignore = 1;
         return;
      }
      if ( ptrSenderName )
      {
         ptrSenderName = extractName( aryExpressMessageBuffer );
      }
   }
   return;
}

void filterPost( register int inputChar )
{
   static int ansistate = 0; /* ANSI state count */
   static int numposts = 0;  /* count of the # of posts received so far */
   static char posthdr[140]; /* store the post header here */
   static char *posthdrp;    /* pointer into posthdr */
   static struct
   {
      unsigned int crlf : 1;    /* need to add a CR/LF */
      unsigned int prochdr : 1; /* process post header */
      unsigned int ignore : 1;  /* kill this post */
      unsigned int secondN : 1; /* send a second n */
   } needs;
   static char *ptrSenderName; /* misc. pointer */
   static char aryTempText[160];
   static int isFriend; /* Current post is by a friend */

   if ( inputChar == -1 )
   { /* control: begin/end of post */
      if ( postProgressState )
      { /* beginning of post */
         posthdrp = posthdr;
         *posthdr = 0;
         needs.crlf = 0;
         needs.prochdr = 1;
         isFriend = 0;
         needs.ignore = 0;
         needs.secondN = 0;
      }
      else
      { /* end of post */
         if ( needs.secondN )
         {
            netPutChar( 'n' );
            byte++;
         }
         filterUrl( " " );
         numposts++;
      }
      return;
   }
   /* If post is killed, do no further processing. */
   if ( needs.ignore )
   { /* ignore our stupid enemies :) */
      return;
   }

   if ( posthdr == posthdrp )
   { /* Check for initial CR/LF pair */
      if ( inputChar == '\r' || inputChar == '\n' )
      {
         needs.crlf = 1;
         return;
      }
   }
   /* At this point we should either insert the character into the post
     * buffer, or echo it to the screen.
     */
   if ( !needs.prochdr )
   {
      /* Store this aryLine for processing */
      if ( inputChar == '\n' )
      {
         filterUrl( thisline );
         thisline[0] = 0;
      }
      else
      {
         /* This is a whole lot faster than calling strcat() */
         char *ptrCursor = thisline;

         for ( ; *ptrCursor; ptrCursor++ )
         { /* Find end of string */
            ;
         }

         *ptrCursor++ = (char)inputChar; /* Copy character to end of string */
         *ptrCursor = 0;
      }

      /* Process ANSI codes in the middle of a post */
      if ( ansistate )
      {
         ansistate--;
         if ( ansistate == 1 )
         {
            if ( flagsConfiguration.shouldDisableBold && inputChar == 109 )
            { /* Turn boldface off */
               printf( "m\033[0" );
               ansistate--;
            }
            else
            {
               lastColor = ansiTransformPost( (char)inputChar, isFriend );
               inputChar = lastColor;
            }
         }
      }
      else if ( inputChar == '\033' )
      { /* Escape character */
         ansistate = 4;
         if ( !flagsConfiguration.useAnsi )
         {
            flagsConfiguration.useAnsi = 1;
            if ( !flagsConfiguration.useBold )
            {
               flagsConfiguration.shouldDisableBold = 1;
            }
         }
      }
      /* Change color for end of more prompt */
      if ( flagsConfiguration.useAnsi && flagsConfiguration.isMorePromptActive && inputChar == ' ' )
      {
         stdPrintf( "\033[3%cm", lastColor = color.text ); /* assignment */
      }

      /* Output character */
      stdPutChar( inputChar );
   }
   else
   {
      *posthdrp++ = (char)inputChar; /* store character in buffer */
      *posthdrp = 0;

      /* If reached a \r it's time to do header processing */
      if ( inputChar == '\r' )
      {
         char *ptrSenderNameForChecks;

         needs.prochdr = 0;

         /* Process for enemy list kill file */
         snprintf( aryTempText, sizeof( aryTempText ), "%s", posthdr );
         ptrSenderNameForChecks = extractNameNoHistory( aryTempText );
         if ( ptrSenderNameForChecks &&
              slistFind( enemyList, ptrSenderNameForChecks, strCompareVoid ) != -1 )
         {
            needs.ignore = 1;
            postHeaderActive = -1;
            postProgressState = -1;
            netPrintf( "%c%c%c%c", IAC, POST_K, numposts & 0xFF, 17 );
            netflush();
            if ( !flagsConfiguration.shouldSquelchPost )
            {
               stdPrintf( "%s[Post by %s killed]\r\n",
                          *posthdr == '\n' ? "\r\n" : "", ptrSenderNameForChecks );
            }
            else
            {
               pendingLinesToEat = ( needs.crlf ? 1 : 2 );
               netPutChar( 'n' );
               netflush();
               byte++;
            }
            return;
         }

         ptrSenderName = extractName( aryTempText );
         isFriend = ( ptrSenderName &&
                      slistFind( friendList, ptrSenderName, fStrCompareVoid ) != -1 )
                       ? 1
                       : 0;
         ansiTransformPostHeader( posthdr, isFriend );
         snprintf( arySavedHeader, sizeof( arySavedHeader ), "%s\r\n", posthdr );
         stdPrintf( "%s%s\r", ( needs.crlf ) ? "\r\n" : "", posthdr );
      }
   }
}

void filterUrl( const char *aryLine )
{
   static int multiline = 0;
   static char aryUrlBuffer[1024];
   char *ptrCursor, *ptrNext;

   if ( !urlQueue )
   { /* Can't store URLs for some reason */
      return;
   }

   if ( !multiline )
   {
      snprintf( aryUrlBuffer, sizeof( aryUrlBuffer ), "%s", aryLine );
   }
   else
   {
      size_t ulen = strlen( aryUrlBuffer );
      if ( ulen < sizeof( aryUrlBuffer ) - 1 )
      {
         snprintf( aryUrlBuffer + ulen, sizeof( aryUrlBuffer ) - ulen, "%s", aryLine );
      }
   }

   {
      size_t urlLength = strlen( aryUrlBuffer );
      while ( urlLength > 0 )
      {
         size_t index = urlLength - 1;
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
      size_t ulen = strlen( aryUrlBuffer );
      if ( ulen < sizeof( aryUrlBuffer ) - 1 )
      {
         snprintf( aryUrlBuffer + ulen, sizeof( aryUrlBuffer ) - ulen, " " );
      }
   }

   if ( !( ptrCursor = strstr( aryUrlBuffer, "http://" ) ) )
   {
      if ( !( ptrCursor = strstr( aryUrlBuffer, "ftp://" ) ) )
      {
         aryUrlBuffer[0] = 0;
         multiline = 0;
         return;
      }
   }
   /* offset unused */

   for ( ptrNext = ptrCursor; *ptrNext; ptrNext++ )
   {
      if ( findChar( ":/abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789$-_@.&+,=;?%~{|}", *ptrNext ) )
      {
         continue;
      }
      *ptrNext = 0;
      /* length unused */

      /* Oops, looks like a multi-aryLine URL */
      if ( ( !multiline && ptrCursor == aryLine &&
             ptrNext > ptrCursor + 77 ) ||
           ( multiline && strlen( aryLine ) > 77 ) )
      {
         if ( strlen( aryLine ) > 77 )
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
      /*	    printf("\r\nSnarfed URL: <%s>\r\n", urls[nurls - 1]); */
      multiline = 0;
      return;
   }

   multiline = 1;
   /*	printf("Multiline URL, got <%s> so far.\r\n", aryUrlBuffer); */
   return;
}

void filterData( register int inputChar )
{
   static int ansistate = 0; /* Counter for ANSI transformations */

   /* Copy the current aryLine (or what we have so far) */
   if ( inputChar == '\n' )
   {
      filterUrl( thisline );
      thisline[0] = 0;
   }
   else
   {
      /* This is faster than calling strcat() */
      char *ptrCursor = thisline;

      while ( *ptrCursor )
      { /* Find end of string */
         ptrCursor++;
      }

      *ptrCursor++ = (char)inputChar; /* Copy character to end of string */
      *ptrCursor = 0;
   }

   /* Auto-answer ANSI question */
   if ( flagsConfiguration.shouldAutoAnswerAnsiPrompt && strstr( thisline, "Are you on an ANSI" ) )
   {
      netPutChar( 'y' );
      byte++;
      *thisline = 0; /* Kill it; we don't need it */
   }

   /* Automatic X reply */
   /*
    if (sendingXState == SX_SENT_NAME) {
   sendingXState = SX_SEND_NEXT;
#if DEBUG
   	stdPrintf("filterData 1 sendingXState is %d, xland is %d\r\n", sendingXState, isXland);
#endif
    }
    */
   /*  if (sendingXState == SX_SEND_NEXT && xlandQueue->nobjs && (isAway || isXland))
   sendAnX();
   */
   if ( sendingXState == SX_SEND_NEXT && !*thisline && inputChar == '\r' )
   {
      sendingXState = SX_NOT;
#if DEBUG
      stdPrintf( "filterData 2 sendingXState is %d, xland is %d\r\n", sendingXState, isXland );
#endif
      if ( xlandQueue->nobjs && ( isAway || isXland ) )
      {
         sendAnX();
      }
   }

   /* Change color for end of more prompt */
   if ( flagsConfiguration.useAnsi && flagsConfiguration.isMorePromptActive && inputChar == ' ' )
   {
      stdPrintf( "\033[3%cm", lastColor = color.text ); /* assignment */
   }

   /* Parse ANSI sequences */
   if ( ansistate )
   {
      ansistate--;
      if ( ansistate == 1 )
      {
         if ( flagsConfiguration.shouldDisableBold && inputChar == 109 )
         { /* Turn boldface off */
            stdPrintf( "m\033[0" );
            ansistate--;
         }
         else
         {
            lastColor = ansiTransform( (char)inputChar );
            inputChar = lastColor;
         }
      }
   }
   else if ( inputChar == '\033' )
   { /* Escape character */
      stdPrintf( "\033[4%cm", color.background );
      ansistate = 4;
      if ( !flagsConfiguration.useAnsi )
      {
         flagsConfiguration.useAnsi = 1;
         if ( !flagsConfiguration.useBold )
         {
            flagsConfiguration.shouldDisableBold = 1;
         }
      }
   }
   if ( pendingLinesToEat > 0 )
   {
      if ( inputChar == '\n' )
      {
         pendingLinesToEat--;
      }
   }
   else
   {
      pendingLinesToEat = 0; /* pendingLinesToEat should never be less than 0 */
      stdPutChar( inputChar );
   }
   return;
}

void reprintLine( void )
{
   char aryLine[320];
   char *ptrCursor;

   strncpy( aryLine, thisline, 320 );
   stdPutChar( '\r' );
   for ( ptrCursor = aryLine; *ptrCursor; ptrCursor++ )
   {
      filterData( *ptrCursor );
   }
   strncpy( thisline, aryLine, 320 );
}

void morePromptHelper( void )
{
   if ( !flagsConfiguration.useAnsi )
   {
      return;
   }

   if ( postProgressState )
   {
      continuedPostHelper();
   }
   else
   {
      continuedDataHelper();
   }
}

void continuedPostHelper( void )
{
   static char aryTempText[] = "\033[32m";
   char *ptrText;

   for ( ptrText = aryTempText; *ptrText; ptrText++ )
   {
      filterPost( *ptrText );
   }
}

void continuedDataHelper( void )
{
   static char aryTempText[] = "\033[32m";
   char *ptrText;

   for ( ptrText = aryTempText; *ptrText; ptrText++ )
   {
      filterData( *ptrText );
   }
}

/* Check for an automatic reply message. Return 1 if this is such a message. */
int isAutomaticReply( const char *message )
{
   const char *ptrCursor;

   /* Find first aryLine */
   ptrCursor = findSubstring( message, ">" );

   /* Wasn't a first aryLine? - move past '>' */
   if ( !ptrCursor )
   {
      return 0;
   }
   ptrCursor++;

   /* Check for valid automatic reply messages */
   if ( !strncmp( ptrCursor, "+!R", 3 ) ||
        !strncmp( ptrCursor, "This message was automatically generated", 40 ) ||
        !strncmp( ptrCursor, "*** ISCA Windows Client", 23 ) )
   {
      return 1;
   }

   /* Looks normal to me... */
   return 0;
}

void notReplyingTransformExpress( char *ptrText, size_t size )
{
   char aryTempText[580];
   char *ptrMessageStart;

   /* Verify this is an X message and set up pointers */
   ptrMessageStart = findSubstring( ptrText, " at " );
   if ( !ptrMessageStart )
   {
      return;
   }

   *( ptrMessageStart++ ) = 0;

   snprintf( aryTempText, sizeof( aryTempText ), "%s (not replying) %s", ptrText, ptrMessageStart );
   snprintf( ptrText, size, "%s", aryTempText );
}

void replyCodeTransformExpress( char *ptrText, size_t size )
{
   char aryTempText[580];
   char *ptrMessageStart;

   /* Verify this is an auto reply and set up pointers */
   ptrMessageStart = findSubstring( ptrText, ">" );
   if ( !ptrMessageStart || strncmp( ptrMessageStart, ">+!R ", 5 ) )
   {
      return;
   }

   *( ++ptrMessageStart ) = 0;
   ptrMessageStart += 4;

   snprintf( aryTempText, sizeof( aryTempText ), "%s%s", ptrText, ptrMessageStart );
   snprintf( ptrText, size, "%s", aryTempText );
}
