#ifndef GLES1

/* NOTE: it's copy of OGL140 renderer ATM */

#include <limits.h>

#include "config.h"
#include "core.h"
#include "sceneobject.h"
#include "types.h"
#include "framework.h"

#define OGL3_NAME "OpenGL 3.1+"

#ifdef GLES2
#  include <GLES2/gl.h>
#else
#  include <GL/glew.h>
#  include <GL/gl.h>
#endif

#define BUFFER_OFFSET(i) ((char *)NULL + (i))

/* bind VBO */
static void bindVBO( glVBO *vbo )
{
   if(_glCore.render.mode == GL_MODE_VERTEX_ARRAY)
      return;

   glBindBuffer( GL_ARRAY_BUFFER, vbo->object );
}

/* unbind VBO */
static void unbindVBO( glVBO *vbo )
{
   if(_glCore.render.mode == GL_MODE_VERTEX_ARRAY)
      return;

   glBindBuffer( GL_ARRAY_BUFFER, 0 );
}

/* bind texture from UVW */
static void bindTexture( glObject *object, unsigned int index )
{
   glBindTexture( GL_TEXTURE_2D,
                  object->material->texture[index]->object );
}

/* texture coordinates */
static void coordPointer( glVBO *vbo, unsigned int index, size_t offset )
{
   if(_glCore.render.mode == GL_MODE_VERTEX_ARRAY)
   {
      /* use coords from texture */
      glTexCoordPointer( 2, GL_FLOAT, 0, &vbo->uvw[ index ].coords[ offset ] );
   }
   else
   {
      glTexCoordPointer( 2, GL_FLOAT, 0, BUFFER_OFFSET( vbo->uvw[ index ].cOffset + offset ) );
   }
}

/* uvw */
static void uvwPointer( glObject *object, size_t offset )
{
   if(!object->material)
      return;

   unsigned int i = 0;
   while( i != _glCore.info.maxTextureUnits )
   {
      if( object->material->texture[i] )
      {
         glActiveTexture( GL_TEXTURE0 + i );
         glClientActiveTexture( GL_TEXTURE0 + i );
         glEnable(GL_TEXTURE_2D);

         bindTexture( object, i );
         coordPointer( object->vbo, object->material->texture[i]->uvw, offset );
      }

      ++i;
   }
}

/* vertices */
static void vertexPointer( glVBO *vbo, size_t offset )
{
   if(_glCore.render.mode == GL_MODE_VERTEX_ARRAY)
      glVertexPointer( 3, GL_FLOAT, 0, &vbo->vertices[ offset ] );
   else
      glVertexPointer( 3, GL_FLOAT, 0, BUFFER_OFFSET( vbo->vOffset + offset ) );
}

/* normals */
static void normalPointer( glVBO *vbo, size_t offset )
{
   if(_glCore.render.mode == GL_MODE_VERTEX_ARRAY)
      glNormalPointer( GL_FLOAT, 0, &vbo->normals[ offset ] );
   else
      glNormalPointer( GL_FLOAT, 0, BUFFER_OFFSET( vbo->nOffset + offset ) );

}

/* colors */
static void colorPointer( glVBO *vbo, size_t offset )
{
#if VERTEX_COLOR
   if(_glCore.render.mode == GL_MODE_VERTEX_ARRAY)
      glColorPointer( 4, GL_UNSIGNED_BYTE, 0, &vbo->colors[ offset ] );
   else
      glColorPointer( 4, GL_UNSIGNED_BYTE, 0, BUFFER_OFFSET( vbo->cOffset + offset ) );
#endif
}

/* indices */
static void elementDraw( glObject *object, unsigned int index )
{
   unsigned int indices_type;
   unsigned int i_use;
   size_t       iOffset;
#if USE_BUFFERS
   unsigned short *indices;
#else
   unsigned int   *indices;
#endif

   if(!object->ibo)
   {
      if(object->vbo->v_use)
         glDrawArrays( object->primitive_type, 0, object->vbo->v_use );

      return;
   }

#if USE_BUFFERS
   indices        = object->ibo->indices[index];

   indices_type   = GL_UNSIGNED_SHORT;
   i_use          = object->ibo->i_use[ index ];
   iOffset        = object->ibo->iOffset[ index ];
#else
   indices        = object->ibo->indices;

   indices_type   = GL_UNSIGNED_INT;
   i_use          = object->ibo->i_use;
   iOffset        = 0;
#endif

   if(i_use)
   {
      /* draw without IBO */
      if(_glCore.render.mode == GL_MODE_VERTEX_ARRAY)
         glDrawElements( object->primitive_type, i_use,
                         indices_type, &indices[ 0 ] );
      else
      {
         glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, object->ibo->object );
         glDrawElements( object->primitive_type, i_use,
                         indices_type, BUFFER_OFFSET( iOffset ) );
         glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, 0 );
      }
   }
   else if(object->vbo->v_use)
      glDrawArrays( object->primitive_type, 0, object->vbo->v_use );
}

/* draw the object */
static void drawObject( glObject *object )
{
#if USE_BUFFERS
   unsigned int i = 0;
   size_t tmp;
   /* with indices */
   if( object->ibo )
   {
      while( i != object->ibo->index_buffer )
      {
         tmp =  i * USHRT_MAX;

         bindVBO( object->vbo );
            uvwPointer( object, tmp );
            vertexPointer( object->vbo, tmp );
            normalPointer( object->vbo, tmp );
            colorPointer( object->vbo, tmp );
         unbindVBO( object->vbo );

         /* bind automatically */
         elementDraw( object, i );

         ++i;
      }
   }
   else
   /* without indices */
   {
      bindVBO( object->vbo );
         uvwPointer( object, 0 );
         vertexPointer( object->vbo, 0 );
         normalPointer( object->vbo, 0 );
         colorPointer( object->vbo, 0 );
      unbindVBO( object->vbo );

      /* bind automatically */
      elementDraw( object, 0 );
   }
#else
   bindVBO( object->vbo );
      uvwPointer( object, 0 );
      vertexPointer( object->vbo, 0 );
      normalPointer( object->vbo, 0 );
      colorPointer( object->vbo, 0 );
   unbindVBO( object->vbo );

   /* binds automatically */
   elementDraw( object, 0 );
#endif
}


/* draw bounding box */
static void drawAABB( glObject *object )
{
   kmVec3 min = object->aabb_box.min;
   kmVec3 max = object->aabb_box.max;
   float points[] = { min.x, min.y, min.z,
                      max.x, min.y, min.z,
                      min.x, min.y, min.z,
                      min.x, max.y, min.z,
                      min.x, min.y, min.z,
                      min.x, min.y, max.z,

                      max.x, max.y, max.z,
                      min.x, max.y, max.z,
                      max.x, max.y, max.z,
                      max.x, min.y, max.z,
                      max.x, max.y, max.z,
                      max.x, max.y, min.z,

                      min.x, max.y, min.z,
                      max.x, max.y, min.z,
                      min.x, max.y, min.z,
                      min.x, max.y, max.z,

                      max.x, min.y, min.z,
                      max.x, max.y, min.z,
                      max.x, min.y, min.z,
                      max.x, min.y, max.z,

                      min.x, min.y, max.z,
                      max.x, min.y, max.z,
                      min.x, min.y, max.z,
                      min.x, max.y, max.z  };

   glDisable(GL_TEXTURE_2D);
   glEnableClientState(GL_VERTEX_ARRAY);
   glDisableClientState(GL_TEXTURE_COORD_ARRAY);
   glDisableClientState(GL_NORMAL_ARRAY);
   glDisableClientState(GL_COLOR_ARRAY);

   glColor4f( 0, 1, 0, 1 );
   glVertexPointer( 3, GL_FLOAT, 0, &points[0] );
   glDrawArrays( GL_LINES, 0, 24 );
}

static void glOGL3_setup( glObject *object )
{
   glEnable(GL_DEPTH_TEST);
   glDepthFunc(GL_LEQUAL);
   glEnable(GL_BLEND);
   glDisable(GL_CULL_FACE);

   if(object->vbo->v_use)
      glEnableClientState(GL_VERTEX_ARRAY);
   else
      glDisableClientState(GL_VERTEX_ARRAY);

   if(object->vbo->n_use)
      glEnableClientState(GL_NORMAL_ARRAY);
   else
      glDisableClientState(GL_NORMAL_ARRAY);

#if VERTEX_COLOR
   if(object->vbo->c_use)
      glEnableClientState(GL_COLOR_ARRAY);
   else
   {
      glDisableClientState(GL_COLOR_ARRAY);
      glColor4f( 1, 1, 1, 1 );
   }
#else
   glColor4f( 1, 1, 1, 1 );
#endif

   if(object->material)
   {
      glEnable(GL_TEXTURE_2D);
      glEnableClientState(GL_TEXTURE_COORD_ARRAY);
   }
   else
   {
      glDisable(GL_TEXTURE_2D);
      glDisableClientState(GL_TEXTURE_COORD_ARRAY);
   }

}

static void glOGL3_draw( glObject *object )
{
   glOGL3_setup( object );

   drawObject( object );
   drawAABB( object );
}

/* OpenGL 3.1+ renderer */
int glOGL3( void )
{
   _glCore.render.draw     = glOGL3_draw;
   _glCore.render.string   = OGL3_NAME;

   /* temp camera */
   kmMat4 camera;
   kmMat4PerspectiveProjection( &camera, kmPI / 2.0,
      (float)_glCore.display.width/(float)_glCore.display.height, 1.0f, 4000.0f );
   glSetProjection( camera );

   return(RETURN_OK);
}

#endif /* GLES2 define don't want this */

/* EoF */
