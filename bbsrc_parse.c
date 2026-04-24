/*
 * Copyright (C) 2024-2026 Stilgar
 * Copyright (C) 1995-2003 Michael Hampton
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

/*
 * This file parses .bbsrc and applies the configured settings.
 */
#include "defs.h"
#include "bbsrc.h"
#include "client.h"
#include "client_globals.h"
#include "color.h"
#include "config_globals.h"
#include "config_menu.h"
#include "filter_globals.h"
#include "utility.h"

#define MAX_LINE_LENGTH 512
#define FRIEND_COMMAND_PREFIX_LEN 7
#define FRIEND_NAME_PARSE_LENGTH 19
#define FRIEND_INFO_OFFSET 30
#define FRIEND_INFO_COPY_LENGTH 53
#define FRIEND_RECORD_MAGIC 0x3231

typedef enum
{
   BBRC_CMD_UNKNOWN = 0,
   BBRC_CMD_AWAYKEY,
   BBRC_CMD_AUTOANSI,
   BBRC_CMD_AUTONAME,
#ifdef ENABLE_SAVE_PASSWORD
   BBRC_CMD_AUTOPASS,
#endif
   BBRC_CMD_BOLD,
   BBRC_CMD_BROWSER,
   BBRC_CMD_CAPTURE,
   BBRC_CMD_CLICKABLE_URLS,
   BBRC_CMD_COLOR,
   BBRC_CMD_COMMANDKEY,
   BBRC_CMD_AUTOCOMPLETE,
   BBRC_CMD_EDITOR,
   BBRC_CMD_ENEMY,
   BBRC_CMD_FRIEND,
   BBRC_CMD_KEYMAP,
   BBRC_CMD_MACRO,
   BBRC_CMD_NEW_AWAY,
   BBRC_CMD_OLD_AWAY,
   BBRC_CMD_QUIT,
   BBRC_CMD_REREAD,
   BBRC_CMD_SHELL,
   BBRC_CMD_SITE,
   BBRC_CMD_SQUELCH,
   BBRC_CMD_SUSP,
   BBRC_CMD_TCP_KEEPALIVE,
   BBRC_CMD_TITLEBAR,
   BBRC_CMD_URL,
   BBRC_CMD_SCREENREADER,
   BBRC_CMD_VERSION,
   BBRC_CMD_XLAND,
   BBRC_CMD_XWRAP
} BbsRcCommandId;

typedef struct
{
   const char *ptrPrefix;
   size_t prefixLength;
   BbsRcCommandId commandId;
} BbsRcCommandSpec;

typedef enum
{
   BBRC_OPTION_INVALID = -1,
   BBRC_OPTION_DISABLED = 0,
   BBRC_OPTION_ENABLED = 1
} BbsRcOptionValue;

typedef struct
{
   int lineNumber;
   int reads;
   int tmpVersion;
   bool shouldShowBrowserMigrationNotice;
   bool shouldRewriteBbsRc;
} BbsRcReadState;

static bool addFriendFromLine( const char *ptrLine );
static void applyBbsRcKeyDefaults( void );
static int ctrl( const char *ptrToken );
static BbsRcCommandId detectBbsRcCommand( const char *ptrLine );
static void ensureDefaultAwayMessage( void );
static bool finalizeBbsRcRead( BbsRcReadState *ptrState );
static void initializeBbsRcDefaults( void );
static void initializeBbsRcLists( void );
static bool isNewAwayMessageCommand( const char *ptrLine );
static BbsRcOptionValue parseBooleanSettingValue( const char *ptrLine, size_t prefixLength, const char *ptrSettingName, bool shouldAllowAnyNonZeroValue );
static bool parseColorScheme( const char *ptrLine, int *ptrColorValues );
static bool parseNamedColorScheme( const char *ptrColorSpec, int *ptrColorValues );
static bool processBbsRcCommandLine( const char *ptrLine, BbsRcReadState *ptrState );
static bool readLegacyBbsFriends( char *ptrLine, BbsRcReadState *ptrState );
static void warnAboutBbsRcConflicts( void );

static bool addFriendFromLine( const char *ptrLine )
{
   friend *ptrFriend;
   int nameLength;
   size_t nameLengthToCopy;

   if ( strlen( ptrLine ) == FRIEND_COMMAND_PREFIX_LEN )
   {
      stdPrintf( "Empty username in 'friend'.\n" );
      return true;
   }
   if ( slistFind( friendList, (void *)( ptrLine + FRIEND_COMMAND_PREFIX_LEN ), fStrCompareVoid ) != -1 )
   {
      stdPrintf( "Duplicate username in 'friend'.\n" );
      return true;
   }

   ptrFriend = (friend *)calloc( 1, sizeof( friend ) );
   if ( !ptrFriend )
   {
      fatalExit( "Out of memory adding 'friend'!\n", "Fatal error" );
      return false;
   }

   if ( strlen( ptrLine ) > FRIEND_INFO_OFFSET )
   {
      strncpy( ptrFriend->info, ptrLine + FRIEND_INFO_OFFSET, FRIEND_INFO_COPY_LENGTH );
      nameLengthToCopy = 0;
      for ( nameLength = FRIEND_NAME_PARSE_LENGTH; nameLength > 0; nameLength-- )
      {
         if ( ptrLine[FRIEND_COMMAND_PREFIX_LEN + nameLength] == ' ' )
         {
            nameLengthToCopy = (size_t)nameLength;
         }
         else
         {
            break;
         }
      }
      strncpy( ptrFriend->name, ptrLine + FRIEND_COMMAND_PREFIX_LEN, nameLengthToCopy );
   }
   else
   {
      strncpy( ptrFriend->name, ptrLine + FRIEND_COMMAND_PREFIX_LEN, FRIEND_NAME_PARSE_LENGTH );
      snprintf( ptrFriend->info, sizeof( ptrFriend->info ), "%s", "(None)" );
   }

   ptrFriend->magic = FRIEND_RECORD_MAGIC;
   if ( !slistAddItem( friendList, ptrFriend, 1 ) )
   {
      fatalExit( "Can't add 'friend' to list!\n", "Fatal error" );
      return false;
   }
   return true;
}

static void applyBbsRcKeyDefaults( void )
{
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
}

/*
 * Given a pointer to a string, this function evaluates it to a control
 * character, translating '^z', '^Z', or an actual ctrl-Z to all be ctrl-Z. If
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

static BbsRcCommandId detectBbsRcCommand( const char *ptrLine )
{
   static const BbsRcCommandSpec aryCommands[] =
      {
         { "reread ", 7, BBRC_CMD_REREAD },
         { "xwrap ", 6, BBRC_CMD_XWRAP },
         { "bold", 4, BBRC_CMD_BOLD },
         { "xland", 5, BBRC_CMD_XLAND },
         { "version ", 8, BBRC_CMD_VERSION },
         { "squelch ", 8, BBRC_CMD_SQUELCH },
         { "keepalive", 9, BBRC_CMD_TCP_KEEPALIVE },
         { "clickableurls", 13, BBRC_CMD_CLICKABLE_URLS },
         { "titlebar", 8, BBRC_CMD_TITLEBAR },
         { "screenreader", 12, BBRC_CMD_SCREENREADER },
         { "autocomplete", 12, BBRC_CMD_AUTOCOMPLETE },
         { "urlsummary", 10, BBRC_CMD_CLICKABLE_URLS },
         { "color ", 6, BBRC_CMD_COLOR },
         { "aryAutoName ", sizeof( "aryAutoName " ) - 1, BBRC_CMD_AUTONAME },
         { "autoansi", 9, BBRC_CMD_AUTOANSI },
#ifdef ENABLE_SAVE_PASSWORD
         { "autopass ", 9, BBRC_CMD_AUTOPASS },
#endif
         { "aryBrowser ", sizeof( "aryBrowser " ) - 1, BBRC_CMD_BROWSER },
         { "aryEditor ", sizeof( "aryEditor " ) - 1, BBRC_CMD_EDITOR },
         { "site ", 5, BBRC_CMD_SITE },
         { "friend ", FRIEND_COMMAND_PREFIX_LEN, BBRC_CMD_FRIEND },
         { "enemy ", 6, BBRC_CMD_ENEMY },
         { "commandkey ", 11, BBRC_CMD_COMMANDKEY },
         { "macrokey ", 9, BBRC_CMD_COMMANDKEY },
         { "awaykey ", 8, BBRC_CMD_AWAYKEY },
         { "quit ", 5, BBRC_CMD_QUIT },
         { "susp ", 5, BBRC_CMD_SUSP },
         { "capture ", 8, BBRC_CMD_CAPTURE },
         { "aryKeyMap ", sizeof( "aryKeyMap " ) - 1, BBRC_CMD_KEYMAP },
         { "url ", 4, BBRC_CMD_URL },
         { "aryMacro ", sizeof( "aryMacro " ) - 1, BBRC_CMD_MACRO },
         { "aryAwayMessageLines ", sizeof( "aryAwayMessageLines " ) - 1, BBRC_CMD_OLD_AWAY },
         { "shellkey ", 9, BBRC_CMD_SHELL },
         { "aryShell ", sizeof( "aryShell " ) - 1, BBRC_CMD_SHELL } };
   size_t itemIndex;

   if ( isNewAwayMessageCommand( ptrLine ) )
   {
      return BBRC_CMD_NEW_AWAY;
   }
   for ( itemIndex = 0; itemIndex < sizeof( aryCommands ) / sizeof( aryCommands[0] ); itemIndex++ )
   {
      if ( !strncmp( ptrLine, aryCommands[itemIndex].ptrPrefix,
                     aryCommands[itemIndex].prefixLength ) )
      {
         return aryCommands[itemIndex].commandId;
      }
   }
   return BBRC_CMD_UNKNOWN;
}

static void ensureDefaultAwayMessage( void )
{
   if ( !**aryAwayMessageLines )
   {
      snprintf( aryAwayMessageLines[0], sizeof( aryAwayMessageLines[0] ),
                "%s", "I'm away from my keyboard right now." );
      *aryAwayMessageLines[1] = 0;
   }
}

static bool finalizeBbsRcRead( BbsRcReadState *ptrState )
{
   char *ptrNameCopy;
   friend *ptrFriend = NULL;

   applyBbsRcKeyDefaults();
   ensureDefaultAwayMessage();

   defaultColors( 0 );
   warnAboutBbsRcConflicts();

   for ( unsigned int zi = 0; zi < friendList->nitems; zi++ )
   {
      ptrFriend = friendList->items[zi];
      if ( !( ptrNameCopy = (char *)calloc( 1, strlen( ptrFriend->name ) + 1 ) ) )
      {
         fatalExit( "Out of memory for list copy!\r\n", "Fatal error" );
         return false;
      }
      snprintf( ptrNameCopy, strlen( ptrFriend->name ) + 1, "%s", ptrFriend->name );
      if ( !( slistAddItem( whoList, ptrNameCopy, 1 ) ) )
      {
         fatalExit( "Out of memory adding item in list copy!\r\n", "Fatal error" );
         return false;
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
      snprintf( aryEditor, sizeof( aryEditor ), "%s", DEFAULT_EDITOR_CONFIG_VALUE );
   }
   if ( version != ptrState->tmpVersion )
   {
      if ( ptrState->reads )
      {
         setup( ptrState->tmpVersion );
      }
      else
      {
         setup( -1 );
      }
   }
   else if ( !flagsConfiguration.hasScreenReaderModeSetting )
   {
      promptForScreenReaderModeIfUnset();
      ptrState->shouldRewriteBbsRc = true;
   }
   if ( !flagsConfiguration.hasTitleBarSetting )
   {
      flagsConfiguration.hasTitleBarSetting = 1;
      ptrState->shouldRewriteBbsRc = true;
   }
   if ( !flagsConfiguration.hasNameAutocompleteSetting )
   {
      defaultNameAutocompleteIfUnset();
      ptrState->shouldRewriteBbsRc = true;
   }
   if ( ptrState->shouldShowBrowserMigrationNotice )
   {
      stdPrintf( "IMPORTANT: Your browser preference was removed due to client updates.\n" );
      stdPrintf( "The client now relies on terminal links and the macOS default browser.\n" );
   }
   if ( ptrState->shouldRewriteBbsRc && !isBbsRcReadOnly )
   {
      writeBbsRc();
   }
   if ( isLoginShell )
   {
      setTerm();
      configBbsRc();
      resetTerm();
   }

   return true;
}

static void initializeBbsRcDefaults( void )
{
   int parseIndex;

   version = INT_VERSION;
   commandKey = -1;
   shellKey = -1;
   captureKey = -1;
   suspKey = -1;
   quitKey = -1;
   awayKey = -1;
   browserKey = -1;

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
   flagsConfiguration.shouldUseAnsi = 0;
   flagsConfiguration.shouldUseBold = 0;
   flagsConfiguration.shouldDisableBold = 0;
   flagsConfiguration.isMorePromptActive = 0;
   flagsConfiguration.shouldAutoAnswerAnsiPrompt = 0;
   flagsConfiguration.shouldUseTcpKeepalive = 1;
   flagsConfiguration.shouldEnableClickableUrls = 1;
   flagsConfiguration.shouldEnableTitleBar = 1;
   flagsConfiguration.hasTitleBarSetting = 0;
   flagsConfiguration.isScreenReaderModeEnabled = 0;
   flagsConfiguration.hasScreenReaderModeSetting = 0;
   flagsConfiguration.shouldEnableNameAutocomplete = 1;
   flagsConfiguration.hasNameAutocompleteSetting = 0;

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
}

static void initializeBbsRcLists( void )
{
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
}

static bool isNewAwayMessageCommand( const char *ptrLine )
{
   return ptrLine[0] == 'a' && ptrLine[1] >= '1' &&
          ptrLine[1] <= '5' && ptrLine[2] == ' ';
}

static BbsRcOptionValue parseBooleanSettingValue( const char *ptrLine, size_t prefixLength, const char *ptrSettingName, bool shouldAllowAnyNonZeroValue )
{
   const char *ptrValue;

   if ( strlen( ptrLine ) == prefixLength )
   {
      return BBRC_OPTION_ENABLED;
   }
   if ( ptrLine[prefixLength] != ' ' )
   {
      stdPrintf( "Invalid definition of %s ignored.\n", ptrSettingName );
      return BBRC_OPTION_INVALID;
   }

   ptrValue = ptrLine + prefixLength + 1;
   while ( *ptrValue != '\0' && isspace( (unsigned char)*ptrValue ) )
   {
      ptrValue++;
   }

   if ( *ptrValue == '\0' || *ptrValue == '1' )
   {
      return BBRC_OPTION_ENABLED;
   }
   if ( *ptrValue == '0' )
   {
      return BBRC_OPTION_DISABLED;
   }
   if ( shouldAllowAnyNonZeroValue && isdigit( (unsigned char)*ptrValue ) )
   {
      return (BbsRcOptionValue)( atoi( ptrValue ) != 0 );
   }

   stdPrintf( "Invalid definition of %s ignored.\n", ptrSettingName );
   return BBRC_OPTION_INVALID;
}

static bool parseColorScheme( const char *ptrLine, int *ptrColorValues )
{
   if ( strlen( ptrLine ) == 6 + COLOR_FIELD_COUNT )
   {
      int colorIndex;

      for ( colorIndex = 0; colorIndex < COLOR_FIELD_COUNT; colorIndex++ )
      {
         ptrColorValues[colorIndex] = colorValueFromLegacyDigit( ptrLine[6 + colorIndex] );
      }
      if ( ptrLine[6 + COLOR_BACKGROUND_INDEX] == '9' )
      {
         ptrColorValues[COLOR_BACKGROUND_INDEX] = COLOR_VALUE_DEFAULT;
      }
      return true;
   }

   return parseNamedColorScheme( ptrLine + 6, ptrColorValues );
}

static bool parseNamedColorScheme( const char *ptrColorSpec, int *ptrColorValues )
{
   int colorIndex;

   colorIndex = 0;
   while ( *ptrColorSpec != '\0' )
   {
      char aryToken[16];
      int colorValue;
      size_t tokenLength;
      const char *ptrTokenStart;

      while ( isspace( (unsigned char)*ptrColorSpec ) )
      {
         ptrColorSpec++;
      }
      if ( *ptrColorSpec == '\0' )
      {
         break;
      }
      if ( colorIndex >= COLOR_FIELD_COUNT )
      {
         return false;
      }

      ptrTokenStart = ptrColorSpec;
      while ( *ptrColorSpec != '\0' && !isspace( (unsigned char)*ptrColorSpec ) )
      {
         ptrColorSpec++;
      }
      tokenLength = (size_t)( ptrColorSpec - ptrTokenStart );
      if ( tokenLength == 0 || tokenLength >= sizeof( aryToken ) )
      {
         return false;
      }

      memcpy( aryToken, ptrTokenStart, tokenLength );
      aryToken[tokenLength] = '\0';

      if ( isdigit( (unsigned char)aryToken[0] ) )
      {
         size_t digitIndex;

         colorValue = 0;
         for ( digitIndex = 0; digitIndex < tokenLength; digitIndex++ )
         {
            if ( !isdigit( (unsigned char)aryToken[digitIndex] ) )
            {
               return false;
            }
            colorValue = colorValue * 10 + ( aryToken[digitIndex] - '0' );
         }
      }
      else
      {
         colorValue = colorValueFromName( aryToken );
      }
      if ( colorValue < 0 )
      {
         return false;
      }

      ptrColorValues[colorIndex++] = colorValue;
   }

   return colorIndex == COLOR_FIELD_COUNT;
}

static bool processBbsRcCommandLine( const char *ptrLine, BbsRcReadState *ptrState )
{
   char aryScratchLine[MAX_LINE_LENGTH + 1];
   int parseIndex;
   const char *ptrToken;
   char *ptrMacroWrite, *ptrNameCopy;
   BbsRcOptionValue optionValue;
   BbsRcCommandId commandId = detectBbsRcCommand( ptrLine );
   bool isHandled = true;

   switch ( commandId )
   {
      case BBRC_CMD_REREAD:
      case BBRC_CMD_XWRAP:
         break;

      case BBRC_CMD_BOLD:
         flagsConfiguration.shouldUseBold = 1;
         break;

      case BBRC_CMD_XLAND:
         isXland = 0;
         break;

      case BBRC_CMD_VERSION:
         ptrState->tmpVersion = atoi( ptrLine + 8 );
         break;

      case BBRC_CMD_SQUELCH:
         switch ( atoi( ptrLine + 8 ) )
         {
            case 1:
               flagsConfiguration.shouldSquelchExpress = 1;
               break;
            case 2:
               flagsConfiguration.shouldSquelchPost = 1;
               break;
            case 3:
               flagsConfiguration.shouldSquelchPost = 1;
               flagsConfiguration.shouldSquelchExpress = 1;
               break;
            default:
               break;
         }
         break;

      case BBRC_CMD_TCP_KEEPALIVE:
         optionValue = parseBooleanSettingValue( ptrLine, 9, "'keepalive'", true );
         if ( optionValue != BBRC_OPTION_INVALID )
         {
            flagsConfiguration.shouldUseTcpKeepalive = (unsigned int)optionValue;
         }
         break;

      case BBRC_CMD_CLICKABLE_URLS:
         optionValue = parseBooleanSettingValue( ptrLine, strlen( "clickableurls" ),
                                                 "clickable URL option", false );
         if ( optionValue != BBRC_OPTION_INVALID )
         {
            flagsConfiguration.shouldEnableClickableUrls = (unsigned int)optionValue;
         }
         break;

      case BBRC_CMD_TITLEBAR:
         optionValue = parseBooleanSettingValue( ptrLine, strlen( "titlebar" ),
                                                 "title bar option", false );
         if ( optionValue != BBRC_OPTION_INVALID )
         {
            flagsConfiguration.shouldEnableTitleBar = (unsigned int)optionValue;
            flagsConfiguration.hasTitleBarSetting = 1;
         }
         break;

      case BBRC_CMD_SCREENREADER:
         optionValue = parseBooleanSettingValue( ptrLine, strlen( "screenreader" ),
                                                 "screen reader option", false );
         if ( optionValue != BBRC_OPTION_INVALID )
         {
            flagsConfiguration.isScreenReaderModeEnabled = (unsigned int)optionValue;
            flagsConfiguration.hasScreenReaderModeSetting = 1;
         }
         break;

      case BBRC_CMD_AUTOCOMPLETE:
         optionValue = parseBooleanSettingValue( ptrLine, strlen( "autocomplete" ),
                                                 "autocomplete option", false );
         if ( optionValue != BBRC_OPTION_INVALID )
         {
            flagsConfiguration.shouldEnableNameAutocomplete = (unsigned int)optionValue;
            flagsConfiguration.hasNameAutocompleteSetting = 1;
         }
         break;

      case BBRC_CMD_COLOR:
         {
            int *ptrColorValues;

            ptrColorValues = (int *)&color;
            if ( !parseColorScheme( ptrLine, ptrColorValues ) )
            {
               stdPrintf( "Invalid 'color' scheme on line %d, ignored.\n",
                          ptrState->lineNumber );
            }
         }
         break;

      case BBRC_CMD_AUTONAME:
         if ( strncmp( ptrLine + ( sizeof( "aryAutoName " ) - 1 ), "Guest", 5 ) )
         {
            strncpy( aryAutoName, ptrLine + ( sizeof( "aryAutoName " ) - 1 ), 21 );
            aryAutoName[20] = 0;
         }
         break;

      case BBRC_CMD_AUTOANSI:
         if ( strlen( ptrLine ) <= 9 || ptrLine[9] != 'N' )
         {
            flagsConfiguration.shouldAutoAnswerAnsiPrompt = 1;
         }
         break;

#ifdef ENABLE_SAVE_PASSWORD
      case BBRC_CMD_AUTOPASS:
         strncpy( aryAutoPassword, ptrLine + 9, 21 );
         aryAutoPassword[20] = 0;
         break;
#endif
      case BBRC_CMD_BROWSER:
         ptrState->shouldShowBrowserMigrationNotice = true;
         ptrState->shouldRewriteBbsRc = true;
         break;

      case BBRC_CMD_EDITOR:
         if ( *aryEditor )
         {
            stdPrintf( "Multiple definition of 'aryEditor' ignored.\n" );
         }
         else
         {
            strncpy( aryEditor, ptrLine + ( sizeof( "aryEditor " ) - 1 ), 72 );
         }
         break;

      case BBRC_CMD_SITE:
         if ( *aryBbsHost )
         {
            stdPrintf( "Multiple definition of 'site' ignored.\n" );
         }
         else
         {
            for ( parseIndex = 5; ( aryBbsHost[parseIndex - 5] = ptrLine[parseIndex] ) &&
                                  ptrLine[parseIndex] != ' ' && parseIndex < 68; parseIndex++ )
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
               if ( ptrLine[parseIndex] )
               {
                  bbsPort = (unsigned short)atoi( ptrLine + parseIndex + 1 );
               }
               else
               {
                  bbsPort = BBS_PORT_NUMBER;
               }
               for ( ; ptrLine[parseIndex] && ptrLine[parseIndex] != ' '; parseIndex++ )
               {
                  ;
               }
               if ( !strncmp( ptrLine + parseIndex, "secure", 6 ) )
               {
                  shouldUseSsl = 1;
               }
            }
            if ( !strcmp( aryBbsHost, "206.217.131.27" ) )
            {
               snprintf( aryBbsHost, sizeof( aryBbsHost ), "%s", BBS_HOSTNAME );
            }
         }
         break;

      case BBRC_CMD_FRIEND:
         if ( bbsFriends && fgets( aryScratchLine, MAX_LINE_LENGTH + 1, bbsFriends ) )
         {
            break;
         }
         if ( !addFriendFromLine( ptrLine ) )
         {
            return false;
         }
         break;

      case BBRC_CMD_ENEMY:
         if ( strlen( ptrLine ) == 6 )
         {
            stdPrintf( "Empty username in 'enemy'.\n" );
         }
         else if ( slistFind( enemyList, (void *)( ptrLine + 6 ), strCompareVoid ) != -1 )
         {
            stdPrintf( "Duplicate username in 'enemy'.\n" );
         }
         else
         {
            ptrNameCopy = (char *)calloc( 1, strlen( ptrLine + 6 ) + 1 );
            if ( !ptrNameCopy )
            {
               fatalExit( "Out of memory adding 'enemy'!\n", "Fatal error" );
               return false;
            }
            snprintf( ptrNameCopy, strlen( ptrLine + 6 ) + 1, "%s", ptrLine + 6 );
            if ( !slistAddItem( enemyList, (void *)ptrNameCopy, 1 ) )
            {
               fatalExit( "Can't add 'enemy' to list!\n", "Fatal error" );
               return false;
            }
         }
         break;

      case BBRC_CMD_COMMANDKEY:
         if ( commandKey >= 0 )
         {
            stdPrintf( "Additional definition for 'commandkey' ignored.\n" );
         }
         else
         {
            if ( !strncmp( ptrLine, "macrokey ", 9 ) )
            {
               commandKey = ctrl( ptrLine + 9 );
            }
            else
            {
               commandKey = ctrl( ptrLine + 11 );
            }
            if ( findChar( "\0x01\0x03\0x04\0x05\b\n\r\0x11\0x13\0x15\0x17\0x18\0x19\0x1a\0x7f",
                           commandKey ) || commandKey >= ' ' )
            {
               stdPrintf( "Illegal value for 'commandkey', using default of 'Esc'.\n" );
               commandKey = 0x1b;
            }
         }
         break;

      case BBRC_CMD_AWAYKEY:
         if ( awayKey >= 0 )
         {
            stdPrintf( "Additional definition for 'awaykey' ignored.\n" );
         }
         else
         {
            awayKey = ctrl( ptrLine + 8 );
         }
         break;

      case BBRC_CMD_QUIT:
         if ( quitKey >= 0 )
         {
            stdPrintf( "Additional definition for 'quit' ignored.\n" );
         }
         else
         {
            quitKey = ctrl( ptrLine + 5 );
         }
         break;

      case BBRC_CMD_SUSP:
         if ( suspKey >= 0 )
         {
            stdPrintf( "Additional definition for 'susp' ignored.\n" );
         }
         else
         {
            suspKey = ctrl( ptrLine + 5 );
         }
         break;

      case BBRC_CMD_CAPTURE:
         if ( captureKey >= 0 )
         {
            stdPrintf( "Additional definition for 'capture' ignored.\n" );
         }
         else
         {
            captureKey = ctrl( ptrLine + 8 );
         }
         break;

      case BBRC_CMD_KEYMAP:
         parseIndex = *( ptrLine + ( sizeof( "aryKeyMap " ) - 1 ) );
         ptrToken = ptrLine + sizeof( "aryKeyMap " );
         if ( *ptrToken++ == ' ' && parseIndex > 32 && parseIndex < 128 )
         {
            aryKeyMap[parseIndex] = *ptrToken;
         }
         else
         {
            stdPrintf( "Invalid value for 'aryKeyMap' ignored.\n" );
         }
         break;

      case BBRC_CMD_URL:
         if ( browserKey >= 0 )
         {
            stdPrintf( "Additional definition for 'url' ignored.\n" );
         }
         else
         {
            browserKey = ctrl( ptrLine + 4 );
         }
         break;

      case BBRC_CMD_MACRO:
         parseIndex = ctrl( ptrLine + ( sizeof( "aryMacro " ) - 1 ) );
         ptrToken = ptrLine + sizeof( "aryMacro " ) +
                    ( ptrLine[sizeof( "aryMacro " ) - 1] == '^' );
         if ( *ptrToken++ == ' ' )
         {
            if ( *aryMacro[parseIndex] )
            {
               stdPrintf( "Additional definition of same 'aryMacro' value ignored.\n" );
            }
            else if ( parseIndex == 'i' && !awayKey && ptrState->tmpVersion < 220 )
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
                  else if ( !iscntrl( parseIndex ) )
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
         else
         {
            stdPrintf( "Syntax error in 'aryMacro', ignored.\n" );
         }
         break;

      case BBRC_CMD_OLD_AWAY:
         {
            int messageLineIndex = 0;

            ptrMacroWrite = aryAwayMessageLines[messageLineIndex];
            ptrToken = ptrLine + ( sizeof( "aryAwayMessageLines " ) - 1 );
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
               else if ( !iscntrl( parseIndex ) )
               {
                  *ptrMacroWrite++ = (char)parseIndex;
               }
            }
            break;
         }

      case BBRC_CMD_NEW_AWAY:
         snprintf( aryAwayMessageLines[ptrLine[1] - '1'],
                   sizeof( aryAwayMessageLines[0] ), "%s", ptrLine + 3 );
         break;

      case BBRC_CMD_SHELL:
         if ( shellKey >= 0 )
         {
            stdPrintf( "Additional definition for 'shellkey' ignored.\n" );
         }
         else
         {
            if ( !strncmp( ptrLine, "shellkey ", 9 ) )
            {
               shellKey = ctrl( ptrLine + 9 );
            }
            else
            {
               shellKey = ctrl( ptrLine + ( sizeof( "aryShell " ) - 1 ) );
            }
         }
         break;

      case BBRC_CMD_UNKNOWN:
      default:
         isHandled = false;
         break;
   }

   if ( !isHandled && *ptrLine != '#' && *ptrLine &&
        strncmp( ptrLine, "friend ", FRIEND_COMMAND_PREFIX_LEN ) )
   {
      stdPrintf( "Syntax error in .bbsrc file in line %d.\n", ptrState->lineNumber );
   }

   return true;
}

static bool readLegacyBbsFriends( char *ptrLine, BbsRcReadState *ptrState )
{
   if ( bbsFriends )
   {
      rewind( bbsFriends );
   }
   while ( readNormalizedLine( bbsFriends, ptrLine, sizeof( char[MAX_LINE_LENGTH + 1] ),
                               &ptrState->lineNumber, &ptrState->reads, ".bbsfriends" ) )
   {
      if ( !strncmp( ptrLine, "friend ", FRIEND_COMMAND_PREFIX_LEN ) )
      {
         if ( !addFriendFromLine( ptrLine ) )
         {
            return false;
         }
      }
   }

   return true;
}

static void warnAboutBbsRcConflicts( void )
{
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
      stdPrintf( "Warning: duplicate definition of 'aryMacro' and 'shellkey'\n" );
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
      stdPrintf( "Warning: duplicate definition of 'quit' and 'shellkey'\n" );
   }
   if ( suspKey >= 0 && suspKey == captureKey )
   {
      stdPrintf( "Warning: duplicate definition of 'susp' and 'capture'\n" );
   }
   if ( suspKey >= 0 && suspKey == shellKey )
   {
      stdPrintf( "Warning: duplicate definition of 'susp' and 'shellkey'\n" );
   }
   if ( captureKey >= 0 && captureKey == shellKey )
   {
      stdPrintf( "Warning: duplicate definition of 'capture' and 'shellkey'\n" );
   }
}

/*
 * Parses the bbsrc file and applies the configured settings.
 */
void readBbsRc( void )
{
   char aryLine[MAX_LINE_LENGTH + 1];
   BbsRcReadState state = { 0 };

   initializeBbsRcLists();
   initializeBbsRcDefaults();

   while ( readNormalizedLine( ptrBbsRc, aryLine, sizeof( aryLine ),
                               &state.lineNumber, &state.reads, ".bbsrc" ) )
   {
      if ( !processBbsRcCommandLine( aryLine, &state ) )
      {
         return;
      }
   }

   if ( !readLegacyBbsFriends( aryLine, &state ) )
   {
      return;
   }

   finalizeBbsRcRead( &state );
}
