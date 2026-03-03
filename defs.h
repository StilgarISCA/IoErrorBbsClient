/*
 * Copyright (C) 2024-2026 Stilgar
 * Copyright (C) 1995-2003 Michael Hampton
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

/*
 * This is where I've put all the #include files, to keep them separate in a
 * single location.  Pure C stuff goes here, the system-specific stuff is kept
 * over in unix.h.
 */

#ifndef DEFS_H_INCLUDED
#define DEFS_H_INCLUDED

#define VERSION "2.3.10-Stilgar"
#define INT_VERSION 2310

#ifndef DEBUG
#define DEBUG 0
#endif
#include "config.h"

#ifdef STDC_HEADERS
#include <stdlib.h>
#endif
#include <sys/types.h>
#ifdef HAVE_SYS_TIME_H
#include <sys/time.h>
#ifdef TIME_WITH_SYS_TIME
#include <time.h>
#endif
#else
#ifdef HAVE_TIME_H
#include <time.h>
#endif
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>
#include <setjmp.h>
#include <signal.h>
#include <assert.h>
#include <limits.h>

#ifndef RETSIGTYPE
#define RETSIGTYPE void
#endif

#ifndef SEEK_SET
#define SEEK_SET 0
#define SEEK_CUR 1
#define SEEK_END 2
#endif

#ifdef HAVE_OPENSSL
#include <openssl/ssl.h>
#endif

/*
 * Use sigsetjmp/siglongjmp behavior when available so signal masks are
 * preserved across jump boundaries.
 */
#define USE_POSIX_SIGSETJMP 1

#include <errno.h>
/* extern int errno; */

#include "sysio.h"

#define COLOR_VALUE_DEFAULT 256

static inline size_t appendAnsiColorSelector( char *ptrBuffer, size_t bufferSize,
                                              size_t writeOffset, int colorValue,
                                              bool isBackground )
{
   if ( writeOffset >= bufferSize )
   {
      return writeOffset;
   }
   if ( colorValue == COLOR_VALUE_DEFAULT )
   {
      return writeOffset + (size_t)snprintf( ptrBuffer + writeOffset,
                                             bufferSize - writeOffset,
                                             "%d",
                                             isBackground ? 49 : 39 );
   }
   if ( colorValue >= 0 && colorValue <= 7 )
   {
      return writeOffset + (size_t)snprintf( ptrBuffer + writeOffset,
                                             bufferSize - writeOffset,
                                             "%d",
                                             ( isBackground ? 40 : 30 ) + colorValue );
   }
   if ( colorValue >= 8 && colorValue <= 15 )
   {
      return writeOffset + (size_t)snprintf( ptrBuffer + writeOffset,
                                             bufferSize - writeOffset,
                                             "%d",
                                             ( isBackground ? 100 : 90 ) + ( colorValue - 8 ) );
   }

   return writeOffset + (size_t)snprintf( ptrBuffer + writeOffset,
                                          bufferSize - writeOffset,
                                          "%d;5;%d",
                                          isBackground ? 48 : 38,
                                          colorValue );
}

static inline int formatAnsiForegroundSequence( char *ptrBuffer, size_t bufferSize,
                                                int colorValue )
{
   size_t safeOffset;
   size_t writeOffset;

   if ( bufferSize == 0 )
   {
      return 0;
   }

   writeOffset = (size_t)snprintf( ptrBuffer, bufferSize, "\033[" );
   writeOffset = appendAnsiColorSelector( ptrBuffer, bufferSize, writeOffset,
                                          colorValue, false );
   safeOffset = writeOffset < bufferSize ? writeOffset : bufferSize - 1;
   snprintf( ptrBuffer + safeOffset, writeOffset < bufferSize ? bufferSize - writeOffset : 0,
             "m" );
   return 1;
}

static inline int formatAnsiBackgroundSequence( char *ptrBuffer, size_t bufferSize,
                                                int colorValue )
{
   size_t safeOffset;
   size_t writeOffset;

   if ( bufferSize == 0 )
   {
      return 0;
   }

   writeOffset = (size_t)snprintf( ptrBuffer, bufferSize, "\033[" );
   writeOffset = appendAnsiColorSelector( ptrBuffer, bufferSize, writeOffset,
                                          colorValue, true );
   safeOffset = writeOffset < bufferSize ? writeOffset : bufferSize - 1;
   snprintf( ptrBuffer + safeOffset, writeOffset < bufferSize ? bufferSize - writeOffset : 0,
             "m" );
   return 1;
}

static inline int formatAnsiDisplayStateSequence( char *ptrBuffer, size_t bufferSize,
                                                  int foregroundColor,
                                                  int backgroundColor,
                                                  bool shouldUseBold )
{
   size_t safeOffset;
   size_t writeOffset;

   if ( bufferSize == 0 )
   {
      return 0;
   }

   writeOffset = (size_t)snprintf( ptrBuffer, bufferSize, "\033[%d;",
                                   shouldUseBold ? 1 : 0 );
   writeOffset = appendAnsiColorSelector( ptrBuffer, bufferSize, writeOffset,
                                          foregroundColor, false );
   if ( writeOffset < bufferSize )
   {
      writeOffset += (size_t)snprintf( ptrBuffer + writeOffset,
                                       bufferSize - writeOffset,
                                       ";" );
   }
   else
   {
      writeOffset++;
   }
   writeOffset = appendAnsiColorSelector( ptrBuffer, bufferSize, writeOffset,
                                          backgroundColor, true );
   safeOffset = writeOffset < bufferSize ? writeOffset : bufferSize - 1;
   snprintf( ptrBuffer + safeOffset, writeOffset < bufferSize ? bufferSize - writeOffset : 0,
             "m" );
   return 1;
}

static inline int formatAnsiResetSequence( char *ptrBuffer, size_t bufferSize )
{
   if ( bufferSize == 0 )
   {
      return 0;
   }

   snprintf( ptrBuffer, bufferSize, "\033[0;39;49m" );
   return 1;
}

typedef struct
{
   char *start;   /* Pointer to beginning of queue */
   int head;      /* Index of current head */
   int tail;      /* Index of current tail */
   int size;      /* Number of objects queue can hold */
   int itemCount; /* Number of objects queued */
   int objsize;   /* Size of one object */
} queue;

#define CTRL_D 4
#define TAB 9
#define CTRL_R 18
#define CTRL_U 21
#define CTRL_W 23
#define CTRL_X 24
#define CTRL_Z 26
#define ESC 27
#define DEL 127

#define ASCII_PRINTABLE_MIN 32
#define ASCII_PRINTABLE_MAX 127
#define ALLOWED_INPUT_CONTROL_CHARS "\3\4\5\b\n\r\27\30\32"

#define BBS_HOSTNAME "bbs.iscabbs.com"
#define BBS_IP_ADDRESS "64.198.88.46"
#define BBS_PORT_NUMBER 23
#define SSL_PORT_NUMBER 992

/* sendingXState defines */
#define SX_WANT_TO 5
#define SENDING_X_STATE_SENT_COMMAND_X 1
#define SX_SENT_NAME 2
#define SX_REPLYING 3
#define SX_SEND_NEXT 8
#define SX_NOT 0

/* Color transform defines */
#define CX_NORMAL 0
#define CX_POST 1
#define CX_EXPRESS 2
#define CX_INFO 3 /* not yet used */

#define MAX_USER_NAME_HISTORY_COUNT 20
#define WINDOW_ROWS_DEFAULT 24
#define WINDOW_ROWS_MIN 5
#define WINDOW_ROWS_MAX 120
#define NAWS_ROWS_MIN 10
#define NAWS_ROWS_MAX 110
typedef struct
{
   unsigned int isPosting : 1;                  /* true if aryUser is currently posting */
   unsigned int isLastSave : 1;                 /* true if last time aryUser edited they saved */
   unsigned int shouldCheckExpress : 1;         /* true if waiting to check BBS for X's */
   unsigned int isConfigMode : 1;               /* true if we are in bbsrc config funcs */
   unsigned int useAnsi : 1;                    /* true if BBS is in ANSI color mode */
   unsigned int useBold : 1;                    /* true if using bold in ANSI color mode */
   unsigned int shouldDisableBold : 1;          /* true if we need to force bold ANSI off */
   unsigned int isMorePromptActive : 1;         /* true if we are inside a MORE prompt */
   unsigned int shouldSquelchPost : 1;          /* true if we should squelch enemy posts */
   unsigned int shouldSquelchExpress : 1;       /* true if we should squelch enemy express */
   unsigned int shouldAutoAnswerAnsiPrompt : 1; /* true if we automatically answer ANSI ? */
   unsigned int shouldUseTcpKeepalive : 1;      /* true if TCP keepalive probes are enabled */
   unsigned int shouldEnableClickableUrls : 1;  /* true if OSC-8 clickable URL output is enabled */
   unsigned int isScreenReaderModeEnabled : 1;  /* true if screen reader friendly mode is enabled */
   unsigned int hasScreenReaderModeSetting : 1; /* true if screen reader mode was set in .bbsrc */
} Flags;

typedef struct
{
   int ( *sortfn )( const void *, const void * ); /* function to sort list; see slist.c */
   unsigned int nitems;                           /* number of items in list */
   void **items;                                  /* dynamic array containing item pointers */
} slist;

typedef struct
{
   int magic;     /* Magic number */
   char name[21]; /* User name */
   char info[54]; /* Friend description */
   time_t time;   /* Time online */
} friend;         /* User list entry */

#define COLOR_FIELD_COUNT 24
#define COLOR_BACKGROUND_INDEX 17

typedef struct
{
   int text;           /* Plain text color */
   int forum;          /* Forum prompt color */
   int number;         /* Numbers and Read cmd prompt color */
   int errorTextColor; /* Warning/error messages color */
   int reserved1;
   int reserved2;
   int reserved3;
   int postdate;       /* Post date stamp color */
   int postname;       /* Post author name color */
   int posttext;       /* Post text color */
   int postfrienddate; /* Post friend date stamp color */
   int postfriendname; /* Post friend name color */
   int postfriendtext; /* Post friend text color */
   int anonymous;      /* Anonymous post header color */
   int moreprompt;     /* More prompt color */
   int reserved4;
   int reserved5;
   int background;        /* Background color */
   int input1;            /* Text input fields */
   int input2;            /* Text input fields (highlight) */
   int expresstext;       /* X message text color */
   int expressname;       /* X message name color */
   int expressfriendtext; /* X message from friend text color */
   int expressfriendname; /* X message from friend name color*/
} Color;

#include "proto.h"

#endif /* DEFS_H_INCLUDED */
