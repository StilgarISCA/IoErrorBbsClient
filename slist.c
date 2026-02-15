/*
 * slist.c - Functions which maintain a sorted (non-linked) list of
 * arbitrary data.
 */

#include "defs.h"
#include "ext.h"
#include <stdarg.h>

/*
 * slistCreate creates a list with the given number of items already
 * allocated.  If the number of items is >0, the pointers must be
 * passed as arguments.
 */
slist *slistCreate( int nitems, int ( *sortfn )( const void *, const void * ), ... )
{
   int itemIndex;
   slist *ptrList;
   va_list argList;

   assert( nitems >= 0 );
   assert( sortfn );

   if ( !( ptrList = (slist *)calloc( 1, sizeof( slist ) ) ) )
   {
      return NULL;
   }
   ptrList->nitems = (unsigned int)nitems;
   ptrList->sortfn = sortfn;
   if ( nitems > 0 )
   {
      if ( !( ptrList->items = (void *)calloc( 1, (size_t)nitems * sizeof( void * ) ) ) )
      {
         return NULL;
      }
      va_start( argList, sortfn );
      for ( itemIndex = 0; itemIndex < nitems; itemIndex++ )
      {
         ptrList->items[itemIndex] = va_arg( argList, void * );
      }
      va_end( argList );
   }
   else
   {
      ptrList->items = NULL;
   }
   return ptrList;
}

/*
 * slistDestroy destroys a list.  It does not destroy the data items
 * in the list.
 */
void slistDestroy( slist *list )
{
   free( list->items );
   list->items = NULL;
   free( list );
}

/*
 * slistDestroyItems destroys the data items in a list.
 */
void slistDestroyItems( slist *list )
{
   unsigned int itemIndex;

   for ( itemIndex = 0; itemIndex < list->nitems; itemIndex++ )
   {
      free( list->items[itemIndex] );
      list->items[itemIndex] = NULL;
   }
}

/*
 * slistAddItem adds an item to the list.
 */
int slistAddItem( slist *list, void *item, int deferSort )
{
   void **ptrItems;

   list->nitems++;
   if ( !( ptrItems = (void *)realloc( list->items, list->nitems * sizeof( void * ) ) ) )
   {
      return 0;
   }
   list->items = ptrItems;
   list->items[list->nitems - 1] = item;
   if ( !deferSort )
   {
      slistSort( list );
   }
   return 1;
}

/*
 * slistRemoveItem removes an item from the list.  It does not free the
 * object being pointed to.
 */
int slistRemoveItem( slist *list, int item )
{
   void **ptrItems;
   unsigned int itemIndex;

   assert( list );
   assert( item >= 0 );
   assert( (unsigned int)item < list->nitems );

   printf( "slistRemoveItem(list, %d): nitems=%u\r\n", item, list->nitems );
   list->items[item] = NULL;
   if ( (unsigned int)item < --list->nitems )
   {
      for ( itemIndex = (unsigned int)item; itemIndex < list->nitems; itemIndex++ )
      {
         list->items[itemIndex] = list->items[itemIndex + 1];
      }
   }
   ptrItems = (void *)realloc( list->items, list->nitems * sizeof( void * ) );
   if ( !ptrItems && list->nitems )
   { /* request failed */
      return 0;
   }

   list->items = ptrItems;
   return 1;
}

/*
 * slistFind locates an item based on the results of the search function.
 * Returns entry on success, -1 on failure.  findfn() should compare a to b
 * and return <0 if a < b, >0 if a > b, or 0 if a == b.
 * Algorithm is a binary search.
 */
int slistFind( slist *list, void *toFind, int ( *findfn )( const void *, const void * ) )
{
   int midIndex;
   int upperBound;
   int lowerBound;
   int compareResult;

   assert( list );
   assert( findfn );

   if ( !toFind )
   { /* Fail if nothing to find */
      return -1;
   }
   if ( list->nitems == 0 )
   {
      return -1;
   }
   upperBound = (int)list->nitems - 1;
   lowerBound = 0;
   while ( upperBound >= lowerBound )
   {
      midIndex = ( upperBound + lowerBound ) / 2;
      compareResult = findfn( toFind, list->items[midIndex] );
      if ( compareResult == 0 )
      {
         return midIndex;
      }
      if ( compareResult < 0 )
      {
         upperBound = midIndex - 1;
      }
      else
      {
         lowerBound = midIndex + 1;
      }
   }
   return -1;
}

/*
 * slistSort sorts the list using qsort. qsort may not be the best choice,
 * but it's good-enough for the BBS client.
 */
void slistSort( slist *list )
{
   assert( list );

   qsort( list->items, list->nitems, sizeof( void * ), list->sortfn );
}

/*
 * slistIntersection creates the intersection of two slists, that is, the
 * list of items which appear on both lists.  We presume that we can create
 * the intersection if and only if the two lists use the same sortfn.  Data
 * members are copied using a shallow copy from list1, since this is typically
 * used for "throwaway" lists...  This function is designed to run in linear
 * linear time for list1 + list2.  Returns the list, an empty list if there
 * are no intersecting items, or NULL on error.  Do NOT destroy the list
 * items; they don't belong to you!
 */
slist *slistIntersection( const slist *list1, const slist *list2 )
{
   int leftIndex;        /* Count of items processed */
   int rightIndex;       /* Count of items processed */
   slist *ptrResultList; /* The list being created */

   if ( !list1 || !list2 || list1->sortfn != list2->sortfn )
   {
      return NULL;
   }
   assert( list1 );
   assert( list2 );
   assert( list1->sortfn == list2->sortfn );

   leftIndex = 0;
   rightIndex = 0;

   ptrResultList = slistCreate( 0, list1->sortfn );
   if ( !ptrResultList )
   {
      return NULL;
   }

   for ( ; leftIndex < (int)list1->nitems; leftIndex++ )
   {
      /*
   	 * Now run through list2 until we find either a matching
   	 * item, or an item that is greater than the one in list1
   	 * that we are currently looking at.
   	 */
      int compareResult;

      /* First item in n2 not less than current item n1 */
      while ( ( compareResult = ptrResultList->sortfn( list1->items[leftIndex], list2->items[rightIndex] ) ) < 0 )
      {
         rightIndex++;
         /* If this happens, we're done */
         if ( rightIndex > (int)list2->nitems )
         {
            break;
         }
      }

      /* If this happens, we're done; nothing else will match */
      if ( rightIndex > (int)list2->nitems )
      {
         break;
      }

      /* If item is not less than and not greater than, it's equal */
      if ( !( ptrResultList->sortfn( list2->items[rightIndex], list1->items[leftIndex] ) < 0 ) )
      {
         if ( !slistAddItem( ptrResultList, list1->items[leftIndex], 1 ) )
         {
            slistDestroy( ptrResultList );
            return NULL;
         }
      }
   }
   slistSort( ptrResultList );
   return ptrResultList;
}
