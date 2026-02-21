/*
 * Copyright (C) 2024-2026 Stilgar
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>

#include "defs.h"
#include "proto.h"
#include "test_helpers.h"

static void slistCreate_WhenCalledWithZeroItems_ReturnsEmptyList( void **state )
{
   // Arrange
   slist *ptrList;

   (void)state;

   // Act
   ptrList = slistCreate( 0, compareStringPointer );

   // Assert
   if ( ptrList == NULL )
   {
      fail_msg( "slistCreate should not return NULL for an empty list" );
   }
   if ( ptrList->nitems != 0 )
   {
      fail_msg( "slistCreate empty list should have 0 items; got %u", ptrList->nitems );
   }
   if ( ptrList->items != NULL )
   {
      fail_msg( "slistCreate empty list should initialize items to NULL" );
   }

   slistDestroy( ptrList );
}

static void slistAddItem_WhenDeferSortIsOff_KeepsListSorted( void **state )
{
   // Arrange
   slist *ptrList;
   char *ptrBeta;
   char *ptrAlpha;
   char *ptrCharlie;
   int addBetaResult;
   int addAlphaResult;
   int addCharlieResult;

   (void)state;

   ptrList = slistCreate( 0, compareStringPointer );
   if ( ptrList == NULL )
   {
      fail_msg( "slistCreate failed in Arrange for sorted add test" );
   }

   ptrBeta = duplicateStringOrFail( "beta", "slistAddItem sorted add test" );
   ptrAlpha = duplicateStringOrFail( "alpha", "slistAddItem sorted add test" );
   ptrCharlie = duplicateStringOrFail( "charlie", "slistAddItem sorted add test" );

   // Act
   addBetaResult = slistAddItem( ptrList, ptrBeta, 0 );
   addAlphaResult = slistAddItem( ptrList, ptrAlpha, 0 );
   addCharlieResult = slistAddItem( ptrList, ptrCharlie, 0 );

   // Assert
   if ( addBetaResult != 1 || addAlphaResult != 1 || addCharlieResult != 1 )
   {
      fail_msg( "slistAddItem should succeed; got results %d, %d, %d",
                addBetaResult, addAlphaResult, addCharlieResult );
   }
   if ( ptrList->nitems != 3 )
   {
      fail_msg( "expected 3 list items after adds; got %u", ptrList->nitems );
   }
   if ( strcmp( ptrList->items[0], "alpha" ) != 0 ||
        strcmp( ptrList->items[1], "beta" ) != 0 ||
        strcmp( ptrList->items[2], "charlie" ) != 0 )
   {
      fail_msg( "list items are not sorted as expected: alpha, beta, charlie" );
   }

   slistDestroyItems( ptrList );
   slistDestroy( ptrList );
}

static void slistFind_WhenItemExists_ReturnsMatchingIndex( void **state )
{
   // Arrange
   slist *ptrList;
   int foundIndex;

   (void)state;

   ptrList = slistCreate( 0, compareStringPointer );
   if ( ptrList == NULL )
   {
      fail_msg( "slistCreate failed in Arrange for slistFind existing-item test" );
   }
   slistAddItem( ptrList, duplicateStringOrFail( "beta", "slistFind existing-item test" ), 0 );
   slistAddItem( ptrList, duplicateStringOrFail( "alpha", "slistFind existing-item test" ), 0 );
   slistAddItem( ptrList, duplicateStringOrFail( "charlie", "slistFind existing-item test" ), 0 );

   // Act
   foundIndex = slistFind( ptrList, "beta", compareStringItem );

   // Assert
   if ( foundIndex != 1 )
   {
      fail_msg( "slistFind should return index 1 for 'beta'; got %d", foundIndex );
   }

   slistDestroyItems( ptrList );
   slistDestroy( ptrList );
}

static void slistFind_WhenItemDoesNotExist_ReturnsMinusOne( void **state )
{
   // Arrange
   slist *ptrList;
   int foundIndex;

   (void)state;

   ptrList = slistCreate( 0, compareStringPointer );
   if ( ptrList == NULL )
   {
      fail_msg( "slistCreate failed in Arrange for slistFind missing-item test" );
   }
   slistAddItem( ptrList, duplicateStringOrFail( "alpha", "slistFind missing-item test" ), 0 );
   slistAddItem( ptrList, duplicateStringOrFail( "charlie", "slistFind missing-item test" ), 0 );

   // Act
   foundIndex = slistFind( ptrList, "beta", compareStringItem );

   // Assert
   if ( foundIndex != -1 )
   {
      fail_msg( "slistFind should return -1 for a missing item; got %d", foundIndex );
   }

   slistDestroyItems( ptrList );
   slistDestroy( ptrList );
}

static void slistRemoveItem_WhenMiddleItemRemoved_ShiftsRemainingItems( void **state )
{
   // Arrange
   slist *ptrList;
   char *ptrRemovedItem;
   int removeResult;

   (void)state;

   ptrList = slistCreate( 0, compareStringPointer );
   if ( ptrList == NULL )
   {
      fail_msg( "slistCreate failed in Arrange for slistRemoveItem test" );
   }
   slistAddItem( ptrList, duplicateStringOrFail( "alpha", "slistRemoveItem middle-item test" ), 1 );
   slistAddItem( ptrList, duplicateStringOrFail( "beta", "slistRemoveItem middle-item test" ), 1 );
   slistAddItem( ptrList, duplicateStringOrFail( "charlie", "slistRemoveItem middle-item test" ), 1 );
   slistSort( ptrList );

   ptrRemovedItem = ptrList->items[1];

   // Act
   removeResult = slistRemoveItem( ptrList, 1 );

   // Assert
   if ( removeResult != 1 )
   {
      fail_msg( "slistRemoveItem should return 1 on success; got %d", removeResult );
   }
   if ( ptrList->nitems != 2 )
   {
      fail_msg( "slistRemoveItem should leave 2 items after removal; got %u", ptrList->nitems );
   }
   if ( strcmp( ptrList->items[0], "alpha" ) != 0 || strcmp( ptrList->items[1], "charlie" ) != 0 )
   {
      fail_msg( "slistRemoveItem should shift items to alpha, charlie order" );
   }

   free( ptrRemovedItem );
   slistDestroyItems( ptrList );
   slistDestroy( ptrList );
}

static void slistDestroyItems_WhenCalled_ClearsItemPointers( void **state )
{
   // Arrange
   slist *ptrList;

   (void)state;

   ptrList = slistCreate( 0, compareStringPointer );
   if ( ptrList == NULL )
   {
      fail_msg( "slistCreate failed in Arrange for slistDestroyItems test" );
   }
   slistAddItem( ptrList, duplicateStringOrFail( "alpha", "slistDestroyItems test" ), 1 );
   slistAddItem( ptrList, duplicateStringOrFail( "beta", "slistDestroyItems test" ), 1 );

   // Act
   slistDestroyItems( ptrList );

   // Assert
   if ( ptrList->items[0] != NULL || ptrList->items[1] != NULL )
   {
      fail_msg( "slistDestroyItems should clear item pointers to NULL after free" );
   }

   slistDestroy( ptrList );
}

int main( void )
{
   const struct CMUnitTest aryTests[] = {
      cmocka_unit_test( slistCreate_WhenCalledWithZeroItems_ReturnsEmptyList ),
      cmocka_unit_test( slistAddItem_WhenDeferSortIsOff_KeepsListSorted ),
      cmocka_unit_test( slistFind_WhenItemExists_ReturnsMatchingIndex ),
      cmocka_unit_test( slistFind_WhenItemDoesNotExist_ReturnsMinusOne ),
      cmocka_unit_test( slistRemoveItem_WhenMiddleItemRemoved_ShiftsRemainingItems ),
      cmocka_unit_test( slistDestroyItems_WhenCalled_ClearsItemPointers ),
   };

   return cmocka_run_group_tests( aryTests, NULL, NULL );
}
