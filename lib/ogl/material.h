#ifndef GL_MATERIAL_H
#define GL_MATERIAL_H

#include "texture.h"

#ifdef __cplusplus
extern "C" {
#endif

/* material flags */
typedef enum
{
   GL_MATERIAL_ALPHA = 1,
   GL_MATERIAL_WIREFRAME,
   GL_MATERIAL_DOUBLE_SIDED,
} GL_MATERIAL_FLAGS;

/* material struct */
typedef struct glMaterial_t
{
   /* texture */
   glTexture      **texture;

   /* draw flags */
   unsigned int flags;

   /* blend func */
   unsigned int blend1;
   unsigned int blend2;

   unsigned int   refCounter;
} glMaterial;

glMaterial*    glNewMaterial( void );			       /* Allocate material */
glMaterial*    glCopyMaterial( glMaterial *src );	       /* Copy material  */
glMaterial*    glRefMaterial( glMaterial *src );	       /* Reference material  */
int            glFreeMaterial( glMaterial *object );	       /* Free material */

/* add texture */
int glMaterialAddTexture( glMaterial *object,
                          unsigned int index,
                          glTexture *texture );

/* free texture */
int glMaterialFreeTexture( glMaterial *object,
                           unsigned int index );

/* free all textures */
int glMaterialFreeTexturesAll( glMaterial *object );

/* ref textures */
glTexture** glMaterialRefTextures( glMaterial *object );

#ifdef __cplusplus
}
#endif

#endif /* GL_MATERIAL_H */

/* EoF */
