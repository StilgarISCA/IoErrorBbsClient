/*
 * Copyright (C) 2024-2026 Stilgar
 * Copyright (C) 1995-2003 Michael Hampton
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "defs.h"
#include "ext.h"

static const char *COLOR_MAIN_MENU_KEYS = "gipxorq \n";
static const char *COLOR_GENERAL_MENU_KEYS = "befntq \n";
static const char *COLOR_INPUT_MENU_KEYS = "ctq \n";
static const char *COLOR_POST_MENU_KEYS = "dntq \n";
static const char *COLOR_EXPRESS_MENU_KEYS = "ntq \n";
static const char *COLOR_USER_OR_FRIEND_KEYS = "ufq \n";
static const char *COLOR_FOREGROUND_KEYS = "krgybmcw";
static const char *COLOR_BACKGROUND_KEYS = "krgybmcwd";

typedef struct
{
   const char *ptrName;
   int colorValue;
} NamedColorSpec;

static const NamedColorSpec aryNamedColors[] =
   {
      { "black", 16 },
      { "red", 160 },
      { "green", 34 },
      { "yellow", 220 },
      { "blue", 26 },
      { "magenta", 91 },
      { "purple", 91 },
      { "cyan", 44 },
      { "white", 231 },
      { "default", 9 } };

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
#define ifzero( x ) if ( ( x ) < 1 || ( x ) > 7 || clearall )
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

void ansiTransformPostHeader( char *ptrText, int isFriend )
{
   char *ptrScan;
   int transformedColor;

   /* Would have been easier with strtok() but can't guarantee it exists. */
   for ( ptrScan = ptrText; *ptrScan; ptrScan++ )
   {
      /* Find an ANSI code */
      if ( *ptrScan == 27 )
      {
         /* transform ANSI code */
         ptrScan += 3;

         switch ( *ptrScan )
         {
            case '6':
               if ( isFriend )
               {
                  transformedColor = color.postfriendname;
               }
               else
               {
                  transformedColor = color.postname;
               }
               break;
            case '5':
               if ( isFriend )
               {
                  transformedColor = color.postfrienddate;
               }
               else
               {
                  transformedColor = color.postdate;
               }
               break;
            case '3':
               transformedColor = color.anonymous;
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
            default:
               transformedColor = colorValueFromLegacyDigit( *ptrScan );
               break;
         }
         *ptrScan = (char)colorValueToLegacyDigit( transformedColor );
         lastColor = transformedColor;
      }
   }
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
      snprintf( aryPromptText, sizeof( aryPromptText ), "\r\n@YG@Ceneral  @YI@Cnput  @YP@Costs  @YX@Cpress  @YO@Cptions  @YR@Ceset  @YQ@Cuit\r\n@YColor config -> @G" );
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

         case 'r':
            stdPrintf( "Reset colors\r\n" );
            defaultColors( 1 );
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
   int inputChar;
   char aryPromptText[100];

   snprintf( aryPromptText, sizeof( aryPromptText ), "@CBlac@Yk  @YR@Red  @YG@Green  @WY@Yellow  @YB@Blue  @YM@Magenta  @YC@Cyan  @YW@White @Y-> @G" );
   colorize( aryPromptText );

   inputChar = readValidatedMenuKey( COLOR_FOREGROUND_KEYS );

   switch ( inputChar )
   {
      case 'r':
         stdPrintf( "Red\r\n\n" );
         inputChar = 1;
         break;
      case 'g':
         stdPrintf( "Green\r\n\n" );
         inputChar = 2;
         break;
      case 'y':
         stdPrintf( "Yellow\r\n\n" );
         inputChar = 3;
         break;
      case 'b':
         stdPrintf( "Blue\r\n\n" );
         inputChar = 4;
         break;
      case 'm':
      case 'p': /* Some people call it purple */
         stdPrintf( "Magenta\r\n\n" );
         inputChar = 5;
         break;
      case 'c':
         stdPrintf( "Cyan\r\n\n" );
         inputChar = 6;
         break;
      case 'w':
         stdPrintf( "White\r\n\n" );
         inputChar = 7;
         break;
      case 'k':
         stdPrintf( "Black\r\n\n" );
         inputChar = 0;
         break;
      default:
         inputChar = 0; /* If your text goes black it's a bug here */
         break;
   }

   return inputChar;
}

int backgroundPicker( void )
{
   int inputChar;
   char aryPromptText[140];

   snprintf( aryPromptText, sizeof( aryPromptText ), "@C@kBlac@Yk @r @WR@Ced @g @WG@Yreen @y @WY@Cellow @b @YB@Ylue @m @WM@Yagenta @c @WC@Yyan @w @YW@Bhite @d @YD@Cefault " );
   colorize( aryPromptText );
   printAnsiBackgroundColor( color.background );
   colorize( " @Y-> @G" );

   inputChar = readValidatedMenuKey( COLOR_BACKGROUND_KEYS );

   switch ( inputChar )
   {
      case 'k':
         stdPrintf( "Black\r\n" );
         inputChar = 0;
         break;
      case 'r':
         stdPrintf( "Red\r\n" );
         inputChar = 1;
         break;
      case 'g':
         stdPrintf( "Green\r\n" );
         inputChar = 2;
         break;
      case 'y':
         stdPrintf( "Yellow\r\n" );
         inputChar = 3;
         break;
      case 'b':
         stdPrintf( "Blue\r\n" );
         inputChar = 4;
         break;
      case 'm':
      case 'p': /* Some people call it purple */
         stdPrintf( "Magenta\r\n" );
         inputChar = 5;
         break;
      case 'c':
         stdPrintf( "Cyan\r\n" );
         inputChar = 6;
         break;
      case 'w':
         stdPrintf( "White\r\n" );
         inputChar = 7;
         break;
      case 'd':
         stdPrintf( "Default\r\n" );
         inputChar = 9;
         break;
      default:
         inputChar = 0; /* If your text goes black it's a bug here */
         break;
   }
   printAnsiBackgroundColor( inputChar );
   stdPrintf( "\n" );

   return inputChar;
}
