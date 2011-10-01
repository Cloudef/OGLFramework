#ifndef DL_MATERIAL_H
#define DL_MATERIAL_H

#include "dlTexture.h"

#ifdef __cplusplus
extern "C" {
#endif

/* material flags */
typedef enum
{
   DL_MATERIAL_ALPHA = 1,
   DL_MATERIAL_WIREFRAME,
   DL_MATERIAL_DOUBLE_SIDED,
} DL_MATERIAL_FLAGS;

/* material struct */
typedef struct dlMaterial_t
{
   /* texture */
   dlTexture      **texture;

   /* draw flags */
   unsigned int flags;

   /* blend func */
   unsigned int blend1;
   unsigned int blend2;

   unsigned int   refCounter;
} dlMaterial;

dlMaterial*    dlNewMaterial( void );			       /* Allocate material */
dlMaterial*    dlCopyMaterial( dlMaterial *src );	       /* Copy material  */
dlMaterial*    dlRefMaterial( dlMaterial *src );	       /* Reference material  */
int            dlFreeMaterial( dlMaterial *object );	       /* Free material */

/* add texture */
int dlMaterialAddTexture( dlMaterial *object,
                          unsigned int index,
                          dlTexture *texture );

/* free texture */
int dlMaterialFreeTexture( dlMaterial *object,
                           unsigned int index );

/* free all textures */
int dlMaterialFreeTexturesAll( dlMaterial *object );

/* ref textures */
dlTexture** dlMaterialRefTextures( dlMaterial *object );

#ifdef __cplusplus
}
#endif

#endif /* DL_MATERIAL_H */

/* EoF */
