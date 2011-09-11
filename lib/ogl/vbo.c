#include <limits.h>

#include "alloc.h"
#include "types.h"
#ifdef VERTEX_COLOR
#  include "scolor.h"
#endif
#include "vbo.h"
#include "config.h"
#include "core.h"

#ifdef GLES2
#  include <GLES2/gl2.h>
#endif
#ifdef GLES1
#  include <GLES/gl.h>
#  include <GLES/glext.h>
#endif
#if !defined(GLES1) && !defined(GLES2)
#  include <GL/glew.h>
#  include <GL/gl.h>
#endif

/* Allocate VBO object */
glVBO* glNewVBO( void )
{
   unsigned int i;

   /* Allocate VBO object */
   glSetAlloc( ALLOC_VBO );
   glVBO *vbo = (glVBO*)glCalloc( 1, sizeof(glVBO) );
   if(!vbo)
      return( NULL );

   /* Default hint */
   vbo->hint = GL_STATIC_DRAW;

   /* Allocate uvws */
   vbo->uvw = glCalloc( _glCore.info.maxTextureUnits, sizeof(glUVW) );
   if(!vbo->uvw)
   {
      glFree(vbo, sizeof(glVBO));
      return( NULL );
   }

   /* Nullify pointers */
   i = 0;
   while(i != _glCore.info.maxTextureUnits)
   {
      vbo->uvw[i].coords  = NULL;
      ++i;
   }

   vbo->tstance   = NULL;
   vbo->vertices  = NULL;
   vbo->normals   = NULL;
#if VERTEX_COLOR
   vbo->colors    = NULL;
#endif

   logGreen();
   glPuts("[A:VBO]");
   logNormal();

   /* Increase ref counter */
   vbo->refCounter++;

   /* Return VBO object */
   return( vbo );
}

/* Copy VBO object */
glVBO* glCopyVBO( glVBO *src )
{
   unsigned int i = 0;
   glVBO *vbo;

   /* Fuuuuuuuuu--- We have non valid object */
   if(!src) return( NULL );

   /* Allocate VBO object */
   glSetAlloc( ALLOC_VBO );
   vbo = (glVBO*)glCalloc( 1, sizeof(glVBO) );
   if(!vbo)
      return( NULL );

   /* Allocate uvws */
   vbo->uvw = glCalloc( _glCore.info.maxTextureUnits, sizeof(glUVW) );
   if(!vbo->uvw)
   {
      glFree(vbo, sizeof(glVBO));
      return( NULL );
   }

   /* Copy data */
   while(i != _glCore.info.maxTextureUnits)
   {
      glCopyCoordBuffer( vbo, src, i );
      ++i;
   }

   glCopyVertexBuffer( vbo, src );
   glCopyNormalBuffer( vbo, src );
#if VERTEX_COLOR
   glCopyColorBuffer( vbo, src );
#endif
   if(src->tstance) glVBOPrepareTstance( vbo );

   vbo->vbo_size  = src->vbo_size;
   vbo->hint      = src->hint;

   logYellow();
   glPuts("[C:VBO]");
   logNormal();

   /* Increase ref counter */
   vbo->refCounter++;

   /* Return VBO object */
   return( vbo );
}

/* Reference VBO object */
glVBO* glRefVBO( glVBO *src )
{
   glVBO *vbo;

   /* Fuuuuuuuuu--- We have non valid object */
   if(!src) return( NULL );

   /* Simple return pointer to same place */
   vbo = src;

   logYellow();
   glPuts("[R:VBO]");
   logNormal();

   /* Increase ref counter */
   vbo->refCounter++;

   /* Return VBO object */
   return( vbo );
}

/* Free VBO object */
int glFreeVBO( glVBO *vbo )
{
   unsigned int i;

   /* Fuuuuuuuuu--- We have non valid object */
   if(!vbo) return( RETURN_NOTHING );

   /* There is still references to this object alive */
   if(--vbo->refCounter != 0) return( RETURN_NOTHING );

   glSetAlloc( ALLOC_VBO );

   /* Free all data */
   i = 0;
   while( i != _glCore.info.maxTextureUnits )
   {
      glFreeCoordBuffer( vbo, i );
      ++i;
   }
   glFree( vbo->uvw, _glCore.info.maxTextureUnits * sizeof(glUVW) );

   if(vbo->tstance) free(vbo->tstance);
   glFreeVertexBuffer( vbo );
   glFreeNormalBuffer( vbo );
#if VERTEX_COLOR
   glFreeColorBuffer( vbo );
#endif

   /* delete vbo */
   if( vbo->object ) glDeleteBuffers(1, &vbo->object);
   vbo->object = 0;

   logRed();
   glPuts("[F:VBO]");
   logNormal();

   /* Free VBO object */
   glFree( vbo, sizeof(glVBO) );
   return( RETURN_OK );
}

/* update vbo */
int glVBOUpdate( glVBO* vbo )
{
   unsigned int i;
   size_t vboOffset = 0, vboSize = 0;
   size_t tmp;

   if(!vbo)
      return( RETURN_FAIL );

   if(!vbo->object)
      return( glVBOConstruct( vbo ) );

   /* already up to date */
   if(vbo->up_to_date)
      return( RETURN_FAIL );

   /* bind buffer */
   glBindBuffer(GL_ARRAY_BUFFER, vbo->object);

   /* calculate total size of vbo */
   i = 0;
   while( i !=  _glCore.info.maxTextureUnits )
   {
      if(vbo->uvw[i].c_use)
         vboSize += vbo->uvw[i].c_use * 2 * sizeof(float);

      ++i;
   }
   if(vbo->v_use)
      vboSize += vbo->v_use * 3 * sizeof(float);
   if(vbo->n_use)
      vboSize += vbo->n_use * 3 * sizeof(float);
#if VERTEX_COLOR
   if(vbo->c_use)
      vboSize += vbo->c_use * 4 * sizeof(uint8_t);
#endif
   vbo->vbo_size = vboSize;

   /* make VBO total size */
   glBufferData(GL_ARRAY_BUFFER, vbo->vbo_size, NULL, vbo->hint);

   /* buffer data to the VBO */
   i = 0;
   while( i != _glCore.info.maxTextureUnits )
   {
      if(vbo->uvw[i].c_use)
      {
         vbo->uvw[i].cOffset = vboOffset;
         tmp = vbo->uvw[i].c_use * 2 * sizeof(float);

         if(vbo->uvw[i].c_use)
            glBufferSubData(GL_ARRAY_BUFFER, vboOffset, tmp, &vbo->uvw[i].coords[0]);

         vboOffset += tmp;
      }

      ++i;
   }

   /* buffer vertices */
   vbo->vOffset = vboOffset;
   if(vbo->v_use)
   {
      tmp = vbo->v_use * 3 * sizeof(float);

      glBufferSubData(GL_ARRAY_BUFFER, vboOffset, tmp, &vbo->vertices[0]);
      vboOffset += tmp;
   }

   /* buffer normals */
   vbo->nOffset = vboOffset;
   if(vbo->n_use)
   {
      tmp = vbo->n_use * 3 * sizeof(float);

      glBufferSubData(GL_ARRAY_BUFFER, vboOffset, tmp, &vbo->normals[0]);
      vboOffset += tmp;
   }

   /* buffer colors */
#if VERTEX_COLOR
   vbo->cOffset = vboOffset;
   if(vbo->c_use)
   {
      glBufferSubData(GL_ARRAY_BUFFER, vboOffset, vbo->ce * 4 * sizeof(uint8_t), &vbo->colors[0]);
   }
#endif
   glBindBuffer(GL_ARRAY_BUFFER, 0);

   /* mark as up to date */
   vbo->up_to_date = 1;

   return( RETURN_OK );
}

/* construct new VBO data */
int glVBOConstruct( glVBO* vbo )
{
   if(_glCore.render.mode == GL_MODE_VERTEX_ARRAY)
      return( RETURN_OK );

   if(!vbo)
      return( RETURN_FAIL );

   if(vbo->object)
      return( RETURN_OK );

   /* generate VBO */
   glGenBuffers(1, &vbo->object );
   if(!vbo->object)
      return( RETURN_FAIL );

   /* this vbo isn't up to date */
   vbo->up_to_date = 0;
   if(glVBOUpdate( vbo ) != RETURN_OK)
   {
      glDeleteBuffers(1, &vbo->object);
      vbo->object = 0;
      return( RETURN_FAIL );
   }

   return( RETURN_OK );
}

/* copy idle vertices from current VBO */
/* not tracked by allocator! */
int glVBOPrepareTstance( glVBO *vbo )
{
   if(!vbo)
      return( RETURN_FAIL );
   if(!vbo->vertices)
      return( RETURN_FAIL );

   /* free old */
   if(vbo->tstance)
      free( vbo->tstance );

   /* copy new tstance vertices */
   vbo->tstance = malloc( vbo->v_num * sizeof(kmVec3) );
   if(!vbo->tstance)
      return( RETURN_FAIL );

   memcpy( vbo->tstance, vbo->vertices,
         vbo->v_num * sizeof(kmVec3) );

   return( RETURN_OK );
}

/* vertices */
int glCopyVertexBuffer( glVBO *vbo, glVBO *src )
{
   if(!vbo || !src)
      return(RETURN_FAIL );

   glSetAlloc( ALLOC_VBO );

   vbo->vertices  = glCopy( src->vertices, src->v_num * sizeof(kmVec3) );
   if(!vbo->vertices)
      return( RETURN_FAIL );

   vbo->v_num     = src->v_num;
   vbo->v_use     = src->v_use;

   return( RETURN_OK );
}

int glFreeVertexBuffer( glVBO *vbo )
{
   if(!vbo)
      return( RETURN_FAIL );

   glSetAlloc( ALLOC_VBO );

   glFree( vbo->vertices, sizeof(kmVec3) * vbo->v_num );
   vbo->vertices = NULL;
   vbo->v_num = 0;
   vbo->v_use = 0;

   /* Mark vbo outdated */
   vbo->up_to_date = 0;

   return( RETURN_OK );
}

int glResetVertexBuffer( glVBO *vbo, unsigned int vertices )
{
   if(!vbo)
      return( RETURN_FAIL );

   glSetAlloc( ALLOC_VBO );

   if(vbo->vertices)
   {
      vbo->vertices = glRealloc( vbo->vertices, vbo->v_num, vertices, sizeof(kmVec3) );
   }
   else
   {
      vbo->vertices = glCalloc( vertices, sizeof(kmVec3) );
   }

   if(!vbo->vertices)
      return( RETURN_FAIL );

   vbo->v_use = 0;
   vbo->v_num = vertices;

   /* Mark VBO outdated */
   vbo->up_to_date = 0;

   return( RETURN_OK );
}

int glInsertVertex( glVBO *vbo,
      const kmScalar x, const kmScalar y, const kmScalar z )
{
   kmVec3 vertex;

   if(!vbo)
      return( RETURN_FAIL );

   if(!vbo->vertices)
      return( RETURN_FAIL );

   glSetAlloc( ALLOC_VBO );

   ++vbo->v_use;
   if(vbo->v_use > vbo->v_num)
   {
      /* Maybe make it realloc more vertices? Or just hope ppl
       * use the reset function above */
      vbo->vertices = glRealloc(vbo->vertices, vbo->v_num, vbo->v_use, sizeof(kmVec3));
      if(!vbo->vertices)
         return( RETURN_FAIL );

      vbo->v_num = vbo->v_use;
   }

   /* Assign vertex */
   vertex.x = x; vertex.y = y; vertex.z = z;
   vbo->vertices[ vbo->v_use - 1 ] = vertex;

   /* Mark VBO outdated */
   vbo->up_to_date = 0;

   return( RETURN_OK );
}

/* vertex uvw */
int glCopyCoordBuffer( glVBO *vbo, glVBO *src, unsigned int index )
{
   if(!vbo || !src)
      return(RETURN_FAIL );

   if(index > _glCore.info.maxTextureUnits)
      return( RETURN_FAIL );

   glSetAlloc( ALLOC_VBO );

   vbo->uvw[index].coords        = glCopy( src->uvw[index].coords, src->uvw[index].c_num * sizeof(kmVec2) );
   if(!vbo->uvw[index].coords)
      return( RETURN_FAIL );

   vbo->uvw[index].c_num         = src->uvw[index].c_num;
   vbo->uvw[index].c_use         = src->uvw[index].c_use;

   return( RETURN_OK );
}

int glFreeCoordBuffer( glVBO *vbo, unsigned int index )
{
   if(!vbo)
      return( RETURN_FAIL );

   if(index > _glCore.info.maxTextureUnits)
      return( RETURN_FAIL );

   glSetAlloc( ALLOC_VBO );

   if( vbo->uvw[index].coords )
      glFree( vbo->uvw[index].coords, sizeof(kmVec2) * vbo->uvw[index].c_num );
   vbo->uvw[index].coords    = NULL;
   vbo->uvw[index].c_use = 0;
   vbo->uvw[index].c_num = 0;

   /* Mark vbo outdated */
   vbo->up_to_date = 0;

   return( RETURN_OK );
}

int glResetCoordBuffer( glVBO *vbo, unsigned int index, unsigned int vertices )
{
   if(!vbo)
      return( RETURN_FAIL );

   if(index > _glCore.info.maxTextureUnits)
      return( RETURN_FAIL );

   glSetAlloc( ALLOC_VBO );

   if(vbo->uvw[index].coords)
   {
      vbo->uvw[index].coords = glRealloc( vbo->uvw[index].coords, vbo->uvw[index].c_num, vertices, sizeof(kmVec2) );
   }
   else
   {
      vbo->uvw[index].coords = glCalloc( vertices, sizeof(kmVec2) );
   }

   if(!vbo->uvw[index].coords)
      return( RETURN_FAIL );

   vbo->uvw[index].c_use = 0;
   vbo->uvw[index].c_num = vertices;

   /* Mark VBO outdated */
   vbo->up_to_date = 0;

   return( RETURN_OK );
}

int glInsertCoord( glVBO *vbo, unsigned int index,
      const kmScalar x, const kmScalar y )
{
   kmVec2 vertex;

   if(index > _glCore.info.maxTextureUnits)
      return( RETURN_FAIL );

   if(!vbo)
      return( RETURN_FAIL );

   if(!vbo->uvw[index].coords)
      return( RETURN_FAIL );

   glSetAlloc( ALLOC_VBO );

   ++vbo->uvw[index].c_use;
   if(vbo->uvw[index].c_use >= vbo->uvw[index].c_num)
   {
      /* Maybe make it realloc more vertices? Or just hope ppl
       * use the reset function above */
      vbo->uvw[index].coords = glRealloc(vbo->uvw[index].coords, vbo->uvw[index].c_num, vbo->uvw[index].c_use, sizeof(kmVec2));
      if(!vbo->uvw[index].coords)
         return( RETURN_FAIL );

      vbo->uvw[index].c_num = vbo->uvw[index].c_use;
   }

   /* Assign vertex */
   vertex.x = x; vertex.y = y;
   vbo->uvw[index].coords[ vbo->uvw[index].c_use - 1 ] = vertex;

   /* Mark VBO outdated */
   vbo->up_to_date = 0;

   return( RETURN_OK );
}

/* vertex normal */
int glCopyNormalBuffer( glVBO *vbo, glVBO *src )
{
   if(!vbo || !src)
      return(RETURN_FAIL );

   glSetAlloc( ALLOC_VBO );

   vbo->normals   = glCopy( src->normals, src->n_num * sizeof(kmVec3) );
   if(!vbo->normals)
      return( RETURN_FAIL );
   vbo->n_num     = src->n_num;
   vbo->n_use     = src->n_use;

   return( RETURN_OK );
}

int glFreeNormalBuffer( glVBO *vbo )
{
   if(!vbo)
      return( RETURN_FAIL );

   glSetAlloc( ALLOC_VBO );

   glFree( vbo->normals, vbo->n_num * sizeof(kmVec3) );
   vbo->normals = NULL;
   vbo->n_use = 0;
   vbo->n_num = 0;

   /* Mark vbo outdated */
   vbo->up_to_date = 0;

   return( RETURN_OK );
}

int glResetNormalBuffer( glVBO *vbo, unsigned int vertices )
{
   if(!vbo)
      return( RETURN_FAIL );

   glSetAlloc( ALLOC_VBO );

   if(vbo->normals)
   {
      vbo->normals = glRealloc( vbo->normals, vbo->n_num, vertices, sizeof(kmVec3) );
   }
   else
   {
      vbo->normals = glCalloc( vertices, sizeof(kmVec3) );
   }

   if(!vbo->normals)
      return( RETURN_FAIL );

   vbo->n_use = 0;
   vbo->n_num = vertices;

   /* Mark vbo outdated */
   vbo->up_to_date = 0;

   return( RETURN_OK );
}

int glInsertNormal( glVBO *vbo,
      const kmScalar x, const kmScalar y, const kmScalar z )
{
   kmVec3 vertex;

   if(!vbo)
      return( RETURN_FAIL );

   glSetAlloc( ALLOC_VBO );

   ++vbo->n_use;
   if(vbo->n_use >= vbo->n_num)
   {
      /* Maybe make it realloc more vertices? Or just hope ppl
       * use the reset function above */
      vbo->normals = glRealloc(vbo->normals, vbo->n_num, vbo->n_use, sizeof(kmVec3));
      if(!vbo->normals)
         return( RETURN_FAIL );

      vbo->n_num = vbo->n_use;
   }

   /* Assign vertex */
   vertex.x = x; vertex.y = y; vertex.z = z;
   vbo->normals[ vbo->n_use - 1 ] = vertex;

   /* Mark vbo outdated */
   vbo->up_to_date = 0;

   return( RETURN_OK );
}

#if VERTEX_COLOR
/* vertex color */
int glCopyColorBuffer( glVBO *vbo, glVBO *src )
{
   if(!vbo || !src)
      return(RETURN_FAIL );

   glSetAlloc( ALLOC_VBO );

   vbo->colors    = glCopy( src->colors, src->c_num * sizeof(sColor) );
   if(!vbo->colors)
      return( RETURN_FAIL );
   vbo->c_num     = src->c_num;
   vbo->c_use     = src->c_use;

   return( RETURN_OK );
}

int glFreeColorBuffer( glVBO *vbo )
{
   if(!vbo)
      return( RETURN_FAIL );

   glSetAlloc( ALLOC_VBO );

   glFree( vbo->colors, vbo->c_num * sizeof(sColor) );
   vbo->colors = NULL;
   vbo->c_num = 0;
   vbo->c_use = 0;

   /* mark VBO outdated */
   vbo->up_to_date = 0;

   return( RETURN_OK );
}

int glResetColorBuffer( glVBO *vbo, unsigned int vertices )
{
   if(!vbo)
      return( RETURN_FAIL );

   glSetAlloc( ALLOC_VBO );

   if(vbo->colors)
   {
      vbo->colors = glRealloc( vbo->colors, vbo->c_num, vertices, sizeof(sColor) );
   }
   else
   {
      vbo->colors = glCalloc( vertices, sizeof(sColor) );
   }

   if(!vbo->colors)
      return( RETURN_FAIL );

   vbo->c_use = 0;
   vbo->c_num = vertices;

   /* mark VBO outdated */
   vbo->up_to_date = 0;

   return( RETURN_OK );
}

int glInsertColor( glVBO *vbo,
      const uint8_t r, const uint8_t g, const uint8_t b, const uint8_t a )
{
   if(!vbo)
      return( RETURN_FAIL );

   glSetAlloc( ALLOC_VBO );

   ++vbo->c_use;
   if(vbo->c_use >= vbo->c_num)
   {
      /* Maybe make it realloc more vertices? Or just hope ppl
       * use the reset function above */
      vbo->colors = glRealloc(vbo->colors, vbo->c_num, vbo->c_use, sizeof(sColor));
      if(!vbo->colors)
         return( RETURN_FAIL );

      vbo->c_num = vbo->c_use;
   }

   /* Assign vertex */
   sColor vertex; vertex.r = r; vertex.g = g; vertex.b = b; vertex.a = a;
   vbo->colors[ vbo->c_use - 1 ] = vertex;

   /* Mark vbo outdated */
   vbo->up_to_date = 0;

   return( RETURN_OK );
}
#endif /* USE_COLOR */
