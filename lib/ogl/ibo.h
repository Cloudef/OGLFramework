#ifndef GL_IBO_H
#define GL_IBO_H

#include <stdint.h>
#include "config.h"

#ifdef __cplusplus
extern "C" {
#endif

/* IBO struct */
typedef struct glIBO_t
{
#if USE_BUFFERS
	unsigned short *indices[GL_MAX_BUFFERS + 1];
   unsigned int   i_num[GL_MAX_BUFFERS + 1], i_use[GL_MAX_BUFFERS + 1];
   unsigned int   index_buffer;

   size_t         iOffset[GL_MAX_BUFFERS + 1];
#else
   unsigned int   *indices;
   unsigned int   i_num, i_use;
#endif

   /* GL IBO Object */
   unsigned int object;
   unsigned int hint;
   uint8_t      up_to_date;
   size_t ibo_size;

	unsigned int refCounter;
} glIBO;

glIBO*      glNewIBO( void );			                  /* Allocate new ibo object */
glIBO*      glCopyIBO( glIBO *src );	               /* Copy ibo object */
glIBO*      glRefIBO( glIBO *src );	                  /* Reference ibo object */
int         glFreeIBO( glIBO *ibo );		            /* Free ibo object */

/* IBO Actions */
int         glIBOConstruct( glIBO *ibo );
int         glIBOUpdate( glIBO *ibo );

/* Index buffer operations */
int         glFreeIndexBuffer( glIBO *ibo );
int         glCopyIndexBuffer( glIBO *ibo, glIBO *src );
int         glResetIndexBuffer( glIBO *ibo, unsigned int indices );
int         glInsertIndex( glIBO *ibo, unsigned int index );

#ifdef __cplusplus
}
#endif

#endif /* GL_IBO_H */
