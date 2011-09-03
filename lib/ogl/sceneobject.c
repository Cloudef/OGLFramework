#include "alloc.h"
#include "types.h"
#include "core.h"
#include "texture.h"

#ifdef GLES2
#	include <GLES2/gl2.h>
#elif  GLES1
#  include <GLES/gl.h>
#  include <GLES/glext.h>
#else
#	include <GL/glew.h>
#  include <GL/gl.h>
#endif

/* Allocate scene object */
glObject* glNewObject( void )
{
	/* Allocate scene object */
	glSetAlloc( ALLOC_SCENEOBJECT );
   glObject *object = (glObject*)glCalloc( 1, sizeof(glObject) );
	if(!object)
      return( NULL );

	/* Nullify pointers */
	object->material	      = NULL;
	object->vbo		         = NULL;
   object->ibo             = NULL;

   object->animator        = NULL;
   object->child           = NULL;

	/* Default primitive type */
	object->primitive_type = GL_TRIANGLE_STRIP;

   /* Default transformation */
   object->scale.x = 100; object->scale.y = 100; object->scale.z = 100;

   /* Update matrix on start */
   object->transform_changed = 1;

   logGreen();
   glPuts("[A:SCENEOBJECT]");
   logNormal();

	/* Increase ref counter */
	object->refCounter++;

	/* Return the instance */
	return( object );
}

/* Copy scene object */
glObject* glCopyObject( glObject *src )
{
   glObject *object;

   /* Fuuuuuuuuu--- We have non valid object */
	if(!src) return( NULL );

	/* Allocate scene object */
   glSetAlloc( ALLOC_SCENEOBJECT );
	object = (glObject*)glCalloc( 1, sizeof(glObject) );
	if(!object)
      return( NULL );

	/* Copy data */
	object->matrix                = src->matrix;
	object->translation	         = src->translation;
	object->rotation              = src->rotation;
	object->scale		            = src->scale;
	object->target			         = src->target;

   object->aabb_box              = src->aabb_box;

	/* Reference data */
	object->material	            = glRefMaterial( src->material );

   if(!src->animator) /* reference when no anim */
      object->vbo		            = glRefVBO( src->vbo );
   else
      object->vbo                = glCopyVBO( src->vbo );

   object->ibo                   = glRefIBO( src->ibo );
   object->animator              = glCopyAnimator( src->animator );

   /* Copy childs */
   object->child                 = glObjectCopyChilds( src );
   object->num_childs            = src->num_childs;

	/* Copy hints */
	object->primitive_type	      = src->primitive_type;

   /* Update it */
   object->transform_changed = 1;

   logYellow();
   glPuts("[C:SCENEOBJECT]");
   logNormal();

	/* Increase ref counter */
	object->refCounter++;

	/* Return the instance */
	return( object );
}

/* Reference scene object */
glObject* glRefObject( glObject *src )
{
   glObject* object;

	/* Fuuuuuuuuu--- We have non valid object */
	if(!src) return( NULL );

	/* Point magic */
	object                      = src;

	/* Reference data */
	object->material	         = glRefMaterial( src->material );
	object->vbo		            = glRefVBO( src->vbo );
   object->ibo                = glRefIBO( src->ibo );
   object->animator           = glRefAnimator( src->animator );

   logYellow();
   glPuts("[R:SCENEOBJECT]");
   logNormal();

	/* Increase ref counter */
	object->refCounter++;

	/* Return the instance */
	return( object );
}

/* Free scene object */
int glFreeObject( glObject *object )
{
   unsigned int i;

	/* Fuuuuuuuuu--- We have non valid object */
   if(!object) return( RETURN_NOTHING );

	/* Free material ( if any ) */
	if(glFreeMaterial( object->material ) == RETURN_OK)
      object->material = NULL;

	/* Free VBO Object */
   if(glFreeVBO( object->vbo )           == RETURN_OK)
      object->vbo = NULL;

   /* Free IBO Object */
   if(glFreeIBO( object->ibo )           == RETURN_OK)
      object->ibo = NULL;

   /* free animator if there is one */
   if(object->animator)
   { if( glFreeAnimator( object->animator ) == RETURN_OK ); object->animator = NULL; }

   /* Free as in, decrease reference on childs */
   i = 0;
   for(; i != object->num_childs; ++i)
   {
      if(glFreeObject( object->child[i] ) == RETURN_OK)
         object->child[i] = NULL;
   }

	/* There is still references to this object alive */
	if(--object->refCounter != 0) return( RETURN_NOTHING );

   glSetAlloc( ALLOC_SCENEOBJECT );

   /* Free child list */
   glFree( object->child, object->num_childs * sizeof(glObject*) );
   object->child = NULL;
   object->num_childs = 0;

   logRed();
   glPuts("[F:SCENEOBJECT]");
   logNormal();

	/* Free scene object */
   glFree( object, sizeof(glObject) );
   return( RETURN_OK );
}

/* draw skeleton */
void glObjectDrawSkeleton( glObject *object )
{
   glBone *bone;
   kmVec3 pos;

   if(!object)
      return;
   if(!object->animator)
      return;

#if 0
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
static void glObjectUpdateSkeletal( glObject *object )
{
   unsigned int x;
   glBone         *bone;
   glVertexWeight *weight;

   kmVec3 tStance;
   kmMat4 boneMat;
   glAnimator *animator = object->animator;

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
void glObjectTick( glObject *object, float tick )
{
   unsigned int i;

   if(!object)
      return;
   if(!object->animator)
      return;

   glAnimatorTick( object->animator, tick );

   if(!object->vbo)
      return;

   /* update vertices */
   i = 0; glObjectUpdateSkeletal( object );
   for(; i != object->num_childs; ++i)
      glObjectUpdateSkeletal( object->child[i] );
}

/* Set animation */
void glObjectSetAnimation( glObject *object, GL_NODE_TYPE index )
{
   if(!object)
      return;
   if(!object->animator)
      return;

   glAnimatorSetAnim( object->animator, index );
}

/* Add child, steals reference */
int glObjectAddChild( glObject *object, glObject *child )
{
   if(!object)
      return( RETURN_FAIL );
   if(!child)
      return( RETURN_FAIL );

   /* alloc */
   glSetAlloc( ALLOC_SCENEOBJECT );
   object->num_childs++;
   if(!object->child)
      object->child = glCalloc( object->num_childs, sizeof(glObject*) );
   else
      object->child = glRealloc( object->child, object->num_childs - 1,
                                 object->num_childs, sizeof(glObject*) );

   /* check success */
   if(!object->child)
      return( RETURN_FAIL );

   /* assign */
   object->child[ object->num_childs - 1 ] = child;

   return( RETURN_OK );
}

/* Reference all childs */
glObject** glObjectRefChilds( glObject *object )
{
   unsigned int i;

   if(!object)
      return( NULL );
   if(!object->child)
      return( NULL );

   /* reference all */
   i = 0;
   for(; i != object->num_childs; ++i)
   {
      glRefObject( object->child[i] );
   }

   return( object->child );
}

/* Copy all childs */
glObject** glObjectCopyChilds( glObject *object )
{
   unsigned int i;
   glObject     **newList;

   if(!object)
      return( NULL );
   if(!object->child)
      return( NULL );

   glSetAlloc( ALLOC_SCENEOBJECT );
   newList = glCalloc( object->num_childs, sizeof(glObject*) );
   if(!newList)
      return( NULL );

   /* reference all */
   i = 0;
   for(; i != object->num_childs; ++i)
   {
      newList[i] = glCopyObject( object->child[i] );
   }

   return( newList );
}

/* Free child */
int glObjectFreeChild( glObject *object, glObject *child )
{
   unsigned int i;
   unsigned int found;
   glObject     **tmp;

   if(!object)
      return( RETURN_FAIL );
   if(!child)
      return( RETURN_FAIL );
   if(!object->child)
      return( RETURN_FAIL );

   /* allocate tmp list */
   glSetAlloc( ALLOC_SCENEOBJECT );
   tmp = glCalloc( object->num_childs, sizeof(glObject*) );
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
      tmp = glRealloc( tmp, object->num_childs, found, sizeof(glObject*) );
      if(!tmp)
         return( RETURN_FAIL );
   }
   else
   {
      glFree( tmp, object->num_childs * sizeof(glObject*) );
      tmp = NULL;
   }

   /* ok, free the old list */
   glFree( object->child, object->num_childs * sizeof(glObject*) );
   glFreeObject( child );

   /* use the new list and new count */
   object->child        = tmp;
   object->num_childs   = found;

   return( RETURN_OK );
}

/* Free all childs */
int glObjectFreeChilds( glObject *object )
{
   unsigned int i;

   if(!object)
      return( RETURN_FAIL );
   if(!object->child)
      return( RETURN_FAIL );

   /* free all */
   i = 0;
   for(; i != object->num_childs; ++i)
   {
      if( glFreeObject( object->child[i] ) == RETURN_OK )
         object->child[i] = NULL;
   }

   glSetAlloc( ALLOC_SCENEOBJECT );
   glFree( object->child, object->num_childs * sizeof(glObject*) );
   object->child = NULL;

   object->num_childs = 0;
    return( RETURN_OK );
}

static void glUpdateMatrix( glObject *object )
{
   kmMat4 translation,
          rotation,
          scale,
          temp;

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

void glDraw( glObject *object )
{
   unsigned int i;

   if(!object)
      return;
   if(!object->vbo)
      return;

   /* don't do any checking here expect for transformation.
    * draw loop is supposed to be fast. */
   if(_glCore.render.mode == GL_MODE_VBO)
   {
      glIBOUpdate( object->ibo );
      glVBOUpdate( object->vbo );
   }

   if(object->transform_changed)
      glUpdateMatrix( object );

   _glCore.render.draw( object );

   /* draw childs */
   i = 0;
   for(; i != object->num_childs; ++i)
      glDraw( object->child[i] );
}

/* Operations */

/* Add texture to UVW, steals reference */
int glObjectAddTexture( glObject *object,
                        unsigned int index,
                        glTexture *texture )
{
   if(!object)
      return( RETURN_FAIL );

   if(!object->vbo)
      return( RETURN_FAIL );

   if(!object->material)
   {
      object->material = glNewMaterial();
      if(!object->material)
         return( RETURN_FAIL );
   }

   glMaterialAddTexture( object->material, index, texture );

   /* assign coords */
   if(object->vbo->uvw[index].c_use)
      texture->uvw = index;
   else
      texture->uvw = 0;

   return( RETURN_OK );
}

/* Free texture on UVW */
int glObjectFreeTexture( glObject *object,
                         unsigned int index )
{
   if(!object)
      return( RETURN_FAIL );

   if(!object->material)
      return( RETURN_FAIL );

   glMaterialFreeTexture( object->material, index );

   return( RETURN_OK );
}

/* Free all textures */
int glObjectFreeTexturesAll( glObject *object )
{
   if(!object)
      return( RETURN_FAIL );

   if(!object->material)
      return( RETURN_FAIL );

   glMaterialFreeTexturesAll( object->material );

   return( RETURN_OK );
}

/* Calculate bounding box */
int glObjectCalculateAABB( glObject *object )
{
   unsigned int v = 0;
   kmVec3 min, max;
   kmAABB aabb_box;

   if(!object)
      return( RETURN_FAIL );

   glVBO *vbo = object->vbo;

   if(!vbo)
      return( RETURN_FAIL );
   if(!vbo->vertices)
      return( RETURN_FAIL );

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

   return( RETURN_OK );
}

/* position sceneobject */
void glPositionObject( glObject *object, kmVec3 position )
{
   unsigned int i;

   object->translation        = position;
   object->transform_changed  = 1;

   i = 0;
   for(; i != object->num_childs; ++i)
      glPositionObject( object->child[i], position );
}

/* position sceneobject */
void glPositionObjectf( glObject *object,
                        const kmScalar x, const kmScalar y, const kmScalar z )
{
   kmVec3 position;
   position.x = x; position.y = y; position.z = z;

   glPositionObject( object, position );
}

/* move sceneobject */
void glMoveObject( glObject *object, const kmVec3 move )
{
   unsigned int i;

   kmVec3Add( &object->translation, &object->translation, &move );
   object->transform_changed  = 1;

   i = 0;
   for(; i != object->num_childs; ++i)
      glMoveObject( object->child[i], move );
}

/* move sceneobject */
void glMoveObjectf( glObject *object,
                    const kmScalar x, const kmScalar y, const kmScalar z )
{
   kmVec3 move;
   move.x = x; move.y = y; move.z = z;

   glMoveObject( object, move );
}

/* rotate sceneobject */
void glRotateObject( glObject *object, const kmVec3 rotate )
{
   unsigned int i;

   object->rotation = rotate;
   object->transform_changed = 1;

   i = 0;
   for(; i != object->num_childs; ++i)
      glRotateObject( object->child[i], rotate );
}

/* rotate sceneobject */
void glRotateObjectf( glObject *object,
                      const kmScalar x, const kmScalar y, const kmScalar z )
{
   kmVec3 rotate;
   rotate.x = x, rotate.y = y; rotate.z = z;

   glRotateObject( object, rotate );
}

/* scale sceneobject */
void glScaleObject( glObject *object, const kmVec3 scale )
{
   unsigned int i;

   object->scale = scale;
   object->transform_changed = 1;

   i = 0;
   for(; i != object->num_childs; ++i)
      glScaleObject( object->child[i], scale );
}

/* scale sceneobject */
void glScaleObjectf( glObject *object,
                     const kmScalar x, const kmScalar y, const kmScalar z )
{
   kmVec3 scale;
   scale.x = x; scale.y = y; scale.z = z;

   glScaleObject( object, scale );
}

/* shift object's texture */
void glShiftObject( glObject *object, unsigned int uvw, int width, int height, unsigned int index, kmVec2 *baseCoords )
{
   glTexture *texture;
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

   if(uvw > _glCore.info.maxTextureUnits)
      return;

   texture = object->material->texture[uvw];
   if(!texture)
      return;

   if(!baseCoords)
      baseCoords = object->vbo->uvw[ uvw ].coords;

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
	for(; x != object->vbo->uvw[ uvw ].c_num; ++x)
	{
		object->vbo->uvw[ uvw ].coords[ x ].x = baseCoords[x].x * awidth  + pos.x / (float)texture->width;
		object->vbo->uvw[ uvw ].coords[ x ].y = baseCoords[x].y * aheight + pos.y / (float)texture->height;
	}

   object->vbo->up_to_date = 0;
	glVBOUpdate( object->vbo );
}

/* offset object's texture */
void glOffsetObjectTexture( glObject *object, unsigned int uvw, int px, int py, int width, int height, kmVec2 *baseCoords)
{
   glTexture *texture;
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

   if(uvw > _glCore.info.maxTextureUnits)
      return;

   texture = object->material->texture[uvw];
   if(!texture)
      return;

   if(!baseCoords)
      baseCoords = object->vbo->uvw[ uvw ].coords;

   awidth  = width  / (float)texture->width;
   aheight = height / (float)texture->height;

   x = 0;
	for(; x != object->vbo->uvw[ uvw ].c_num; ++x)
	{
		object->vbo->uvw[ uvw ].coords[ x ].x = baseCoords[x].x * awidth  + px / (float)texture->width;
		object->vbo->uvw[ uvw ].coords[ x ].y = baseCoords[x].y * aheight + py / (float)texture->height;
	}

   object->vbo->up_to_date = 0;
	glVBOUpdate( object->vbo );
}

/* EoF */
