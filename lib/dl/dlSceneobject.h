#ifndef DL_SCENEOBJECT_H
#define DL_SCENEOBJECT_H

#include "dlVbo.h"
#include "dlIbo.h"
#include "dlMaterial.h"
#include "skeletal/dlAnimator.h"

#ifdef __cplusplus
extern "C" {
#endif

/* sceneobject struct */
typedef struct dlObject_t
{
   dlMaterial  *material;
   dlVBO       *vbo;
   dlIBO       *ibo;

   /* Animator */
   dlAnimator  *animator;

   /* Matrix */
   kmMat4 matrix;

   /* Translation */
   kmVec3 translation;
   kmVec3 rotation;
   kmVec3 scale;
   kmVec3 target;

   /* Bounding box */
   kmAABB aabb_box;

   /* GL Primitive type,
    * GL_TRIANGlES, GL_TRIANGLE_STRIP etc */
   unsigned int primitive_type;

   uint8_t      transform_changed;

   /* childs */
   struct dlObject_t **child;
   unsigned int num_childs;

   unsigned int refCounter;
} dlObject;

dlObject*   dlNewObject( void );		      /* Allocate sceneobject */
dlObject*   dlCopyObject( dlObject *src );	      /* Copy sceneobject  */
dlObject*   dlRefObject( dlObject *src );	      /* Reference sceneobject  */
int         dlFreeObject( dlObject *object );	      /* Free sceneobject */
void        dlDraw( dlObject *object );               /* Draw sceneobject */

void        dlObjectDrawSkeleton( dlObject *object );
void        dlObjectTick( dlObject *object, float tick );
void        dlObjectSetAnimation( dlObject *object, DL_NODE_TYPE );

int         dlObjectAddChild( dlObject*, dlObject* );      /* Add child */
dlObject**  dlObjectRefChilds( dlObject* );                /* Reference childs */
dlObject**  dlObjectCopyChilds( dlObject* );               /* Reference childs */
int         dlObjectFreeChild( dlObject*, dlObject* );     /* Remove child */
int         dlObjectFreeChilds( dlObject* );               /* Free all childs */

/* Operations */

/* Calculate AABB */
int dlObjectCalculateAABB( dlObject* );

/* Translation */
void dlPositionObject(  dlObject*, const kmVec3 );
void dlPositionObjectf( dlObject*,
                        const kmScalar x, const kmScalar y, const kmScalar z );

void dlMoveObject(  dlObject*, const kmVec3 );
void dlMoveObjectf( dlObject*,
                    const kmScalar x, const kmScalar y, const kmScalar z );

/* Rotation */
void dlRotateObject(  dlObject*, const kmVec3 );
void dlRotateObjectf( dlObject*,
                      const kmScalar x, const kmScalar y, const kmScalar z );

/* Scaling */
void dlScaleObject(  dlObject*, const kmVec3 );
void dlScaleObjectf( dlObject*,
                     const kmScalar x, const kmScalar y, const kmScalar z );

/* Shift object's texture to part from atlas texture */
void dlShiftObject( dlObject *object, int width, int height, unsigned int index, kmVec2 *baseCoords );
void dlOffsetObjectTexture( dlObject *object, int x, int y, int width, int height, kmVec2 *baseCoords );

/* Some basic geometry */
dlObject*   dlNewPlane( const kmScalar, const kmScalar, int center );
dlObject*   dlNewGrid( int, int, const kmScalar );
dlObject*   dlNewSprite( const char *image, unsigned int flags, int center );
dlObject*   dlNewStaticModel( const char *file );
dlObject*   dlNewDynamicModel( const char *file );

#ifdef __cplusplus
}
#endif

#endif /* GL_SCENEOBJECT_H */

/* EoF */
