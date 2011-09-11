#ifndef GL_CAMERA_H
#define GL_CAMERA_H

#include <stdint.h>
#include "kazmath/kazmath.h"

#ifdef __cplusplus
extern "C" {
#endif

/* sceneobject struct */
typedef struct glCamera_t
{
   kmVec3 target;
   kmVec3 upVector;

   kmVec3 translation;
   kmVec3 rotation;

   kmVec2 viewCut;
   kmVec2 viewSize;

   float fov;
   float aspect;
   float zNear;
   float zFar;

   kmMat4 projection;
   kmMat4 view;
   kmMat4 matrix;

   uint8_t transform_changed;
   uint8_t viewport_changed;

   unsigned int refCounter;
} glCamera;

glCamera*   glNewCamera( void );		   /* Allocate camera */
glCamera*   glCopyCamera( glCamera *src );	   /* Copy camera  */
glCamera*   glRefCamera( glCamera *src );	   /* Reference camera  */
int         glFreeCamera( glCamera *object );	   /* Free camera */

/* translation */

void glTargetCamera( glCamera *object, const kmVec3 ); /* Target camera */
void glTargetCameraf( glCamera *object, const kmScalar, const kmScalar,
                      const kmScalar );
void glPositionCamera( glCamera *object, const kmVec3 ); /* Position camera */
void glPositionCameraf( glCamera *object, const kmScalar, const kmScalar,
                        const kmScalar );
void glMoveCamera( glCamera *object, const kmVec3 ); /* Move camera */
void glMoveCameraf( glCamera *object, const kmScalar, const kmScalar,
                    const kmScalar );
void glRotateCamera( glCamera *object, const kmVec3 ); /* Rotate camera */
void glRotateCameraf( glCamera *object, const kmScalar, const kmScalar,
                      const kmScalar );

/* settings */

void glCameraSetView( glCamera *object, const kmScalar, const kmScalar, const kmScalar,
                      const kmScalar );
void glCameraSetFov( glCamera *object, const kmScalar );
void glCameraSetRange( glCamera *object, const kmScalar, const kmScalar );

/* operation */

void glCameraRender( glCamera *object ); /* Render using the camera */

#ifdef __cplusplus
}
#endif

#endif /* GL_CAMERA_H */

/* EoF */
