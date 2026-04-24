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

/*
 * Opens the bbsfriends file, warning the user if it can't be opened or can't
 * be opened for write, returning the file pointer if it was opened
 * successfully.
 */
FILE *openBbsFriends( void )
{
   FILE *ptrFileHandle;

   ptrFileHandle = fopen( aryBbsFriendsName, "r" );
   return ( ptrFileHandle );
}


/*
 * Opens the bbsrc file, warning the user if it can't be opened or can't be
 * opened for write, returning the file pointer if it was opened successfully.
 */
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

