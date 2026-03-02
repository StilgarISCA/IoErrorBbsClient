/*
 * Copyright (C) 2024-2026 Stilgar
 * Copyright (C) 1995-2003 Michael Hampton
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "defs.h"
#include "ext.h"

static const char *COLOR_MAIN_MENU_KEYS = "gipsoxq \n";
static const char *COLOR_GENERAL_MENU_KEYS = "befntq \n";
static const char *COLOR_INPUT_MENU_KEYS = "ctq \n";
static const char *COLOR_POST_MENU_KEYS = "dntq \n";
static const char *COLOR_EXPRESS_MENU_KEYS = "ntq \n";
static const char *COLOR_RESET_MENU_KEYS = "dcq \n";
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

/*
 * defaultColors is called once with an arg of 1 before the bbsrc file is
 * read.  This initializes all the color variables.  It is then called again
 * after the bbsrc file is read, with an arg of 0.  This helps with reserved
 * fields which might become used after a user upgrades to a later version
 * which might use those reserved fields - they will get their default values
 * instead of zero, which would render as black.
 */
#define ifzero( x ) if ( ( x ) < 0 || clearall )
void defaultColors( int clearall )
{
   ifzero( color.text ) color.text = 2;
   ifzero( color.forum ) color.forum = 3;
   ifzero( color.number ) color.number = 6;
   ifzero( color.errorTextColor ) color.errorTextColor = 1;
   color.reserved1 = 0;
   color.reserved2 = 0;
   color.reserved3 = 0;
   ifzero( color.postdate ) color.postdate = 5;
   ifzero( color.postname ) color.postname = 6;
   ifzero( color.posttext ) color.posttext = 2;
   ifzero( color.postfrienddate ) color.postfrienddate = 5;
   ifzero( color.postfriendname ) color.postfriendname = 1;
   ifzero( color.postfriendtext ) color.postfriendtext = 2;
   ifzero( color.anonymous ) color.anonymous = 3;
   ifzero( color.moreprompt ) color.moreprompt = 3;
   color.reserved4 = 0;
   color.reserved5 = 0;
   if ( clearall )
   {
      color.background = 0;
   }
   ifzero( color.input1 ) color.input1 = 2;
   ifzero( color.input2 ) color.input2 = 6;
   ifzero( color.expresstext ) color.expresstext = 2;
   ifzero( color.expressname ) color.expressname = 2;
   ifzero( color.expressfriendname ) color.expressfriendname = 2;
   ifzero( color.expressfriendtext ) color.expressfriendtext = 2;
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
   colorize( "\r\n@Y[K]@C Black           @Y[R]@C Red             @Y[G]@C Green           @Y[Y]@C Yellow\r\n" );
   colorize( "@Y[B]@C Blue            @Y[M]@C Magenta         @Y[C]@C Cyan            @Y[W]@C White\r\n" );
   colorize( "@Y[1]@C Bright black   @Y[2]@C Bright red      @Y[3]@C Bright green    @Y[4]@C Bright yellow\r\n" );
   colorize( "@Y[5]@C Bright blue    @Y[6]@C Bright magenta  @Y[7]@C Bright cyan     @Y[8]@C Bright white\r\n" );
   colorize( "@YSelect color -> @G" );
}

static void printBackgroundPickerMenu( void )
{
   colorize( "\r\n@Y[K]@C Black           @Y[R]@C Red             @Y[G]@C Green           @Y[Y]@C Yellow\r\n" );
   colorize( "@Y[B]@C Blue            @Y[M]@C Magenta         @Y[C]@C Cyan            @Y[W]@C White\r\n" );
   colorize( "@Y[1]@C Bright black   @Y[2]@C Bright red      @Y[3]@C Bright green    @Y[4]@C Bright yellow\r\n" );
   colorize( "@Y[5]@C Bright blue    @Y[6]@C Bright magenta  @Y[7]@C Bright cyan     @Y[8]@C Bright white\r\n" );
   colorize( "@Y[D]@C Default\r\n" );
   colorize( "@YSelect background -> @G" );
}

void colorblindColors( void )
{
   color.text = 231;
   color.forum = 75;
   color.number = 214;
   color.errorTextColor = 166;
   color.reserved1 = 16;
   color.reserved2 = 16;
   color.reserved3 = 16;
   color.postdate = 75;
   color.postname = 214;
   color.posttext = 231;
   color.postfrienddate = 25;
   color.postfriendname = 175;
   color.postfriendtext = 231;
   color.anonymous = 221;
   color.moreprompt = 221;
   color.reserved4 = 16;
   color.reserved5 = 16;
   color.background = 16;
   color.input1 = 231;
   color.input2 = 36;
   color.expresstext = 231;
   color.expressname = 214;
   color.expressfriendname = 175;
   color.expressfriendtext = 231;
}

static void presetColorConfig( void )
{
   stdPrintf( "Color presets\r\n\n" );
   colorize( "@YD@Cefault  @YC@Colorblind  @YQ@Cuit@Y -> @G" );

   switch ( readValidatedMenuKey( COLOR_RESET_MENU_KEYS ) )
   {
      case 'd':
         stdPrintf( "Default\r\n" );
         defaultColors( 1 );
         break;

      case 'c':
         stdPrintf( "Colorblind\r\n" );
         colorblindColors();
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

static void printAnsiForegroundColor( int colorValue )
{
   char aryAnsiSequence[32];

   formatAnsiForegroundSequence( aryAnsiSequence, sizeof( aryAnsiSequence ),
                                 colorValue );
   stdPrintf( "%s", aryAnsiSequence );
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
                                   flagsConfiguration.useBold );
   stdPrintf( "%s", aryAnsiSequence );
}

static int transformPostHeaderColor( int inputChar, int isFriend )
{
   switch ( inputChar )
   {
      case '6':
         if ( isFriend )
         {
            return color.postfriendname;
         }
         return color.postname;
      case '5':
         if ( isFriend )
         {
            return color.postfrienddate;
         }
         return color.postdate;
      case '3':
         return color.anonymous;
      case '2':
         if ( isFriend )
         {
            return color.postfriendtext;
         }
         return color.posttext;
      default:
         return colorValueFromLegacyDigit( inputChar );
   }
}

static void printGeneralColorPreview( void )
{
   printAnsiDisplayState( color.forum, color.background );
   stdPrintf( "Lobby> " );
   printAnsiForegroundColor( color.text );
   stdPrintf( "Enter message\r\n\n" );
   printAnsiForegroundColor( color.errorTextColor );
   stdPrintf( "Only Sysops may post to the lobby\r\n\n" );
   printAnsiForegroundColor( color.forum );
   stdPrintf( "Lobby> " );
   printAnsiForegroundColor( color.text );
   stdPrintf( "Goto " );
   printAnsiForegroundColor( color.forum );
   stdPrintf( "[Babble]  " );
   printAnsiForegroundColor( color.number );
   stdPrintf( "150" );
   printAnsiForegroundColor( color.text );
   stdPrintf( " messages, " );
   printAnsiForegroundColor( color.number );
   stdPrintf( "1" );
   printAnsiForegroundColor( color.text );
   stdPrintf( " new\r\n" );
}

static void printPostColorPreview( int dateColor, int textColor,
                                   int nameColor, const char *ptrName )
{
   printAnsiForegroundColor( dateColor );
   stdPrintf( "Jan  1, 2000 11:01" );
   printAnsiForegroundColor( textColor );
   stdPrintf( " from " );
   printAnsiForegroundColor( nameColor );
   stdPrintf( "%s", ptrName );
   printAnsiForegroundColor( textColor );
   stdPrintf( "\r\nHi there!\r\n" );
   printAnsiForegroundColor( color.forum );
   stdPrintf( "[Lobby> msg #1]\r\n" );
}

static void printExpressColorPreview( int textColor, int nameColor,
                                      const char *ptrName )
{
   printAnsiForegroundColor( textColor );
   stdPrintf( "*** Message (#1) from " );
   printAnsiForegroundColor( nameColor );
   stdPrintf( "%s", ptrName );
   printAnsiForegroundColor( textColor );
   stdPrintf( " at 11:01 ***\r\n>Hi there!\r\n" );
}

static void printInputColorPreview( void )
{
   printAnsiForegroundColor( color.text );
   stdPrintf( "Message eXpress\r\nRecipient: " );
   printAnsiForegroundColor( color.input1 );
   stdPrintf( "Exam" );
   printAnsiForegroundColor( color.input2 );
   stdPrintf( "ple User\r\n" );
   printAnsiForegroundColor( color.input1 );
   stdPrintf( ">Hi there!\r\n" );
   printAnsiForegroundColor( color.text );
   stdPrintf( "Message received by Example User.\r\n" );
}

int ansiTransform( int inputChar )
{
   int transformedColor;

   transformedColor = colorValueFromLegacyDigit( inputChar );
   switch ( inputChar )
   {
      case '6':
         transformedColor = color.number;
         break;
      case '3':
         transformedColor = color.forum;
         break;
      case '2':
         transformedColor = color.text;
         break;
      case '1':
         transformedColor = color.errorTextColor;
         break;
      default:
         break;
   }

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
   if ( !flagsConfiguration.useAnsi )
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
                                    color.expressfriendtext );
      formatAnsiForegroundSequence( aryNameColor, sizeof( aryNameColor ),
                                    color.expressfriendname );
   }
   else
   {
      formatAnsiForegroundSequence( aryMessageColor, sizeof( aryMessageColor ),
                                    color.expresstext );
      formatAnsiForegroundSequence( aryNameColor, sizeof( aryNameColor ),
                                    color.expressname );
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

   transformedColor = colorValueFromLegacyDigit( inputChar );
   switch ( inputChar )
   {
      case '3':
         transformedColor = color.moreprompt;
         break;
      case '2':
         if ( isFriend )
         {
            transformedColor = color.postfriendtext;
         }
         else
         {
            transformedColor = color.posttext;
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
   if ( !flagsConfiguration.useAnsi )
   {
      stdPrintf( "\r\nWARNING:  Color is off.  You will not be able to preview your selections." );
   }
   while ( true )
   {
      snprintf( aryPromptText, sizeof( aryPromptText ), "\r\n@YG@Ceneral  @YI@Cnput  @YP@Costs  pre@YS@Cets  @YO@Cptions  @YX@Cpress  @YQ@Cuit\r\n@YColor config -> @G" );
      colorize( aryPromptText );

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
            stdPrintf( "Post colors\r\n\n" );
            postColorConfig();
            break;

         case 's':
            presetColorConfig();
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
              flagsConfiguration.useBold ? "Yes" : "No" );
   flagsConfiguration.useBold = (unsigned int)yesNoDefault( flagsConfiguration.useBold );
   if ( flagsConfiguration.useAnsi )
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

      snprintf( aryPromptText, sizeof( aryPromptText ), "\r\n@YB@Cackground  @YE@Crror  @YF@Corum  @YN@Cumber  @YT@Cext  @YQ@Cuit@Y -> @G" );
      colorize( aryPromptText );

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

      snprintf( aryPromptText, sizeof( aryPromptText ), "\r\n@YT@Cext  @YC@Completion  @YQ@Cuit@Y -> @G" );
      colorize( aryPromptText );

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
            color.input2 = colorPicker();
            break;
         case 't':
            stdPrintf( "Text\r\n\n" );
            color.input1 = colorPicker();
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

      printPostColorPreview( color.postdate, color.posttext,
                             color.postname, A_USER );
      menuOption = postColorMenu();
      switch ( menuOption )
      {
         case 'q':
         case ' ':
         case '\n':
            return;
            /* NOTREACHED */
         case 'd':
            color.postdate = colorPicker();
            break;
         case 'n':
            color.postname = colorPicker();
            break;
         case 't':
            color.posttext = colorPicker();
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

      printPostColorPreview( color.postfrienddate, color.postfriendtext,
                             color.postfriendname, A_FRIEND );
      menuOption = postColorMenu();
      switch ( menuOption )
      {
         case 'q':
         case ' ':
         case '\n':
            return;
            /* NOTREACHED */
         case 'd':
            color.postfrienddate = colorPicker();
            break;
         case 'n':
            color.postfriendname = colorPicker();
            break;
         case 't':
            color.postfriendtext = colorPicker();
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

   snprintf( aryPromptText, sizeof( aryPromptText ), "\r\n@YD@Cate  @YN@Came  @YT@Cext  @YQ@Cuit@Y -> @G" );
   colorize( aryPromptText );

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

      printExpressColorPreview( color.expresstext, color.expressname,
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
            color.expressname = colorPicker();
            break;
         case 't':
            color.expresstext = colorPicker();
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

      printExpressColorPreview( color.expressfriendtext,
                                color.expressfriendname, A_FRIEND );
      menuOption = expressColorMenu();
      switch ( menuOption )
      {
         case 'q':
         case ' ':
         case '\n':
            return;
            /* NOTREACHED */
         case 'n':
            color.expressfriendname = colorPicker();
            break;
         case 't':
            color.expressfriendtext = colorPicker();
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

   snprintf( aryPromptText, sizeof( aryPromptText ), "\r\n@YN@Came  @YT@Cext  @YQ@Cuit@Y -> @G" );
   colorize( aryPromptText );

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
   char aryPromptText[100];

   snprintf( aryPromptText, sizeof( aryPromptText ), "@GConfigure for @YU@Cser @Gor @YF@Criend @Y-> @G" );
   colorize( aryPromptText );

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
