#ifndef DL_TEXTURE_H
#define DL_TEXTURE_H

#include <stdint.h>
#include "kazmath/kazmath.h"

#ifdef __cplusplus
extern "C" {
#endif

/* texture flags, block to allow conflict with actual SOIL library */
#ifndef SOIL_FLAG_T
typedef enum
{
   SOIL_FLAG_POWER_OF_TWO        = 1,
   SOIL_FLAG_MIPMAPS             = 2,
   SOIL_FLAG_TEXTURE_REPEATS     = 4,
   SOIL_FLAG_MULTIPLY_ALPHA      = 8,
   SOIL_FLAG_INVERT_Y            = 16,
   SOIL_FLAG_COMPRESS_TO_DXT     = 32,
   SOIL_FLAG_DDS_LOAD_DIRECT     = 64,
   SOIL_FLAG_NTSC_SAFE_RGB       = 128,
   SOIL_FLAG_CoCg_Y              = 256,
   SOIL_FLAG_TEXTURE_RECTANGLE   = 512,
   SOIL_FLAG_DEFAULTS            = 1024
} SOIL_FLAG_T;
#define SOIL_FLAG_T
#endif

/* texture struct */
typedef struct dlTexture_t
{
   /* GL Texture object */
   unsigned int object;

   /* string used to identify in texture cache */
   char *file;

   /* Image data */
   unsigned char *data;

   /* Dimensions */
   int width;
   int height;
   uint8_t channels;

   /* size of data */
   size_t size;

   /* uvw index */
   unsigned int  uvw;

   unsigned int refCounter;
} dlTexture;

typedef struct dlTextureCache_t
{
   dlTexture    **texture;
   unsigned int num_textures;
} dlTextureCache;

dlTexture* dlNewTexture( const char *file, unsigned int flags );    /* Allocate texture */
dlTexture* dlCopyTexture( dlTexture* );         /* Copy texture */
dlTexture* dlRefTexture( dlTexture* );          /* Ref texture */
int        dlFreeTexture( dlTexture* );         /* Free texture */

/* Operations */

/* Create dl texture manually */
int dlTextureCreate(  dlTexture*, unsigned char *data,
    int width, int height, int channels, unsigned int flags );

/* Save texture to TGA */
int dlTextureSave( dlTexture *texture, const char *path );

/* Check if texture is in cache */
dlTexture* dlTextureCheckCache( const char *file );

/* add texture to cache */
int dlTextureAddCache( dlTexture *texture );

/* remove from cache */
int dlTextureRemoveCache( dlTexture *texture );

/* Init texture cache */
int dlTextureInitCache( void );

/* Free texture cache */
int dlTextureFreeCache( void );

#ifdef __cplusplus
}
#endif

#endif /* DL_TEXTURE_H */

/* EoF */
