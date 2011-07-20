#include "camera.h"
#include "framework.h"
#include "types.h"
#include "alloc.h"

/* Calculate projection matrix */
static void glCameraCalculateProjectionMatrix( glCamera *object )
{
   kmMat4PerspectiveProjection( &object->projection, object->fov, object->aspect,
                                object->zNear, object->zFar );
}

/* Calculate view matrix */
static void glCameraCalculateViewMatrix( glCamera *object )
{
   kmVec3 tgtv, upvector;
   kmScalar dp;

   kmVec3Subtract( &tgtv, &object->target, &object->translation );

   kmVec3Normalize( &tgtv, &tgtv );
   kmVec3Normalize( &upvector, &object->upVector );

   dp = kmVec3Dot( &tgtv, &upvector );
   if( dp == 1.f )
   {
      upvector.x += 0.5f;
   }

   kmMat4LookAt( &object->view, &object->translation, &object->target, &upvector );

   /* Frustum recalculation here */

   kmMat4Multiply( &object->matrix, &object->projection, &object->view );
}

/* Reset camera */
static void glCameraReset( glCamera *object )
{
   object->upVector.x = 0;
   object->upVector.y = 1;
   object->upVector.z = 0;

   object->rotation.x = 0;
   object->rotation.y = 0;
   object->rotation.z = 0;

   object->target.x = 0;
   object->target.y = 0;
   object->target.z = 0;

   object->translation.x = 0;
   object->translation.y = 0;
   object->translation.z = 40;   /* Take camera bit back */

   object->viewCut.x = 0;
   object->viewCut.y = 0;
   object->viewSize.x = _glCore.display.width;
   object->viewSize.y = _glCore.display.height;

   glViewport( (int)object->viewCut.x,  (int)object->viewCut.y,
               (int)object->viewSize.x, (int)object->viewSize.y );

   object->transform_changed = 1;
   object->viewport_changed  = 1;

   object->aspect = (float)_glCore.display.width/(float)_glCore.display.height;

   glCameraCalculateProjectionMatrix( object );
   glCameraCalculateViewMatrix( object );
}

/* Allocate camera */
glCamera* glNewCamera( void )
{
	/* Allocate camera */
	glCamera *object = (glCamera*)glCalloc( 1, sizeof(glCamera) );
	if(!object)
      return( NULL );

   /* Defaults */
   object->zNear = 1.0;
   object->zFar  = 3800.0;
   object->fov   = kmPI / 2.0;

   /* Reset */
   glCameraReset( object );

	/* Increase ref counter */
	object->refCounter++;

	/* Return the instance */
	return( object );
}

/* Copy camera */
glCamera* glCopyCamera( glCamera *src )
{
   glCamera *object;

   /* Fuuuuuuuuu--- We have non valid object */
	if(!src) return( NULL );

	/* Allocate scene object */
	object = (glCamera*)glCalloc( 1, sizeof(glCamera) );
	if(!object)
      return( NULL );

	/* Increase ref counter */
	object->refCounter++;

	/* Return the instance */
	return( object );
}

/* Reference camera */
glCamera* glRefCamera( glCamera *src )
{
   glCamera* object;

	/* Fuuuuuuuuu--- We have non valid object */
	if(!src) return( NULL );

	/* Point magic */
	object                      = src;

	/* Increase ref counter */
	object->refCounter++;

	/* Return the instance */
	return( object );
}

/* Free camera */
int glFreeCamera( glCamera *object )
{
	/* Fuuuuuuuuu--- We have non valid object */
   if(!object) return( RETURN_NOTHING );

	/* There is still references to this object alive */
	if(--object->refCounter != 0) return( RETURN_NOTHING );

   /* Check if this is active */
   if(glGetCamera() == object)
      glSetCamera( NULL );

	/* Free camera */
   glFree( object, sizeof(glCamera) );
   return( RETURN_OK );
}

/* Position camera */
void glPositionCamera( glCamera *object, const kmVec3 position )
{
   if(!object)
      return;

   object->translation = position;
   object->transform_changed = 1;
}

/* Position camera */
void glPositionCameraf( glCamera *object, const kmScalar x, const kmScalar y,
                        const kmScalar z )
{
   kmVec3 position;
   position.x = x;
   position.y = y;
   position.z = z;

   glPositionCamera( object, position );
}

/* Move camera */
void glMoveCamera( glCamera *object, const kmVec3 move )
{
   if(!object)
      return;

   kmVec3Add( &object->translation, &object->translation, &move );
   object->transform_changed = 1;
}

/* Move camera */
void glMoveCameraf( glCamera *object, const kmScalar x, const kmScalar y,
                    const kmScalar z )
{
   kmVec3 move;
   move.x = x;
   move.y = y;
   move.z = z;

   glMoveCamera( object, move );
}

/* Rotate camera */
void glRotateCamera( glCamera *object, const kmVec3 rotation )
{
   kmVec3 rotToDir, forwards;
   forwards.x = 0;
   forwards.y = 0;
   forwards.z = 1;

   if(!object)
      return;

   object->rotation = rotation;

   /* update target */
   kmVec3RotationToDirection( &rotToDir, &object->rotation, &forwards );
   kmVec3Add( &object->target, &object->translation, &rotToDir );

   object->transform_changed = 1;
}

/* Rotate camera */
void glRotateCameraf( glCamera *object, const kmScalar x, const kmScalar y,
                      const kmScalar z )
{
   kmVec3 rotation;
   rotation.x = x;
   rotation.y = y;
   rotation.z = z;

   glRotateCamera( object, rotation );
}

/* Target camera */
void glTargetCamera( glCamera *object, const kmVec3 target )
{
   kmVec3 toTarget;

   if(!object)
      return;

   object->target = target;
   kmVec3Subtract( &toTarget, &object->target, &object->translation );

   /* update rotation */
   kmVec3GetHorizontalAngle( &object->rotation, &toTarget );

   object->transform_changed = 1;
}

/* Target camera */
void glTargetCameraf( glCamera *object, const kmScalar x, const kmScalar y,
                      const kmScalar z )
{
   kmVec3 target;
   target.x = x;
   target.y = y;
   target.z = z;

   glTargetCamera( object, target );
}

/* Set viewport of camera */
void glCameraSetView( glCamera *object, const kmScalar x, const kmScalar y,
                                        const kmScalar w, const kmScalar h )
{
   if(!object)
      return;

   object->viewCut.x  = x;
   object->viewCut.y  = y;
   object->viewSize.x = w;
   object->viewSize.y = h;

   object->viewport_changed = 1;
}

/* Set fov of camera */
void glCameraSetFov( glCamera *object, const kmScalar fov )
{
   if(!object)
      return;

   object->fov = fov;
}

/* Set camera range */
void glCameraSetRange( glCamera *object, const kmScalar zNear, const kmScalar zFar )
{
   if(!object)
      return;

   object->zNear = zNear;
   object->zFar  = zFar;
}

/* Render using this camera */
void glCameraRender( glCamera *object )
{
   if(!object)
      return;

   if( object->viewport_changed || glGetCamera() != object )
   {
      glViewport( (int)object->viewCut.x,  (int)object->viewCut.y,
                  (int)object->viewSize.x, (int)object->viewSize.y );
      object->viewport_changed = 0;
   }

   /* switch active */
   glSetCamera( object );

   if(object->transform_changed)
   {
      glCameraCalculateViewMatrix( object );
      object->transform_changed = 0;
   }

   glSetProjection( object->matrix );
}

/* EoF */
