/*
 * Copyright (C) 2024-2026 Stilgar
 * Copyright (C) 1995-2003 Michael Hampton
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef CLIENT_GLOBALS_H_INCLUDED
#define CLIENT_GLOBALS_H_INCLUDED

#include "defs.h"

extern char aryEscape[2];
extern int lastPtr;                                       /* which aryLastName we are currently on */
extern Flags flagsConfiguration;                          /* Miscellaneous flags */
extern Color color;                                       /* color transformations */
extern char aryLastName[MAX_USER_NAME_HISTORY_COUNT][21]; /* last username received in post or X */
extern bool isAway;                                       /* away from keyboard? */
extern bool isXland;                                      /* X Land - auto-fill-in-recipient */
extern queue *xlandQueue;                                 /* X Land name queue */
extern char aryAutoName[21];                              /* Automatic login name */
#ifdef ENABLE_SAVE_PASSWORD
extern char aryAutoPassword[21]; /* Automatic password */
extern bool isAutoPasswordSent;  /* Set after password sent to BBS */
#endif
extern bool isAutoLoggedIn; /* Has autologin been done? */
extern int sendingXState;   /* automatically sending an X? */
extern int aryReplyRecipient[21];
extern char aryUser[80];     /* username */
extern char aryEditor[80];   /* name of aryEditor to invoke */
extern char aryMyEditor[80]; /* name of aryUser's preferred aryEditor */

extern int childPid; /* process id of child process */
#ifdef USE_POSIX_SIGSETJMP
extern sigjmp_buf jumpEnv; /* Yuck!  I have to use longjmp!  Gag! */
#else
extern jmp_buf jumpEnv; /* Yuck!  I have to use longjmp!  Gag! */
#endif

extern int rows;      /* number of rows on aryUser's screen */
extern int oldRows;   /* previous value of rows */
extern int lastColor; /* last color code received from BBS */
extern bool isLoginShell; /* whether this client is a login shell */
extern int version;       /* Client version number */

#endif /* CLIENT_GLOBALS_H_INCLUDED */
