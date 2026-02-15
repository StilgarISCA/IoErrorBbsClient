/*
 * This file handles parsing of the bbsrc file, and setting of all the options
 * it allows.  It is just generic C code, easily extensible to allow the
 * addition of features in the future.
 */
#include "defs.h"
#include "ext.h"

/*
 * Given a pointer to a string, this function evaluates it to a control
 * character, translating '^z', '^Z', or an actual ctrl-Z to all be ctrl-Z, If
 * the character is not a control character, it is simply returned as is.
 */
static int ctrl( const char *ptrToken )
{
   int parseIndex = *ptrToken;

   if ( parseIndex == '^' )
   {
      if ( ( ( parseIndex = *++ptrToken ) >= '@' && parseIndex <= '_' ) || parseIndex == '?' )
      {
         parseIndex ^= 0x40;
      }
      else if ( parseIndex >= 'a' && parseIndex <= 'z' )
      {
         parseIndex ^= 0x60;
      }
   }
   if ( parseIndex == '\r' )
   {
      parseIndex = '\n';
   }
   return ( parseIndex );
}

/*
 * Parses the bbsrc file, setting necessary globals depending on the content of
 * the bbsrc, or returning an error if the bbsrc couldn't be properly parsed.
 */
#define MAX_LINE_LENGTH 83
void readBbsRc( void )
{
   char aryLine[MAX_LINE_LENGTH + 1];
   char aryScratchLine[MAX_LINE_LENGTH + 1];
   int parseIndex;
   const char *ptrToken;
   char *ptrMacroWrite, *ptrNameCopy;
   int lineNumber = 0;
   int reads = 0;
   int nameLength;
   size_t hold = 0;
   int tmpVersion = 0;
   friend *ptrFriend = NULL;

   version = INT_VERSION;
   commandKey = -1;
   shellKey = -1;
   captureKey = -1;
   suspKey = -1;
   quitKey = -1;
   awayKey = -1;
   browserKey = -1;
   if ( !( friendList = slistCreate( 0, fSortCompareVoid ) ) )
   {
      fatalExit( "Can't create 'friend' list!\n", "Fatal error" );
   }
   if ( !( enemyList = slistCreate( 0, sortCompareVoid ) ) )
   {
      fatalExit( "Can't create 'enemy' list!\n", "Fatal error" );
   }
   if ( !( whoList = slistCreate( 0, sortCompareVoid ) ) )
   {
      fatalExit( "Can't create saved who list!\n", "Fatal error" );
   }

   for ( parseIndex = 0; parseIndex <= 127; parseIndex++ )
   {
      aryKeyMap[parseIndex] = (char)parseIndex;
      *aryMacro[parseIndex] = 0;
   }
   isXland = 1;
   xlandQueue = newQueue( 21, MAX_USER_NAME_HISTORY_COUNT );
   if ( !xlandQueue )
   {
      isXland = 0;
   }
   urlQueue = newQueue( 1024, 10 );

   isAutoLoggedIn = 0;
   *aryAutoName = 0;
#ifdef ENABLE_SAVE_PASSWORD
   isAutoPasswordSent = 0;
   *aryAutoPassword = 0;
#endif

   *aryEditor = 0;
   *aryBbsHost = 0;
   ptrBbsRc = findBbsRc();
   bbsFriends = findBbsFriends();
   flagsConfiguration.useAnsi = 0;
   flagsConfiguration.useBold = 0;
   flagsConfiguration.shouldDisableBold = 0;
   flagsConfiguration.isMorePromptActive = 0;
   flagsConfiguration.shouldAutoAnswerAnsiPrompt = 0;
   flagsConfiguration.shouldRunBrowserInBackground = 0;

   defaultColors( 1 );

   whoListProgress = 0;
   ptrPostBuffer = 0;
   postHeaderActive = 0;
   highestExpressMessageId = 0;
   isExpressMessageHeaderActive = 0;
   postProgressState = 0;
   isPostJustEnded = 0;
   isExpressMessageInProgress = 0;
   shouldSendExpressMessage = 0;
   pendingLinesToEat = 0;
   shouldUseSsl = 0;
   ptrExpressMessageBuffer = aryExpressMessageBuffer;

   while ( ptrBbsRc && fgets( aryLine, MAX_LINE_LENGTH + 1, ptrBbsRc ) )
   {
      reads++;
      lineNumber++;
      if ( (int)strlen( aryLine ) >= MAX_LINE_LENGTH )
      {
         stdPrintf( "Line %d in .bbsrc too long, ignored.\n", lineNumber );
         while ( (int)strlen( aryLine ) >= MAX_LINE_LENGTH && aryLine[MAX_LINE_LENGTH - 1] != '\n' )
         {
            fgets( aryLine, MAX_LINE_LENGTH + 1, ptrBbsRc );
         }
         continue;
      }
      {
         size_t lineLength = strlen( aryLine );
         while ( lineLength > 0 )
         {
            size_t index = lineLength - 1;
            if ( aryLine[index] == ' ' || aryLine[index] == '\t' ||
                 aryLine[index] == '\n' || aryLine[index] == '\r' )
            {
               aryLine[index] = 0;
            }
            else
            {
               break;
            }
            lineLength = index;
         }
      }

      /* Just ignore these for now, they'll be quietly erased... */
      if ( !strncmp( aryLine, "reread ", 7 ) )
      {
         ;
      }
      else if ( !strncmp( aryLine, "xwrap ", 6 ) )
      {
         ;

         /* Client configuration options (current) */
      }
      else if ( !strncmp( aryLine, "bold", 4 ) )
      {
         flagsConfiguration.useBold = 1;
      }
      else if ( !strncmp( aryLine, "xland", 5 ) )
      {
         isXland = 0;
      }
      else if ( !strncmp( aryLine, "version ", 8 ) )
      {
         tmpVersion = atoi( aryLine + 8 );
      }
      else if ( !strncmp( aryLine, "squelch ", 8 ) )
      {
         switch ( atoi( aryLine + 8 ) )
         {
            case 3:
               flagsConfiguration.shouldSquelchPost = 1;
               flagsConfiguration.shouldSquelchExpress = 1;
               break;
            case 2:
               flagsConfiguration.shouldSquelchPost = 1;
               break;
            case 1:
               flagsConfiguration.shouldSquelchExpress = 1;
               break;
            default:
               break;
         }
      }
      else if ( !strncmp( aryLine, "color ", 6 ) )
      {
         if ( strlen( aryLine ) != 6 + sizeof color )
         {
            stdPrintf( "Invalid 'color' scheme on line %d, ignored.\n", lineNumber );
         }
         else
         {
            bcopy( aryLine + 6, (void *)&color, sizeof color );
         }
      }
      else if ( !strncmp( aryLine, "aryAutoName ", sizeof( "aryAutoName " ) - 1 ) )
      {
         if ( strncmp( aryLine + ( sizeof( "aryAutoName " ) - 1 ), "Guest", 5 ) )
         {
            strncpy( aryAutoName, aryLine + ( sizeof( "aryAutoName " ) - 1 ), 21 );
            aryAutoName[20] = 0;
         }
      }
      else if ( !strncmp( aryLine, "autoansi", 9 ) )
      {
         if ( strlen( aryLine ) <= 9 || aryLine[9] != 'N' )
         {
            flagsConfiguration.shouldAutoAnswerAnsiPrompt = 1;
         }
#ifdef ENABLE_SAVE_PASSWORD
      }
      else if ( !strncmp( aryLine, "autopass ", 9 ) )
      {
         strncpy( aryAutoPassword, aryLine + 9, 21 );
         aryAutoPassword[20] = 0;
#endif
      }
      else if ( !strncmp( aryLine, "aryBrowser ", sizeof( "aryBrowser " ) - 1 ) )
      {
         if ( strlen( aryLine ) < 13 )
         {
            stdPrintf( "Invalid definition of 'aryBrowser' ignored.\n" );
         }
         else
         {
            flagsConfiguration.shouldRunBrowserInBackground = ( aryLine[11] == '0' ) ? 0 : 1;
            strncpy( aryBrowser, aryLine + 13, 80 );
         }
      }
      else if ( !strncmp( aryLine, "aryEditor ", sizeof( "aryEditor " ) - 1 ) )
      {
         if ( *aryEditor )
         {
            stdPrintf( "Multiple definition of 'aryEditor' ignored.\n" );
         }
         else
         {
            strncpy( aryEditor, aryLine + ( sizeof( "aryEditor " ) - 1 ), 72 );
         }
      }
      else if ( !strncmp( aryLine, "site ", 5 ) )
      {
         if ( *aryBbsHost )
         {
            stdPrintf( "Multiple definition of 'site' ignored.\n" );
         }
         else
         {
            for ( parseIndex = 5; ( aryBbsHost[parseIndex - 5] = aryLine[parseIndex] ) && aryLine[parseIndex] != ' ' && parseIndex < 68; parseIndex++ )
            {
               ;
            }
            if ( parseIndex == 68 || parseIndex == 5 )
            {
               stdPrintf( "Illegal hostname in 'site', using default.\n" );
               *aryBbsHost = 0;
            }
            else
            {
               aryBbsHost[parseIndex - 5] = 0;
               if ( aryLine[parseIndex] )
               {
                  bbsPort = (unsigned short)atoi( aryLine + parseIndex + 1 );
               }
               else
               {
                  bbsPort = BBS_PORT_NUMBER;
               }
               for ( ; aryLine[parseIndex] && aryLine[parseIndex] != ' '; parseIndex++ )
               {
                  ;
               }
               if ( !strncmp( aryLine + parseIndex, "secure", 6 ) )
               {
                  shouldUseSsl = 1;
               }
            }
            if ( !strcmp( aryBbsHost, "128.255.200.69" ) ||
                 !strcmp( aryBbsHost, "128.255.85.69" ) ||
                 !strcmp( aryBbsHost, "128.255.95.69" ) ||
                 !strcmp( aryBbsHost, "128.255.3.160" ) ||
                 !strcmp( aryBbsHost, "bbs.iscabbs.info" ) )
            {                                                                    /* Old addresses */
               snprintf( aryBbsHost, sizeof( aryBbsHost ), "%s", BBS_HOSTNAME ); /* changed to new */
            }
         }
      }
      else if ( ( !strncmp( aryLine, "friend ", 7 ) ) && ( !bbsFriends || !fgets( aryScratchLine, MAX_LINE_LENGTH + 1, bbsFriends ) ) )
      {
         if ( strlen( aryLine ) == 7 )
         {
            stdPrintf( "Empty username in 'friend'.\n" );
         }
         else
         {
            if ( slistFind( friendList, aryLine + 7, fStrCompareVoid ) != -1 )
            {
               stdPrintf( "Duplicate username in 'friend'.\n" );
            }
            else if ( strlen( aryLine ) > 30 )
            {
               ptrFriend = (friend *)calloc( 1, sizeof( friend ) );
               if ( !ptrFriend )
               {
                  fatalExit( "Out of memory adding 'friend'!\n", "Fatal error" );
               }
               strncpy( ptrFriend->info, aryLine + 30, 53 );
               for ( nameLength = 19; nameLength > 0; nameLength-- )
               {
                  if ( aryLine[7 + nameLength] == ' ' )
                  {
                     hold = (size_t)nameLength;
                  }
                  else
                  {
                     break;
                  }
               }
               strncpy( ptrFriend->name, aryLine + 7, hold );
            }
            else
            {
               ptrFriend = (friend *)calloc( 1, sizeof( friend ) );
               if ( !ptrFriend )
               {
                  fatalExit( "Out of memory adding 'friend'!\n", "Fatal error" );
               }
               strncpy( ptrFriend->name, aryLine + 7, 19 );
               hold = sizeof( ptrFriend->name );
               snprintf( ptrFriend->info, sizeof( ptrFriend->info ), "%s", "(None)" );
            }
            ptrFriend->magic = 0x3231;
            if ( !slistAddItem( friendList, ptrFriend, 1 ) )
            {
               fatalExit( "Can't add 'friend'!\n", "Fatal error" );
            }
         }
      }
      else if ( !strncmp( aryLine, "enemy ", 6 ) )
      {
         if ( strlen( aryLine ) == 6 )
         {
            stdPrintf( "Empty username in 'enemy'.\n" );
         }
         else if ( slistFind( enemyList, aryLine + 6, strCompareVoid ) != -1 )
         {
            stdPrintf( "Duplicate username in 'enemy'.\n" );
         }
         else
         {
            ptrNameCopy = (char *)calloc( 1, strlen( aryLine + 6 ) + 1 );
            if ( !ptrNameCopy )
            {
               fatalExit( "Out of memory adding 'enemy'!\n", "Fatal error" );
            }
            snprintf( ptrNameCopy, strlen( aryLine + 6 ) + 1, "%s", aryLine + 6 );
            if ( !slistAddItem( enemyList, (void *)ptrNameCopy, 1 ) )
            {
               fatalExit( "Can't add 'enemy' to list!\n", "Fatal error" );
            }
         }
      }
      else if ( !strncmp( aryLine, "commandkey ", 11 ) ||
                !strncmp( aryLine, "macrokey ", 9 ) )
      {
         if ( commandKey >= 0 )
         {
            stdPrintf( "Additional definition for 'commandkey' ignored.\n" );
         }
         else
         {
            if ( !strncmp( aryLine, "macrokey ", 9 ) )
            {
               commandKey = ctrl( aryLine + 9 );
            }
            else
            {
               commandKey = ctrl( aryLine + 11 );
            }
            if ( findChar( "\0x01\0x03\0x04\0x05\b\n\r\0x11\0x13\0x15\0x17\0x18\0x19\0x1a\0x7f", commandKey ) || commandKey >= ' ' )
            {
               stdPrintf( "Illegal value for 'commandkey', using default of 'Esc'.\n" );
               commandKey = 0x1b;
            }
         }
      }
      else if ( !strncmp( aryLine, "awaykey ", 8 ) )
      {
         if ( awayKey >= 0 )
         {
            stdPrintf( "Additional definition for 'awaykey' ignored.\n" );
         }
         else
         {
            awayKey = ctrl( aryLine + 8 );
         }
      }
      else if ( !strncmp( aryLine, "quit ", 5 ) )
      {
         if ( quitKey >= 0 )
         {
            stdPrintf( "Additional definition for 'quit' ignored.\n" );
         }
         else
         {
            quitKey = ctrl( aryLine + 5 );
         }
      }
      else if ( !strncmp( aryLine, "susp ", 5 ) )
      {
         if ( suspKey >= 0 )
         {
            stdPrintf( "Additional definition for 'susp' ignored.\n" );
         }
         else
         {
            suspKey = ctrl( aryLine + 5 );
         }
      }
      else if ( !strncmp( aryLine, "capture ", 8 ) )
      {
         if ( captureKey >= 0 )
         {
            stdPrintf( "Additional definition for 'capture' ignored.\n" );
         }
         else
         {
            captureKey = ctrl( aryLine + 8 );
         }
      }
      else if ( !strncmp( aryLine, "aryKeyMap ", sizeof( "aryKeyMap " ) - 1 ) )
      {
         parseIndex = *( aryLine + ( sizeof( "aryKeyMap " ) - 1 ) );
         ptrToken = aryLine + sizeof( "aryKeyMap " );
         if ( *ptrToken++ == ' ' && parseIndex > 32 && parseIndex < 128 )
         {
            aryKeyMap[parseIndex] = *ptrToken;
         }
         else
         {
            stdPrintf( "Invalid value for 'aryKeyMap' ignored.\n" );
         }
      }
      else if ( !strncmp( aryLine, "url ", 4 ) )
      {
         if ( browserKey >= 0 )
         {
            stdPrintf( "Additional definition for 'url' ignored.\n" );
         }
         else
         {
            browserKey = ctrl( aryLine + 4 );
         }
      }
      else if ( !strncmp( aryLine, "aryMacro ", sizeof( "aryMacro " ) - 1 ) )
      {
         parseIndex = ctrl( aryLine + ( sizeof( "aryMacro " ) - 1 ) );
         ptrToken = aryLine + sizeof( "aryMacro " ) + ( aryLine[sizeof( "aryMacro " ) - 1] == '^' );
         if ( *ptrToken++ == ' ' )
         {
            if ( *aryMacro[parseIndex] )
            {
               stdPrintf( "Additional definition of same 'aryMacro' value ignored.\n" );
            }
            else
            {
               /* Import 'i' aryMacro to aryAwayMessageLines */
               if ( parseIndex == 'i' && !awayKey && tmpVersion < 220 )
               {
                  int messageLineIndex = 0;
                  ptrMacroWrite = aryAwayMessageLines[0];
                  awayKey = 'i';
                  while ( ( parseIndex = *ptrToken++ ) )
                  {
                     if ( parseIndex == '^' && *ptrToken != '^' )
                     {
                        parseIndex = ctrl( ptrToken++ - 1 );
                     }
                     if ( parseIndex == '\r' )
                     {
                        parseIndex = '\n';
                     }
                     if ( parseIndex == '\n' )
                     {
                        ptrMacroWrite = aryAwayMessageLines[++messageLineIndex];
                     }
                     else if ( iscntrl( parseIndex ) )
                     {
                        continue;
                     }
                     else
                     {
                        *ptrMacroWrite++ = (char)parseIndex;
                     }
                  }
               }
               else
               {
                  ptrMacroWrite = aryMacro[parseIndex];
                  while ( ( parseIndex = *ptrToken++ ) )
                  {
                     if ( parseIndex == '^' && *ptrToken != '^' )
                     {
                        parseIndex = ctrl( ptrToken++ - 1 );
                     }
                     if ( parseIndex == '\r' )
                     {
                        parseIndex = '\n';
                     }
                     *ptrMacroWrite++ = (char)parseIndex;
                  }
               }
            }
         }
         else
         {
            stdPrintf( "Syntax error in 'aryMacro', ignored.\n" );
         }
      }
      else if ( !strncmp( aryLine, "aryAwayMessageLines ", sizeof( "aryAwayMessageLines " ) - 1 ) )
      {
         /* Import old isAway messages */
         int messageLineIndex = 0;
         ptrMacroWrite = aryAwayMessageLines[messageLineIndex];
         ptrToken = aryLine + ( sizeof( "aryAwayMessageLines " ) - 1 );
         while ( ( parseIndex = *ptrToken++ ) )
         {
            if ( parseIndex == '^' && *ptrToken != '^' )
            {
               parseIndex = ctrl( ptrToken++ - 1 );
            }
            if ( parseIndex == '\r' )
            {
               parseIndex = '\n';
            }
            if ( parseIndex == '\n' )
            {
               ptrMacroWrite = aryAwayMessageLines[++messageLineIndex];
            }
            else if ( iscntrl( parseIndex ) )
            {
               continue;
            }
            else
            {
               *ptrMacroWrite++ = (char)parseIndex;
            }
         }
      }
      else if ( aryLine[0] == 'a' && aryLine[1] >= '1' && aryLine[1] <= '5' &&
                aryLine[2] == ' ' )
      {
         /* New isAway messages */
         snprintf( aryAwayMessageLines[aryLine[1] - '1'], sizeof( aryAwayMessageLines[0] ), "%s", aryLine + 3 );
      }
      else if ( !strncmp( aryLine, "aryShell ", sizeof( "aryShell " ) - 1 ) )
      {
         if ( shellKey >= 0 )
         {
            stdPrintf( "Additional definition for 'aryShell' ignored.\n" );
         }
         else
         {
            shellKey = ctrl( aryLine + ( sizeof( "aryShell " ) - 1 ) );
         }
      }
      else if ( *aryLine != '#' && *aryLine && strncmp( aryLine, "friend ", 7 ) )
      {
         stdPrintf( "Syntax error in .bbsrc file in line %d.\n", lineNumber );
      }
   }

   if ( bbsFriends )
   {
      rewind( bbsFriends );
   }
   while ( bbsFriends && fgets( aryLine, MAX_LINE_LENGTH + 1, bbsFriends ) )
   {
      reads++;
      lineNumber++;
      if ( strlen( aryLine ) >= MAX_LINE_LENGTH )
      {
         stdPrintf( "Line %d in .bbsfriends too long, ignored.\n", lineNumber );
         while ( strlen( aryLine ) >= MAX_LINE_LENGTH && aryLine[MAX_LINE_LENGTH - 1] != '\n' )
         {
            fgets( aryLine, MAX_LINE_LENGTH + 1, bbsFriends );
         }
         continue;
      }
      {
         size_t lineLength = strlen( aryLine );
         while ( lineLength > 0 )
         {
            size_t index = lineLength - 1;
            if ( aryLine[index] == ' ' || aryLine[index] == '\t' ||
                 aryLine[index] == '\n' || aryLine[index] == '\r' )
            {
               aryLine[index] = 0;
            }
            else
            {
               break;
            }
            lineLength = index;
         }
      }

      if ( !strncmp( aryLine, "friend ", 7 ) )
      {
         if ( strlen( aryLine ) == 7 )
         {
            stdPrintf( "Empty username in 'friend'.\n" );
         }
         else
         {
            if ( slistFind( friendList, aryLine + 7, fStrCompareVoid ) != -1 )
            {
               stdPrintf( "Duplicate username in 'friend'.\n" );
            }
            else if ( strlen( aryLine ) > 30 )
            {
               ptrFriend = (friend *)calloc( 1, sizeof( friend ) );
               if ( !ptrFriend )
               {
                  fatalExit( "Out of memory adding 'friend'!\n", "Fatal error" );
               }
               strncpy( ptrFriend->info, aryLine + 30, 53 );
               for ( nameLength = 19; nameLength > 0; nameLength-- )
               {
                  if ( aryLine[7 + nameLength] == ' ' )
                  {
                     hold = (size_t)nameLength;
                  }
                  else
                  {
                     break;
                  }
               }
               strncpy( ptrFriend->name, aryLine + 7, hold );
            }
            else
            {
               strncpy( ptrFriend->name, aryLine + 7, 19 );
               hold = sizeof( ptrFriend->name );
               snprintf( ptrFriend->info, sizeof( ptrFriend->info ), "%s", "(None)" );
            }
            ptrFriend->magic = 0x3231;
            if ( !slistAddItem( friendList, ptrFriend, 1 ) )
            {
               fatalExit( "Can't add 'friend' to list!\n", "Fatal error" );
            }
         }
      }
   }

   /*
    if (!bbsrc || !reads) {
   commandKey = ESC;
   awayKey = 'a';
   quitKey = CTRL_D;
   suspKey = CTRL_Z;
   captureKey = 'c';
   shellKey = '!';
    }
    */
   if ( commandKey == -1 )
   {
      commandKey = ESC;
   }
   if ( awayKey == -1 )
   {
      awayKey = 'a';
   }
   if ( quitKey == -1 )
   {
      quitKey = CTRL_D;
   }
   if ( suspKey == -1 )
   {
      suspKey = CTRL_Z;
   }
   if ( captureKey == -1 )
   {
      captureKey = 'c';
   }
   if ( shellKey == -1 )
   {
      shellKey = '!';
   }
   if ( browserKey == -1 )
   {
      browserKey = 'w';
   }

   if ( !**aryAwayMessageLines )
   {
      snprintf( aryAwayMessageLines[0], sizeof( aryAwayMessageLines[0] ), "%s", "I'm away from my keyboard right now." );
      *aryAwayMessageLines[1] = 0;
   }

   defaultColors( 0 );

   if ( quitKey >= 0 && *aryMacro[quitKey] )
   {
      stdPrintf( "Warning: duplicate definition of 'aryMacro' and 'quit'\n" );
   }
   if ( suspKey >= 0 && *aryMacro[suspKey] )
   {
      stdPrintf( "Warning: duplicate definition of 'aryMacro' and 'susp'\n" );
   }
   if ( captureKey >= 0 && *aryMacro[captureKey] )
   {
      stdPrintf( "Warning: duplicate definition of 'aryMacro' and 'capture'\n" );
   }
   if ( shellKey >= 0 && *aryMacro[captureKey] )
   {
      stdPrintf( "Warning: duplicate definition of 'aryMacro' and 'aryShell'\n" );
   }
   if ( quitKey >= 0 && quitKey == suspKey )
   {
      stdPrintf( "Warning: duplicate definition of 'quit' and 'susp'\n" );
   }
   if ( quitKey >= 0 && quitKey == captureKey )
   {
      stdPrintf( "Warning: duplicate definition of 'quit' and 'capture'\n" );
   }
   if ( quitKey >= 0 && quitKey == shellKey )
   {
      stdPrintf( "Warning: duplicate definition of 'quit' and 'aryShell'\n" );
   }
   if ( suspKey >= 0 && suspKey == captureKey )
   {
      stdPrintf( "Warning: duplicate definition of 'susp' and 'capture'\n" );
   }
   if ( suspKey >= 0 && suspKey == shellKey )
   {
      stdPrintf( "Warning: duplicate definition of 'susp' and 'aryShell'\n" );
   }
   if ( captureKey >= 0 && captureKey == shellKey )
   {
      stdPrintf( "Warning: duplicate definition of 'capture' and 'aryShell'\n" );
   }

   /* Load who list */
   for ( unsigned int zi = 0; zi < friendList->nitems; zi++ )
   {
      ptrFriend = friendList->items[zi];
      if ( !( ptrNameCopy = (char *)calloc( 1, strlen( ptrFriend->name ) + 1 ) ) )
      {
         fatalExit( "Out of memory for list copy!\r\n", "Fatal error" );
      }
      snprintf( ptrNameCopy, strlen( ptrFriend->name ) + 1, "%s", ptrFriend->name );
      if ( !( slistAddItem( whoList, ptrNameCopy, 1 ) ) )
      {
         fatalExit( "Out of memory adding item in list copy!\r\n", "Fatal error" );
      }
   }

   slistSort( friendList );
   slistSort( enemyList );
   slistSort( whoList );

   if ( !*aryBbsHost )
   {
      snprintf( aryBbsHost, sizeof( aryBbsHost ), "%s", BBS_HOSTNAME );
      bbsPort = BBS_PORT_NUMBER;
   }
   if ( !*aryEditor )
   {
      snprintf( aryEditor, sizeof( aryEditor ), "%s", aryMyEditor );
   }
   if ( version != tmpVersion )
   {
      if ( reads )
      {
         setup( tmpVersion );
      }
      else
      {
         setup( -1 );
      }
   }
   if ( isLoginShell )
   {
      setTerm();
      configBbsRc();
      resetTerm();
   }
}

/*
 * Opens the bbsrc file, warning the user if it can't be opened or can't be
 * opened for write, returning the file pointer if it was opened successfully.
 */
FILE *openBbsRc( void )
{
   FILE *ptrFileHandle;
   int savedErrno;

   ptrFileHandle = fopen( aryBbsRcName, "r+" );
   if ( !ptrFileHandle )
   {
      savedErrno = errno;
      ptrFileHandle = fopen( aryBbsRcName, "w+" );
   }
   if ( !ptrFileHandle )
   {
      ptrFileHandle = fopen( aryBbsRcName, "r" );
      if ( ptrFileHandle )
      {
         isBbsRcReadOnly = 1;
         errno = savedErrno;
         sPerror( "Configuration is read-only", "Warning" );
      }
      else
      {
         sPerror( "Can't open configuration file", "Warning" );
      }
   }
   return ( ptrFileHandle );
}

/*
 * Opens the bbsfriends file, warning the user if it can't be opened or can't
 * be opened for write, returning the file pointer if it was opened
 * successfully.
 */
FILE *openBbsFriends( void )
{
   FILE *ptrFileHandle;

   ptrFileHandle = fopen( aryBbsFriendsName, "r" );
   return ( ptrFileHandle );
}
