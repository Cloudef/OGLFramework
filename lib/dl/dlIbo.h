#ifndef DL_IBO_H
#define DL_IBO_H

#include <stdint.h>
#include "dlConfig.h"

#ifdef __cplusplus
extern "C" {
#endif

/* IBO struct */
typedef struct dlIBO_t
{
#if USE_BUFFERS
   unsigned short *indices[dl_MAX_BUFFERS + 1];
   unsigned int   i_num[dl_MAX_BUFFERS + 1], i_use[dl_MAX_BUFFERS + 1];
   unsigned int   index_buffer;

   size_t         iOffset[dl_MAX_BUFFERS + 1];
#else
   unsigned int   *indices;
   unsigned int   i_num, i_use;
#endif

   /* dl IBO Object */
   unsigned int object;
   unsigned int hint;
   uint8_t      up_to_date;
   size_t       ibo_size;

   unsigned int refCounter;
} dlIBO;

dlIBO*      dlNewIBO( void );		     /* Allocate new ibo object */
dlIBO*      dlCopyIBO( dlIBO *src );	     /* Copy ibo object */
dlIBO*      dlRefIBO( dlIBO *src );	     /* Reference ibo object */
int         dlFreeIBO( dlIBO *ibo );	     /* Free ibo object */

/* IBO Actions */
int         dlIBOConstruct( dlIBO *ibo );
int         dlIBOUpdate( dlIBO *ibo );

/* Index buffer operations */
int         dlFreeIndexBuffer( dlIBO *ibo );
int         dlCopyIndexBuffer( dlIBO *ibo, dlIBO *src );
int         dlResetIndexBuffer( dlIBO *ibo, unsigned int indices );
int         dlInsertIndex( dlIBO *ibo, unsigned int index );

#ifdef __cplusplus
}
#endif

#endif /* DL_IBO_H */
