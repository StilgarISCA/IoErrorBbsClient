/*
 * Copyright (C) 2024-2026 Stilgar
 * Copyright (C) 1995-2003 Michael Hampton
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

/*
 * This is the home of all global variables (see global.c)
 */

#ifndef EXT_H_INCLUDED
#define EXT_H_INCLUDED

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
extern bool isAutoLoggedIn;            /* Has autologin been done? */
extern int sendingXState;              /* automatically sending an X? */
extern int aryReplyRecipient[21];      /* reply recipient for autoreply */
extern char aryBbsRcName[PATH_MAX];    /* bbsrc filename (usually ~/.bbsrc) */
extern FILE *ptrBbsRc;                 /* file descriptor of .bbsrc file */
extern bool isBbsRcReadOnly;           /* set if .bbsrc file is read-only */
extern char aryTempFileName[PATH_MAX]; /* bbstmp filename (usually ~/.bbstmp) */
extern FILE *tempFile;                 /* file pointer to above */
extern char aryUser[80];               /* username */
extern char aryEditor[80];             /* name of aryEditor to invoke */
extern char aryMyEditor[80];           /* name of aryUser's preferred aryEditor */

extern int childPid; /* process id of child process */
#ifdef USE_POSIX_SIGSETJMP
extern sigjmp_buf jumpEnv; /* Yuck!  I have to use longjmp!  Gag! */
#else
extern jmp_buf jumpEnv; /* Yuck!  I have to use longjmp!  Gag! */
#endif

extern char aryBbsHost[64];         /* name of bbs host (bbs.isca.uiowa.edu) */
extern char aryCommandLineHost[64]; /* name of bbs host from command line */
extern unsigned short bbsPort;      /* port to connect to (23 or 992) */
extern unsigned short cmdLinePort;  /* port to connect to from command line */
extern bool shouldUseSsl;           /* Whether the connection should be secure */
extern bool isSsl;                  /* Whether the current connection is secured */

extern slist *friendList;                       /* 'friend' list */
extern unsigned char arySavedWhoNames[150][21]; /* array for saved who list */
extern int savedWhoCount;                       /* pointer to end of saved who list */
extern slist *enemyList;                        /* 'enemy' list */
extern slist *whoList;                          /* saved who list */
extern char arySavedHeader[160];                /* for saving our message header */

extern char aryMacro[128][72];           /* array for macros */
extern char aryAwayMessageLines[6][80];  /* Away from keyboard message */
extern int commandKey;                   /* hotkey for signalling a aryMacro follows */
extern int quitKey;                      /* hotkey to quit (commandKey quitKey) */
extern int suspKey;                      /* hotkey for suspending (" suspKey) */
extern int captureKey;                   /* Toggle text capture key (" captureKey) */
extern int capture;                      /* Capture status */
extern int shellKey;                     /* hotkey for shelling out (" shellKey) */
extern char aryShell[PATH_MAX];          /* User's preferred aryShell */
extern int awayKey;                      /* Hotkey for isAway from keyboard */
extern int browserKey;                   /* Hotkey to spawn a Web aryBrowser */
extern char aryBrowser[PATH_MAX];        /* User's preferred Web aryBrowser */
extern char aryDefaultBrowser[PATH_MAX]; /* Detected system default aryBrowser */

#ifdef HAVE_OPENSSL
extern SSL *ssl; /* SSL connection */
#endif
extern int net;                               /* file descriptor of network socket */
extern FILE *netOutputFile;                   /* file pointer for output to net */
extern unsigned char aryNetInputBuffer[2048]; /* buffer for input from net */
/* This is necessary because we aren't using buffered input under VMS */
extern unsigned char *ptrNetInput;            /* buffer pointer for input from net */
extern ssize_t netInputLength;                /* length of current input buffer from net */
extern unsigned char aryPtyInputBuffer[1024]; /* buffer for input from pty */
extern unsigned char *ptrPtyInput;            /* buffer pointer for input from pty */
extern ssize_t ptyInputLength;                /* length of current input buffer from pty */

extern int rows;       /* number of rows on aryUser's screen */
extern int oldRows;    /* previous value of rows */
extern char lastColor; /* last color code received from BBS */

extern long byte;                         /* current byte (remotely synched with bbs) */
extern long targetByte;                   /* where the client wants to get */
extern long bytePosition;                 /* where the client is */
extern unsigned char arySavedBytes[1000]; /* buffer to arySavedBytes past aryUser bytes */
extern bool shouldIgnoreNetwork;          /* Temporarily don't check for network input */

extern char aryBbsFriendsName[PATH_MAX];       /* added friends file name */
extern FILE *bbsFriends;                       /* added friends file */
extern unsigned char arySavedWhoInfo[150][80]; /* added saved info array */
                                               /* for saved who list */
extern char aryExpressParsing[400];            /* for incoming X messages */
extern bool isLoginShell;                      /* whether this client is a login shell */
extern char aryKeyMap[128];                    /* key remapping array */
extern int version;                            /* Client version number */

/* Below variables were removed from telnet.c telReceive() */
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

#endif /* EXT_H_INCLUDED */
