/*
 * Copyright (C) 2024-2026 Stilgar
 * Copyright (C) 1995-2003 Michael Hampton
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef FILTER_GLOBALS_H_INCLUDED
#define FILTER_GLOBALS_H_INCLUDED

#include "defs.h"

extern slist *friendList;                       /* 'friend' list */
extern unsigned char arySavedWhoNames[150][21]; /* array for saved who list */
extern int savedWhoCount;                       /* pointer to end of saved who list */
extern slist *enemyList;                        /* 'enemy' list */
extern slist *whoList;                          /* saved who list */
extern char aryFilterLine[320];                 /* current filter line buffer */
extern char arySavedHeader[160];                /* for saving our message header */
extern unsigned char arySavedWhoInfo[150][80];  /* added saved info array */
extern char aryExpressParsing[400];             /* for incoming X messages */

extern int whoListProgress;                /* Are we currently receiving a who list? */
extern char aryPostBuffer[160];            /* Buffer for post header (for kill files) */
extern int ptrPostBuffer;                  /* Pointer for post header buffer */
extern int postHeaderActive;               /* True if post header being received */
extern char aryExpressMessageBuffer[8192]; /* Buffer for X message */
extern int highestExpressMessageId;        /* Highest X message counter */
extern char *ptrExpressMessageBuffer;      /* Pointer for X message buffer */
extern bool isExpressMessageHeaderActive;  /* True if X message header being received */
extern int postProgressState;              /* True while we are receiving a post */
extern bool isPostJustEnded;               /* True for a line after a post received */
extern bool isExpressMessageInProgress;    /* True while we are receiving an X message */
extern bool shouldSendExpressMessage;
extern int pendingLinesToEat;

extern queue *urlQueue; /* Structure holding recently seen URLs */

#endif /* FILTER_GLOBALS_H_INCLUDED */
