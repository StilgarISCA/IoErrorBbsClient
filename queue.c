/* queue.c
 * Fns. for manipulating queue objects
 */

#include "defs.h"
#include "ext.h"

/* Make a queue containing nobjs objects of size size.  Return a pointer to
 * the queue or NULL if it could not be created.
 */
queue *newQueue( int size, int nobjs )
{
   queue *ptrQueue;

   if ( !( ptrQueue = (queue *)calloc( 1, sizeof( queue ) + (size_t)size * (size_t)nobjs ) ) )
   {
      return (queue *)NULL;
   }

   ptrQueue->start = (char *)( ptrQueue + 1 );
   ptrQueue->size = nobjs;
   ptrQueue->objsize = size;
   ptrQueue->head = 0;
   ptrQueue->tail = 0;
   ptrQueue->nobjs = 0;

   return ptrQueue;
}

/* Delete a queue and free the memory.  Does not delete the queue if it still
 * contains any objects.  Returns 1 if the queue was deleted and 0 if not.
 */
int safeDeleteQueue( queue *ptrQueue )
{
   if ( ptrQueue->nobjs )
   {
      return 0;
   }
   free( ptrQueue );
   return 1;
}

/* Delete a queue and free the memory.  Always deletes the queue and returns
 * 0 on success.
 */
int deleteQueue( queue *ptrQueue )
{
   free( ptrQueue );
   return 0;
}

/* Insert an object into the queue.  obj is a pointer to the object, and
 * q is a pointer to the queue.  Returns 1 on success, 0 if the queue is
 * full.
 */
int pushQueue( const char *ptrObject, queue *ptrQueue )
{
   int remainingBytes;
   char *ptrQueueWrite; /* Pointer into the queue */

   if ( ptrQueue->nobjs >= ptrQueue->size )
   { /* Is the queue full? */
      return 0;
   }

   ptrQueue->nobjs++;

   /* Find the target address within the queue to insert object. */
   ptrQueueWrite = ptrQueue->start + ptrQueue->objsize * ptrQueue->tail;

#if DEBUG
   stdPrintf( "{Queuing %s, %d objects} ", ptrObject, ptrQueue->nobjs );
#endif
   /* Copy the object into its queue position */
   for ( remainingBytes = ptrQueue->objsize; --remainingBytes >= 0; *ptrQueueWrite++ = *ptrObject++ )
   {
      ;
   }

   /* Wrap around if we've gone past the end of the queue. */
   if ( ++ptrQueue->tail >= ptrQueue->size )
   {
      ptrQueue->tail = 0;
   }

   return 1;
}

/* Remove an object from the queue.  q is a pointer to the queue.  The object
 * is copied into the location pointed to by obj.  Returns 0 if the queue is
 * empty, 1 on success.
 */
int popQueue( char *ptrObject, queue *ptrQueue )
{
   int remainingBytes;
   const char *ptrQueueRead; /* Pointer into the queue */

   if ( ptrQueue->nobjs <= 0 )
   {
      return ptrQueue->nobjs = 0; /* Queue is empty */
   }

   ptrQueue->nobjs--; /* Removing an object... */

   /* Find the object within the queue. */
   ptrQueueRead = ptrQueue->start + ( ptrQueue->objsize * ptrQueue->head );

#if DEBUG
   stdPrintf( "{Dequeuing %s, %d objects}\r\n", ptrQueueRead, ptrQueue->nobjs );
#endif
   /* Copy the object. */
   for ( remainingBytes = ptrQueue->objsize; --remainingBytes >= 0; *ptrObject++ = *ptrQueueRead++ )
   {
      ;
   }

   if ( ++ptrQueue->head >= ptrQueue->size )
   {
      ptrQueue->head = 0;
   }

   return 1;
}

/* isQueued checks to see if a character string is currently queued.
 * Returns 1 if the string is queued, 0 if not.
 */
int isQueued( const char *ptrObject, queue *ptrQueue )
{
   char *ptrQueueEntry; /* Pointer inside queue */
   int objectIndex;     /* Object counter */

   /* Move to head of queue. */
   for ( ptrQueueEntry = ptrQueue->start + ( ptrQueue->objsize * ptrQueue->head ), objectIndex = 0; objectIndex < ptrQueue->nobjs; objectIndex++ )
   {
      /* Do the comparison. */
      if ( !strcmp( ptrQueueEntry, ptrObject ) )
      {
         return 1;
      }
      ptrQueueEntry += ptrQueue->objsize;
      if ( ptrQueueEntry >= (char *)( ptrQueue->start + ( ptrQueue->objsize * ptrQueue->size ) ) )
      {
         ptrQueueEntry = ptrQueue->start;
      }
   }
   return 0;
}
