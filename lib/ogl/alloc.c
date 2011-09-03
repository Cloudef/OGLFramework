#include "core.h"
#include "types.h"
#include "logfile_wrapper.h"
#include "alloc.h"

#include <malloc.h>
#include <string.h>

#ifdef DEBUG
gleAlloc   GL_D_ALLOC                     = ALLOC_CORE;
static size_t     GL_ALLOC [ ALLOC_LAST ] =
{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
static char*      GL_ALLOCN[ ALLOC_LAST ] =
{ "Core", "Camera", "Sceneobject", "IBO", "VBO", "Animation", "Bone", "Animator", "Evaluator", "Material", "Texture", "Texture Cache", "Atlas", "Total" };

#define ALLOC_CRITICAL 100 * 1048576 /* 100 MiB */
#define ALLOC_HIGH     80  * 1048576 /* 80  MiB */
#define ALLOC_AVERAGE  40  * 1048576 /* 40  MiB */
#endif

/* fake allocation
 * use when doing allocations using normal operation, but want to keep statistics */
void glFakeAlloc( size_t size )
{
#ifdef DEBUG
   GL_ALLOC[ GL_D_ALLOC ] += size;
#endif
}

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

#ifdef DEBUG
   GL_ALLOC[ GL_D_ALLOC ] += size;
#endif
   return( ptr );
}

/* internal calloc function.
 * keep track of allocations and prints output on failure */
void* glCalloc( unsigned int items, size_t size )
{
   void *ptr = calloc( items, size );
   if(!ptr)
   {
      logRed();
      glPrint("[ALLOC] failed to allocate %lu bytes\n", (size_t)items * size);
      logNormal();

      return(NULL);
   }

#ifdef DEBUG
   GL_ALLOC[ GL_D_ALLOC ] += items * size;
#endif
   return( ptr );
}

/* internal realloc function.
 * keep track of allocations and prints output on failure */
void* glRealloc( void *ptr, unsigned int old_items, unsigned int items, size_t size )
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


#ifdef DEBUG
   GL_ALLOC[ GL_D_ALLOC ] -= old_items * size;
   GL_ALLOC[ GL_D_ALLOC ] += items * size;
#endif
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

   /* no need to increase size, glCalloc does it already */
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
#ifdef DEBUG
   GL_ALLOC[ GL_D_ALLOC ] -= size;
#endif
   return( RETURN_OK );
}

/* output memory usage graph */
void glMemoryGraph( void )
{
#ifdef DEBUG
   unsigned int i;

   glPuts("");
   logWhite(); glPuts("--- Memory Graph ---");
   i = 0; GL_ALLOC[ ALLOC_TOTAL ] = 0;
   for(; i != ALLOC_LAST; ++i)
   {
      if( i == ALLOC_TOTAL )
      { logWhite(); glPuts("--------------------"); }

      if( GL_ALLOC[ i ] >= ALLOC_CRITICAL )     logRed();
      else if( GL_ALLOC[ i ] >= ALLOC_HIGH )    logBlue();
      else if( GL_ALLOC[ i ] >= ALLOC_AVERAGE ) logYellow();
      else logGreen();
      glPrint("%13s : ",    GL_ALLOCN[ i ]); logWhite();
      if( GL_ALLOC[ i ] / 1048576 != 0 )
         glPrint("%.2f MiB\n", (float)GL_ALLOC[ i ] / 1048576 );
      else if( GL_ALLOC[ i ] / 1024 != 0 )
         glPrint("%.2f KiB\n", (float)GL_ALLOC[ i ] / 1024 );
      else
         glPrint("%lu B\n", GL_ALLOC[ i ] );

      /* increase total */
      GL_ALLOC[ ALLOC_TOTAL ] += GL_ALLOC[ i ];
   }
   logWhite(); glPuts("--------------------"); logNormal();
   glPuts("");

#else
   logBlue(); glPuts( "-- Memory graph only available on debug build --" ); logNormal();
#endif
}
