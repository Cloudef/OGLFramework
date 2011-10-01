#ifndef DL_MALLOC_H
#define DL_MALLOC_H

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
} dleAlloc;

#ifdef DEBUG
extern dleAlloc DL_D_ALLOC;
#define dlSetAlloc( X ) DL_D_ALLOC = X;
#else
#define dlSetAlloc( X ) ;
#endif

/* internal allocation functions */
void dlFakeAlloc( size_t ); /* fake allocation */
void* dlMalloc( size_t );
void* dlCalloc( unsigned int, size_t );
void* dlRealloc( void*, unsigned int, unsigned int, size_t );
void* dlCopy( void*, size_t );
int dlFree( void*, size_t );

#ifdef __cplusplus
}
#endif

#endif /* DL_MALLOC_H */

/* EoF */
