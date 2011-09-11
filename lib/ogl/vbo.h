#ifndef GL_VBO_H
#define GL_VBO_H

#include <stdint.h>

#include "kazmath/kazmath.h"
#include "scolor.h"
#include "config.h"
#include "texture.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct glUVW_t
{
   /* coordinates */
   kmVec2         *coords;
   unsigned int   c_num, c_use;

   /* VBO Offset */
   size_t cOffset;

} glUVW;

/* VBO struct */
typedef struct glVBO_t
{
   /* vertice, coord, normal arrays */
   kmVec3   *vertices;
   glUVW    *uvw;
   kmVec3   *normals;

   /* only used for animation */
   kmVec3   *tstance;

#if VERTEX_COLOR
   sColor   *colors;
   unsigned int c_num, c_use;
#endif

   /* num = amount allocated, use = amount used
    * num for memcpys, use for everything else */
   unsigned int v_num, v_use;
   unsigned int n_num, n_use;

   /* GL VBO object */
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
} glVBO;

glVBO*      glNewVBO( void );	       /* Allocate new vbo object */
glVBO*      glCopyVBO( glVBO *src );   /* Copy vbo object */
glVBO*      glRefVBO( glVBO *src );    /* Reference vbo object */
int         glFreeVBO( glVBO *vbo );   /* Free vbo object */

/* VBO actions */
int         glVBOConstruct( glVBO *vbo );
int         glVBOUpdate( glVBO *vbo );

/* copy tstance vertices if animation is used */
int glVBOPrepareTstance( glVBO *vbo );

/* Vertex buffer operations */
int         glFreeVertexBuffer( glVBO *vbo );
int         glCopyVertexBuffer( glVBO *vbo, glVBO *src );
int         glResetVertexBuffer( glVBO *vbo, unsigned int vertices );
int         glInsertVertex( glVBO *vbo,
                            const kmScalar x, const kmScalar y, const kmScalar z );

#if VERTEX_COLOR
/* Normal buffer operations */
int         glFreeColorBuffer( glVBO *vbo );
int         glCopyColorBuffer( glVBO *vbo, glVBO *src );
int         glResetColorBuffer( glVBO *vbo, unsigned int vertices );
int         glInsertColor( glVBO *vbo,
                           const uint8_t r, const uint8_t g, const uint8_t b, const uint8_t a );
#endif

/* Texture coord buffer operations */
int         glFreeCoordBuffer( glVBO *vbo, unsigned int index );
int         glCopyCoordBuffer( glVBO *vbo, glVBO *src, unsigned int index );
int         glResetCoordBuffer( glVBO *vbo, unsigned int index,
                                 unsigned int vertices );
int         glInsertCoord( glVBO *vbo, unsigned int index,
                           const kmScalar x, const kmScalar y );

/* Normal buffer operations */
int         glFreeNormalBuffer( glVBO *vbo );
int         glCopyNormalBuffer( glVBO *vbo, glVBO *src );
int         glResetNormalBuffer( glVBO *vbo, unsigned int vertices );
int         glInsertNormal( glVBO *vbo,
                            const kmScalar x, const kmScalar y, const kmScalar z );

#ifdef __cplusplus
}
#endif

#endif /* GL_VBO_H */
