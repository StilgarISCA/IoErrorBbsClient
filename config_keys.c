/*
 * Copyright (C) 2024-2026 Stilgar
 * Copyright (C) 1995-2003 Michael Hampton
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

/*
 * This file handles hotkey and macro configuration from the client
 * configuration menu.
 */
#include "client_globals.h"
#include "color.h"
#include "config_globals.h"
#include "config_menu.h"
#include "defs.h"
#include "utility.h"
static const char *CONFIG_MACRO_MENU_KEYS = "elq \n";

static void editConfiguredMacros( void );
static void listConfiguredMacros( void );

/// @brief Configure the client's hotkeys from the options menu.
///
/// @return This function does not return a value.
void configureHotkeys( void )
{
   stdPrintf( "Hotkeys\r\n\n" );
   stdPrintf( "Enter command key (%s) -> ", strCtrl( commandKey ) );
   while ( true )
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
      stdPrintf( "Enter key to start a new shell (%s) -> ", strCtrl( shellKey ) );
      stdPrintf( "%s\r\n", strCtrl( shellKey = newKey( shellKey ) ) );
   }
   stdPrintf( "Enter key to toggle capture mode (%s) -> ", strCtrl( captureKey ) );
   stdPrintf( "%s\r\n", strCtrl( captureKey = newKey( captureKey ) ) );
   stdPrintf( "Enter key to enable away from keyboard (%s) -> ", strCtrl( awayKey ) );
   stdPrintf( "%s\r\n", strCtrl( awayKey = newKey( awayKey ) ) );
   stdPrintf( "Enter key to browse a website (%s) -> ", strCtrl( browserKey ) );
   stdPrintf( "%s\r\n", strCtrl( browserKey = newKey( browserKey ) ) );
}

/// @brief Run the macro configuration submenu.
///
/// @return This function does not return a value.
void configureMacros( void )
{
   int inputChar;

   stdPrintf( "Macros\r\n" );
   for ( inputChar = 0; inputChar != 'q'; )
   {
      printThemedMnemonicText( "\r\n<E>dit  <L>ist  <Q>uit", color.number );
      printThemedMnemonicText( "\r\nMacro config -> ", color.forum );
      printAnsiForegroundColorValue( color.text );
      inputChar = readValidatedMenuKey( CONFIG_MACRO_MENU_KEYS );
      switch ( inputChar )
      {
         case 'e':
            editConfiguredMacros();
            break;

         case 'l':
            listConfiguredMacros();
            break;

         case 'q':
         case ' ':
         case '\n':
            stdPrintf( "Quit\r\n" );
            inputChar = 'q';
            break;
      }
   }
}

/// @brief Prompt for a macro key and edit macros until the command key ends the loop.
///
/// @return This function does not return a value.
static void editConfiguredMacros( void )
{
   stdPrintf( "Edit\r\n" );
   while ( true )
   {
      int inputChar;

      stdPrintf( "\r\nMacro to edit (%s to end) -> ", strCtrl( commandKey ) );
      inputChar = newKey( -1 );
      if ( inputChar == commandKey || inputChar == ' ' || inputChar == '\n' ||
           inputChar == '\r' )
      {
         break;
      }
      stdPrintf( "%s\r\n", strCtrl( inputChar ) );
      newMacro( inputChar );
   }
   stdPrintf( "Done\r\n" );
}

/// @brief Display the currently configured macros.
///
/// Output is paged through the normal `more()` prompt.
///
/// @return This function does not return a value.
static void listConfiguredMacros( void )
{
   int innerIndex;
   int itemIndex;
   int lines;

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
}

/// @brief Read a replacement hotkey while avoiding collisions with other hotkeys.
///
/// The raw key is read with `getKey()` so terminal translations do not change
/// the configured value.
///
/// @param oldkey Existing hotkey value, or `-1` to skip conflict checks against
/// the current key being replaced.
///
/// @return The accepted replacement key, or `oldkey` if the user keeps the
/// current binding.
int newKey( int oldkey )
{
   while ( true )
   {
      int inputChar;

      inputChar = getKey();
      if ( ( ( inputChar == ' ' || inputChar == '\n' || inputChar == '\r' ) &&
             oldkey >= 0 ) ||
           inputChar == oldkey )
      {
         return oldkey;
      }
      if ( oldkey >= 0 &&
           ( inputChar == commandKey || inputChar == suspKey ||
             inputChar == quitKey || inputChar == shellKey ||
             inputChar == captureKey || inputChar == awayKey ||
             inputChar == browserKey ) )
      {
         stdPrintf( "\r\nThat key is already in use for another hotkey, try again -> " );
      }
      else
      {
         return inputChar;
      }
   }
}

/// @brief Edit the macro bound to a specific key.
///
/// @param which Key code whose macro should be updated.
///
/// @return This function does not return a value.
void newMacro( int which )
{
   register int itemIndex;

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
      stdPrintf( "\r\nNo current aryMacro for '%s'.\r\nDo you want to make one? (Y/N) -> ",
                 strCtrl( which ) );
   }
   if ( !yesNo() )
   {
      return;
   }
   stdPrintf( "\r\nEnter new aryMacro (use %s to end)\r\n -> ",
              strCtrl( commandKey ) );
   for ( itemIndex = 0;; itemIndex++ )
   {
      register int inputChar;

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
         {
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

/// @brief Format a key value in printable control-key notation.
///
/// @param inputChar Key code to format.
///
/// @return A pointer to a static buffer containing either the literal printable
/// character or a caret-style control representation.
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
   return aryControlText;
}
