/*
 * Copyright (C) 2024-2026 Stilgar
 * Copyright (C) 1995-2003 Michael Hampton
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

/*
 * This file handles command-line argument parsing.
 */
#include "defs.h"
#include "ext.h"
#include "proto.h"

/*
 * Process command line arguments.  argv[1] is an alternate host, if present,
 * and argv[2] is an alternate port, if present, and argv[1] is also present.
 */
void arguments( int argc, char **argv )
{
   if ( argc > 1 )
   {
      snprintf( aryCommandLineHost, sizeof( aryCommandLineHost ), "%s", argv[1] );
   }
   else
   {
      *aryCommandLineHost = 0;
   }
   if ( argc > 2 )
   {
      cmdLinePort = (unsigned short)atoi( argv[2] );
   }
   else
   {
      cmdLinePort = 0;
   }
   if ( argc > 3 )
   {
      if ( !strncmp( argv[3], "secure", 6 ) || !strncmp( argv[3], "ssl", 6 ) )
      {
         shouldUseSsl = 1;
      }
      else
      {
         shouldUseSsl = 0;
      }
   }
}
