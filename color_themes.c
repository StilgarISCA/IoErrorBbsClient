/*
 * Copyright (C) 2024-2026 Stilgar
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "defs.h"
#include "ext.h"

/*
 * defaultColors is called once with an arg of 1 before the bbsrc file is
 * read.  This initializes all the color variables.  It is then called again
 * after the bbsrc file is read, with an arg of 0.  This helps with theme
 * fields which might become used after a user upgrades to a later version.
 * They will get their default values instead of zero, which would render
 * as black.
 */
#define ifzero( x ) if ( ( x ) < 0 || clearall )
void defaultColors( int clearall )
{
   ifzero( color.text ) color.text = 2;
   ifzero( color.forum ) color.forum = 3;
   ifzero( color.number ) color.number = 6;
   ifzero( color.errorTextColor ) color.errorTextColor = 1;
   ifzero( color.ansiBlackTextColor ) color.ansiBlackTextColor = 2;
   ifzero( color.ansiBlueTextColor ) color.ansiBlueTextColor = 4;
   ifzero( color.ansiMagentaTextColor ) color.ansiMagentaTextColor = 5;
   ifzero( color.postdate ) color.postdate = 5;
   ifzero( color.postname ) color.postname = 6;
   ifzero( color.posttext ) color.posttext = 2;
   ifzero( color.postfrienddate ) color.postfrienddate = 5;
   ifzero( color.postfriendname ) color.postfriendname = 1;
   ifzero( color.postfriendtext ) color.postfriendtext = 2;
   ifzero( color.anonymous ) color.anonymous = 3;
   ifzero( color.moreprompt ) color.moreprompt = 3;
   ifzero( color.ansiWhiteTextColor ) color.ansiWhiteTextColor = 7;
   color.reserved5 = 7;
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

void brilliantColors( void )
{
   color.text = 10;
   color.forum = 11;
   color.number = 14;
   color.errorTextColor = 9;
   color.ansiBlackTextColor = 10;
   color.ansiBlueTextColor = 12;
   color.ansiMagentaTextColor = 13;
   color.postdate = 13;
   color.postname = 14;
   color.posttext = 10;
   color.postfrienddate = 13;
   color.postfriendname = 9;
   color.postfriendtext = 10;
   color.anonymous = 11;
   color.moreprompt = 11;
   color.ansiWhiteTextColor = 15;
   color.reserved5 = 15;
   color.background = 0;
   color.input1 = 10;
   color.input2 = 14;
   color.expresstext = 10;
   color.expressname = 10;
   color.expressfriendname = 10;
   color.expressfriendtext = 10;
}

void colorblindColors( void )
{
   color.text = 231;
   color.forum = 75;
   color.number = 214;
   color.errorTextColor = 166;
   color.ansiBlackTextColor = 146;
   color.ansiBlueTextColor = 75;
   color.ansiMagentaTextColor = 175;
   color.postdate = 75;
   color.postname = 214;
   color.posttext = 231;
   color.postfrienddate = 25;
   color.postfriendname = 175;
   color.postfriendtext = 231;
   color.anonymous = 221;
   color.moreprompt = 221;
   color.ansiWhiteTextColor = 221;
   color.reserved5 = 231;
   color.background = 16;
   color.input1 = 231;
   color.input2 = 214;
   color.expresstext = 231;
   color.expressname = 214;
   color.expressfriendname = 175;
   color.expressfriendtext = 231;
}

void hotDogColors( void )
{
   color.text = 220;
   color.forum = 196;
   color.number = 220;
   color.errorTextColor = 231;
   color.ansiBlackTextColor = 130;
   color.ansiBlueTextColor = 214;
   color.ansiMagentaTextColor = 130;
   color.postdate = 226;
   color.postname = 226;
   color.posttext = 214;
   color.postfrienddate = 226;
   color.postfriendname = 226;
   color.postfriendtext = 214;
   color.anonymous = 226;
   color.moreprompt = 220;
   color.ansiWhiteTextColor = 220;
   color.reserved5 = 130;
   color.background = 16;
   color.input1 = 220;
   color.input2 = 231;
   color.expresstext = 214;
   color.expressname = 226;
   color.expressfriendname = 226;
   color.expressfriendtext = 214;
}
