/*
 * Copyright (C) 2024-2026 Stilgar
 * Copyright (C) 1995-2003 Michael Hampton
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

/*
 * This is the file for I/O defines, different systems do certain things
 * differently, such system-specific stuff should be put here.
 */

#ifndef SYSIO_H_INCLUDED
#define SYSIO_H_INCLUDED

static inline bool isPtyInputAvailable( void )
{
   return ptrPtyInput - aryPtyInputBuffer < ptyInputLength;
}

static inline int ptyget( void )
{
   if ( isPtyInputAvailable() )
   {
      return *ptrPtyInput++;
   }

   ptyInputLength = read( 0, aryPtyInputBuffer, sizeof( aryPtyInputBuffer ) );
   if ( ptyInputLength < 0 )
   {
      return -1;
   }

   ptrPtyInput = aryPtyInputBuffer;
   return *ptrPtyInput++;
}

static inline bool isNetworkInputAvailable( void )
{
   return ptrNetInput - aryNetInputBuffer < netInputLength;
}

static inline int netget( void )
{
   if ( isNetworkInputAvailable() )
   {
      return *ptrNetInput++;
   }

   netInputLength = read( net, aryNetInputBuffer, sizeof( aryNetInputBuffer ) );
   if ( netInputLength <= 0 )
   {
      return -1;
   }

   ptrNetInput = aryNetInputBuffer;
   return *ptrNetInput++;
}

static inline int netput( int inputChar )
{
   return putc( inputChar, netOutputFile );
}

static inline int netflush( void )
{
   return fflush( netOutputFile );
}

#endif /* SYSIO_H_INCLUDED */
