/*
 * Copyright (C) 2024-2026 Stilgar
 * Copyright (C) 1995-2003 Michael Hampton
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

/*
 * This file opens .bbsrc and legacy friends files.
 */
#include "defs.h"
#include "bbsrc.h"
#include "client.h"
#include "config_globals.h"
#include "utility.h"

/// @brief Open the legacy friends file if it exists.
///
/// @return A readable stream for `aryBbsFriendsName`, or `NULL` if the file
/// could not be opened.
FILE *openBbsFriends( void )
{
   FILE *ptrFileHandle;

   ptrFileHandle = fopen( aryBbsFriendsName, "r" );
   return ( ptrFileHandle );
}


/// @brief Open the main client configuration file.
///
/// The function first tries read-write access, then creates the file if needed,
/// and finally falls back to read-only access with a warning.
///
/// @return A stream for `aryBbsRcName`, or `NULL` if the file could not be
/// opened at all.
FILE *openBbsRc( void )
{
   FILE *ptrFileHandle;
   int savedErrno;

   ptrFileHandle = fopen( aryBbsRcName, "r+" );
   if ( !ptrFileHandle )
   {
      savedErrno = errno;
      ptrFileHandle = fopen( aryBbsRcName, "w+" );
   }
   if ( !ptrFileHandle )
   {
      ptrFileHandle = fopen( aryBbsRcName, "r" );
      if ( ptrFileHandle )
      {
         isBbsRcReadOnly = 1;
         errno = savedErrno;
         sPerror( "Configuration is read-only", "Warning" );
      }
      else
      {
         sPerror( "Can't open configuration file", "Warning" );
      }
   }
   return ( ptrFileHandle );
}
