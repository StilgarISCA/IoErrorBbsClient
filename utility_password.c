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
/// @brief Decode a saved password with the legacy rolling ASCII algorithm.
///
/// This helper preserves the original 1995 algorithm by jhp (Marx Marvelous).
/// The scheme is intentionally weak and is kept only for backward
/// compatibility with existing saved passwords.
///
/// @param ptrDestination Output buffer that receives the decoded text.
/// @param src Encoded input string.
/// @param seedLength Initial rolling seed.
///
/// @return `ptrDestination`.
char *jhpdecode( char *ptrDestination, const char *src, size_t seedLength )
{
   char *ptrDestIterator; // dest iterator
   char inputChar;        // a single character

   ptrDestIterator = ptrDestination;
   while ( ( inputChar = *src++ ) != 0 )
   {
      seedLength = ( seedLength + inputChar - 32 ) % 95;
      *ptrDestIterator++ = seedLength + 32;
   }
   *ptrDestIterator = 0;
   return ptrDestination;
}

/// @brief Encode a password with the legacy rolling ASCII algorithm.
///
/// This helper preserves the original 1995 algorithm by jhp (Marx Marvelous).
/// The scheme is intentionally weak and is kept only for backward
/// compatibility with existing saved passwords.
///
/// @param ptrDestination Output buffer that receives the encoded text.
/// @param src Plaintext input string.
/// @param seedLength Initial rolling seed.
///
/// @return `ptrDestination`.
char *jhpencode( char *ptrDestination, const char *src, size_t seedLength )
{
   char *ptrDestIterator; // dest iterator
   char inputChar;        // a single character

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
