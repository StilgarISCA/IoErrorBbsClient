/*
 * Copyright (C) 2024-2026 Stilgar
 * Copyright (C) 1995-2003 Michael Hampton
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

/*
 * Program entry point.
 */
#include "defs.h"
#include "client_globals.h"
#include "proto.h"

int main( int argc, char *argv[] )
{
   aryEscape[0] = '\033';
   aryEscape[1] = '\0';
   if ( *argv[0] == '-' )
   {
      isLoginShell = 1;
   }
   else
   {
      isLoginShell = 0;
   }
   initialize();
   findHome();
   readBbsRc();
   openTmpFile();
   arguments( argc, argv );
   connectBbs();
   sigInit();
   telInit();
   setTerm();
   looper();
   exit( 0 );
   return ( 0 );
}
