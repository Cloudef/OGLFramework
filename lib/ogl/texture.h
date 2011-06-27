#ifndef GL_TEXTURE_H
#define GL_TEXTURE_H

#include <stdint.h>

#include "kazmath/kazmath.h"

#ifdef __cplusplus
extern "C" {
#endif

/* texture flags, block to allow conflict with actual SOIL library */
#ifndef SOIL_FLAG_T
typedef enum
{
   SOIL_FLAG_DEFAULT = 0,
	SOIL_FLAG_POWER_OF_TWO = 1,
	SOIL_FLAG_MIPMAPS = 2,
	SOIL_FLAG_TEXTURE_REPEATS = 4,
	SOIL_FLAG_MULTIPLY_ALPHA = 8,
	SOIL_FLAG_INVERT_Y = 16,
	SOIL_FLAG_COMPRESS_TO_DXT = 32,
	SOIL_FLAG_DDS_LOAD_DIRECT = 64,
	SOIL_FLAG_NTSC_SAFE_RGB = 128,
	SOIL_FLAG_CoCg_Y = 256,
	SOIL_FLAG_TEXTURE_RECTANGLE = 512,
   SOIL_FLAG_NONE = 1024
} SOIL_FLAG_T;
#define SOIL_FLAG_T
#endif

/* texture struct */
typedef struct
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

   /* uvw index */
   unsigned int  uvw;

 	unsigned int refCounter;
} glTexture;

typedef struct
{
   glTexture    **texture;
   unsigned int num_textures;
} glTextureCache;

extern glTextureCache GL_TEXTURE_CACHE;

glTexture* glNewTexture( const char *file, unsigned int flags );    /* Allocate texture */
glTexture* glCopyTexture( glTexture* );         /* Copy texture */
glTexture* glRefTexture( glTexture* );          /* Ref texture */
int        glFreeTexture( glTexture* );         /* Free texture */

/* Operations */

/* Create GL texture manually */
int glTextureCreate(  glTexture*, unsigned char *data,
      int width, int height, int channels, unsigned int flags );

/* Save texture to TGA */
int glTextureSave( glTexture *texture, const char *path );

/* Check if texture is in cache */
glTexture* glTextureCheckCache( const char *file );

/* add texture to cache */
int glTextureAddCache( glTexture *texture );

/* remove from cache */
int glTextureRemoveCache( glTexture *texture );

/* Init texture cache */
int glTextureInitCache( void );

/* Free texture cache */
int glTextureFreeCache( void );

#ifdef __cplusplus
}
#endif

#endif /* GL_TEXTURE_H */

/* EoF */
