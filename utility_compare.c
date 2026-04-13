/*
 * Copyright (C) 2024-2026 Stilgar
 * Copyright (C) 1995-2003 Michael Hampton
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

/*
 * This file handles comparison helpers used by lists and searches.
 */
#include "defs.h"
#include "ext.h"

int fStrCompare( const char *ptrName, const friend *ptrFriend )
{
   return strcmp( ptrName, ptrFriend->name );
}

int fStrCompareVoid( const void *ptrName, const void *ptrFriend )
{
   return fStrCompare( (const char *)ptrName, (const friend *)ptrFriend );
}

int sortCompare( char **ptrLeft, char **ptrRight )
{
   return strcmp( *ptrLeft, *ptrRight );
}

int sortCompareVoid( const void *ptrLeft, const void *ptrRight )
{
   const char *const *ptrLeftString = (const char *const *)ptrLeft;
   const char *const *ptrRightString = (const char *const *)ptrRight;
   return strcmp( *ptrLeftString, *ptrRightString );
}

int strCompareVoid( const void *ptrLeft, const void *ptrRight )
{
   return strcmp( (const char *)ptrLeft, (const char *)ptrRight );
}

int fSortCompare( const friend *const *ptrLeft, const friend *const *ptrRight )
{
   assert( ( *ptrLeft )->magic == 0x3231 );
   assert( ( *ptrRight )->magic == 0x3231 );

   return strcmp( ( *ptrLeft )->name, ( *ptrRight )->name );
}

int fSortCompareVoid( const void *ptrLeft, const void *ptrRight )
{
   return fSortCompare( (const friend *const *)ptrLeft, (const friend *const *)ptrRight );
}
