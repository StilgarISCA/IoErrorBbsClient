/*
 * This file handles configuration of the bbsrc file.  Its somewhat sloppy but
 * it should do the job.  Someone else can put a nicer interface on it.
 */
#include "defs.h"
#include "ext.h"

#define GREETING \
   "\r\nWelcome to IO ERROR's ISCA BBS Client!  Please take a moment to familiarize\r\nyourself with some of our new features.\r\n\n"
#define UPGRADE \
   "Thank you for upgrading to the latest version of IO ERROR's ISCA BBS Client!\r\nPlease take a moment to familiarize yourself with our new features."
#define DOWNGRADE \
   "You appear to have downgraded your version of IO ERROR's ISCA BBS Client.\r\nIf you continue running this client, you may lose some of your preferences and\r\nfeatures you are accustomed to.  Please visit the above web site to upgrade\r\nto the latest version of IO ERROR's ISCA BBS Client."
#define BBSRC_INFO \
   "IO ERROR's ISCA BBS Client integrates the contents of the .bbsrc and\r\n.bbsfriends file into a single file.  This change is fully compatible with\r\nolder clients, however those clients might re-create the .bbsfriends file.\r\nThis should not be a problem for most people; however, we recommend making a\r\nbackup copy of your .bbsrc and .bbsfriends files.  If for some reason you NEED\r\nthe .bbsrc and .bbsfriends files separated, DO NOT RUN THIS CLIENT."
#define COLOR_INFO \
   "IO ERROR's ISCA BBS Client allows you to choose what colors posts and express\r\nmessages are displayed with.  Use the <C>olor menu in the client configuration\r\nmenu to create your customized color scheme."
#define ENEMY_INFO \
   "You can now turn off the notification of killed posts and express messages\r\nfrom people on your enemy list.\r\n\nSelect Yes to be notified, or No to not be notified."
#define SELECT_URL \
   "You can now go directly to a Web site address you see in a post or express\r\nmessage by pressing the command key and <w>.  You can also change this key in\r\nthe client configuration.  You can define a Web browser in the client configuration\r\nor otherwise I will try to start Netscape."
#define ADVANCED_OPTIONS \
   "Advanced users may wish to use the configuration menu now to change options\r\nbefore logging in."

/*
 * First time setup borrowed from Client 9 with permission.
 */

/*
 * Performs first time setup for new features.
 */
void setup( int newVersion )
{
   setTerm();
   if ( newVersion < 1 )
   {
      stdPrintf( GREETING );
   }
   else if ( newVersion > INT_VERSION )
   {
      if ( !sPrompt( DOWNGRADE, "Continue running this client?", 0 ) )
      {
         myExit();
      }
   }
   else
   {
      sInfo( UPGRADE, "Upgrade" );
   }
   fflush( stdout );

   /* bbsrc file */
   if ( newVersion < 5 )
   {
      if ( !sPrompt( BBSRC_INFO, "Continue running this client?", 1 ) )
      {
         myExit();
      }
   }
   if ( newVersion < 220 )
   {
      if ( sPrompt( ENEMY_INFO, "Notify when posts and express messages from enemies are killed?", 1 ) )
      {
         flagsConfiguration.shouldSquelchPost = 0;
         flagsConfiguration.shouldSquelchExpress = 0;
      }
      else
      {
         flagsConfiguration.shouldSquelchPost = 1;
         flagsConfiguration.shouldSquelchExpress = 1;
      }

      fflush( stdout );
      sInfo( COLOR_INFO, "Colors" );
   }
   if ( newVersion < 237 )
   {
      sInfo( SELECT_URL, "Web sites" );
   }
   if ( sPrompt( ADVANCED_OPTIONS, "Configure the client now?", 0 ) )
   {
      configBbsRc();
   }
   else
   {
      writeBbsRc();
   }
   resetTerm();
   return;
}

/*
 * Changes settings in bbsrc file and saves it.
 */
void configBbsRc( void )
{
   char aryMenuLine[80];
   register int inputChar;
   register int itemIndex;
   register int innerIndex;
   unsigned int invalid;
   int lines;

   flagsConfiguration.isConfigMode = 1;
   if ( isBbsRcReadOnly )
   {
      stdPrintf( "\r\nConfiguration file is read-only, cannot arySavedBytes configuration for next session.\r\n" );
   }
   else if ( !ptrBbsRc )
   {
      stdPrintf( "\r\nNo configuration file, cannot arySavedBytes configuration for next session.\r\n" );
   }
   for ( ;; )
   {
      if ( flagsConfiguration.useAnsi )
      {
         colorize( "\r\n@YC@Color  @YE@Cnemy list  @YF@Criend list  @YH@Cotkeys\r\n@YI@Cnfo  @YM@Cacros  @YO@Cptions  @YX@Cpress  @YQ@Cuit@Y" );
      }
      else
      {
         stdPrintf( "\r\n<C>olor <E>nemy list <F>riend list <H>otkeys\r\n<I>aryInfo  <M>acros <O>ptions <X>press <Q>uit" );
      }
      colorize( "\r\nClient config -> @G" );
      for ( invalid = 0;; )
      {
         inputChar = inKey();
         if ( !findChar( "CcEeFfHhIiKkMmOoQqXx \n", inputChar ) )
         {
            if ( invalid++ )
            {
               flushInput( invalid );
            }
            continue;
         }
         break;
      }
      switch ( inputChar )
      {
         case 'c':
         case 'C':
            colorConfig();
            break;

         case 'x':
         case 'X':
            expressConfig();
            break;

         case 'i':
         case 'I':
            information();
            break;

         case 'o':
         case 'O':
            stdPrintf( "Options\r\n" );
            if ( !isLoginShell )
            {
               stdPrintf( "\r\nEnter name of local aryEditor to use (%s) -> ", aryEditor );
               getString( 72, aryMenuLine, -999 );
               if ( *aryMenuLine )
               {
                  snprintf( aryEditor, sizeof( aryEditor ), "%s", aryMenuLine );
               }
            }
            stdPrintf( "Show long who list by default? (%s) -> ", ( aryKeyMap['w'] == 'w' ) ? "No" : "Yes" );
            if ( yesNoDefault( ( aryKeyMap['w'] != 'w' ) ? 1 : 0 ) )
            {
               aryKeyMap['w'] = 'W';
               aryKeyMap['W'] = 'w';
            }
            else
            {
               aryKeyMap['w'] = 'w';
               aryKeyMap['W'] = 'W';
            }
            stdPrintf( "Show full profile by default? (%s) -> ", ( aryKeyMap['p'] == 'p' ) ? "No" : "Yes" );
            if ( yesNoDefault( ( aryKeyMap['p'] != 'p' ) ? 1 : 0 ) )
            {
               aryKeyMap['p'] = 'P';
               aryKeyMap['P'] = 'p';
            }
            else
            {
               aryKeyMap['p'] = 'p';
               aryKeyMap['P'] = 'P';
            }
            stdPrintf( "Enter name of site to connect to (%s) -> ", aryBbsHost );
            getString( 64, aryMenuLine, -999 );
            if ( *aryMenuLine )
            {
               snprintf( aryBbsHost, sizeof( aryBbsHost ), "%s", aryMenuLine );
            }
#if 0
       stdPrintf("Use secure (SSL) connection to this site? (%s) -> ",
   		    shouldUseSsl ? "Yes" : "No");
       if (yesNoDefault(shouldUseSsl))
   	shouldUseSsl = 1;
       else
#endif
            shouldUseSsl = 0;
            if ( ( !bbsPort || bbsPort == BBS_PORT_NUMBER ) && shouldUseSsl )
            {
               bbsPort = SSL_PORT_NUMBER;
            }
            else if ( ( !bbsPort || bbsPort == SSL_PORT_NUMBER ) && !shouldUseSsl )
            {
               bbsPort = BBS_PORT_NUMBER;
            }
            stdPrintf( "Enter port number to connect to (%d) -> ", bbsPort );
            getString( 5, aryMenuLine, -999 );
            if ( *aryMenuLine )
            {
               bbsPort = (unsigned short)atoi( aryMenuLine );
            }
            if ( !bbsPort )
            {
               if ( shouldUseSsl )
               {
                  bbsPort = SSL_PORT_NUMBER;
               }
               else
               {
                  bbsPort = BBS_PORT_NUMBER;
               }
            }
            stdPrintf( "Enter the Web aryBrowser to use (%s) -> ", aryBrowser );
            getString( 80, aryMenuLine, -999 );
            if ( *aryMenuLine )
            {
               strncpy( aryBrowser, aryMenuLine, 80 );
            }
            stdPrintf( "Does %s run in a separate window? (%s) -> ", aryBrowser,
                       flagsConfiguration.shouldRunBrowserInBackground ? "Yes" : "No" );
            flagsConfiguration.shouldRunBrowserInBackground = (unsigned int)yesNoDefault( flagsConfiguration.shouldRunBrowserInBackground );
            break;

         case 'h':
         case 'H':
            stdPrintf( "Hotkeys\r\n\n" );
            stdPrintf( "Enter command key (%s) -> ", strCtrl( commandKey ) );
            for ( ;; )
            {
               stdPrintf( "%s\r\n", strCtrl( commandKey = newKey( commandKey ) ) );
               if ( commandKey < ' ' )
               {
                  break;
               }
               stdPrintf( "You must use a control character for your command key, try again -> " );
            }
            stdPrintf( "Enter key to quit client (%s) -> ", strCtrl( quitKey ) );
            stdPrintf( "%s\r\n", strCtrl( quitKey = newKey( quitKey ) ) );
            if ( !isLoginShell )
            {
               stdPrintf( "Enter key to suspend client (%s) -> ", strCtrl( suspKey ) );
               stdPrintf( "%s\r\n", strCtrl( suspKey = newKey( suspKey ) ) );
               stdPrintf( "Enter key to start a new aryShell (%s) -> ", strCtrl( shellKey ) );
               stdPrintf( "%s\r\n", strCtrl( shellKey = newKey( shellKey ) ) );
            }
            stdPrintf( "Enter key to toggle capture mode (%s) -> ", strCtrl( captureKey ) );
            stdPrintf( "%s\r\n", strCtrl( captureKey = newKey( captureKey ) ) );
            stdPrintf( "Enter key to enable away from keyboard (%s) -> ", strCtrl( awayKey ) );
            stdPrintf( "%s\r\n", strCtrl( awayKey = newKey( awayKey ) ) );
            stdPrintf( "Enter key to browse a Web site (%s) -> ", strCtrl( browserKey ) );
            stdPrintf( "%s\r\n", strCtrl( browserKey = newKey( browserKey ) ) );
            break;

         case 'f':
         case 'F':
            stdPrintf( "Friend list\r\n" );
            editUsers( friendList, fStrCompareVoid, "friend" );
            break;

         case 'e':
         case 'E':
            stdPrintf( "Enemy list\r\n" );
            editUsers( enemyList, strCompareVoid, "enemy" );
            break;

         case 'm':
         case 'M':
            stdPrintf( "Macros\r\n" );
            for ( ; inputChar != 'q'; )
            {
               if ( flagsConfiguration.useAnsi )
               {
                  colorize( "\r\n@YE@Cdit  @YL@Cist  @YQ@Cuit\r\n@YMacro config -> @G" );
               }
               else
               {
                  stdPrintf( "\r\n<E>dit <L>ist <Q>uit\r\nMacro config -> " );
               }
               for ( invalid = 0;; )
               {
                  inputChar = inKey();
                  if ( !findChar( "EeLlQq \n", inputChar ) )
                  {
                     if ( invalid++ )
                     {
                        flushInput( invalid );
                     }
                     continue;
                  }
                  break;
               }
               switch ( inputChar )
               {
                  case 'e':
                  case 'E':
                     stdPrintf( "Edit\r\n" );
                     for ( ;; )
                     {
                        stdPrintf( "\r\nMacro to edit (%s to end) -> ", strCtrl( commandKey ) );
                        inputChar = newKey( -1 );
                        if ( inputChar == commandKey || inputChar == ' ' || inputChar == '\n' || inputChar == '\r' )
                        {
                           break;
                        }
                        stdPrintf( "%s\r\n", strCtrl( inputChar ) );
                        newMacro( inputChar );
                     }
                     stdPrintf( "Done\r\n" );
                     break;

                  case 'l':
                  case 'L':
                     stdPrintf( "List\r\n\n" );
                     for ( itemIndex = 0, lines = 1; itemIndex < 128; itemIndex++ )
                     {
                        if ( *aryMacro[itemIndex] )
                        {
                           stdPrintf( "'%s': \"", strCtrl( itemIndex ) );
                           for ( innerIndex = 0; aryMacro[itemIndex][innerIndex]; innerIndex++ )
                           {
                              stdPrintf( "%s", strCtrl( aryMacro[itemIndex][innerIndex] ) );
                           }
                           stdPrintf( "\"\r\n" );
                           if ( ++lines == rows - 1 && more( &lines, -1 ) < 0 )
                           {
                              break;
                           }
                        }
                     }
                     break;

                  case 'q':
                  case 'Q':
                  case ' ':
                  case '\n':
                     stdPrintf( "Quit\r\n" );
                     inputChar = 'q';
                     break;
               }
            }
            break;

         case 'q':
         case 'Q':
         case ' ':
         case '\n':
            stdPrintf( "Quit\r\n" );
            flagsConfiguration.isConfigMode = 0;
            if ( isBbsRcReadOnly || !ptrBbsRc )
            {
               return;
            }
            writeBbsRc();
            return;
            /* NOTREACHED */

         default:
            break;
      }
   }
}

void expressConfig( void )
{
   unsigned int invalid = 0;
   int inputChar;

   stdPrintf( "Express\r\n" );

   for ( ;; )
   {
      if ( flagsConfiguration.useAnsi )
      {
         colorize( "\r\n@YA@Cway  @YX@CLand  @YQ@Cuit\r\n@YExpress config -> @G" );
      }
      else
      {
         stdPrintf( "\r\n<A>way <X>Land <Q>uit\r\nExpress config -> " );
      }

      for ( invalid = 0;; )
      {
         inputChar = inKey();
         if ( !findChar( "AaXxQq \n", inputChar ) )
         {
            if ( invalid++ )
            {
               flushInput( invalid );
            }
            continue;
         }
         break;
      }

      switch ( inputChar )
      {
         case 'a':
         case 'A':
            stdPrintf( "Away from keyboard\r\n\n" );
            newAwayMessage();
            break;

         case 'x':
         case 'X':
            stdPrintf( "XLand\r\n\nAutomatically reply to X messages you receive? (%s) -> ", isXland ? "Yes" : "No" );
            isXland = yesNoDefault( isXland );
            break;

         case 'q':
         case 'Q':
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
      for ( itemIndex = 0; itemIndex < 5 && *aryAwayMessageLines[itemIndex]; itemIndex++ )
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
   for ( itemIndex = 0; itemIndex < 5; itemIndex++ )
   {
      *aryAwayMessageLines[itemIndex] = 0;
   }
   for ( itemIndex = 0; itemIndex < 5 && ( !itemIndex || *aryAwayMessageLines[itemIndex - 1] ); itemIndex++ )
   {
      stdPrintf( ">" );
      getString( itemIndex ? 78 : 74, aryAwayMessageLines[itemIndex], itemIndex );
   }
}

void writeBbsRc( void )
{
   char aryColorBytes[40];
   int itemIndex, innerIndex;
   const friend *ptrFriend;

   deleteFile( aryBbsFriendsName );
   rewind( ptrBbsRc );
   fprintf( ptrBbsRc, "aryEditor %s\n", aryEditor );
   /* Change:  site line will always be written */
   fprintf( ptrBbsRc, "site %s %d%s\n", aryBbsHost, bbsPort,
            shouldUseSsl ? " secure" : "" );
   fprintf( ptrBbsRc, "commandkey %s\n", strCtrl( commandKey ) );
   fprintf( ptrBbsRc, "quit %s\n", strCtrl( quitKey ) );
   fprintf( ptrBbsRc, "susp %s\n", strCtrl( suspKey ) );
   fprintf( ptrBbsRc, "aryShell %s\n", strCtrl( shellKey ) );
   fprintf( ptrBbsRc, "capture %s\n", strCtrl( captureKey ) );
   fprintf( ptrBbsRc, "awaykey %s\n", strCtrl( awayKey ) );
   fprintf( ptrBbsRc, "squelch %d\n", ( flagsConfiguration.shouldSquelchPost ? 2 : 0 ) + ( flagsConfiguration.shouldSquelchExpress ? 1 : 0 ) );
   fprintf( ptrBbsRc, "aryBrowser %d %s\n", flagsConfiguration.shouldRunBrowserInBackground ? 1 : 0, aryBrowser );
   if ( *aryAutoName )
   {
      fprintf( ptrBbsRc, "aryAutoName %s\n", aryAutoName );
   }
#ifdef ENABLE_SAVE_PASSWORD
   if ( *aryAutoPassword )
      fprintf( ptrBbsRc, "autopass %s\n", aryAutoPassword );
#endif
   bcopy( (void *)&color, aryColorBytes, sizeof color );
   aryColorBytes[sizeof color] = 0;
   fprintf( ptrBbsRc, "color %s\n", aryColorBytes );
   if ( flagsConfiguration.shouldAutoAnswerAnsiPrompt )
   {
      fprintf( ptrBbsRc, "autoansi\n" );
   }
   if ( **aryAwayMessageLines )
   {
      for ( itemIndex = 0; itemIndex < 5 && *aryAwayMessageLines[itemIndex]; itemIndex++ )
      {
         fprintf( ptrBbsRc, "a%d %s\n", itemIndex + 1, aryAwayMessageLines[itemIndex] );
      }
   }
   fprintf( ptrBbsRc, "version %d\n", version );
   if ( flagsConfiguration.useBold )
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

/*
 * Gets a new hotkey value or returns the old value if the default is taken. If
 * the old value is specified as -1, no checking is done to see if the new
 * value doesn't conflict with other hotkeys.  Calls getKey() instead of
 * inKey() to avoid the character translation (since the hotkey values are
 * checked within inKey() instead of getKey())
 */
int newKey( int oldkey )
{
   int inputChar;

   for ( ;; )
   {
      inputChar = getKey();
      if ( ( ( inputChar == ' ' || inputChar == '\n' || inputChar == '\r' ) && oldkey >= 0 ) || inputChar == oldkey )
      {
         return ( oldkey );
      }
      if ( oldkey >= 0 && ( inputChar == commandKey || inputChar == suspKey || inputChar == quitKey || inputChar == shellKey || inputChar == captureKey || inputChar == awayKey || inputChar == browserKey ) )
      {
         stdPrintf( "\r\nThat key is already in use for another hotkey, try again -> " );
      }
      else
      {
         return ( inputChar );
      }
   }
}

/*
 * Gets a new value for aryMacro 'which'.
 */
void newMacro( int which )
{
   register int itemIndex;
   register int inputChar;

   if ( *aryMacro[which] )
   {
      stdPrintf( "\r\nCurrent aryMacro for '%s' is: \"", strCtrl( which ) );
      for ( itemIndex = 0; aryMacro[which][itemIndex]; itemIndex++ )
      {
         stdPrintf( "%s", strCtrl( aryMacro[which][itemIndex] ) );
      }
      stdPrintf( "\"\r\nDo you wish to change this? (Y/N) -> " );
   }
   else
   {
      stdPrintf( "\r\nNo current aryMacro for '%s'.\r\nDo you want to make one? (Y/N) -> ", strCtrl( which ) );
   }
   if ( !yesNo() )
   {
      return;
   }
   stdPrintf( "\r\nEnter new aryMacro (use %s to end)\r\n -> ", strCtrl( commandKey ) );
   for ( itemIndex = 0;; itemIndex++ )
   {
      inputChar = inKey();
      if ( inputChar == '\b' )
      {
         if ( itemIndex )
         {
            if ( aryMacro[which][itemIndex - 1] < ' ' )
            {
               printf( "\b \b" );
            }
            itemIndex--;
            printf( "\b \b" );
         }
         itemIndex--;
         continue;
      }
      if ( inputChar == commandKey )
      {
         aryMacro[which][itemIndex] = 0;
         for ( itemIndex = 0; aryMacro[which][itemIndex]; itemIndex++ )
         { /* Shut up!! */
            capPrintf( "%s", strCtrl( aryMacro[which][itemIndex] ) );
         }
         stdPrintf( "\r\n" );
         return;
      }
      else if ( itemIndex == 70 )
      {
         itemIndex--;
         continue;
      }
      printf( "%s", strCtrl( aryMacro[which][itemIndex] = (char)inputChar ) );
   }
}

/*
 * Returns a string representation of inputChar suitable for printing.  If inputChar is a
 * regular character it will be printed normally, if it is a control character
 * it is printed as in the Unix ctlecho mode (itemIndex.e. ctrl-A is printed as ^A)
 */
char *strCtrl( int inputChar )
{
   static char aryControlText[3];

   if ( inputChar <= 31 || inputChar == DEL )
   {
      aryControlText[0] = '^';
      aryControlText[1] = (char)( inputChar == 10 ? 'M' : ( inputChar ^ 0x40 ) );
   }
   else
   {
      aryControlText[0] = (char)inputChar;
      aryControlText[1] = 0;
   }
   aryControlText[2] = 0;
   return ( aryControlText );
}

/*
 * Does the editing of the friend and enemy lists.
 */
void editUsers( slist *list, int ( *findfn )( const void *, const void * ), const char *name )
{
   register int inputChar;
   register int itemIndex = 0;
   unsigned int invalid = 0;
   int lines;
   char *ptrUserName;
   char aryInfo[50];
   char aryDisplayLine[80];
   char *ptrEnemyName;
   friend *ptrFriend;

   for ( ;; )
   {
      /* Build menu */
      if ( !strncmp( name, "enemy", 5 ) )
      {
         if ( flagsConfiguration.useAnsi )
         {
            colorize( "\r\n@YA@Cdd  @YD@Celete  @YL@Cist  @YO@Cptions  @YQ@Cuit@Y" );
         }
         else
         {
            stdPrintf( "\r\n<A>dd <D>elete <L>ist <O>ptions <Q>uit" );
         }
      }
      else if ( flagsConfiguration.useAnsi )
      {
         colorize( "\r\n@YA@Cdd  @YD@Celete  @YE@Cdit  @YL@Cist  @YQ@Cuit@Y" );
      }
      else
      {
         stdPrintf( "\r\n<A>dd <D>elete <E>dit <L>ist <Q>uit" );
      }
      snprintf( aryDisplayLine, sizeof( aryDisplayLine ), "\r\n%c%s list -> @G", toupper( name[0] ), name + 1 );
      colorize( aryDisplayLine );

      inputChar = inKey();
      switch ( inputChar )
      {
         case 'a':
         case 'A':
            stdPrintf( "Add\r\n" );
            stdPrintf( "\r\nUser to add to your %s list -> ", name );
            ptrUserName = getName( -999 );
            if ( *ptrUserName )
            {
               if ( slistFind( list, ptrUserName, findfn ) != -1 )
               {
                  stdPrintf( "\r\n%s is already on your %s list.\r\n", ptrUserName, name );
                  itemIndex = -1;
               }
#if DEBUG
               printf( "{%d %s} ", itemIndex, ptrUserName );
#endif
               if ( itemIndex < 0 )
               {
                  break;
               }
               if ( !strcmp( name, "friend" ) )
               {
                  if ( !( ptrFriend = (friend *)calloc( 1, sizeof( friend ) ) ) )
                  {
                     fatalExit( "Out of memory adding 'friend'!\n", "Fatal error" );
                  }
                  snprintf( ptrFriend->name, sizeof( ptrFriend->name ), "%s", ptrUserName );
                  stdPrintf( "Enter info for %s: ", ptrUserName );
                  getString( 48, aryInfo, -999 );
                  snprintf( ptrFriend->info, sizeof( ptrFriend->info ), "%s", ( *aryInfo ) ? aryInfo : "(None)" );
                  ptrFriend->magic = 0x3231;
                  if ( !slistAddItem( list, ptrFriend, 0 ) )
                  {
                     fatalExit( "Can't add 'friend'!\n", "Fatal error" );
                  }
               }
               else
               { /* enemy list */
                  ptrEnemyName = (char *)calloc( 1, strlen( ptrUserName ) + 1 );
                  if ( !ptrEnemyName )
                  {
                     fatalExit( "Out of memory adding 'enemy'!\r\n", "Fatal error" );
                  }
                  else
                  {
                     snprintf( ptrEnemyName, strlen( ptrUserName ) + 1, "%s", ptrUserName ); /* 2.1.2 bugfix */
                  }
                  if ( !slistAddItem( list, ptrEnemyName, 0 ) )
                  {
                     fatalExit( "Can't add 'enemy'!\r\n", "Fatal error" );
                  }
               }
               stdPrintf( "\r\n%s was added to your %s list.\r\n", ptrUserName, name );
            }
            break;

         case 'd':
         case 'D':
            stdPrintf( "Delete\r\n\nUser to delete from your %s list -> ", name );
            ptrUserName = getName( -999 );
            if ( *ptrUserName )
            {
               itemIndex = slistFind( list, ptrUserName, findfn );
               if ( itemIndex != -1 )
               {
                  free( list->items[itemIndex] );
                  if ( !slistRemoveItem( list, itemIndex ) )
                  {
                     fatalExit( "Can't remove aryUser!\r\n", "Fatal error" );
                  }
                  stdPrintf( "\r\n%s was deleted from your %s list.\r\n", ptrUserName, name );
               }
               else
               {
                  stdPrintf( "\r\n%s is not in your %s list.\r\n", ptrUserName, name );
               }
            }
            break;

         case 'e':
         case 'E':
            if ( !strncmp( name, "friend", 6 ) )
            {
               stdPrintf( "Edit\r\nName of aryUser to edit: " );
               ptrUserName = getName( -999 );
               if ( *ptrUserName )
               {
                  if ( ( itemIndex = slistFind( list, ptrUserName, findfn ) ) != -1 )
                  {
                     ptrFriend = list->items[itemIndex];
                     if ( !strcmp( ptrFriend->name, ptrUserName ) )
                     {
                        stdPrintf( "Current info: %s\r\n", ptrFriend->info );
                        stdPrintf( "Return to leave unchanged, NONE to erase.\r\n" );
                        stdPrintf( "Enter new info: " );
                        getString( 48, aryInfo, -999 );
                        if ( *aryInfo )
                        {
                           if ( !strcmp( aryInfo, "NONE" ) )
                           {
                              snprintf( ptrFriend->info, sizeof( ptrFriend->info ), "%s", "(None)" );
                           }
                           else
                           {
                              snprintf( ptrFriend->info, sizeof( ptrFriend->info ), "%s", aryInfo );
                           }
                        }
                     }
                  }
                  else
                  {
                     stdPrintf( "\r\n%s is not in your %s list.\r\n", ptrUserName, name );
                  }
               }
               break;
            }
            else
            {
               if ( invalid++ )
               {
                  flushInput( invalid );
               }
               continue;
            }

         case 'l':
         case 'L':
            stdPrintf( "List\r\n\n" );
            if ( !strcmp( name, "friend" ) )
            {
               lines = 1;
               for ( itemIndex = 0; itemIndex < (int)list->nitems; itemIndex++ )
               {
                  ptrFriend = list->items[itemIndex];
                  snprintf( aryDisplayLine, sizeof( aryDisplayLine ), "@Y%-20s @C%s@G\r\n", ptrFriend->name, ptrFriend->info );
                  colorize( aryDisplayLine );
                  lines++;
                  if ( lines == rows - 1 && more( &lines, -1 ) < 0 )
                  {
                     break;
                  }
               }
            }
            else
            {
               lines = 1;
               for ( itemIndex = 0; itemIndex < (int)list->nitems; itemIndex++ )
               {
                  stdPrintf( "%-19s%s", list->items[itemIndex], ( itemIndex % 4 ) == 3 ? "\r\n" : " " );
                  if ( ( itemIndex % 4 ) == 3 )
                  {
                     lines++;
                  }
                  if ( lines == rows - 1 && more( &lines, -1 ) < 0 )
                  {
                     break;
                  }
               }
               if ( itemIndex % 4 )
               {
                  stdPrintf( "\r\n" );
               }
            }
            break;

         case 'q':
         case 'Q':
         case '\n':
         case ' ':
            stdPrintf( "Quit\r\n" );
            return;

         case 'o':
         case 'O':
            if ( !strncmp( name, "enemy", 5 ) )
            {
               stdPrintf( "Options\r\n\nNotify when an enemy's post is killed? (%s) -> ",
                          flagsConfiguration.shouldSquelchPost ? "No" : "Yes" );
               flagsConfiguration.shouldSquelchPost = !yesNoDefault( !flagsConfiguration.shouldSquelchPost );
               stdPrintf( "Notify when an enemy's eXpress message is killed? (%s) -> ",
                          flagsConfiguration.shouldSquelchExpress ? "No" : "Yes" );
               flagsConfiguration.shouldSquelchExpress = !yesNoDefault( !flagsConfiguration.shouldSquelchExpress );
            }
            /* Fall through */

         default:
            if ( invalid++ )
            {
               flushInput( invalid );
            }
            continue;
      }
      invalid = 0;
   }
}
