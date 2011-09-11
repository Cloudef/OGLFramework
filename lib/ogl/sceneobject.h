#ifndef GL_SCENEOBJECT_H
#define GL_SCENEOBJECT_H

#include "vbo.h"
#include "ibo.h"
#include "material.h"
#include "skeletal/animator.h"

#ifdef __cplusplus
extern "C" {
#endif

/* sceneobject struct */
typedef struct glObject_t
{
   glMaterial  *material;
   glVBO       *vbo;
   glIBO       *ibo;

   /* Animator */
   glAnimator  *animator;

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
    * GL_TRIANGLES, GL_TRIANGLE_STRIP etc */
   unsigned int primitive_type;

   uint8_t      transform_changed;

   /* childs */
   struct glObject_t **child;
   unsigned int num_childs;

   unsigned int refCounter;
} glObject;

glObject*   glNewObject( void );		      /* Allocate sceneobject */
glObject*   glCopyObject( glObject *src );	      /* Copy sceneobject  */
glObject*   glRefObject( glObject *src );	      /* Reference sceneobject  */
int         glFreeObject( glObject *object );	      /* Free sceneobject */
void        glDraw( glObject *object );               /* Draw sceneobject */

void        glObjectDrawSkeleton( glObject *object );
void        glObjectTick( glObject *object, float tick );
void        glObjectSetAnimation( glObject *object, GL_NODE_TYPE );

int         glObjectAddChild( glObject*, glObject* );      /* Add child */
glObject**  glObjectRefChilds( glObject* );                /* Reference childs */
glObject**  glObjectCopyChilds( glObject* );               /* Reference childs */
int         glObjectFreeChild( glObject*, glObject* );     /* Remove child */
int         glObjectFreeChilds( glObject* );               /* Free all childs */

/* Operations */

/* Calculate AABB */
int glObjectCalculateAABB( glObject* );

/* Add texture to the object */
int glObjectAddTexture( glObject *object,
                        unsigned int index,
                        glTexture *texture );

/* Free texture */
int glObjectFreeTexture( glObject *object,
                         unsigned int index );

/* Free all textures */
int glObjectFreeTexturesAll( glObject *object );

/* Translation */
void glPositionObject(  glObject*, const kmVec3 );
void glPositionObjectf( glObject*,
                        const kmScalar x, const kmScalar y, const kmScalar z );

void glMoveObject(  glObject*, const kmVec3 );
void glMoveObjectf( glObject*,
                    const kmScalar x, const kmScalar y, const kmScalar z );

/* Rotation */
void glRotateObject(  glObject*, const kmVec3 );
void glRotateObjectf( glObject*,
                      const kmScalar x, const kmScalar y, const kmScalar z );

/* Scaling */
void glScaleObject(  glObject*, const kmVec3 );
void glScaleObjectf( glObject*,
                     const kmScalar x, const kmScalar y, const kmScalar z );

/* Shift object's texture to part from atlas texture */
void glShiftObject( glObject*, unsigned int uvw, int width, int height, unsigned int index, kmVec2 *baseCoords );
void glOffsetObjectTexture( glObject *object, unsigned int uvw, int x, int y, int width, int height, kmVec2 *baseCoords );

/* Some basic geometry */
glObject*   glNewPlane( const kmScalar, const kmScalar, int center );
glObject*   glNewGrid( int, int, const kmScalar );
glObject*   glNewStaticModel( const char *file );
glObject*   glNewDynamicModel( const char *file );

#ifdef __cplusplus
}
#endif

#endif /* GL_SCENEOBJECT_H */

/* EoF */
