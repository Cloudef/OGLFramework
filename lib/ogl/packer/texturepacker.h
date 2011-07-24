#ifndef TEXTURE_PACKER_H
#define TEXTURE_PACKER_H

#ifdef __cplusplus
extern "C" {
#endif

typedef struct
{
   int                debugCount;
   struct node_tt    *freeList;
   int                textureIndex;
   int                textureCount;
   struct texture_tt *textures;
   int                longestEdge;
   int                totalArea;
} glTexturePacker;

/* set texture amount */
void glTexturePackerSetCount( glTexturePacker *tp, int tcount );

/* add texture */
int glTexturePackerAdd( glTexturePacker *tp, int wid, int hit );

/* pack the textures, return is amount of wasted/unused area */
int glTexturePackerPack( glTexturePacker *tp, int *width, int *height, int forcePowerOfTwo, int onePixelBorder );

/* return 1 if the texture has been rotated 90 degrees */
int glTexturePackerGetLocation( glTexturePacker *tp, int index, int *x, int *y, int *wid, int *hit );

glTexturePacker*   glNewTexturePacker(void);
void               glFreeTexturePacker(glTexturePacker *tp);

#ifdef __cplusplus
}
#endif

#endif /* TEXTURE_PACKER_H */
