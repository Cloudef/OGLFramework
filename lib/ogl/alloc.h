#ifndef GL_MALLOC_H
#define GL_MALLOC_H

#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum
{
   ALLOC_CORE,          /* Framework internal allocs */
   ALLOC_CAMERA,        /* Cameras */
   ALLOC_SCENEOBJECT,   /* Sceneobjects */
   ALLOC_IBO,           /* IBOs */
   ALLOC_VBO,           /* VBOs */
   ALLOC_ANIM,          /* Animations */
   ALLOC_BONE,          /* Bones */
   ALLOC_ANIMATOR,      /* Animators */
   ALLOC_EVALUATOR,     /* Evaluators */
   ALLOC_SHADER,        /* Shaders */
   ALLOC_MATERIAL,      /* Materials */
   ALLOC_TEXTURE,       /* Textures */
   ALLOC_TEXTURE_CACHE, /* Texture cache */
   ALLOC_ATLAS,         /* Atlases */
   ALLOC_TOTAL,         /* Total */
   ALLOC_LAST
} gleAlloc;

#ifdef DEBUG
extern gleAlloc GL_D_ALLOC;
#define glSetAlloc( X ) GL_D_ALLOC = X;
#else
#define glSetAlloc( X ) ;
#endif

/* internal allocation functions */
void glFakeAlloc( size_t ); /* fake allocation */
void* glMalloc( size_t );
void* glCalloc( unsigned int, size_t );
void* glRealloc( void*, unsigned int, unsigned int, size_t );
void* glCopy( void*, size_t );
int glFree( void*, size_t );

#ifdef __cplusplus
}
#endif

#endif /* GL_MALLOC_H */

/* EoF */
