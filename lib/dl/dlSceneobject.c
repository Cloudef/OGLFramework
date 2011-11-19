#include "dlAlloc.h"
#include "dlTypes.h"
#include "dlCore.h"
#include "dlTexture.h"
#include "dlLog.h"

#ifdef GLES2
#  include <GLES2/gl2.h>
#elif  GLES1
#  include <GLES/gl.h>
#  include <GLES/glext.h>
#else
#  include <GL/glew.h>
#  include <GL/gl.h>
#endif

/* Allocate scene object */
dlObject* dlNewObject( void )
{
   TRACE();

   /* Allocate scene object */
   dlSetAlloc( ALLOC_SCENEOBJECT );
   dlObject *object = (dlObject*)dlCalloc( 1, sizeof(dlObject) );
   if(!object)
      return( NULL );

   /* Nullify pointers */
   object->material	   = NULL;
   object->vbo		   = NULL;
   object->ibo             = NULL;

   object->animator        = NULL;
   object->child           = NULL;

   /* Default primitive type */
   object->primitive_type = GL_TRIANGLE_STRIP;

   /* Default transformation */
   object->scale.x = 100; object->scale.y = 100; object->scale.z = 100;

   /* Update matrix on start */
   object->transform_changed = 1;

   LOGOK("SCENEOBJECT", "NEW");

   /* Increase ref counter */
   object->refCounter++;

   /* Return the instance */
   RET("%p", object);
   return( object );
}

/* Copy scene object */
dlObject* dlCopyObject( dlObject *src )
{
   dlObject *object;
   CALL("%p", src);

   /* Fuuuuuuuuu--- We have non valid object */
   if(!src) return( NULL );

   /* Allocate scene object */
   dlSetAlloc( ALLOC_SCENEOBJECT );
   object = (dlObject*)dlCalloc( 1, sizeof(dlObject) );
   if(!object)
      return( NULL );

   /* Copy data */
   object->matrix                = src->matrix;
   object->translation	         = src->translation;
   object->rotation              = src->rotation;
   object->scale		 = src->scale;
   object->target	         = src->target;

   object->aabb_box              = src->aabb_box;

   /* Reference data */
   object->material	         = dlRefMaterial( src->material );

   if(!src->animator) /* reference when no anim */
      object->vbo		 = dlRefVBO( src->vbo );
   else
      object->vbo                = dlCopyVBO( src->vbo );

   object->ibo                   = dlRefIBO( src->ibo );
   object->animator              = dlCopyAnimator( src->animator );

   /* Copy childs */
   object->child                 = dlObjectCopyChilds( src );
   object->num_childs            = src->num_childs;

   /* Copy hints */
   object->primitive_type	 = src->primitive_type;

   /* Update it */
   object->transform_changed = 1;

   LOGWARN("SCENEOBJECT", "COPY");

   /* Increase ref counter */
   object->refCounter++;

   /* Return the instance */
   RET("%p", object);
   return( object );
}

/* Reference scene object */
dlObject* dlRefObject( dlObject *src )
{
   dlObject* object;
   CALL("%p", src);

  /* Fuuuuuuuuu--- We have non valid object */
  if(!src) return( NULL );

  /* Point magic */
  object                      = src;

  /* Reference data */
  object->material	    = dlRefMaterial( src->material );
  object->vbo		    = dlRefVBO( src->vbo );
  object->ibo                 = dlRefIBO( src->ibo );
  object->animator            = dlRefAnimator( src->animator );

  LOGWARN("SCENEOBJECT", "REF");

  /* Increase ref counter */
  object->refCounter++;

  /* Return the instance */
  RET("%p", object);
  return( object );
}

/* Free scene object */
int dlFreeObject( dlObject *object )
{
   unsigned int i;
   CALL("%p", object);

   /* Fuuuuuuuuu--- We have non valid object */
   if(!object) { RET("%d", RETURN_NOTHING); return( RETURN_NOTHING ); }

   /* Free material ( if any ) */
   if(dlFreeMaterial( object->material ) == RETURN_OK)
      object->material = NULL;

   /* Free VBO Object */
   if(dlFreeVBO( object->vbo )           == RETURN_OK)
      object->vbo = NULL;

   /* Free IBO Object */
   if(dlFreeIBO( object->ibo )           == RETURN_OK)
      object->ibo = NULL;

   /* free animator if there is one */
   if(object->animator)
   { if( dlFreeAnimator( object->animator ) == RETURN_OK ); object->animator = NULL; }

   /* Free as in, decrease reference on childs */
   i = 0;
   for(; i != object->num_childs; ++i)
   {
      if(dlFreeObject( object->child[i] ) == RETURN_OK)
         object->child[i] = NULL;
   }

   /* There is still references to this object alive */
   if(--object->refCounter != 0) return( RETURN_NOTHING );

   dlSetAlloc( ALLOC_SCENEOBJECT );

   /* Free child list */
   dlFree( object->child, object->num_childs * sizeof(dlObject*) );
   object->child = NULL;
   object->num_childs = 0;

   LOGERR("SCENEOBJECT", "FREE");

   /* Free scene object */
   dlFree( object, sizeof(dlObject) );

   RET("%d", RETURN_OK);
   return( RETURN_OK );
}

/* draw skeleton */
void dlObjectDrawSkeleton( dlObject *object )
{
#if 0
   dlBone *bone;
   kmVec3 pos;

   if(!object)
      return;
   if(!object->animator)
      return;

   glDisable( GL_DEPTH_TEST );
   glBegin( GL_LINES );
   bone = object->animator->bone;
   for(; bone; bone = bone->next)
   {
      pos.x = 5; pos.y = 5; pos.z = 5;
      kmVec3Transform( &pos, &pos,  &bone->globalMatrix );
      glVertex3f( pos.x, pos.y, pos.z );
   }
   glEnd();
   glEnable( GL_DEPTH_TEST );
#endif
}

/* update skeletal animation */
static void dlObjectUpdateSkeletal( dlObject *object )
{
   unsigned int x;
   dlBone         *bone;
   dlVertexWeight *weight;

   kmVec3 tStance;
   kmMat4 boneMat;
   dlAnimator *animator;

   CALL("%p", object);

   animator = object->animator;

   /* TO-DO: Shader implentation */
   /* Reset all vertices to 0 here */
   x = 0;
   for(; x != object->vbo->v_num; ++x)
   {
      object->vbo->vertices[x].x = 0;
      object->vbo->vertices[x].y = 0;
      object->vbo->vertices[x].z = 0;
   }

   bone = animator->bone;
   for(; bone; bone = bone->next)
   {
      boneMat = bone->globalMatrix;
      weight = bone->weight;
      for(; weight; weight = weight->next)
      {
         /* Get bone matrices */
         /* and shift t-stance vertices */

         tStance  = object->vbo->tstance[weight->vertex];
         kmVec3Transform( &tStance, &tStance, &boneMat );

         object->vbo->vertices[weight->vertex].x += tStance.x * weight->value;
         object->vbo->vertices[weight->vertex].y += tStance.y * weight->value;
         object->vbo->vertices[weight->vertex].z += tStance.z * weight->value;

         /*
          * index  = object->bone[i]->weight[x].vertex;
          * weight = object->bone[i]->weight[x].weight;
          * kmVec3 tStance = TSTANCE[ index ];
          * tStance *= boneMat;
          *
          * vbo->vertices[index].x += tStance.x*weight;
          * vbo->vertices[index].y += tStance.y*weight;
          * vbo->vertices[index].z += tStance.z*weight;
          */
      }
   }

   /* VBO needs update */
   object->vbo->up_to_date = 0;
}

/* Update animation */
void dlObjectTick( dlObject *object, float tick )
{
   unsigned int i;
   CALL("%p, %f", object, tick);

   if(!object)
      return;
   if(!object->animator)
      return;

   dlAnimatorTick( object->animator, tick );

   if(!object->vbo)
      return;

   /* update vertices */
   i = 0; dlObjectUpdateSkeletal( object );
   for(; i != object->num_childs; ++i)
      dlObjectUpdateSkeletal( object->child[i] );
}

/* Set animation */
void dlObjectSetAnimation( dlObject *object, DL_NODE_TYPE index )
{
   CALL("%p, %d", object, index);

   if(!object)
      return;
   if(!object->animator)
      return;

   dlAnimatorSetAnim( object->animator, index );
}

/* Add child, steals reference */
int dlObjectAddChild( dlObject *object, dlObject *child )
{
   CALL("%p, %p", object, child);

   if(!object)
      return( RETURN_FAIL );
   if(!child)
      return( RETURN_FAIL );

   /* alloc */
   dlSetAlloc( ALLOC_SCENEOBJECT );
   object->num_childs++;
   if(!object->child)
      object->child = dlCalloc( object->num_childs, sizeof(dlObject*) );
   else
      object->child = dlRealloc( object->child, object->num_childs - 1,
                                 object->num_childs, sizeof(dlObject*) );

   /* check success */
   if(!object->child)
      return( RETURN_FAIL );

   /* assign */
   object->child[ object->num_childs - 1 ] = child;

   RET("%d", RETURN_OK);
   return( RETURN_OK );
}

/* Reference all childs */
dlObject** dlObjectRefChilds( dlObject *object )
{
   unsigned int i;
   CALL("%p", object);

   if(!object)
      return( NULL );
   if(!object->child)
      return( NULL );

   /* reference all */
   i = 0;
   for(; i != object->num_childs; ++i)
   {
      dlRefObject( object->child[i] );
   }

   RET("%p", object->child);
   return( object->child );
}

/* Copy all childs */
dlObject** dlObjectCopyChilds( dlObject *object )
{
   unsigned int i;
   dlObject     **newList;
   CALL("%p", object);

   if(!object)
      return( NULL );
   if(!object->child)
      return( NULL );

   dlSetAlloc( ALLOC_SCENEOBJECT );
   newList = dlCalloc( object->num_childs, sizeof(dlObject*) );
   if(!newList)
      return( NULL );

   /* reference all */
   i = 0;
   for(; i != object->num_childs; ++i)
   {
      newList[i] = dlCopyObject( object->child[i] );
   }

   RET("%p", newList);
   return( newList );
}

/* Free child */
int dlObjectFreeChild( dlObject *object, dlObject *child )
{
   unsigned int i;
   unsigned int found;
   dlObject     **tmp;

   CALL("%p, %p", object, child);

   if(!object)
      return( RETURN_FAIL );
   if(!child)
      return( RETURN_FAIL );
   if(!object->child)
      return( RETURN_FAIL );

   /* allocate tmp list */
   dlSetAlloc( ALLOC_SCENEOBJECT );
   tmp = dlCalloc( object->num_childs, sizeof(dlObject*) );
   if(!tmp)
      return( RETURN_FAIL );

   /* add everything expect the one we are looking for to tmp list */
   i = 0;
   found = 0;
   for(; i != object->num_childs; ++i)
   {
      if( object->child[i] != child )
      {
         tmp[++found] = object->child[i];
      }
   }

   if(found)
   {
      /* resize list */
      tmp = dlRealloc( tmp, object->num_childs, found, sizeof(dlObject*) );
      if(!tmp)
         return( RETURN_FAIL );
   }
   else
   {
      dlFree( tmp, object->num_childs * sizeof(dlObject*) );
      tmp = NULL;
   }

   /* ok, free the old list */
   dlFree( object->child, object->num_childs * sizeof(dlObject*) );
   dlFreeObject( child );

   /* use the new list and new count */
   object->child        = tmp;
   object->num_childs   = found;

   RET("%d", RETURN_OK);
   return( RETURN_OK );
}

/* Free all childs */
int dlObjectFreeChilds( dlObject *object )
{
   unsigned int i;
   CALL("%p", object);

   if(!object)
      return( RETURN_FAIL );
   if(!object->child)
      return( RETURN_FAIL );

   /* free all */
   i = 0;
   for(; i != object->num_childs; ++i)
   {
      if( dlFreeObject( object->child[i] ) == RETURN_OK )
         object->child[i] = NULL;
   }

   dlSetAlloc( ALLOC_SCENEOBJECT );
   dlFree( object->child, object->num_childs * sizeof(dlObject*) );
   object->child = NULL;
   object->num_childs = 0;

   RET("%d", RETURN_OK);
   return( RETURN_OK );
}

static void dlUpdateMatrix( dlObject *object )
{
   kmMat4 translation,
          rotation,
          scale,
          temp;

   CALL("%p", object);

   /* translation */
   kmMat4Translation( &translation,
                      object->translation.x,
                      object->translation.y,
                      object->translation.z  );

   /* rotation */
   kmMat4RotationX( &rotation, kmDegreesToRadians(object->rotation.x) );
   kmMat4Multiply(  &rotation, &rotation,
                    kmMat4RotationY( &temp, kmDegreesToRadians(object->rotation.y) ) );
   kmMat4Multiply(  &rotation, &rotation,
                    kmMat4RotationZ( &temp, kmDegreesToRadians(object->rotation.z) ) );

   /* scale */
   kmMat4Scaling( &scale,
                  object->scale.x,
                  object->scale.y,
                  object->scale.z  );

   /* build matrix */
   kmMat4Multiply( &translation, &translation, &rotation );
   kmMat4Multiply( &object->matrix, &translation, &scale );
   object->transform_changed = 0;
}

void dlDraw( dlObject *object )
{
   unsigned int i;
   CALL("%p", object);

   if(!object)
      return;
   if(!object->vbo)
      return;

   /* don't do any checking here expect for transformation.
    * draw loop is supposed to be fast. */
   if(_dlCore.render.mode == DL_MODE_VBO)
   {
      dlIBOUpdate( object->ibo );
      dlVBOUpdate( object->vbo );
   }

   if(object->transform_changed)
      dlUpdateMatrix( object );

   _dlCore.render.draw( object );

   /* draw childs */
   i = 0;
   for(; i != object->num_childs; ++i)
      dlDraw( object->child[i] );
}

/* Operations */

/* Calculate bounding box */
int dlObjectCalculateAABB( dlObject *object )
{
   unsigned int v = 0;
   kmVec3 min, max;
   kmAABB aabb_box;

   CALL("%p", object);

   if(!object)
      return( RETURN_FAIL );

   dlVBO *vbo = object->vbo;

   if(!vbo)
   { RET("%d", RETURN_FAIL); return( RETURN_FAIL ); }
   if(!vbo->vertices)
   { RET("%d", RETURN_FAIL); return( RETURN_FAIL ); }

   min = vbo->vertices[0],
   max = vbo->vertices[0];

   /* find min and max corners */
   for(; v != vbo->v_use; ++v)
   {
      if(vbo->vertices[v].x < min.x)
         min.x = vbo->vertices[v].x;
      if(vbo->vertices[v].x > max.x)
         max.x = vbo->vertices[v].x;

      if(vbo->vertices[v].y < min.y)
         min.y = vbo->vertices[v].y;
      if(vbo->vertices[v].y > max.y)
         max.y = vbo->vertices[v].y;

      if(vbo->vertices[v].z < min.z)
         min.z = vbo->vertices[v].z;
      if(vbo->vertices[v].z > max.z)
         max.z = vbo->vertices[v].z;
   }

   /* assign aabb to object */
   aabb_box.min = min;
   aabb_box.max = max;
   object->aabb_box = aabb_box;

#if 0
   printf("v_use: %u\n", vbo->v_num);
   printf("min: %f, %f, %f\n", min.x, min.y, min.z );
   printf("max: %f, %f, %f\n", max.x, max.y, max.z );
#endif

   RET("%d", RETURN_OK);
   return( RETURN_OK );
}

/* position sceneobject */
void dlPositionObject( dlObject *object, kmVec3 position )
{
   unsigned int i;

   object->translation        = position;
   object->transform_changed  = 1;

   i = 0;
   for(; i != object->num_childs; ++i)
      dlPositionObject( object->child[i], position );
}

/* position sceneobject */
void dlPositionObjectf( dlObject *object,
                        const kmScalar x, const kmScalar y, const kmScalar z )
{
   kmVec3 position;
   position.x = x; position.y = y; position.z = z;

   dlPositionObject( object, position );
}

/* move sceneobject */
void dlMoveObject( dlObject *object, const kmVec3 move )
{
   unsigned int i;

   kmVec3Add( &object->translation, &object->translation, &move );
   object->transform_changed  = 1;

   i = 0;
   for(; i != object->num_childs; ++i)
      dlMoveObject( object->child[i], move );
}

/* move sceneobject */
void dlMoveObjectf( dlObject *object,
                    const kmScalar x, const kmScalar y, const kmScalar z )
{
   kmVec3 move;
   move.x = x; move.y = y; move.z = z;

   dlMoveObject( object, move );
}

/* rotate sceneobject */
void dlRotateObject( dlObject *object, const kmVec3 rotate )
{
   unsigned int i;

   object->rotation = rotate;
   object->transform_changed = 1;

   i = 0;
   for(; i != object->num_childs; ++i)
      dlRotateObject( object->child[i], rotate );
}

/* rotate sceneobject */
void dlRotateObjectf( dlObject *object,
                      const kmScalar x, const kmScalar y, const kmScalar z )
{
   kmVec3 rotate;
   rotate.x = x, rotate.y = y; rotate.z = z;

   dlRotateObject( object, rotate );
}

/* scale sceneobject */
void dlScaleObject( dlObject *object, const kmVec3 scale )
{
   unsigned int i;

   object->scale = scale;
   object->transform_changed = 1;

   i = 0;
   for(; i != object->num_childs; ++i)
      dlScaleObject( object->child[i], scale );
}

/* scale sceneobject */
void dlScaleObjectf( dlObject *object,
                     const kmScalar x, const kmScalar y, const kmScalar z )
{
   kmVec3 scale;
   scale.x = x; scale.y = y; scale.z = z;

   dlScaleObject( object, scale );
}

/* shift object's texture */
void dlShiftObject( dlObject *object, int width, int height, unsigned int index, kmVec2 *baseCoords )
{
   dlTexture *texture;
   unsigned int windex, hindex, x;
   float awidth, aheight;
   kmVec2 pos;

   if(!object)
      return;
   if(!object->vbo)
      return;
   if(!object->material)
      return;
   if(!object->material->texture)
      return;

   texture = object->material->texture;
   if(!texture)
      return;

   if(!baseCoords)
      baseCoords = object->vbo->uvw[ texture->uvw ].coords;

   windex = 0;
   hindex = 1;

   x = 0;
   for(; x < index; x++)
   {
      if( width * windex >= texture->width - width )
      {
         windex = 0;
         ++hindex;
      }
      else
      {
         ++windex;
      }
   }
   pos.x = width * windex;
   pos.y = texture->height - height * hindex;

   awidth  = width  / (float)texture->width;
   aheight = height / (float)texture->height;

   x = 0;
   for(; x != object->vbo->uvw[ texture->uvw ].c_num; ++x)
   {
      object->vbo->uvw[ texture->uvw ].coords[ x ].x = baseCoords[x].x * awidth  + pos.x / (float)texture->width;
      object->vbo->uvw[ texture->uvw ].coords[ x ].y = baseCoords[x].y * aheight + pos.y / (float)texture->height;
   }

   object->vbo->up_to_date = 0;
   dlVBOUpdate( object->vbo );
}

/* offset object's texture */
void dlOffsetObjectTexture( dlObject *object, int px, int py, int width, int height, kmVec2 *baseCoords)
{
   dlTexture *texture;
   float awidth, aheight;
   unsigned int x;

   if(!object)
      return;
   if(!object->vbo)
      return;
   if(!object->material)
      return;
   if(!object->material->texture)
      return;

   texture = object->material->texture;
   if(!texture)
      return;

   if(!baseCoords)
      baseCoords = object->vbo->uvw[ texture->uvw ].coords;

   awidth  = width  / (float)texture->width;
   aheight = height / (float)texture->height;

   x = 0;
   for(; x != object->vbo->uvw[ texture->uvw ].c_num; ++x)
   {
      object->vbo->uvw[ texture->uvw ].coords[ x ].x = baseCoords[x].x * awidth  + px / (float)texture->width;
      object->vbo->uvw[ texture->uvw ].coords[ x ].y = baseCoords[x].y * aheight + py / (float)texture->height;
   }

   object->vbo->up_to_date = 0;
   dlVBOUpdate( object->vbo );
}

/* EoF */
