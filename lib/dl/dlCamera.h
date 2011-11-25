#ifndef DL_CAMERA_H
#define DL_CAMERA_H

#include <stdint.h>
#include "kazmath/kazmath.h"

#ifdef __cplusplus
extern "C" {
#endif

/* sceneobject struct */
typedef struct dlCamera_t
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
} dlCamera;

dlCamera*   dlNewCamera( void );		   /* Allocate camera */
dlCamera*   dlCopyCamera( dlCamera *src );	   /* Copy camera  */
dlCamera*   dlRefCamera( dlCamera *src );	   /* Reference camera  */
int         dlFreeCamera( dlCamera *object );	   /* Free camera */

/* translation */

void dlTargetCamera( dlCamera *object, kmVec3* ); /* Target camera */
void dlTargetCameraf( dlCamera *object, const kmScalar, const kmScalar,
                      const kmScalar );
void dlPositionCamera( dlCamera *object, kmVec3* ); /* Position camera */
void dlPositionCameraf( dlCamera *object, const kmScalar, const kmScalar,
                        const kmScalar );
void dlMoveCamera( dlCamera *object, kmVec3* ); /* Move camera */
void dlMoveCameraf( dlCamera *object, const kmScalar, const kmScalar,
                    const kmScalar );
void dlRotateCamera( dlCamera *object, kmVec3* ); /* Rotate camera */
void dlRotateCameraf( dlCamera *object, const kmScalar, const kmScalar,
                      const kmScalar );

/* settings */

void dlCameraSetView( dlCamera *object, const kmScalar, const kmScalar, const kmScalar,
                      const kmScalar );
void dlCameraSetFov( dlCamera *object, const kmScalar );
void dlCameraSetRange( dlCamera *object, const kmScalar, const kmScalar );

/* operation */

void dlCameraRender( dlCamera *object ); /* Render using the camera */

#ifdef __cplusplus
}
#endif

#endif /* DL_CAMERA_H */
