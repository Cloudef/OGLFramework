#include "dlCore.h"
#include "dlTypes.h"
#include "dlLog.h"
#include "dlAlloc.h"

#include <malloc.h>
#include <string.h>

#define DL_DEBUG_CHANNEL "ALLOC"

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
   CALL("%llu", size);
#ifdef DEBUG
   DL_ALLOC[ DL_D_ALLOC ] += size;
#endif
}

/* internal calloc function.
 * keep track of allocations and prints output on failure */
void* dlMalloc( size_t size )
{
   void *ptr;
   CALL("%llu", size);

   ptr = malloc( size );
   if(!ptr)
   {
      LOGERRP("Failed to allocate %llu bytes", (size_t)size);

      RET("%p", NULL);
      return(NULL);
   }

#ifdef DEBUG
   DL_ALLOC[ DL_D_ALLOC ] += size;
#endif

   RET("%p", ptr);
   return( ptr );
}

/* internal calloc function.
 * keep track of allocations and prints output on failure */
void* dlCalloc( unsigned int items, size_t size )
{
   void *ptr;
   CALL("%llu", size);

   ptr = calloc( items, size );
   if(!ptr)
   {
      LOGERRP("Failed to allocate %llu bytes", (size_t)items * size);

      RET("%p", NULL);
      return(NULL);
   }

#ifdef DEBUG
   DL_ALLOC[ DL_D_ALLOC ] += items * size;
#endif

   RET("%p", ptr);
   return( ptr );
}

/* internal realloc function.
 * keep track of allocations and prints output on failure */
void* dlRealloc( void *ptr, unsigned int old_items, unsigned int items, size_t size )
{
   void *ptr2;
   CALL("%p, %u, %u, %llu", ptr, old_items, items, size);

   ptr2 = realloc( ptr, (size_t)items * size );
   if(!ptr2)
   {
      ptr2 = dlMalloc( items * size );
      if(!ptr2)
      {
         LOGERRP("Failed to allocate %llu bytes for copy reallocation",
                (size_t)items * size);

         RET("%p", ptr);
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

   RET("%p", ptr);
   return( ptr );
}

/* internal memcpy function.
 * keep track of allocations and prints output on failure */
void* dlCopy( void *ptr, size_t size )
{
   void *ptr2;
   CALL("%p, %llu", ptr, size);

   if(!ptr)
   { RET("%p", NULL); return(NULL); }

   ptr2 = dlMalloc( size );
   if(!ptr2)
   {
      LOGERRP("Failed to allocate %lu bytes for copy", size);

      RET("%p", NULL);
      return(NULL);
   }

   /* no need to increase size, dlMalloc does it already */
   memcpy(ptr2, ptr, size);

   RET("%p", ptr2);
   return(ptr2);
}

/* internal free function.
 * keep track of allocations and prints output on failure */
int dlFree( void *ptr, size_t size )
{
   CALL("%p, %llu", ptr, size);

   if(!ptr)
   { RET("%d", RETURN_OK); return( RETURN_OK ); }

   free( ptr ); ptr = NULL;
#ifdef DEBUG
   DL_ALLOC[ DL_D_ALLOC ] -= size;
#endif

   RET("%d", RETURN_OK);
   return( RETURN_OK );
}

/* output memory usage graph */
void dlMemoryGraph( void )
{
   TRACE();
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
