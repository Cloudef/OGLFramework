#ifndef DL_VBO_H
#define DL_VBO_H

#include <stdint.h>

#include "kazmath/kazmath.h"
#include "dlScolor.h"
#include "dlConfig.h"
#include "dlTexture.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct dlUVW_t
{
   /* coordinates */
   kmVec2         *coords;
   unsigned int   c_num, c_use;

   /* VBO Offset */
   size_t cOffset;

} dlUVW;

/* VBO struct */
typedef struct dlVBO_t
{
   /* vertice, coord, normal arrays */
   kmVec3   *vertices;
   dlUVW    *uvw;
   kmVec3   *normals;

   /* only used for animation */
   kmVec3   *tstance;

#if VERTEX_COLOR
   dlColor   *colors;
   unsigned int c_num, c_use;
#endif

   /* num = amount allocated, use = amount used
    * num for memcpys, use for everything else */
   unsigned int v_num, v_use;
   unsigned int n_num, n_use;

   /* dl VBO object */
   unsigned int object;
   int          hint;
   uint8_t      up_to_date;

   /* VBO Offsets */
   size_t vbo_size;
   size_t vOffset, nOffset;
#if VERTEX_COLOR
   size_t cOffset;
#endif

   unsigned int refCounter;
} dlVBO;

dlVBO*      dlNewVBO( void );	       /* Allocate new vbo object */
dlVBO*      dlCopyVBO( dlVBO *src );   /* Copy vbo object */
dlVBO*      dlRefVBO( dlVBO *src );    /* Reference vbo object */
int         dlFreeVBO( dlVBO *vbo );   /* Free vbo object */

/* VBO actions */
int         dlVBOConstruct( dlVBO *vbo );
int         dlVBOUpdate( dlVBO *vbo );

/* copy tstance vertices if animation is used */
int dlVBOPrepareTstance( dlVBO *vbo );

/* Vertex buffer operations */
int         dlFreeVertexBuffer( dlVBO *vbo );
int         dlCopyVertexBuffer( dlVBO *vbo, dlVBO *src );
int         dlResetVertexBuffer( dlVBO *vbo, unsigned int vertices );
int         dlInsertVertex( dlVBO *vbo,
                            const kmScalar x, const kmScalar y, const kmScalar z );

#if VERTEX_COLOR
/* Normal buffer operations */
int         dlFreeColorBuffer( dlVBO *vbo );
int         dlCopyColorBuffer( dlVBO *vbo, dlVBO *src );
int         dlResetColorBuffer( dlVBO *vbo, unsigned int vertices );
int         dlInsertColor( dlVBO *vbo,
                           const uint8_t r, const uint8_t g, const uint8_t b, const uint8_t a );
#endif

/* Texture coord buffer operations */
int         dlFreeCoordBuffer( dlVBO *vbo, unsigned int index );
int         dlCopyCoordBuffer( dlVBO *vbo, dlVBO *src, unsigned int index );
int         dlResetCoordBuffer( dlVBO *vbo, unsigned int index,
                                 unsigned int vertices );
int         dlInsertCoord( dlVBO *vbo, unsigned int index,
                           const kmScalar x, const kmScalar y );

/* Normal buffer operations */
int         dlFreeNormalBuffer( dlVBO *vbo );
int         dlCopyNormalBuffer( dlVBO *vbo, dlVBO *src );
int         dlResetNormalBuffer( dlVBO *vbo, unsigned int vertices );
int         dlInsertNormal( dlVBO *vbo,
                            const kmScalar x, const kmScalar y, const kmScalar z );

#ifdef __cplusplus
}
#endif

#endif /* DL_VBO_H */
