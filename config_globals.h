/*
 * Copyright (C) 2024-2026 Stilgar
 * Copyright (C) 1995-2003 Michael Hampton
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef CONFIG_GLOBALS_H_INCLUDED
#define CONFIG_GLOBALS_H_INCLUDED

#include "defs.h"

extern char aryBbsRcName[PATH_MAX];      // bbsrc filename (usually ~/.bbsrc)
extern FILE *ptrBbsRc;                   // file descriptor of .bbsrc file
extern bool isBbsRcReadOnly;             // set if .bbsrc file is read-only
extern char aryTempFileName[PATH_MAX];   // bbstmp filename (usually ~/.bbstmp)
extern FILE *tempFile;                   // file pointer to above
extern char aryBbsFriendsName[PATH_MAX]; // added friends file name
extern FILE *bbsFriends;                 // added friends file

extern char aryBbsHost[64];         // name of bbs host (bbs.iscabbs.com)
extern char aryCommandLineHost[64]; // name of bbs host from command line
extern unsigned short bbsPort;      // port to connect to (23 or 992)
extern unsigned short cmdLinePort;  // port to connect to from command line
extern bool shouldUseSsl;           // Whether the connection should be secure
extern bool isSsl;                  // Whether the current connection is secured

extern char aryMacro[128][72];          // array for macros
extern char aryAwayMessageLines[6][80]; // Away from keyboard message
extern int commandKey;                  // hotkey for signalling a aryMacro follows
extern int quitKey;                     // hotkey to quit (commandKey quitKey)
extern int suspKey;                     // hotkey for suspending (" suspKey)
extern int captureKey;                  // Toggle text capture key (" captureKey)
extern int capture;                     // Capture status
extern int shellKey;                    // hotkey for shelling out (" shellKey)
extern char aryShell[PATH_MAX];         // Shell command launched by the client
extern int awayKey;                     // Hotkey for isAway from keyboard
extern int browserKey;                  // Hotkey to launch web browser
extern char aryKeyMap[128];             // key remapping array

#endif // CONFIG_GLOBALS_H_INCLUDED
