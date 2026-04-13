/*
 * Copyright (C) 2024-2026 Stilgar
 * Copyright (C) 1995-2003 Michael Hampton
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

/*
 * This file handles configuration of the bbsrc file.  Its somewhat sloppy but
 * it should do the job.  Someone else can put a nicer interface on it.
 */
#include "defs.h"
#include "ext.h"

static const char *CONFIG_MAIN_MENU_KEYS = "cefhikmoqx \n";

#define GREETING \
   "\r\nWelcome to IO ERROR's ISCA BBS Client!  Please take a moment to familiarize\r\nyourself with some of our new features.\r\n\n"
#define UPGRADE \
   "Thank you for upgrading to the latest version of IO ERROR's ISCA BBS Client!\r\nPlease take a moment to familiarize yourself with our new features."
#define DOWNGRADE \
   "You appear to have downgraded your version of IO ERROR's ISCA BBS Client.\r\nIf you continue running this client, you may lose some of your preferences and\r\nfeatures you are accustomed to.  Please visit the above website to upgrade\r\nto the latest version of IO ERROR's ISCA BBS Client."
#define BBSRC_INFO \
   "IO ERROR's ISCA BBS Client integrates the contents of the .bbsrc and\r\n.bbsfriends file into a single file.  This change is fully compatible with\r\nolder clients, however those clients might re-create the .bbsfriends file.\r\nThis should not be a problem for most people; however, we recommend making a\r\nbackup copy of your .bbsrc and .bbsfriends files.  If for some reason you NEED\r\nthe .bbsrc and .bbsfriends files separated, DO NOT RUN THIS CLIENT."
#define COLOR_INFO \
   "IO ERROR's ISCA BBS Client allows you to choose what colors posts and express\r\nmessages are displayed with.  Use the <C>olor menu in the client configuration\r\nmenu to create your customized color scheme."
#define ENEMY_INFO \
   "You can now turn off the notification of killed posts and express messages\r\nfrom people on your enemy list.\r\n\nSelect Yes to be notified, or No to not be notified."
#define ADVANCED_OPTIONS \
   "Advanced users may wish to use the configuration menu now to change options\r\nbefore logging in."
#define SCREEN_READER_INFO \
   "Screen reader friendly mode keeps this client easier for VoiceOver and other\r\nscreen readers to follow.  You can change this later from the Options menu."

static const char *describeKeyForHelp( int inputChar )
{
   switch ( inputChar )
   {
      case ESC:
         return "Esc";

      case ' ':
         return "Space";

      case '\n':
      case '\r':
         return "Return";

      case '\t':
         return "Tab";

      default:
         return strCtrl( inputChar );
   }
}

void promptForScreenReaderModeIfUnset( void )
{
   if ( flagsConfiguration.hasScreenReaderModeSetting )
   {
      return;
   }

   flagsConfiguration.isScreenReaderModeEnabled =
      (unsigned int)sPrompt( SCREEN_READER_INFO,
                             "Use screen reader friendly mode?",
                             0 );
   flagsConfiguration.hasScreenReaderModeSetting = 1;
}

void defaultNameAutocompleteIfUnset( void )
{
   if ( flagsConfiguration.hasNameAutocompleteSetting )
   {
      return;
   }

   flagsConfiguration.shouldEnableNameAutocomplete =
      (unsigned int)!flagsConfiguration.isScreenReaderModeEnabled;
   flagsConfiguration.hasNameAutocompleteSetting = 1;
}

static void setKeyDefaultToUppercase( int lowerKey, bool shouldUseUppercaseByDefault )
{
   int upperKey;

   upperKey = toupper( lowerKey );
   if ( shouldUseUppercaseByDefault )
   {
      aryKeyMap[lowerKey] = (char)upperKey;
      aryKeyMap[upperKey] = (char)lowerKey;
   }
   else
   {
      aryKeyMap[lowerKey] = (char)lowerKey;
      aryKeyMap[upperKey] = (char)upperKey;
   }
}

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
      char aryUrlInfo[512];

      snprintf( aryUrlInfo,
                sizeof( aryUrlInfo ),
                "You can go directly to a website address you see in a post or express\r\nmessage by pressing <%s> then <%s>.  You can also change these keys in\r\nthe client configuration.  Clickable URLs are also emitted directly to modern\r\nmacOS terminals using OSC 8 links.",
                describeKeyForHelp( commandKey ),
                describeKeyForHelp( browserKey ) );
      sInfo( aryUrlInfo, "Websites" );
   }
   promptForScreenReaderModeIfUnset();
   defaultNameAutocompleteIfUnset();
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

   flagsConfiguration.isConfigMode = 1;
   if ( isBbsRcReadOnly )
   {
      stdPrintf( "\r\nConfiguration file is read-only, unable to save configuration for next session.\r\n" );
   }
   else if ( !ptrBbsRc )
   {
      stdPrintf( "\r\nNo configuration file, unable to save configuration for next session.\r\n" );
   }
   while ( true )
   {
      printThemedMnemonicText( "\r\n<C>olor  <E>nemy list  <F>riend list  <H>otkeys\r\n<I>nfo  <M>acros  <O>ptions  <X>press  <Q>uit", color.number );
      printThemedMnemonicText( "\r\nClient config -> ", color.forum );
      printAnsiForegroundColorValue( color.text );
      inputChar = readValidatedMenuKey( CONFIG_MAIN_MENU_KEYS );
      switch ( inputChar )
      {
         case 'c':
            colorConfig();
            break;

         case 'x':
            expressConfig();
            break;

         case 'i':
            information();
            break;

         case 'o':
            stdPrintf( "Options\r\n" );
            stdPrintf( "Use screen reader friendly mode? (%s) -> ",
                       flagsConfiguration.isScreenReaderModeEnabled ? "Yes" : "No" );
            flagsConfiguration.isScreenReaderModeEnabled =
               (unsigned int)yesNoDefault( flagsConfiguration.isScreenReaderModeEnabled );
            flagsConfiguration.hasScreenReaderModeSetting = 1;
            if ( flagsConfiguration.isScreenReaderModeEnabled )
            {
               flagsConfiguration.shouldEnableClickableUrls = 0;
               flagsConfiguration.shouldEnableNameAutocomplete = 0;
            }
            if ( !isLoginShell )
            {
               stdPrintf( "Enter local editor to use (%s uses shell default) -> ", aryEditor );
               getString( 72, aryMenuLine, -999 );
               if ( *aryMenuLine )
               {
                  snprintf( aryEditor, sizeof( aryEditor ), "%s", aryMenuLine );
               }
            }
            stdPrintf( "Show long who list by default? (%s) -> ", ( aryKeyMap['w'] == 'w' ) ? "No" : "Yes" );
            setKeyDefaultToUppercase( 'w', yesNoDefault( ( aryKeyMap['w'] != 'w' ) ? 1 : 0 ) );
            stdPrintf( "Show full profile by default? (%s) -> ", ( aryKeyMap['p'] == 'p' ) ? "No" : "Yes" );
            setKeyDefaultToUppercase( 'p', yesNoDefault( ( aryKeyMap['p'] != 'p' ) ? 1 : 0 ) );
            stdPrintf( "Enter name of site to connect to (%s) -> ", aryBbsHost );
            getString( 64, aryMenuLine, -999 );
            if ( *aryMenuLine )
            {
               snprintf( aryBbsHost, sizeof( aryBbsHost ), "%s", aryMenuLine );
            }
#if 0
	      stdPrintf("Use secure (SSL) connection to this site? (%s) -> ",
		    shouldUseSsl ? "Yes" : "No");
	      if ( yesNoDefault( shouldUseSsl ) )
	      {
	         shouldUseSsl = 1;
	      }
	      else
	      {
	         shouldUseSsl = 0;
	      }
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
            stdPrintf( "Try to keep idle connections alive with TCP probes? (%s) -> ",
                       flagsConfiguration.shouldUseTcpKeepalive ? "Yes" : "No" );
            flagsConfiguration.shouldUseTcpKeepalive = (unsigned int)yesNoDefault( flagsConfiguration.shouldUseTcpKeepalive );
            flagsConfiguration.hasTitleBarSetting = 1;
            stdPrintf( "Update terminal title bar? (%s) -> ",
                       flagsConfiguration.shouldEnableTitleBar ? "Yes" : "No" );
            flagsConfiguration.shouldEnableTitleBar =
               (unsigned int)yesNoDefault( flagsConfiguration.shouldEnableTitleBar );
            stdPrintf( "Append OSC 8 URL summaries to posts & mail? (%s) -> ",
                       flagsConfiguration.shouldEnableClickableUrls ? "Yes" : "No" );
            flagsConfiguration.shouldEnableClickableUrls = (unsigned int)yesNoDefault( flagsConfiguration.shouldEnableClickableUrls );
            stdPrintf( "Autocomplete username in recipient prompts? (%s) -> ",
                       flagsConfiguration.shouldEnableNameAutocomplete ? "Yes" : "No" );
            flagsConfiguration.shouldEnableNameAutocomplete =
               (unsigned int)yesNoDefault( flagsConfiguration.shouldEnableNameAutocomplete );
            flagsConfiguration.hasNameAutocompleteSetting = 1;
            break;

         case 'h':
            configureHotkeys();
            break;

         case 'f':
            stdPrintf( "Friend list\r\n" );
            editUsers( friendList, fStrCompareVoid, "friend" );
            break;

         case 'e':
            stdPrintf( "Enemy list\r\n" );
            editUsers( enemyList, strCompareVoid, "enemy" );
            break;

         case 'm':
            configureMacros();
            break;

         case 'q':
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

void writeBbsRc( void )
{
   int itemIndex, innerIndex;
   const char *ptrColorName;
   const friend *ptrFriend;
   const int *ptrColorValues;

   deleteFile( aryBbsFriendsName );
   rewind( ptrBbsRc );
   fprintf( ptrBbsRc, "aryEditor %s\n", aryEditor );
   /* Change:  site line will always be written */
   fprintf( ptrBbsRc, "site %s %d%s\n", aryBbsHost, bbsPort,
            shouldUseSsl ? " secure" : "" );
   fprintf( ptrBbsRc, "commandkey %s\n", strCtrl( commandKey ) );
   fprintf( ptrBbsRc, "quit %s\n", strCtrl( quitKey ) );
   fprintf( ptrBbsRc, "susp %s\n", strCtrl( suspKey ) );
   fprintf( ptrBbsRc, "shellkey %s\n", strCtrl( shellKey ) );
   fprintf( ptrBbsRc, "capture %s\n", strCtrl( captureKey ) );
   fprintf( ptrBbsRc, "awaykey %s\n", strCtrl( awayKey ) );
   fprintf( ptrBbsRc, "squelch %d\n", ( flagsConfiguration.shouldSquelchPost ? 2 : 0 ) + ( flagsConfiguration.shouldSquelchExpress ? 1 : 0 ) );
   fprintf( ptrBbsRc, "keepalive %d\n", flagsConfiguration.shouldUseTcpKeepalive ? 1 : 0 );
   fprintf( ptrBbsRc, "clickableurls %d\n", flagsConfiguration.shouldEnableClickableUrls ? 1 : 0 );
   fprintf( ptrBbsRc, "titlebar %d\n", flagsConfiguration.shouldEnableTitleBar ? 1 : 0 );
   fprintf( ptrBbsRc, "screenreader %d\n", flagsConfiguration.isScreenReaderModeEnabled ? 1 : 0 );
   fprintf( ptrBbsRc, "autocomplete %d\n", flagsConfiguration.shouldEnableNameAutocomplete ? 1 : 0 );
   if ( *aryAutoName )
   {
      fprintf( ptrBbsRc, "aryAutoName %s\n", aryAutoName );
   }
#ifdef ENABLE_SAVE_PASSWORD
   if ( *aryAutoPassword )
   {
      fprintf( ptrBbsRc, "autopass %s\n", aryAutoPassword );
   }
#endif
   ptrColorValues = (const int *)&color;
   fprintf( ptrBbsRc, "color" );
   for ( itemIndex = 0; itemIndex < COLOR_FIELD_COUNT; itemIndex++ )
   {
      ptrColorName = colorNameFromValue( ptrColorValues[itemIndex] );
      if ( ptrColorName != NULL )
      {
         fprintf( ptrBbsRc, " %s", ptrColorName );
      }
      else
      {
         fprintf( ptrBbsRc, " %d", ptrColorValues[itemIndex] );
      }
   }
   fprintf( ptrBbsRc, "\n" );
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
   if ( flagsConfiguration.shouldUseBold )
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
