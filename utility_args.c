/*
 * Copyright (C) 2024-2026 Stilgar
 * Copyright (C) 1995-2003 Michael Hampton
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

/*
 * This file handles command-line argument parsing.
 */
#include "client.h"
#include "config_globals.h"
#include "defs.h"
/// @brief Parse optional command-line host and port overrides.
///
/// @param argc Argument count from `main()`.
/// @param argv Argument vector from `main()`.
///
/// @return This function does not return a value.
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
}
