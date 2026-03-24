/*
 * Copyright (C) 2024-2026 Stilgar
 * Copyright (C) 1995-2003 Michael Hampton
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "defs.h"
#include "ext.h"

static const char *COLOR_MAIN_MENU_KEYS = "pgisoxq \n";
static const char *COLOR_GENERAL_MENU_KEYS = "befntq \n";
static const char *COLOR_INPUT_MENU_KEYS = "ctq \n";
static const char *COLOR_POST_MENU_KEYS = "dntq \n";
static const char *COLOR_EXPRESS_MENU_KEYS = "ntq \n";
static const char *COLOR_RESET_MENU_KEYS = "dbchq \n";
static const char *COLOR_USER_OR_FRIEND_KEYS = "ufq \n";
static const char *COLOR_FOREGROUND_KEYS = "krgybmcw12345678";
static const char *COLOR_BACKGROUND_KEYS = "krgybmcwd12345678";

typedef struct
{
   const char *ptrName;
   int colorValue;
} NamedColorSpec;

typedef struct
{
   int keyChar;
   int colorValue;
   const char *ptrDisplayName;
} PickerColorOption;

typedef struct
{
   int keyChar;
   const char *ptrLabel;
   int textColor;
   int accentColor;
   int backgroundColor;
} PresetMenuOption;

static void printAnsiDisplayState( int foregroundColor, int backgroundColor );
static int transformIncomingAnsiColor( int inputChar );

static const NamedColorSpec aryNamedColors[] =
   {
      { "brightblack", 8 },
      { "brightred", 9 },
      { "brightgreen", 10 },
      { "brightyellow", 11 },
      { "brightblue", 12 },
      { "brightmagenta", 13 },
      { "brightpurple", 13 },
      { "brightcyan", 14 },
      { "brightwhite", 15 },
      { "black", 16 },
      { "red", 160 },
      { "green", 34 },
      { "yellow", 220 },
      { "blue", 26 },
      { "magenta", 91 },
      { "purple", 91 },
      { "cyan", 44 },
      { "white", 231 },
      { "default", COLOR_VALUE_DEFAULT } };

static const PickerColorOption aryForegroundPickerOptions[] =
   {
      { 'k', 0, "Black" },
      { 'r', 1, "Red" },
      { 'g', 2, "Green" },
      { 'y', 3, "Yellow" },
      { 'b', 4, "Blue" },
      { 'm', 5, "Magenta" },
      { 'c', 6, "Cyan" },
      { 'w', 7, "White" },
      { '1', 8, "Bright black" },
      { '2', 9, "Bright red" },
      { '3', 10, "Bright green" },
      { '4', 11, "Bright yellow" },
      { '5', 12, "Bright blue" },
      { '6', 13, "Bright magenta" },
      { '7', 14, "Bright cyan" },
      { '8', 15, "Bright white" } };

static const PickerColorOption aryBackgroundPickerOptions[] =
   {
      { 'k', 0, "Black" },
      { 'r', 1, "Red" },
      { 'g', 2, "Green" },
      { 'y', 3, "Yellow" },
      { 'b', 4, "Blue" },
      { 'm', 5, "Magenta" },
      { 'c', 6, "Cyan" },
      { 'w', 7, "White" },
      { '1', 8, "Bright black" },
      { '2', 9, "Bright red" },
      { '3', 10, "Bright green" },
      { '4', 11, "Bright yellow" },
      { '5', 12, "Bright blue" },
      { '6', 13, "Bright magenta" },
      { '7', 14, "Bright cyan" },
      { '8', 15, "Bright white" },
      { 'd', COLOR_VALUE_DEFAULT, "Default" } };

static const PresetMenuOption aryPresetMenuOptions[] =
   {
      { 'd', "Default", 2, 3, 0 },
      { 'b', "Brilliant", 10, 11, 0 },
      { 'c', "Colorblind", 231, 75, 16 },
      { 'h', "Hotdog stand", 220, 196, 16 } };

static bool isColorNameMatch( const char *ptrLeft, const char *ptrRight )
{
   while ( *ptrLeft && *ptrRight )
   {
      if ( tolower( (unsigned char)*ptrLeft ) != tolower( (unsigned char)*ptrRight ) )
      {
         return false;
      }
      ptrLeft++;
      ptrRight++;
   }

   return *ptrLeft == '\0' && *ptrRight == '\0';
}

static void printPresetMenuItem( const PresetMenuOption *ptrOption )
{
   stdPrintf( " " );
   printAnsiDisplayState( ptrOption->textColor, ptrOption->backgroundColor );
   printAnsiForegroundColorValue( ptrOption->accentColor );
   stdPrintf( "%c", toupper( ptrOption->keyChar ) );
   printAnsiForegroundColorValue( ptrOption->textColor );
   stdPrintf( "%s\r\n", ptrOption->ptrLabel + 1 );
}

int colorValueFromLegacyDigit( int inputChar )
{
   if ( inputChar >= '0' && inputChar <= '9' )
   {
      return inputChar - '0';
   }

   return inputChar;
}

int colorValueToLegacyDigit( int colorValue )
{
   return colorValue + '0';
}

int formatTransformedAnsiForegroundSequence( char *ptrBuffer, size_t bufferSize,
                                             int inputChar, int isPostContext,
                                             int isFriend )
{
   int transformedColor;

   if ( isPostContext )
   {
      transformedColor = ansiTransformPost( inputChar, isFriend );
   }
   else
   {
      transformedColor = ansiTransform( inputChar );
   }

   lastColor = transformedColor;
   return formatAnsiForegroundSequence( ptrBuffer, bufferSize, transformedColor );
}

int colorValueFromName( const char *ptrColorName )
{
   size_t itemIndex;

   if ( ptrColorName == NULL || *ptrColorName == '\0' )
   {
      return -1;
   }

   for ( itemIndex = 0; itemIndex < sizeof( aryNamedColors ) / sizeof( aryNamedColors[0] ); itemIndex++ )
   {
      if ( isColorNameMatch( ptrColorName, aryNamedColors[itemIndex].ptrName ) )
      {
         return aryNamedColors[itemIndex].colorValue;
      }
   }

   return -1;
}

const char *colorNameFromValue( int colorValue )
{
   size_t itemIndex;

   for ( itemIndex = 0; itemIndex < sizeof( aryNamedColors ) / sizeof( aryNamedColors[0] ); itemIndex++ )
   {
      if ( aryNamedColors[itemIndex].colorValue == colorValue )
      {
         return aryNamedColors[itemIndex].ptrName;
      }
   }

   return NULL;
}

static const PickerColorOption *findPickerColorOption( const PickerColorOption *ptrOptions,
                                                       size_t itemCount,
                                                       int keyChar )
{
   size_t itemIndex;

   for ( itemIndex = 0; itemIndex < itemCount; itemIndex++ )
   {
      if ( ptrOptions[itemIndex].keyChar == keyChar )
      {
         return &ptrOptions[itemIndex];
      }
   }

   return NULL;
}

static void printForegroundPickerMenu( void )
{
   printThemedMnemonicText( "\r\n[<K>] Black           [<R>] Red             [<G>] Green           [<Y>] Yellow\r\n", color.number );
   printThemedMnemonicText( "[<B>] Blue            [<M>] Magenta         [<C>] Cyan            [<W>] White\r\n", color.number );
   printThemedMnemonicText( "[<1>] Bright black   [<2>] Bright red      [<3>] Bright green    [<4>] Bright yellow\r\n", color.number );
   printThemedMnemonicText( "[<5>] Bright blue    [<6>] Bright magenta  [<7>] Bright cyan     [<8>] Bright white\r\n", color.number );
   printThemedMnemonicText( "Select color -> ", color.forum );
   printAnsiForegroundColorValue( color.text );
}

static void printBackgroundPickerMenu( void )
{
   printThemedMnemonicText( "\r\n[<K>] Black           [<R>] Red             [<G>] Green           [<Y>] Yellow\r\n", color.number );
   printThemedMnemonicText( "[<B>] Blue            [<M>] Magenta         [<C>] Cyan            [<W>] White\r\n", color.number );
   printThemedMnemonicText( "[<1>] Bright black   [<2>] Bright red      [<3>] Bright green    [<4>] Bright yellow\r\n", color.number );
   printThemedMnemonicText( "[<5>] Bright blue    [<6>] Bright magenta  [<7>] Bright cyan     [<8>] Bright white\r\n", color.number );
   printThemedMnemonicText( "[<D>] Default\r\n", color.number );
   printThemedMnemonicText( "Select background -> ", color.forum );
   printAnsiForegroundColorValue( color.text );
}

static void presetColorConfig( void )
{
   size_t optionIndex;

   stdPrintf( "Color presets\r\n\n" );
   for ( optionIndex = 0;
         optionIndex < sizeof( aryPresetMenuOptions ) / sizeof( aryPresetMenuOptions[0] );
         optionIndex++ )
   {
      printPresetMenuItem( &aryPresetMenuOptions[optionIndex] );
   }
   printAnsiDisplayState( color.text, color.background );
   printThemedMnemonicText( " Quit\r\n", color.number );
   printThemedMnemonicText( "Select preset -> ", color.forum );
   printAnsiForegroundColorValue( color.text );

   switch ( readValidatedMenuKey( COLOR_RESET_MENU_KEYS ) )
   {
      case 'd':
         stdPrintf( "Default\r\n" );
         defaultColors( 1 );
         break;

      case 'b':
         stdPrintf( "Brilliant\r\n" );
         brilliantColors();
         break;

      case 'c':
         stdPrintf( "Colorblind\r\n" );
         colorblindColors();
         break;

      case 'h':
         stdPrintf( "Hotdog Stand\r\n" );
         hotDogColors();
         break;

      case 'q':
      case ' ':
      case '\n':
         stdPrintf( "Quit\r\n" );
         break;

      default:
         break;
   }
}

static void printAnsiBackgroundColor( int colorValue )
{
   char aryAnsiSequence[32];

   formatAnsiBackgroundSequence( aryAnsiSequence, sizeof( aryAnsiSequence ),
                                 colorValue );
   stdPrintf( "%s", aryAnsiSequence );
}

static void printAnsiDisplayState( int foregroundColor, int backgroundColor )
{
   char aryAnsiSequence[32];

   formatAnsiDisplayStateSequence( aryAnsiSequence, sizeof( aryAnsiSequence ),
                                   foregroundColor, backgroundColor,
                                   flagsConfiguration.shouldUseBold );
   stdPrintf( "%s", aryAnsiSequence );
}

static int transformPostHeaderColor( int inputChar, int isFriend )
{
   switch ( inputChar )
   {
      case '6':
         if ( isFriend )
         {
            return color.postFriendName;
         }
         return color.postName;
      case '5':
         if ( isFriend )
         {
            return color.postFriendDate;
         }
         return color.postDate;
      case '3':
         return color.anonymous;
      case '2':
         if ( isFriend )
         {
            return color.postFriendText;
         }
         return color.postText;
      default:
         return transformIncomingAnsiColor( inputChar );
   }
}

static void printGeneralColorPreview( void )
{
   printAnsiDisplayState( color.forum, color.background );
   stdPrintf( "Lobby> " );
   printAnsiForegroundColorValue( color.text );
   stdPrintf( "Enter message\r\n\n" );
   printAnsiForegroundColorValue( color.errorTextColor );
   stdPrintf( "Only Sysops may post to the lobby\r\n\n" );
   printAnsiForegroundColorValue( color.forum );
   stdPrintf( "Lobby> " );
   printAnsiForegroundColorValue( color.text );
   stdPrintf( "Goto " );
   printAnsiForegroundColorValue( color.forum );
   stdPrintf( "[Babble]  " );
   printAnsiForegroundColorValue( color.number );
   stdPrintf( "150" );
   printAnsiForegroundColorValue( color.text );
   stdPrintf( " messages, " );
   printAnsiForegroundColorValue( color.number );
   stdPrintf( "1" );
   printAnsiForegroundColorValue( color.text );
   stdPrintf( " new\r\n" );
}

static void printPostColorPreview( int dateColor, int textColor,
                                   int nameColor, const char *ptrName )
{
   printAnsiForegroundColorValue( dateColor );
   stdPrintf( "Jan  1, 2000 11:01" );
   printAnsiForegroundColorValue( textColor );
   stdPrintf( " from " );
   printAnsiForegroundColorValue( nameColor );
   stdPrintf( "%s", ptrName );
   printAnsiForegroundColorValue( textColor );
   stdPrintf( "\r\nHi there!\r\n" );
   printAnsiForegroundColorValue( color.forum );
   stdPrintf( "[Lobby> msg #1]\r\n" );
}

static void printExpressColorPreview( int textColor, int nameColor,
                                      const char *ptrName )
{
   printAnsiForegroundColorValue( textColor );
   stdPrintf( "*** Message (#1) from " );
   printAnsiForegroundColorValue( nameColor );
   stdPrintf( "%s", ptrName );
   printAnsiForegroundColorValue( textColor );
   stdPrintf( " at 11:01 ***\r\n>Hi there!\r\n" );
}

static void printInputColorPreview( void )
{
   printAnsiForegroundColorValue( color.text );
   stdPrintf( "Message eXpress\r\nRecipient: " );
   printAnsiForegroundColorValue( color.inputText );
   stdPrintf( "Exam" );
   printAnsiForegroundColorValue( color.inputHighlight );
   stdPrintf( "ple User\r\n" );
   printAnsiForegroundColorValue( color.inputText );
   stdPrintf( ">Hi there!\r\n" );
   printAnsiForegroundColorValue( color.text );
   stdPrintf( "Message received by Example User.\r\n" );
}

int ansiTransform( int inputChar )
{
   int transformedColor;

   transformedColor = transformIncomingAnsiColor( inputChar );

   return transformedColor;
}

void ansiTransformExpress( char *ptrText, size_t size )
{
   char aryTempText[580];
   char aryMessageColor[32];
   char aryNameColor[32];
   char aryResetColor[32];
   char *ptrExpressSender, *ptrExpressMarker;

   /* Insert color only when ANSI is being used */
   if ( !flagsConfiguration.shouldUseAnsi )
   {
      return;
   }

   /* Verify this is an X message and set up pointers */
   ptrExpressSender = findSubstring( ptrText, ") to " );
   ptrExpressMarker = findSubstring( ptrText, ") from " );
   if ( !ptrExpressSender && !ptrExpressMarker )
   {
      return;
   }
   if ( ( ptrExpressMarker && ptrExpressMarker < ptrExpressSender ) || !ptrExpressSender )
   {
      ptrExpressSender = ptrExpressMarker + 2;
   }

   ptrExpressMarker = findSubstring( ptrText, " at " );
   if ( !ptrExpressMarker )
   {
      return;
   }

   ptrExpressSender += 4;
   *( ptrExpressSender++ ) = 0;
   *( ptrExpressMarker++ ) = 0;

   if ( slistFind( friendList, ptrExpressSender, fStrCompareVoid ) != -1 )
   {
      formatAnsiForegroundSequence( aryMessageColor, sizeof( aryMessageColor ),
                                    color.expressFriendText );
      formatAnsiForegroundSequence( aryNameColor, sizeof( aryNameColor ),
                                    color.expressFriendName );
   }
   else
   {
      formatAnsiForegroundSequence( aryMessageColor, sizeof( aryMessageColor ),
                                    color.expressText );
      formatAnsiForegroundSequence( aryNameColor, sizeof( aryNameColor ),
                                    color.expressName );
   }
   formatAnsiForegroundSequence( aryResetColor, sizeof( aryResetColor ),
                                 color.text );
   snprintf( aryTempText, sizeof( aryTempText ), "%s%s %s%s%s %s%s",
             aryMessageColor, ptrText, aryNameColor, ptrExpressSender,
             aryMessageColor, ptrExpressMarker, aryResetColor );
   lastColor = color.text;
   snprintf( ptrText, size, "%s", aryTempText );
}

int ansiTransformPost( int inputChar, int isFriend )
{
   int transformedColor;

   transformedColor = transformIncomingAnsiColor( inputChar );
   switch ( inputChar )
   {
      case '3':
         transformedColor = color.morePrompt;
         break;
      case '2':
         if ( isFriend )
         {
            transformedColor = color.postFriendText;
         }
         else
         {
            transformedColor = color.postText;
         }
         break;
      case '1':
         transformedColor = color.errorTextColor;
         break;
      default:
         break;
   }
   return transformedColor;
}

static int transformIncomingAnsiColor( int inputChar )
{
   switch ( inputChar )
   {
      case '0':
         return color.ansiBlackTextColor;
      case '1':
         return color.errorTextColor;
      case '2':
         return color.text;
      case '3':
         return color.forum;
      case '4':
         return color.ansiBlueTextColor;
      case '5':
         return color.ansiMagentaTextColor;
      case '6':
         return color.number;
      case '7':
         return color.ansiWhiteTextColor;
      default:
         return colorValueFromLegacyDigit( inputChar );
   }
}

void ansiTransformPostHeader( char *ptrText, size_t bufferSize, int isFriend )
{
   char aryTransformedHeader[320];
   char aryAnsiSequence[32];
   char *ptrScan;
   size_t writeOffset;

   writeOffset = 0;

   /* Rewrite simple ESC[3xm foreground sequences into full palette-aware ANSI. */
   for ( ptrScan = ptrText; *ptrScan != '\0' && writeOffset < sizeof( aryTransformedHeader ) - 1; )
   {
      if ( ptrScan[0] == '\033' && ptrScan[1] == '[' && ptrScan[2] == '3' &&
           ptrScan[3] != '\0' && ptrScan[4] == 'm' )
      {
         lastColor = transformPostHeaderColor( ptrScan[3], isFriend );
         formatAnsiForegroundSequence( aryAnsiSequence, sizeof( aryAnsiSequence ),
                                       lastColor );
         writeOffset += (size_t)snprintf( aryTransformedHeader + writeOffset,
                                          sizeof( aryTransformedHeader ) - writeOffset,
                                          "%s", aryAnsiSequence );
         ptrScan += 5;
         continue;
      }

      aryTransformedHeader[writeOffset++] = *ptrScan++;
   }

   aryTransformedHeader[writeOffset] = '\0';
   snprintf( ptrText, bufferSize, "%s", aryTransformedHeader );
}

void colorConfig( void )
{
   char aryPromptText[110];

   stdPrintf( "Color\r\n" );
   if ( !flagsConfiguration.shouldUseAnsi )
   {
      stdPrintf( "\r\nWARNING:  Color is off.  You will not be able to preview your selections." );
   }
   while ( true )
   {
      snprintf( aryPromptText, sizeof( aryPromptText ), "\r\n<P>resets  <G>eneral  <I>nput  po<S>ts  <O>ptions  <X>press  <Q>uit" );
      printThemedMnemonicText( aryPromptText, color.number );
      printThemedMnemonicText( "\r\nColor config -> ", color.forum );
      printAnsiForegroundColorValue( color.text );

      int inputChar = readValidatedMenuKey( COLOR_MAIN_MENU_KEYS );

      switch ( inputChar )
      {
         case 'g':
            stdPrintf( "General\r\n\n" );
            generalColorConfig();
            break;
         case 'i':
            stdPrintf( "Input\r\n\n" );
            inputColorConfig();
            break;
         case 'o':
            stdPrintf( "Options\r\n\n" );
            colorOptions();
            break;

         case 'p':
            presetColorConfig();
            break;

         case 's':
            stdPrintf( "Post colors\r\n\n" );
            postColorConfig();
            break;

         case 'x':
            stdPrintf( "Express colors\r\n\n" );
            expressColorConfig();
            break;

         case 'q':
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

void colorOptions( void )
{
   stdPrintf( "Automatically answer the ANSI terminal question? (%s) -> ",
              flagsConfiguration.shouldAutoAnswerAnsiPrompt ? "Yes" : "No" );
   flagsConfiguration.shouldAutoAnswerAnsiPrompt = (unsigned int)yesNoDefault( flagsConfiguration.shouldAutoAnswerAnsiPrompt );
   stdPrintf( "Use bold ANSI colors when ANSI is enabled? (%s) -> ",
              flagsConfiguration.shouldUseBold ? "Yes" : "No" );
   flagsConfiguration.shouldUseBold = (unsigned int)yesNoDefault( flagsConfiguration.shouldUseBold );
   if ( flagsConfiguration.shouldUseAnsi )
   {
      printAnsiDisplayState( lastColor, color.background );
   }
}

#define A_USER "Example User"
#define A_FRIEND "Example Friend"

void generalColorConfig( void )
{
   char aryPromptText[100];

   while ( true )
   {
      printGeneralColorPreview();

      snprintf( aryPromptText, sizeof( aryPromptText ), "\r\n<B>ackground  <E>rror  <F>orum  <N>umber  <T>ext  <Q>uit -> " );
      printThemedMnemonicText( aryPromptText, color.number );
      printAnsiForegroundColorValue( color.text );

      int menuOption = readValidatedMenuKey( COLOR_GENERAL_MENU_KEYS );

      switch ( menuOption )
      {
         case 'q':
         case ' ':
         case '\n':
            stdPrintf( "Quit\r\n" );
            return;
            /* NOTREACHED */
         case 'b':
            stdPrintf( "Background\r\n\n" );
            color.background = backgroundPicker();
            break;
         case 'e':
            stdPrintf( "Error\r\n\n" );
            color.errorTextColor = colorPicker();
            break;
         case 'f':
            stdPrintf( "Forum\r\n\n" );
            color.forum = colorPicker();
            break;
         case 'n':
            stdPrintf( "Number\r\n\n" );
            color.number = colorPicker();
            break;
         case 't':
            stdPrintf( "Text\r\n\n" );
            color.text = colorPicker();
            break;
         default:
            break;
      }
   }
}

void inputColorConfig( void )
{
   char aryPromptText[100];

   while ( true )
   {
      printInputColorPreview();

      snprintf( aryPromptText, sizeof( aryPromptText ), "\r\n<T>ext  <C>ompletion  <Q>uit -> " );
      printThemedMnemonicText( aryPromptText, color.number );
      printAnsiForegroundColorValue( color.text );

      int menuOption = readValidatedMenuKey( COLOR_INPUT_MENU_KEYS );

      switch ( menuOption )
      {
         case 'q':
         case ' ':
         case '\n':
            stdPrintf( "Quit\r\n" );
            return;
            /* NOTREACHED */
         case 'c':
            stdPrintf( "Completion\r\n\n" );
            color.inputHighlight = colorPicker();
            break;
         case 't':
            stdPrintf( "Text\r\n\n" );
            color.inputText = colorPicker();
            break;
         default:
            break;
      }
   }
}

void postColorConfig( void )
{
   while ( true )
   {
      switch ( userOrFriend() )
      {
         case 'u':
            postUserColorConfig();
            break;
         case 'f':
            postFriendColorConfig();
            break;
         default:
            return;
      }
   }
}

void postUserColorConfig( void )
{
   while ( true )
   {
      int menuOption;

      printPostColorPreview( color.postDate, color.postText,
                             color.postName, A_USER );
      menuOption = postColorMenu();
      switch ( menuOption )
      {
         case 'q':
         case ' ':
         case '\n':
            return;
            /* NOTREACHED */
         case 'd':
            color.postDate = colorPicker();
            break;
         case 'n':
            color.postName = colorPicker();
            break;
         case 't':
            color.postText = colorPicker();
            break;
         default:
            break;
      }
   }
}

void postFriendColorConfig( void )
{
   while ( true )
   {
      int menuOption;

      printPostColorPreview( color.postFriendDate, color.postFriendText,
                             color.postFriendName, A_FRIEND );
      menuOption = postColorMenu();
      switch ( menuOption )
      {
         case 'q':
         case ' ':
         case '\n':
            return;
            /* NOTREACHED */
         case 'd':
            color.postFriendDate = colorPicker();
            break;
         case 'n':
            color.postFriendName = colorPicker();
            break;
         case 't':
            color.postFriendText = colorPicker();
            break;
         default:
            break;
      }
   }
}

char postColorMenu( void )
{
   int inputChar;
   char aryPromptText[100];

   snprintf( aryPromptText, sizeof( aryPromptText ), "\r\n<D>ate  <N>ame  <T>ext  <Q>uit -> " );
   printThemedMnemonicText( aryPromptText, color.number );
   printAnsiForegroundColorValue( color.text );

   inputChar = readValidatedMenuKey( COLOR_POST_MENU_KEYS );

   switch ( inputChar )
   {
      case 'd':
         stdPrintf( "Date\r\n\n" );
         break;
      case 'n':
         stdPrintf( "Name\r\n\n" );
         break;
      case 't':
         stdPrintf( "Text\r\n\n" );
         break;
      case 'q':
      case ' ':
      case '\n':
         stdPrintf( "Quit\r\n\n" );
         break;
      default:
         break;
   }

   return (char)inputChar;
}

void expressColorConfig( void )
{
   while ( true )
   {
      switch ( userOrFriend() )
      {
         case 'u':
            expressUserColorConfig();
            break;
         case 'f':
            expressFriendColorConfig();
            break;
         default:
            return;
      }
   }
}

void expressUserColorConfig( void )
{
   while ( true )
   {
      int menuOption;

      printExpressColorPreview( color.expressText, color.expressName,
                                A_USER );
      menuOption = expressColorMenu();
      switch ( menuOption )
      {
         case 'q':
         case ' ':
         case '\n':
            return;
            /* NOTREACHED */
         case 'n':
            color.expressName = colorPicker();
            break;
         case 't':
            color.expressText = colorPicker();
            break;
         default:
            break;
      }
   }
}

void expressFriendColorConfig( void )
{
   while ( true )
   {
      int menuOption;

      printExpressColorPreview( color.expressFriendText,
                                color.expressFriendName, A_FRIEND );
      menuOption = expressColorMenu();
      switch ( menuOption )
      {
         case 'q':
         case ' ':
         case '\n':
            return;
            /* NOTREACHED */
         case 'n':
            color.expressFriendName = colorPicker();
            break;
         case 't':
            color.expressFriendText = colorPicker();
            break;
         default:
            break;
      }
   }
}

char expressColorMenu( void )
{
   int inputChar;
   char aryPromptText[100];

   snprintf( aryPromptText, sizeof( aryPromptText ), "\r\n<N>ame  <T>ext  <Q>uit -> " );
   printThemedMnemonicText( aryPromptText, color.number );
   printAnsiForegroundColorValue( color.text );

   inputChar = readValidatedMenuKey( COLOR_EXPRESS_MENU_KEYS );

   switch ( inputChar )
   {
      case 'n':
         stdPrintf( "Name\r\n\n" );
         break;
      case 't':
         stdPrintf( "Text\r\n\n" );
         break;
      case 'q':
      case ' ':
      case '\n':
         stdPrintf( "Quit\r\n\n" );
         break;
      default:
         break;
   }

   return (char)inputChar;
}

char userOrFriend( void )
{
   int inputChar;

   printThemedMnemonicText( "Configure for ", color.text );
   printThemedMnemonicText( "<U>ser", color.number );
   printThemedMnemonicText( " or ", color.text );
   printThemedMnemonicText( "<F>riend", color.number );
   printThemedMnemonicText( " -> ", color.forum );
   printAnsiForegroundColorValue( color.text );

   inputChar = readValidatedMenuKey( COLOR_USER_OR_FRIEND_KEYS );

   switch ( inputChar )
   {
      case 'u':
         stdPrintf( "User\r\n\n" );
         break;
      case 'f':
         stdPrintf( "Friend\r\n\n" );
         break;
      default:
         stdPrintf( "Quit\r\n" );
         break;
   }

   return (char)inputChar;
}

int colorPicker( void )
{
   const PickerColorOption *ptrOption;
   int inputChar;

   printForegroundPickerMenu();

   inputChar = readValidatedMenuKey( COLOR_FOREGROUND_KEYS );
   ptrOption = findPickerColorOption( aryForegroundPickerOptions,
                                      sizeof( aryForegroundPickerOptions ) / sizeof( aryForegroundPickerOptions[0] ),
                                      inputChar );
   if ( ptrOption == NULL )
   {
      return 0;
   }

   stdPrintf( "%s\r\n\n", ptrOption->ptrDisplayName );
   return ptrOption->colorValue;
}

int backgroundPicker( void )
{
   const PickerColorOption *ptrOption;
   int inputChar;

   printBackgroundPickerMenu();

   inputChar = readValidatedMenuKey( COLOR_BACKGROUND_KEYS );
   ptrOption = findPickerColorOption( aryBackgroundPickerOptions,
                                      sizeof( aryBackgroundPickerOptions ) / sizeof( aryBackgroundPickerOptions[0] ),
                                      inputChar );
   if ( ptrOption == NULL )
   {
      return 0;
   }

   stdPrintf( "%s\r\n", ptrOption->ptrDisplayName );
   printAnsiBackgroundColor( ptrOption->colorValue );
   stdPrintf( "\n" );

   return ptrOption->colorValue;
}
