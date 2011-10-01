#ifndef DL_TEXTURE_PACKER_H
#define DL_TEXTURE_PACKER_H

#ifdef __cplusplus
extern "C" {
#endif

typedef struct dlTexturePacker_t
{
   int                debugCount;
   struct node_tt    *freeList;
   int                textureIndex;
   int                textureCount;
   struct texture_tt *textures;
   int                longestEdge;
   int                totalArea;
} dlTexturePacker;

/* set texture amount */
void dlTexturePackerSetCount( dlTexturePacker *tp, int tcount );

/* add texture */
int dlTexturePackerAdd( dlTexturePacker *tp, int wid, int hit );

/* pack the textures, return is amount of wasted/unused area */
int dlTexturePackerPack( dlTexturePacker *tp, int *width, int *height, int forcePowerOfTwo, int onePixelBorder );

/* return 1 if the texture has been rotated 90 degrees */
int dlTexturePackerGetLocation( dlTexturePacker *tp, int index, int *x, int *y, int *wid, int *hit );

dlTexturePacker*   dlNewTexturePacker(void);
void               dlFreeTexturePacker(dlTexturePacker *tp);

#ifdef __cplusplus
}
#endif

#endif /* DL_TEXTURE_PACKER_H */
