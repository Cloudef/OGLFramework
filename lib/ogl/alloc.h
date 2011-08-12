#ifndef GL_MALLOC_H
#define GL_MALLOC_H

#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

/* internal allocation functions */
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
