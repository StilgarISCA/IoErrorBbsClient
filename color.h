/*
 * Copyright (C) 2024-2026 Stilgar
 * Copyright (C) 1995-2003 Michael Hampton
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef COLOR_H_INCLUDED
#define COLOR_H_INCLUDED

#include "defs.h"

void ansiTransformExpress( char *ptrText, size_t size );
void ansiTransformPostHeader( char *ptrText, size_t bufferSize, int isFriend );
void brilliantColors( void );
void colorConfig( void );
void colorOptions( void );
void colorblindColors( void );
void defaultColors( int clearall );
void expressColorConfig( void );
void expressFriendColorConfig( void );
void expressUserColorConfig( void );
void hotDogColors( void );
void generalColorConfig( void );
void inputColorConfig( void );
void postColorConfig( void );
void postFriendColorConfig( void );
void postUserColorConfig( void );
void printAnsiBackgroundColorValue( int colorValue );
void printAnsiDisplayStateValue( int foregroundColor, int backgroundColor );
void printAnsiForegroundColorValue( int colorValue );
void printAnsiResetValue( void );
void printThemedMnemonicText( const char *ptrText, int defaultColor );

int ansiTransform( int inputChar );
int ansiTransformPost( int inputChar, int isFriend );
int backgroundPicker( void );
int colorize( const char *str );
int colorPicker( void );
int colorValueFromLegacyDigit( int inputChar );
int colorValueFromName( const char *ptrColorName );
int colorValueToLegacyDigit( int colorValue );
int formatTransformedAnsiForegroundSequence( char *ptrBuffer, size_t bufferSize,
                                            int inputChar, int isPostContext,
                                            int isFriend );

const char *colorNameFromValue( int colorValue );

char expressColorMenu( void );
char generalColorMenu( void );
char postColorMenu( void );
char userOrFriend( void );

#endif /* COLOR_H_INCLUDED */
