#ifndef DL_ATLAS_H
#define DL_ATLAS_H

#include "dlTexture.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct dlAtlasArea_t
{
   int x1;
   int x2;
   int y1;
   int y2;
   int rotated;
} dlAtlasArea;

typedef struct dlAtlasRect_t
{
   dlTexture      *texture;
   unsigned int   index;
   dlAtlasArea    packed;
} dlAtlasRect;

typedef struct dlAtlas_t
{
   /* textures and how they are
    * placed in atlas */
   dlAtlasRect *rect;
   int num_textures;

   /* combined atlas texture */
   dlTexture   *texture;

   int refCounter;
} dlAtlas;

/* new atlas */
dlAtlas* dlNewAtlas(void);

/* reference atlas */
dlAtlas* dlRefAtlas( dlAtlas* );

/* free atlas, reference the atlas texture if you want to keep it */
int      dlFreeAtlas( dlAtlas* );

/* add texture, steals reference */
int dlAtlasAddTexture( dlAtlas*, dlTexture* );

/* free texture */
int dlAtlasFreeTexture( dlAtlas*, dlTexture* );

/* free all textures */
int dlAtlasFreeTextures( dlAtlas* );

/* ref all textures */
dlAtlasRect* dlAtlasRefTextures( dlAtlas* );

/* pack atlas */
dlTexture* dlAtlasPack( dlAtlas*, int pow2, int border );

/* return packed area */
dlAtlasArea* dlAtlasGetPacked( dlAtlas *atlas, dlTexture *texture );

/* get transformed coordinates */
int dlAtlasGetTransformed( dlAtlas *atlas, dlTexture *texture, kmVec2 *coord );

#ifdef __cplusplus
}
#endif

#endif /* DL_ATLAS_H */
