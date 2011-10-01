#include "dlCamera.h"
#include "dlFramework.h"
#include "dlTypes.h"
#include "dlAlloc.h"
#include "dlLog.h"

#ifdef GLES2
#	include <GLES2/gl2.h>
#elif  GLES1
#  include <GLES/gl.h>
#  include <GLES/glext.h>
#else
#	include <GL/glew.h>
#  include <GL/gl.h>
#endif

/* Calculate projection matrix */
static void dlCameraCalculateProjectionMatrix( dlCamera *object )
{
   kmMat4PerspectiveProjection( &object->projection, object->fov, object->aspect,
                                object->zNear, object->zFar );
}

/* Calculate view matrix */
static void dlCameraCalculateViewMatrix( dlCamera *object )
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
static void dlCameraReset( dlCamera *object )
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
   object->viewSize.x = _dlCore.display.width;
   object->viewSize.y = _dlCore.display.height;

   glViewport( (int)object->viewCut.x,  (int)object->viewCut.y,
               (int)object->viewSize.x, (int)object->viewSize.y );

   object->transform_changed = 1;
   object->viewport_changed  = 1;

   object->aspect = (float)_dlCore.display.width/(float)_dlCore.display.height;

   dlCameraCalculateProjectionMatrix( object );
   dlCameraCalculateViewMatrix( object );
}

/* Allocate camera */
dlCamera* dlNewCamera( void )
{
   /* Allocate camera */
   dlSetAlloc( ALLOC_CAMERA );
   dlCamera *object = (dlCamera*)dlCalloc( 1, sizeof(dlCamera) );
   if(!object)
      return( NULL );

   /* Defaults */
   object->zNear = 1.0;
   object->zFar  = 3800.0;
   object->fov   = kmPI / 2.0;

   /* Reset */
   dlCameraReset( object );

   logGreen();
   dlPuts("[A:CAMERA]");
   logNormal();

   /* Increase ref counter */
   object->refCounter++;

   /* Return the instance */
   return( object );
}

/* Copy camera */
dlCamera* dlCopyCamera( dlCamera *src )
{
   dlCamera *object;

   /* Fuuuuuuuuu--- We have non valid object */
   if(!src) return( NULL );

   /* Allocate scene object */
   dlSetAlloc( ALLOC_CAMERA );
   object = (dlCamera*)dlCalloc( 1, sizeof(dlCamera) );
   if(!object)
      return( NULL );

   logYellow();
   dlPuts("[C:CAMERA]");
   logNormal();

   /* Increase ref counter */
   object->refCounter++;

   /* Return the instance */
   return( object );
}

/* Reference camera */
dlCamera* dlRefCamera( dlCamera *src )
{
   dlCamera* object;

   /* Fuuuuuuuuu--- We have non valid object */
   if(!src) return( NULL );

   /* Point magic */
   object                      = src;

   logYellow();
   dlPuts("[R:CAMERA]");
   logNormal();

   /* Increase ref counter */
   object->refCounter++;

   /* Return the instance */
   return( object );
}

/* Free camera */
int dlFreeCamera( dlCamera *object )
{
   /* Fuuuuuuuuu--- We have non valid object */
   if(!object) return( RETURN_NOTHING );

   /* There is still references to this object alive */
   if(--object->refCounter != 0) return( RETURN_NOTHING );

   dlSetAlloc( ALLOC_CAMERA );

   /* Check if this is active */
   if(dlGetCamera() == object)
      dlSetCamera( NULL );

   logRed();
   dlPuts("[F:CAMERA]");
   logNormal();

   /* Free camera */
   dlFree( object, sizeof(dlCamera) );
   return( RETURN_OK );
}

/* Position camera */
void dlPositionCamera( dlCamera *object, const kmVec3 position )
{
   if(!object)
      return;

   object->translation = position;
   object->transform_changed = 1;
}

/* Position camera */
void dlPositionCameraf( dlCamera *object, const kmScalar x, const kmScalar y,
                        const kmScalar z )
{
   kmVec3 position;
   position.x = x;
   position.y = y;
   position.z = z;

   dlPositionCamera( object, position );
}

/* Move camera */
void dlMoveCamera( dlCamera *object, const kmVec3 move )
{
   if(!object)
      return;

   kmVec3Add( &object->translation, &object->translation, &move );
   object->transform_changed = 1;
}

/* Move camera */
void dlMoveCameraf( dlCamera *object, const kmScalar x, const kmScalar y,
                    const kmScalar z )
{
   kmVec3 move;
   move.x = x;
   move.y = y;
   move.z = z;

   dlMoveCamera( object, move );
}

/* Rotate camera */
void dlRotateCamera( dlCamera *object, const kmVec3 rotation )
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
void dlRotateCameraf( dlCamera *object, const kmScalar x, const kmScalar y,
                      const kmScalar z )
{
   kmVec3 rotation;
   rotation.x = x;
   rotation.y = y;
   rotation.z = z;

   dlRotateCamera( object, rotation );
}

/* Target camera */
void dlTargetCamera( dlCamera *object, const kmVec3 target )
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
void dlTargetCameraf( dlCamera *object, const kmScalar x, const kmScalar y,
                      const kmScalar z )
{
   kmVec3 target;
   target.x = x;
   target.y = y;
   target.z = z;

   dlTargetCamera( object, target );
}

/* Set viewport of camera */
void dlCameraSetView( dlCamera *object, const kmScalar x, const kmScalar y,
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
void dlCameraSetFov( dlCamera *object, const kmScalar fov )
{
   if(!object)
      return;

   object->fov = fov;
}

/* Set camera range */
void dlCameraSetRange( dlCamera *object, const kmScalar zNear, const kmScalar zFar )
{
   if(!object)
      return;

   object->zNear = zNear;
   object->zFar  = zFar;
}

/* Render using this camera */
void dlCameraRender( dlCamera *object )
{
   if(!object)
      return;

   if( object->viewport_changed || dlGetCamera() != object )
   {
      glViewport( (int)object->viewCut.x,  (int)object->viewCut.y,
                  (int)object->viewSize.x, (int)object->viewSize.y );
      object->viewport_changed = 0;
   }

   if(object->transform_changed)
   {
      dlCameraCalculateViewMatrix( object );
      object->transform_changed = 0;
   }

   /* switch active */
   dlSetCamera( object );
}

/* EoF */
