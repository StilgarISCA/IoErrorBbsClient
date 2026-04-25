/*
 * Copyright (C) 2024-2026 Stilgar
 * Copyright (C) 1995-2003 Michael Hampton
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef FILTER_GLOBALS_H_INCLUDED
#define FILTER_GLOBALS_H_INCLUDED

#include "defs.h"

extern char aryExpressMessageBuffer[8192]; // Buffer for X message
extern char aryExpressParsing[400];        // for incoming X messages
extern char aryFilterLine[320];            // current filter line buffer
extern char aryPostBuffer[160];            // Buffer for post header (for kill files)
extern char arySavedHeader[160];           // Saved message header
extern slist *enemyList;                   // 'enemy' list
extern slist *friendList;                  // 'friend' list
extern int highestExpressMessageId;        // Highest X message counter
extern bool isExpressMessageHeaderActive;  // True if X message header being received
extern bool isExpressMessageInProgress;    // True while an X message is being received
extern bool isPostJustEnded;               // True for a line after a post received
extern int pendingLinesToEat;
extern int postHeaderActive;          // True if post header being received
extern int postProgressState;         // True while a post is being received
extern int ptrPostBuffer;             // Pointer for post header buffer
extern char *ptrExpressMessageBuffer; // Pointer for X message buffer
extern int savedWhoCount;             // pointer to end of saved who list
extern bool shouldSendExpressMessage;

extern unsigned char arySavedWhoInfo[150][80];  // added saved info array
extern unsigned char arySavedWhoNames[150][21]; // array for saved who list
extern slist *whoList;                          // saved who list
extern int whoListProgress;                     // True while a who list is being received

extern queue *urlQueue; // Structure holding recently seen URLs

#endif // FILTER_GLOBALS_H_INCLUDED
