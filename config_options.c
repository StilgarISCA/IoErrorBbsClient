/*
 * Copyright (C) 2024-2026 Stilgar
 * Copyright (C) 1995-2003 Michael Hampton
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

/*
 * This file handles setup defaults and options-menu configuration.
 */
#include "client.h"
#include "client_globals.h"
#include "config_globals.h"
#include "config_menu.h"
#include "defs.h"
#include "getline_input.h"
#include "utility.h"
#define SCREEN_READER_INFO \
   "Screen reader friendly mode keeps this client easier for VoiceOver and other\r\nscreen readers to follow.  You can change this later from the Options menu."

static void setKeyDefaultToUppercase( int lowerKey,
                                      bool shouldUseUppercaseByDefault );

/// @brief Run the general options submenu and update in-memory settings.
///
/// @return This function does not return a value.
void configureOptionsMenu( void )
{
   char aryMenuLine[80];

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
      stdPrintf( "Enter local editor to use (%s uses shell default) -> ",
                 aryEditor );
      getString( 72, aryMenuLine, -999 );
      if ( *aryMenuLine )
      {
         snprintf( aryEditor, sizeof( aryEditor ), "%s", aryMenuLine );
      }
   }
   stdPrintf( "Show long who list by default? (%s) -> ",
              ( aryKeyMap['w'] == 'w' ) ? "No" : "Yes" );
   setKeyDefaultToUppercase( 'w',
                             yesNoDefault( ( aryKeyMap['w'] != 'w' ) ? 1 : 0 ) );
   stdPrintf( "Show full profile by default? (%s) -> ",
              ( aryKeyMap['p'] == 'p' ) ? "No" : "Yes" );
   setKeyDefaultToUppercase( 'p',
                             yesNoDefault( ( aryKeyMap['p'] != 'p' ) ? 1 : 0 ) );
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
   if ( !bbsPort || bbsPort == SSL_PORT_NUMBER || bbsPort == BBS_PORT_NUMBER )
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
      bbsPort = BBS_PORT_NUMBER;
   }
   stdPrintf( "Try to keep idle connections alive with TCP probes? (%s) -> ",
              flagsConfiguration.shouldUseTcpKeepalive ? "Yes" : "No" );
   flagsConfiguration.shouldUseTcpKeepalive =
      (unsigned int)yesNoDefault( flagsConfiguration.shouldUseTcpKeepalive );
   flagsConfiguration.hasTitleBarSetting = 1;
   stdPrintf( "Update terminal title bar? (%s) -> ",
              flagsConfiguration.shouldEnableTitleBar ? "Yes" : "No" );
   flagsConfiguration.shouldEnableTitleBar =
      (unsigned int)yesNoDefault( flagsConfiguration.shouldEnableTitleBar );
   stdPrintf( "Append OSC 8 URL summaries to posts & mail? (%s) -> ",
              flagsConfiguration.shouldEnableClickableUrls ? "Yes" : "No" );
   flagsConfiguration.shouldEnableClickableUrls =
      (unsigned int)yesNoDefault( flagsConfiguration.shouldEnableClickableUrls );
   stdPrintf( "Autocomplete username in recipient prompts? (%s) -> ",
              flagsConfiguration.shouldEnableNameAutocomplete ? "Yes" : "No" );
   flagsConfiguration.shouldEnableNameAutocomplete =
      (unsigned int)yesNoDefault( flagsConfiguration.shouldEnableNameAutocomplete );
   flagsConfiguration.hasNameAutocompleteSetting = 1;
}

/// @brief Set the default autocomplete behavior when the user has not chosen one yet.
///
/// Screen reader mode disables this feature by default.
///
/// @return This function does not return a value.
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

/// @brief Prompt for screen reader mode when no explicit setting has been saved.
///
/// @return This function does not return a value.
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

/// @brief Flip a two-key command mapping between lowercase and uppercase defaults.
///
/// @param lowerKey Lowercase command character in the key map.
/// @param shouldUseUppercaseByDefault Non-zero to map the lowercase key to the
/// uppercase action by default, zero to restore the normal lowercase default.
///
/// @return This function does not return a value.
static void setKeyDefaultToUppercase( int lowerKey,
                                      bool shouldUseUppercaseByDefault )
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
