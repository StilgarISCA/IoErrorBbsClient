/*
 * Copyright (C) 2024-2026 Stilgar
 * Copyright (C) 1995-2003 Michael Hampton
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef NETWORK_GLOBALS_H_INCLUDED
#define NETWORK_GLOBALS_H_INCLUDED

#include "defs.h"

#ifdef HAVE_OPENSSL
extern SSL *ssl; // SSL connection
#endif
extern bool arySavedByteCanReplay[1000];      // true if the saved byte can be replayed
extern unsigned char aryNetInputBuffer[2048]; // buffer for input from net
extern unsigned char aryPtyInputBuffer[1024]; // buffer for input from pty
extern unsigned char arySavedBytes[1000];     // buffer to arySavedBytes past aryUser bytes

extern long byte;                     // current byte (remotely synched with bbs)
extern long bytePosition;             // where the client is
extern int lastInteractiveInputChar;  // last interactive character sent to BBS
extern int net;                       // file descriptor of network socket
extern ssize_t netInputLength;        // length of current input buffer from net
extern FILE *netOutputFile;           // file pointer for output to net
extern unsigned char *ptrNetInput;    // buffer pointer for input from net
extern unsigned char *ptrPtyInput;    // buffer pointer for input from pty
extern ssize_t ptyInputLength;        // length of current input buffer from pty
extern bool shouldIgnoreNetwork;      // Temporarily skip network input checks
extern int suppressedPromptInputChar; // replayed prompt input character to ignore
extern long targetByte;               // where the client wants to get
extern bool wasLastInputReplayed;     // true if the last input came from replay

#endif // NETWORK_GLOBALS_H_INCLUDED
