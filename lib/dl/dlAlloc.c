#include "dlCore.h"
#include "dlTypes.h"
#include "dlLog.h"
#include "dlAlloc.h"

#include <malloc.h>
#include <string.h>

#ifdef DEBUG
dleAlloc   DL_D_ALLOC                     = ALLOC_CORE;
static size_t     DL_ALLOC [ ALLOC_LAST ] =
{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
static char*      DL_ALLOCN[ ALLOC_LAST ] =
{ "Core", "Camera", "Sceneobject", "IBO", "VBO", "Animation", "Bone", "Animator", "Evaluator", "Shader", "Material", "Texture", "Texture Cache", "Atlas", "Total" };

#define ALLOC_CRITICAL 100 * 1048576 /* 100 MiB */
#define ALLOC_HIGH     80  * 1048576 /* 80  MiB */
#define ALLOC_AVERAGE  40  * 1048576 /* 40  MiB */
#endif

/* fake allocation
 * use when doing allocations using normal operation, but want to keep statistics */
void dlFakeAlloc( size_t size )
{
#ifdef DEBUG
   DL_ALLOC[ DL_D_ALLOC ] += size;
#endif
}

/* internal calloc function.
 * keep track of allocations and prints output on failure */
void* dlMalloc( size_t size )
{
   void *ptr = malloc( size );
   if(!ptr)
   {
      logRed();
      dlPrint("[ALLOC] failed to allocate %lu bytes\n", (size_t)size);
      logNormal();

      return(NULL);
   }

#ifdef DEBUG
   DL_ALLOC[ DL_D_ALLOC ] += size;
#endif
   return( ptr );
}

/* internal calloc function.
 * keep track of allocations and prints output on failure */
void* dlCalloc( unsigned int items, size_t size )
{
   void *ptr = calloc( items, size );
   if(!ptr)
   {
      logRed();
      dlPrint("[ALLOC] failed to allocate %lu bytes\n", (size_t)items * size);
      logNormal();

      return(NULL);
   }

#ifdef DEBUG
   DL_ALLOC[ DL_D_ALLOC ] += items * size;
#endif
   return( ptr );
}

/* internal realloc function.
 * keep track of allocations and prints output on failure */
void* dlRealloc( void *ptr, unsigned int old_items, unsigned int items, size_t size )
{
   void *ptr2 = realloc( ptr, (size_t)items * size );
   if(!ptr2)
   {
      ptr2 = dlCalloc( items, size );
      if(!ptr2)
      {
         logRed();
         dlPrint("[ALLOC] failed to allocate %lu bytes for copy from reallocate\n",
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


#ifdef DEBUG
   DL_ALLOC[ DL_D_ALLOC ] -= old_items * size;
   DL_ALLOC[ DL_D_ALLOC ] += items * size;
#endif
   return( ptr );
}

/* internal memcpy function.
 * keep track of allocations and prints output on failure */
void* dlCopy( void *ptr, size_t size )
{
   if(!ptr)
      return(NULL);

   void *ptr2 = dlCalloc( 1, size );
   if(!ptr2)
   {
      logRed();
      dlPrint("[ALLOC] failed to allocate %lu bytes for copy\n", size);
      logNormal();

      return(NULL);
   }

   /* no need to increase size, dlCalloc does it already */
   memcpy(ptr2, ptr, size);
   return(ptr2);
}

/* internal free function.
 * keep track of allocations and prints output on failure */
int dlFree( void *ptr, size_t size )
{
   if(!ptr)
      return( RETURN_OK );

   free( ptr ); ptr = NULL;
#ifdef DEBUG
   DL_ALLOC[ DL_D_ALLOC ] -= size;
#endif
   return( RETURN_OK );
}

/* output memory usage graph */
void dlMemoryGraph( void )
{
#ifdef DEBUG
   unsigned int i;

   dlPuts("");
   logWhite(); dlPuts("--- Memory Graph ---");
   i = 0; DL_ALLOC[ ALLOC_TOTAL ] = 0;
   for(; i != ALLOC_LAST; ++i)
   {
      if( i == ALLOC_TOTAL )
      { logWhite(); dlPuts("--------------------"); }

      if( DL_ALLOC[ i ] >= ALLOC_CRITICAL )     logRed();
      else if( DL_ALLOC[ i ] >= ALLOC_HIGH )    logBlue();
      else if( DL_ALLOC[ i ] >= ALLOC_AVERAGE ) logYellow();
      else logGreen();
      dlPrint("%13s : ",    DL_ALLOCN[ i ]); logWhite();
      if( DL_ALLOC[ i ] / 1048576 != 0 )
         dlPrint("%.2f MiB\n", (float)DL_ALLOC[ i ] / 1048576 );
      else if( DL_ALLOC[ i ] / 1024 != 0 )
         dlPrint("%.2f KiB\n", (float)DL_ALLOC[ i ] / 1024 );
      else
         dlPrint("%lu B\n", DL_ALLOC[ i ] );

      /* increase total */
      DL_ALLOC[ ALLOC_TOTAL ] += DL_ALLOC[ i ];
   }
   logWhite(); dlPuts("--------------------"); logNormal();
   dlPuts("");

#else
   logBlue(); dlPuts( "-- Memory graph only available on debug build --" ); logNormal();
#endif
}
