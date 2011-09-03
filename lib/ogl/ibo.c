#include <limits.h>

#include "alloc.h"
#include "types.h"
#include "ibo.h"
#include "core.h"
#include "config.h"

#ifdef GLES2
#	include <GLES2/gl2.h>
#endif
#ifdef GLES1
#  include <GLES/gl.h>
#  include <GLES/glext.h>
#endif
#if !defined(GLES1) && !defined(GLES2)
#	include <GL/glew.h>
#  include <GL/gl.h>
#endif

/* Allocate IBO object */
glIBO* glNewIBO( void )
{
#if USE_BUFFERS
   unsigned int i;
#endif

	/* Allocate IBO object */
   glSetAlloc( ALLOC_IBO );
   glIBO *ibo = (glIBO*)glCalloc( 1, sizeof(glIBO) );
   if(!ibo)
      return( NULL );

   /* Default hint */
   ibo->hint = GL_STATIC_DRAW;

#if USE_BUFFERS
   i = 0;
   while(i != GL_MAX_BUFFERS)
   {
      ibo->indices[i] = NULL;
      ++i;
   }
#else
   ibo->indices   = NULL;
#endif

   logGreen();
   glPuts("[A:IBO]");
   logNormal();

	/* Increase ref counter */
	ibo->refCounter++;

	/* Return IBO object */
	return( ibo );
}

/* Copy IBO object */
glIBO* glCopyIBO( glIBO *src )
{
   glIBO *ibo;

	/* Fuuuuuuuuu--- We have non valid object */
	if(!src) return( NULL );

	/* Allocate IBO object */
   glSetAlloc( ALLOC_IBO );
	ibo = (glIBO*)glCalloc( 1, sizeof(glIBO) );
   if(!ibo)
       return( NULL );

	/* Copy data */
   glCopyIndexBuffer( ibo, src );

   ibo->ibo_size	= src->ibo_size;
	ibo->hint	   = src->hint;

   logYellow();
   glPuts("[C:IBO]");
   logNormal();

	/* Increase ref counter */
	ibo->refCounter++;

	/* Return IBO object */
	return( ibo );
}

/* Reference IBO object */
glIBO* glRefIBO( glIBO *src )
{
   glIBO *ibo;

	/* Fuuuuuuuuu--- We have non valid object */
	if(!src) return( NULL );

	/* Simple return pointer to same place */
	ibo = src;

   logYellow();
   glPuts("[R:IBO]");
   logNormal();

	/* Increase ref counter */
	ibo->refCounter++;

	/* Return IBO object */
	return( ibo );
}

/* Free IBO object */
int glFreeIBO( glIBO *ibo )
{
	/* Fuuuuuuuuu--- We have non valid object */
	if(!ibo) return( RETURN_NOTHING );

	/* There is still references to this object alive */
	if(--ibo->refCounter != 0) return( RETURN_NOTHING );

   glSetAlloc( ALLOC_IBO );

	/* Free all data */
   glFreeIndexBuffer( ibo );

   /* delete ibo */
   if( ibo->object ) glDeleteBuffers(1, &ibo->object);
   ibo->object = 0;

   logRed();
   glPuts("[F:IBO]");
   logNormal();

	/* Free IBO object */
	glFree( ibo, sizeof(glIBO) );
   return( RETURN_OK );
}

/* update IBO */
int glIBOUpdate( glIBO* ibo )
{
#if USE_BUFFERS
   unsigned int i;
   size_t tmp;
#endif

   if(!ibo)
      return( RETURN_FAIL );

   if(!ibo->object)
      return( glIBOConstruct( ibo ) );

   /* already up to date */
   if(ibo->up_to_date)
      return( RETURN_FAIL );

   ibo->ibo_size = 0;
#if USE_BUFFERS
   i = 0;
   while( i != GL_MAX_BUFFERS )
   {
      tmp = ibo->i_use[i] * sizeof( unsigned short );

      ibo->iOffset[i]    = ibo->ibo_size;
      ibo->ibo_size     += tmp;

      ++i;
   }
#else
   ibo->ibo_size = ibo->i_use * sizeof( unsigned int );
#endif

   /* bind buffer */
   glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo->object);

   /* make IBO total size */
   glBufferData(GL_ELEMENT_ARRAY_BUFFER, ibo->ibo_size, NULL, ibo->hint);

#if USE_BUFFERS
   i = 0;
   while( i != GL_MAX_BUFFERS )
   {
      if(ibo->i_use[i])
         glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, ibo->iOffset[i],
                         ibo->i_use[i] * sizeof( unsigned short ), &ibo->indices[i][0]);

      ++i;
   }
#else
   if(ibo->i_use)
      glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, ibo->ibo_size, &ibo->indices[0]);
#endif

   glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

   /* mark as up to date */
   ibo->up_to_date = 1;

   return( RETURN_OK );
}

/* construct new IBO data */
int glIBOConstruct( glIBO* ibo )
{
   if(_glCore.render.mode == GL_MODE_VERTEX_ARRAY)
      return( RETURN_OK );

   if(!ibo)
      return( RETURN_FAIL );

   if(ibo->object)
      return( RETURN_OK );

   /* generate IBO */
   glGenBuffers(1, &ibo->object );
   if(!ibo->object)
      return( RETURN_FAIL );

   /* this ibo isn't up to date */
   ibo->up_to_date = 0;
   if(glIBOUpdate( ibo ) != RETURN_OK)
   {
      glDeleteBuffers(1, &ibo->object);
      ibo->object = 0;
      return( RETURN_FAIL );
   }

   return( RETURN_OK );
}

/* indices */
int glCopyIndexBuffer( glIBO *ibo, glIBO *src )
{
#if USE_BUFFERS
   unsigned int i;
#endif

   if(!ibo || !src)
      return(RETURN_FAIL );

   glSetAlloc( ALLOC_IBO );

#if USE_BUFFERS
   i = 0;
   while(i != GL_MAX_BUFFERS)
   {
      ibo->indices[i]= glCopy( src->indices[i], src->i_num[i] * sizeof(unsigned short) );
      ibo->i_num[i]  = src->i_num[i];
      ibo->i_use[i]  = src->i_use[i];

      ++i;
   }
#else
   ibo->indices   = glCopy( src->indices, src->i_num * sizeof(unsigned int) );
   ibo->i_num     = src->i_num;
   ibo->i_use     = src->i_use;
#endif

   return( RETURN_OK );
}

int glFreeIndexBuffer( glIBO *ibo )
{
#if USE_BUFFERS
   unsigned int i;
#endif

   if(!ibo)
      return( RETURN_FAIL );

   glSetAlloc( ALLOC_IBO );

#if USE_BUFFERS
   i = 0;
   while(i != GL_MAX_BUFFERS)
   {
      glFree( ibo->indices[i], ibo->i_num[i] * sizeof(unsigned short) );
      ibo->indices[i] = NULL;

      ibo->i_num[i] = 0;
      ibo->i_use[i] = 0;

      ++i;
   }
#else
   glFree( ibo->indices, ibo->i_num * sizeof(unsigned int) );
   ibo->indices = NULL;

   ibo->i_num = 0;
   ibo->i_use = 0;
#endif

   /* mark IBO as outdated */
   ibo->up_to_date = 0;

   return( RETURN_OK );
}

int glResetIndexBuffer( glIBO *ibo, unsigned int indices )
{
   if(!ibo)
      return( RETURN_FAIL );

   glSetAlloc( ALLOC_IBO );

#if USE_BUFFERS
   unsigned int thisMany = indices / USHRT_MAX;
   if(thisMany > GL_MAX_BUFFERS)
      return( RETURN_FAIL );

   unsigned int i = 0;
   unsigned int actual_amount;
   while(i != thisMany + 1)
   {
      if( indices > USHRT_MAX )
      {
         if(i)
            actual_amount = indices / (USHRT_MAX * i);
         else
            actual_amount = USHRT_MAX;
         if(actual_amount > USHRT_MAX)
            actual_amount = USHRT_MAX;
      }
      else
         actual_amount = indices;

      if(ibo->indices[i])
      {
         ibo->indices[i] = glRealloc( ibo->indices[i], ibo->i_num[i], actual_amount, sizeof(unsigned short) );
      }
      else
      {
         ibo->indices[i] = glCalloc( actual_amount, sizeof(unsigned short) );
      }

      if(!ibo->indices[i])
         return( RETURN_FAIL );

      ibo->i_use[i] = 0;
      ibo->i_num[i] = actual_amount;

      ++i;
   }
   ibo->index_buffer = 1;
#else
   if(ibo->indices)
   {
      ibo->indices = glRealloc( ibo->indices, ibo->i_num, indices, sizeof(unsigned int) );
   }
   else
   {
      ibo->indices = glCalloc( indices, sizeof(unsigned int) );
   }

   if(!ibo->indices)
      return( RETURN_FAIL );

   ibo->i_use = 0;
   ibo->i_num = indices;
#endif

   /* mark IBO as outdated */
   ibo->up_to_date = 0;

   return( RETURN_OK );
}

int glInsertIndex( glIBO *ibo,
                    unsigned int index )
{
   if(!ibo)
      return( RETURN_FAIL );

   glSetAlloc( ALLOC_IBO );

#if USE_BUFFERS
   /* select buffer to put the index */
   unsigned int i = index / USHRT_MAX;
   if(i > GL_MAX_BUFFERS)
      return( RETURN_FAIL );

   if(i > ibo->index_buffer)
      ibo->index_buffer = i;

   ++ibo->i_use[ i ];
   if(ibo->i_use[ i ] >= ibo->i_num[ i ])
   {
      /* Maybe make it realloc more vertices? Or just hope ppl
       * use the reset function above */
      ibo->indices[ i ] = glRealloc(ibo->indices[ i ], ibo->i_num[ i ], ibo->i_use[ i ], sizeof(unsigned short));
      if(!ibo->indices[ i ])
         return( RETURN_FAIL );

      ibo->i_num[ i ] = ibo->i_use[ i ];
   }

   /* Assign index */
   ibo->indices[ i ][ ibo->i_use[ i ] - 1 ] = index;
#else
   ++ibo->i_use;
   if(ibo->i_use > ibo->i_num)
   {
      ibo->indices = glRealloc(ibo->indices, ibo->i_num, ibo->i_use, sizeof(unsigned int));
      if(!ibo->indices)
         return( RETURN_FAIL );

      ibo->i_num = ibo->i_use;
   }

   /* Assign index */
   ibo->indices[ ibo->i_use - 1 ] = index;
#endif

   /* mark IBO as outdated */
   ibo->up_to_date = 0;

   return( RETURN_OK );
}
