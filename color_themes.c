/*
 * Copyright (C) 2024-2026 Stilgar
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "defs.h"
#include "client_globals.h"
#include "proto.h"

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
   ifzero( color.postDate ) color.postDate = 5;
   ifzero( color.postName ) color.postName = 6;
   ifzero( color.postText ) color.postText = 2;
   ifzero( color.postFriendDate ) color.postFriendDate = 5;
   ifzero( color.postFriendName ) color.postFriendName = 1;
   ifzero( color.postFriendText ) color.postFriendText = 2;
   ifzero( color.anonymous ) color.anonymous = 3;
   ifzero( color.morePrompt ) color.morePrompt = 3;
   ifzero( color.ansiWhiteTextColor ) color.ansiWhiteTextColor = 7;
   color.reserved5 = 7;
   if ( clearall )
   {
      color.background = 0;
   }
   ifzero( color.inputText ) color.inputText = 2;
   ifzero( color.inputHighlight ) color.inputHighlight = 6;
   ifzero( color.expressText ) color.expressText = 2;
   ifzero( color.expressName ) color.expressName = 2;
   ifzero( color.expressFriendName ) color.expressFriendName = 2;
   ifzero( color.expressFriendText ) color.expressFriendText = 2;
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
   color.postDate = 13;
   color.postName = 14;
   color.postText = 10;
   color.postFriendDate = 13;
   color.postFriendName = 9;
   color.postFriendText = 10;
   color.anonymous = 11;
   color.morePrompt = 11;
   color.ansiWhiteTextColor = 15;
   color.reserved5 = 15;
   color.background = 0;
   color.inputText = 10;
   color.inputHighlight = 14;
   color.expressText = 10;
   color.expressName = 10;
   color.expressFriendName = 10;
   color.expressFriendText = 10;
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
   color.postDate = 75;
   color.postName = 214;
   color.postText = 231;
   color.postFriendDate = 25;
   color.postFriendName = 175;
   color.postFriendText = 231;
   color.anonymous = 221;
   color.morePrompt = 221;
   color.ansiWhiteTextColor = 221;
   color.reserved5 = 231;
   color.background = 16;
   color.inputText = 231;
   color.inputHighlight = 214;
   color.expressText = 231;
   color.expressName = 214;
   color.expressFriendName = 175;
   color.expressFriendText = 231;
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
   color.postDate = 226;
   color.postName = 226;
   color.postText = 214;
   color.postFriendDate = 226;
   color.postFriendName = 226;
   color.postFriendText = 214;
   color.anonymous = 226;
   color.morePrompt = 220;
   color.ansiWhiteTextColor = 220;
   color.reserved5 = 130;
   color.background = 16;
   color.inputText = 220;
   color.inputHighlight = 231;
   color.expressText = 214;
   color.expressName = 226;
   color.expressFriendName = 226;
   color.expressFriendText = 214;
}
