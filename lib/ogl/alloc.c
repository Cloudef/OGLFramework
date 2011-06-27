#include "core.h"
#include "types.h"
#include "logfile_wrapper.h"

#include <malloc.h>
#include <string.h>

/* internal calloc function.
 * keep track of allocations and prints output on failure */
void* glMalloc( size_t size )
{
   void *ptr = malloc( size );
   if(!ptr)
   {
      logRed();
      glPrint("[ALLOC] failed to allocate %lu bytes\n", (size_t)size);
      logNormal();

      return(NULL);
   }

   _glCore.memory += size;
   return( ptr );
}

/* internal calloc function.
 * keep track of allocations and prints output on failure */
void* glCalloc( int items, size_t size )
{
   void *ptr = calloc( items, size );
   if(!ptr)
   {
      logRed();
      glPrint("[ALLOC] failed to allocate %lu bytes\n", (size_t)items * size);
      logNormal();

      return(NULL);
   }

   _glCore.memory += items * size;
   return( ptr );
}

/* internal realloc function.
 * keep track of allocations and prints output on failure */
void* glRealloc( void *ptr, int old_items, int items, size_t size )
{
   void *ptr2 = realloc( ptr, (size_t)items * size );
   if(!ptr2)
   {
      ptr2 = glCalloc( items, size );
      if(!ptr2)
      {
         logRed();
         glPrint("[ALLOC] failed to allocate %lu bytes for copy from reallocate\n",
               (size_t)items * size);
         logNormal();
         return( ptr );
      }

      memcpy( ptr2, ptr, old_items * size );
      free( ptr );
      ptr = ptr2;
   }
   else
   {
      ptr = ptr2;
   }

   _glCore.memory -= old_items * size;
   _glCore.memory += items * size;
   return( ptr );
}

/* internal memcpy function.
 * keep track of allocations and prints output on failure */
void* glCopy( void *ptr, size_t size )
{
   if(!ptr)
      return(NULL);

   void *ptr2 = glCalloc( 1, size );
   if(!ptr2)
   {
      logRed();
      glPrint("[ALLOC] failed to allocate %lu bytes for copy\n", size);
      logNormal();

      return(NULL);
   }

   /* no need to increase size, glCalloc does it already :) */
   memcpy(ptr2, ptr, size);
   return(ptr2);
}

/* internal free function.
 * keep track of allocations and prints output on failure */
int glFree( void *ptr, size_t size )
{
   if(!ptr)
      return( RETURN_OK );

   free( ptr ); ptr = NULL;
   _glCore.memory -= size;
   return( RETURN_OK );
}
