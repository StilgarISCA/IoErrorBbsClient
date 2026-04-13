/*
 * Copyright (C) 2024-2026 Stilgar
 * Copyright (C) 1995-2003 Michael Hampton
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

/*
 * This file handles the legacy saved-password encoding helpers.
 */
#include "defs.h"

#ifdef ENABLE_SAVE_PASSWORD
/*
 * Encode/decode password with a simple algorithm.
 * jhp 5Feb95 (Marx Marvelous)
 *
 * This code is horribly insecure.  Don't use it for any passwords
 * you care about!  Also note it's closely tied to ASCII and won't
 * work with a non-ASCII system.  - IO
 */
char *jhpdecode( char *ptrDestination, const char *src, size_t seedLength )
{
   char *ptrDestIterator; /* dest iterator */
   char inputChar;        /* a single character */

   ptrDestIterator = ptrDestination;
   while ( ( inputChar = *src++ ) != 0 )
   {
      seedLength = ( seedLength + inputChar - 32 ) % 95;
      *ptrDestIterator++ = seedLength + 32;
   }
   *ptrDestIterator = 0;
   return ptrDestination;
}

char *jhpencode( char *ptrDestination, const char *src, size_t seedLength )
{
   char *ptrDestIterator; /* dest iterator */
   char inputChar;        /* a single character */

   ptrDestIterator = ptrDestination;
   while ( ( inputChar = *src++ ) != 0 )
   {
      *ptrDestIterator++ = ( inputChar - 32 - seedLength + 95 ) % 95 + 32;
      seedLength = inputChar - 32;
   }
   *ptrDestIterator = 0;
   return ptrDestination;
}
#endif
