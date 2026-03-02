/*
 * Copyright (C) 2024-2026 Stilgar
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "defs.h"
#include "ext.h"

/*
 * defaultColors is called once with an arg of 1 before the bbsrc file is
 * read.  This initializes all the color variables.  It is then called again
 * after the bbsrc file is read, with an arg of 0.  This helps with reserved
 * fields which might become used after a user upgrades to a later version
 * which might use those reserved fields. They will get their default values
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

void hotDogColors( void )
{
   color.text = 220;
   color.forum = 196;
   color.number = 220;
   color.errorTextColor = 231;
   color.reserved1 = 16;
   color.reserved2 = 16;
   color.reserved3 = 16;
   color.postdate = 226;
   color.postname = 226;
   color.posttext = 214;
   color.postfrienddate = 226;
   color.postfriendname = 226;
   color.postfriendtext = 214;
   color.anonymous = 226;
   color.moreprompt = 220;
   color.reserved4 = 16;
   color.reserved5 = 16;
   color.background = 16;
   color.input1 = 220;
   color.input2 = 231;
   color.expresstext = 214;
   color.expressname = 226;
   color.expressfriendname = 226;
   color.expressfriendtext = 214;
}
