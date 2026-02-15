/* IO ERROR's BBS client 2.3, Michael Hampton.  Modified from the original
 * ISCA BBS client 1.5 and patches.  Copyright 1999 Michael Hampton.
 * Internet: error@citadel.org  WWW: http://ioerror.bbsclient.net/
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation; either version 2, or (at your option) any later
 * version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program; if not, write to the Free Software Foundation, Inc., 675 Mass
 * Ave, Cambridge, MA 02139, USA.
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

/*
 * defaultColors is called once with an arg of 1 before the bbsrc file is
 * read.  This initializes all the color variables.  It is then called again
 * after the bbsrc file is read, with an arg of 0.  This helps with reserved
 * fields which might become used after a user upgrades to a later version
 * which might use those reserved fields - they will get their default values
 * instead of zero, which would render as black.
 */
#define ifzero( x ) if ( ( x ) < '1' || ( x ) > '7' || clearall )
void defaultColors( int clearall )
{
   ifzero( color.text ) color.text = '2';
   ifzero( color.forum ) color.forum = '3';
   ifzero( color.number ) color.number = '6';
   ifzero( color.errorTextColor ) color.errorTextColor = '1';
   color.reserved1 = '0';
   color.reserved2 = '0';
   color.reserved3 = '0';
   ifzero( color.postdate ) color.postdate = '5';
   ifzero( color.postname ) color.postname = '6';
   ifzero( color.posttext ) color.posttext = '2';
   ifzero( color.postfrienddate ) color.postfrienddate = '5';
   ifzero( color.postfriendname ) color.postfriendname = '1';
   ifzero( color.postfriendtext ) color.postfriendtext = '2';
   ifzero( color.anonymous ) color.anonymous = '3';
   ifzero( color.moreprompt ) color.moreprompt = '3';
   color.reserved4 = '0';
   color.reserved5 = '0';
   if ( clearall )
   {
      color.background = '0';
   }
   ifzero( color.input1 ) color.input1 = '2';
   ifzero( color.input2 ) color.input2 = '6';
   ifzero( color.expresstext ) color.expresstext = '2';
   ifzero( color.expressname ) color.expressname = '2';
   ifzero( color.expressfriendname ) color.expressfriendname = '2';
   ifzero( color.expressfriendtext ) color.expressfriendtext = '2';
}

char ansiTransform( char inputChar )
{
   switch ( inputChar )
   {
      case '6':
         inputChar = color.number;
         break;
      case '3':
         inputChar = color.forum;
         break;
      case '2':
         inputChar = color.text;
         break;
      case '1':
         inputChar = color.errorTextColor;
         break;
      default:
         break;
   }

   return (char)inputChar;
}

void ansiTransformExpress( char *ptrText, size_t size )
{
   char aryTempText[580];
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
      snprintf( aryTempText, sizeof( aryTempText ), "\033[3%cm%s \033[3%cm%s\033[3%cm %s\033[3%cm",
                color.expressfriendtext, ptrText, color.expressfriendname, ptrExpressSender,
                color.expressfriendtext, ptrExpressMarker, color.text );
   }
   else
   {
      snprintf( aryTempText, sizeof( aryTempText ), "\033[3%cm%s \033[3%cm%s\033[3%cm %s\033[3%cm",
                color.expresstext, ptrText, color.expressname, ptrExpressSender,
                color.expresstext, ptrExpressMarker, color.text );
   }
   lastColor = color.text;
   snprintf( ptrText, size, "%s", aryTempText );
}

char ansiTransformPost( char inputChar, int isFriend )
{
   switch ( inputChar )
   {
      case '3':
         inputChar = color.moreprompt;
         break;
      case '2':
         if ( isFriend )
         {
            inputChar = color.postfriendtext;
         }
         else
         {
            inputChar = color.posttext;
         }
         break;
      case '1':
         inputChar = color.errorTextColor;
         break;
      default:
         break;
   }
   return (char)inputChar;
}

void ansiTransformPostHeader( char *ptrText, int isFriend )
{
   char *ptrScan;

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
                  *ptrScan = color.postfriendname;
               }
               else
               {
                  *ptrScan = color.postname;
               }
               break;
            case '5':
               *ptrScan = color.postdate;
               break;
            case '3':
               *ptrScan = color.anonymous;
               break;
            case '2':
               if ( isFriend )
               {
                  *ptrScan = color.postfriendtext;
               }
               else
               {
                  *ptrScan = color.posttext;
               }
               break;
            default:
               break;
         }
         lastColor = *ptrScan;
      }
   }
}

void colorConfig( void )
{
   char aryPromptText[110];
   int inputChar;

   stdPrintf( "Color\r\n" );
   if ( !flagsConfiguration.useAnsi )
   {
      stdPrintf( "\r\nWARNING:  Color is off.  You will not be able to preview your selections." );
   }
   while ( true )
   {
      snprintf( aryPromptText, sizeof( aryPromptText ), "\r\n@YG@Ceneral  @YI@Cnput  @YP@Costs  @YX@Cpress  @YO@Cptions  @YR@Ceset  @YQ@Cuit\r\n@YColor config -> @G" );
      colorize( aryPromptText );

      inputChar = readValidatedMenuKey( COLOR_MAIN_MENU_KEYS );

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
      printf( "\033[%cm\033[3%c;4%cm", flagsConfiguration.useBold ? '1' : '0', lastColor,
              color.background );
   }
}

#define A_USER "Example User"
#define A_FRIEND "Example Friend"
#define GEN_FMT_STR "\033[4%c;3%cmLobby> \033[3%cmEnter message\r\n\n\033[3%cmOnly Sysops may post to the lobby\r\n\n\033[3%cmLobby> \033[3%cmGoto \033[3%cm[Babble]  \033[3%cm150\033[3%cm messages, \033[3%cm1\033[3%cm new\r\n"
#define POST_FMT_STR "\033[3%cmJan  1, 2000 11:01\033[3%cm from \033[3%cm%s\033[3%cm\r\nHi there!\r\n\033[3%cm[Lobby> msg #1]\r\n"
#define EXPRESS_FMT_STR "\033[3%cm*** Message (#1) from \033[3%cm%s\033[3%cm at 11:01 ***\r\n>Hi there!\r\n"
#define INPUT_FMT_STR "\033[3%cmMessage eXpress\r\nRecipient: \033[3%cmExam\033[3%cmple User\r\n\033[3%cm>Hi there!\r\n\033[3%cmMessage received by Example User.\r\n"

void generalColorConfig( void )
{
   int menuOption;
   char aryPromptText[100];

   while ( true )
   {
      stdPrintf( GEN_FMT_STR, color.background, color.forum,
                 color.text, color.errorTextColor, color.forum,
                 color.text, color.forum, color.number,
                 color.text, color.number, color.text );

      snprintf( aryPromptText, sizeof( aryPromptText ), "\r\n@YB@Cackground  @YE@Crror  @YF@Corum  @YN@Cumber  @YT@Cext  @YQ@Cuit@Y -> @G" );
      colorize( aryPromptText );

      menuOption = readValidatedMenuKey( COLOR_GENERAL_MENU_KEYS );

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
   int menuOption;
   char aryPromptText[100];

   while ( true )
   {
      stdPrintf( INPUT_FMT_STR, color.text, color.input1,
                 color.input2, color.input1, color.text );

      snprintf( aryPromptText, sizeof( aryPromptText ), "\r\n@YT@Cext  @YC@Completion  @YQ@Cuit@Y -> @G" );
      colorize( aryPromptText );

      menuOption = readValidatedMenuKey( COLOR_INPUT_MENU_KEYS );

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
   int menuOption;

   while ( true )
   {
      stdPrintf( POST_FMT_STR, color.postdate, color.posttext,
                 color.postname, A_USER, color.posttext, color.forum );
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
   int menuOption;

   while ( true )
   {
      stdPrintf( POST_FMT_STR, color.postfrienddate, color.postfriendtext,
                 color.postfriendname, A_FRIEND, color.postfriendtext,
                 color.forum );
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
   int menuOption;

   while ( true )
   {
      stdPrintf( EXPRESS_FMT_STR, color.expresstext,
                 color.expressname, A_USER, color.expresstext );
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
   int menuOption;

   while ( true )
   {
      stdPrintf( EXPRESS_FMT_STR, color.expressfriendtext,
                 color.expressfriendname, A_FRIEND, color.expressfriendtext );
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

char colorPicker( void )
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
         inputChar = '1';
         break;
      case 'g':
         stdPrintf( "Green\r\n\n" );
         inputChar = '2';
         break;
      case 'y':
         stdPrintf( "Yellow\r\n\n" );
         inputChar = '3';
         break;
      case 'b':
         stdPrintf( "Blue\r\n\n" );
         inputChar = '4';
         break;
      case 'm':
      case 'p': /* Some people call it purple */
         stdPrintf( "Magenta\r\n\n" );
         inputChar = '5';
         break;
      case 'c':
         stdPrintf( "Cyan\r\n\n" );
         inputChar = '6';
         break;
      case 'w':
         stdPrintf( "White\r\n\n" );
         inputChar = '7';
         break;
      case 'k':
         stdPrintf( "Black\r\n\n" );
         inputChar = '0';
         break;
      default:
         inputChar = '0'; /* If your text goes black it's a bug here */
         break;
   }

   return (char)inputChar;
}

char backgroundPicker( void )
{
   int inputChar;
   char aryPromptText[140];

   snprintf( aryPromptText, sizeof( aryPromptText ), "@C@kBlac@Yk @r @WR@Ced @g @WG@Yreen @y @WY@Cellow @b @YB@Ylue @m @WM@Yagenta @c @WC@Yyan @w @YW@Bhite @d @YD@Cefault \033[4%cm @Y-> @G",
             color.background );
   colorize( aryPromptText );

   inputChar = readValidatedMenuKey( COLOR_BACKGROUND_KEYS );

   switch ( inputChar )
   {
      case 'k':
         stdPrintf( "Black\r\n" );
         inputChar = '0';
         break;
      case 'r':
         stdPrintf( "Red\r\n" );
         inputChar = '1';
         break;
      case 'g':
         stdPrintf( "Green\r\n" );
         inputChar = '2';
         break;
      case 'y':
         stdPrintf( "Yellow\r\n" );
         inputChar = '3';
         break;
      case 'b':
         stdPrintf( "Blue\r\n" );
         inputChar = '4';
         break;
      case 'm':
      case 'p': /* Some people call it purple */
         stdPrintf( "Magenta\r\n" );
         inputChar = '5';
         break;
      case 'c':
         stdPrintf( "Cyan\r\n" );
         inputChar = '6';
         break;
      case 'w':
         stdPrintf( "White\r\n" );
         inputChar = '7';
         break;
      case 'd':
         stdPrintf( "Default\r\n" );
         inputChar = '9';
         break;
      default:
         inputChar = '0'; /* If your text goes black it's a bug here */
         break;
   }
   stdPrintf( "\033[4%cm\n", inputChar );

   return (char)inputChar;
}
