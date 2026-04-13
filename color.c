/*
 * Copyright (C) 2024-2026 Stilgar
 * Copyright (C) 1995-2003 Michael Hampton
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "defs.h"
#include "ext.h"

typedef struct
{
   const char *ptrName;
   int colorValue;
} NamedColorSpec;

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
