#ifndef GL_ATLAS_H
#define GL_ATLAS_H

#include "texture.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct glAtlasArea_t
{
   int x1;
   int x2;
   int y1;
   int y2;
   int rotated;
} glAtlasArea;

typedef struct glAtlasRect_t
{
   glTexture      *texture;
   unsigned int   index;
   glAtlasArea    packed;
} glAtlasRect;

typedef struct glAtlas_t
{
   /* textures and how they are
    * placed in atlas */
   glAtlasRect *rect;
   int num_textures;

   /* combined atlas texture */
   glTexture   *texture;

   int refCounter;
} glAtlas;

/* new atlas */
glAtlas* glNewAtlas(void);

/* reference atlas */
glAtlas* glRefAtlas( glAtlas* );

/* free atlas, reference the atlas texture if you want to keep it */
int      glFreeAtlas( glAtlas* );

/* add texture, steals reference */
int glAtlasAddTexture( glAtlas*, glTexture* );

/* free texture */
int glAtlasFreeTexture( glAtlas*, glTexture* );

/* free all textures */
int glAtlasFreeTextures( glAtlas* );

/* ref all textures */
glAtlasRect* glAtlasRefTextures( glAtlas* );

/* pack atlas */
glTexture* glAtlasPack( glAtlas*, int pow2, int border );

/* return packed area */
glAtlasArea* glAtlasGetPacked( glAtlas *atlas, glTexture *texture );

/* get transformed coordinates */
int glAtlasGetTransformed( glAtlas *atlas, glTexture *texture, kmVec2 *coord );

#ifdef __cplusplus
}
#endif

#endif /* GL_ATLAS_H */

/* EoF */
