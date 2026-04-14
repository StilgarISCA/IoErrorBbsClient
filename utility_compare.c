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
#include "proto.h"

static int compareFriendNames( const friend *ptrLeft, const friend *ptrRight );
static int compareStringPointers( const char *const *ptrLeft, const char *const *ptrRight );

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
   return compareStringPointers( (const char *const *)ptrLeft,
                                 (const char *const *)ptrRight );
}

int sortCompareVoid( const void *ptrLeft, const void *ptrRight )
{
   return compareStringPointers( (const char *const *)ptrLeft,
                                 (const char *const *)ptrRight );
}

int strCompareVoid( const void *ptrLeft, const void *ptrRight )
{
   return strcmp( (const char *)ptrLeft, (const char *)ptrRight );
}

static int compareFriendNames( const friend *ptrLeft, const friend *ptrRight )
{
   assert( ptrLeft->magic == 0x3231 );
   assert( ptrRight->magic == 0x3231 );

   return strcmp( ptrLeft->name, ptrRight->name );
}

static int compareStringPointers( const char *const *ptrLeft, const char *const *ptrRight )
{
   return strcmp( *ptrLeft, *ptrRight );
}

int fSortCompare( const friend *const *ptrLeft, const friend *const *ptrRight )
{
   return compareFriendNames( *ptrLeft, *ptrRight );
}

int fSortCompareVoid( const void *ptrLeft, const void *ptrRight )
{
   return fSortCompare( (const friend *const *)ptrLeft, (const friend *const *)ptrRight );
}
