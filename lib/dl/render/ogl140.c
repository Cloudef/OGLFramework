#ifndef GLES2

#include <limits.h>
#include <stdint.h>

#include "dlConfig.h"
#include "dlCore.h"
#include "dlSceneobject.h"
#include "dlTypes.h"
#include "dlFramework.h"

#define OGL140_NAME "OpenGL 1.4+"

#ifdef GLES1
#  include <GLES/gl.h>
#  include <GLES/glext.h>
#else
#  include <GL/glew.h>
#  include <GL/gl.h>
#endif

#define BUFFER_OFFSET(i) ((char *)NULL + (i))

typedef struct
{
   uint8_t vertex;
   uint8_t coord;
   uint8_t normal;
#if VERTEX_COLOR
   uint8_t color;
#endif
   uint8_t depth;
   uint8_t texture;
   uint8_t cull;

   uint8_t alpha;
   unsigned int blend1;
   unsigned int blend2;

   unsigned int active_texture;
   unsigned int last_texture;
} dlState;

/* global draw state */
static dlState draw;

/* bind VBO */
static void bindVBO( dlVBO *vbo )
{
   if(_dlCore.render.mode == DL_MODE_VERTEX_ARRAY)
      return;

   glBindBuffer( GL_ARRAY_BUFFER, vbo->object );
}

/* unbind VBO */
static void unbindVBO( dlVBO *vbo )
{
   if(_dlCore.render.mode == DL_MODE_VERTEX_ARRAY)
      return;

   glBindBuffer( GL_ARRAY_BUFFER, 0 );
}

/* bind texture from UVW */
static void bindTexture( dlObject *object )
{
   if(!object->material->texture->object)
       return;
   if(object->material->texture->object == draw.last_texture)
       return;

   draw.last_texture = object->material->texture->object;
   glBindTexture( GL_TEXTURE_2D,
                  object->material->texture->object );
}

/* texture coordinates */
static void coordPointer( dlVBO *vbo, unsigned int index, size_t offset )
{
   if(!vbo->uvw[ index ].c_use)
      return;

   if(_dlCore.render.mode == DL_MODE_VERTEX_ARRAY)
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
static void uvwPointer( dlObject *object, size_t offset )
{
   if(!draw.texture)
      return;

   bindTexture( object );
   coordPointer( object->vbo, object->material->texture->uvw, offset );
}

/* vertices */
static void vertexPointer( dlVBO *vbo, size_t offset )
{
   if(!draw.vertex)
      return;

   if(_dlCore.render.mode == DL_MODE_VERTEX_ARRAY)
      glVertexPointer( 3, GL_FLOAT, 0, &vbo->vertices[ offset ] );
   else
      glVertexPointer( 3, GL_FLOAT, 0, BUFFER_OFFSET( vbo->vOffset + offset ) );
}

/* normals */
static void normalPointer( dlVBO *vbo, size_t offset )
{
   if(!draw.normal)
      return;

   if(_dlCore.render.mode == DL_MODE_VERTEX_ARRAY)
      glNormalPointer( GL_FLOAT, 0, &vbo->normals[ offset ] );
   else
      glNormalPointer( GL_FLOAT, 0, BUFFER_OFFSET( vbo->nOffset + offset ) );

}

/* colors */
static void colorPointer( dlVBO *vbo, size_t offset )
{
#if VERTEX_COLOR
   if(_dlCore.render.mode == DL_MODE_VERTEX_ARRAY)
      glColorPointer( 4, GL_UNSIGNED_BYTE, 0, &vbo->colors[ offset ] );
   else
      glColorPointer( 4, GL_UNSIGNED_BYTE, 0, BUFFER_OFFSET( vbo->cOffset + offset ) );
#endif
}

/* indices */
static void elementDraw( dlObject *object, unsigned int index )
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
      if(_dlCore.render.mode == DL_MODE_VERTEX_ARRAY)
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
static void drawObject( dlObject *object )
{
#if USE_BUFFERS
   unsigned int i = 0;
   size_t tmp;

   /* with indices */
   if( object->ibo )
   {
      i = 0;
      for(; i != object->ibo->index_buffer; ++i )
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
      vertexPointer( object->vbo, 0 );
      uvwPointer( object, 0 );
      normalPointer( object->vbo, 0 );
      colorPointer( object->vbo, 0 );
   unbindVBO( object->vbo );

   /* binds automatically */
   elementDraw( object, 0 );
#endif
}


/* draw bounding box */
static void drawAABB( dlObject *object )
{
   kmVec3 min = object->aabb_box.min;
   kmVec3 max = object->aabb_box.max;
   const float points[] = {
                      min.x, min.y, min.z,
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

   if(draw.texture)
      glDisable(GL_TEXTURE_2D);
   if(draw.coord)
      glDisableClientState(GL_TEXTURE_COORD_ARRAY);
   if(draw.normal)
      glDisableClientState(GL_NORMAL_ARRAY);

   draw.normal  = 0;
   draw.texture = 0;
   draw.coord   = 0;

   glColor4f( 0, 1, 0, 1 );
   glVertexPointer( 3, GL_FLOAT, 0, &points[0] );
   glDrawArrays( GL_LINES, 0, 24 );
   glColor4f( 1, 1, 1, 1 );
}

static void dlOGL140_setup( dlObject *object )
{
   /* current object's state */
   dlState state;
   state.depth  = 0;
   state.vertex = 0;
   state.normal = 0;
   state.coord  = 0;
   state.cull   = 0;
   state.texture= 0;

   /* depth for now always */
   state.depth = 1;

   /* material flags */
   if( object->material )
   {
      /* properities */
      state.alpha  = (object->material->flags  & DL_MATERIAL_ALPHA);
      state.cull   = !(object->material->flags & DL_MATERIAL_DOUBLE_SIDED);
      state.blend1 = object->material->blend1;
      state.blend2 = object->material->blend2;
   }
   else
   {
      state.alpha = 0;
      state.cull  = 1;
      state.blend1 = draw.blend1;
      state.blend2 = draw.blend2;
   }

   /* we are going to use vertices */
   if(object->vbo->v_use)
      state.vertex = 1;

   /* we are going to use normals */
   if(object->vbo->n_use)
      state.normal = 1;

#if VERTEX_COLOR
   state.color = 0;
   if(object->vbo->c_use)
      state.color  = 1;
#endif

   /* we are going to use coords and texture */
   if(object->material)
   {
      if(object->material->texture)
      {
         if(object->vbo->uvw[0].c_use)
         {
            state.texture = 1;
            state.coord   = 1;
         }
      }
   }

   /* check state */
   if(draw.cull != state.cull)
   {
      if(state.cull)
         glEnable( GL_CULL_FACE );
      else
         glDisable( GL_CULL_FACE );

      draw.cull = state.cull;
   }

   /* check state */
   if(draw.depth != state.depth)
   {
      if(state.depth)
      {
         glEnable(GL_DEPTH_TEST);
         glDepthFunc(GL_LEQUAL);
      }
      else
      {
         glDisable(GL_DEPTH_TEST);
      }

      draw.depth = state.depth;
   }

   /* check state */
   if(draw.vertex != state.vertex)
   {
      if(state.vertex)
         glEnableClientState(GL_VERTEX_ARRAY);
      else
         glDisableClientState(GL_VERTEX_ARRAY);

      draw.vertex = state.vertex;
   }


   /* check state */
   if(draw.normal != state.normal)
   {
      if(state.normal)
         glEnableClientState(GL_NORMAL_ARRAY);
      else
         glDisableClientState(GL_NORMAL_ARRAY);

      draw.normal = state.normal;
   }

#if VERTEX_COLOR
   /* check state */
   if(draw.color != state.color)
   {
      if(state.color)
         lEnableClientState(GL_COLOR_ARRAY);
      else
      {
         glDisableClientState(GL_COLOR_ARRAY);
         glColor4f( 1, 1, 1, 1 );
      }

      draw.color = state.color;
   }
#endif

   /* check state */
   if(draw.coord != state.coord)
   {
      if(state.coord)
         glEnableClientState(GL_TEXTURE_COORD_ARRAY);
      else
         glDisableClientState(GL_TEXTURE_COORD_ARRAY);

      draw.coord = state.coord;
   }

   /* check state */
   if(draw.texture != state.texture)
   {
      if(state.texture)
      {
         glEnable(GL_TEXTURE_2D);
         glColor4f( 1, 1, 1, 1 );
      }
      else
      {
         glDisable(GL_TEXTURE_2D);
         glColor4f( 1, 1, 1, 1 );
      }

      draw.texture = state.texture;
   }

   if(draw.alpha != state.alpha)
   {
      if(state.alpha)
         glEnable(GL_BLEND);
      else
         glDisable(GL_BLEND);

      draw.alpha = state.alpha;
   }

   if(draw.blend1 != state.blend1 || draw.blend2 != state.blend2)
   {
      glBlendFunc( state.blend1, state.blend2 );

      draw.blend1 = state.blend1;
      draw.blend2 = state.blend2;
   }

}

static void dlOGL140_draw( dlObject *object )
{
   glMatrixMode(GL_PROJECTION);
   glLoadMatrixf( (float*)&_dlCore.render.projection );

   glMatrixMode(GL_MODELVIEW);
   glLoadMatrixf( (float*)&object->matrix );

   dlOGL140_setup( object );

   drawObject( object );
   drawAABB( object );
}

/* OpenGL 1.4+ renderer */
int dlOGL140( void )
{
   _dlCore.render.draw     = dlOGL140_draw;
   _dlCore.render.string   = OGL140_NAME;

   draw.vertex  = 0;
   draw.coord   = 0;
   draw.normal  = 0;
#if VERTEX_COLOR
   draw.color   = 0;
#endif
   draw.texture = 0;
   draw.depth   = 0;
   draw.cull    = 0;

   draw.alpha   = 0;
   draw.blend1  = 0;
   draw.blend2  = 0;

   draw.active_texture = 0;
   draw.last_texture   = 0;

   return(RETURN_OK);
}

#endif /* GLES2 define don't want this */

/* EoF */
