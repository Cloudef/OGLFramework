#include <SDL/SDL.h>

#include "atlaspacker/dlAtlasPacker.h"
#include "dlAtlas.h"
#include "dlTypes.h"
#include "dlAlloc.h"
#include "dlSceneobject.h"
#include "dlFramework.h"
#include "dlLog.h"

#include "SOIL.h"
#include "SDL/SDL_rotozoom.h"

/* allocate new atlas */
dlAtlas* dlNewAtlas(void)
{
   dlAtlas *atlas;

   dlSetAlloc( ALLOC_ATLAS );
   atlas = dlCalloc( 1, sizeof(dlAtlas) );
   if(!atlas)
      return( NULL );

   /* nullify */
   atlas->rect    = NULL;
   atlas->texture = NULL;

   logGreen();
   dlPuts("[A:ATLAS]");
   logNormal();

   atlas->refCounter++;
   return( atlas );
}

/* reference atlas */
dlAtlas* dlRefAtlas( dlAtlas *src )
{
   dlAtlas *atlas;

   /* non valid */
   if(!src)
      return( NULL );

   /* point */
   atlas = src;

   /* ref textures */
   atlas->rect    = dlAtlasRefTextures( atlas );
   atlas->texture = dlRefTexture( atlas->texture );

   logYellow();
   dlPuts("[R:ATLAS]");
   logNormal();

   atlas->refCounter++;
   return( atlas );
}

/* free atlas */
int dlFreeAtlas( dlAtlas *atlas )
{
   unsigned int i;

   /* non valid */
   if(!atlas)
      return( RETURN_FAIL );

   /* Free as in, decrease reference on childs */
   i = 0;
   for(; i != atlas->num_textures; ++i)
   {
      if(dlFreeTexture( atlas->rect[i].texture ) == RETURN_OK)
         atlas->rect[i].texture = NULL;
   }

   /* Free combined atlas texture */
   dlFreeTexture( atlas->texture );

   /* still hold reference? */
   if(--atlas->refCounter != 0)
      return( RETURN_NOTHING );

   dlSetAlloc( ALLOC_ATLAS );

   /* free everything */
   dlFree( atlas->rect, atlas->num_textures * sizeof(dlAtlasRect) );
   atlas->rect          = NULL;
   atlas->num_textures  = 0;

   logRed();
   dlPuts("[F:ATLAS]");
   logNormal();

   dlFree( atlas, sizeof(dlAtlas) );
   return( RETURN_OK );
}

/* add texture */
int dlAtlasAddTexture( dlAtlas *atlas, dlTexture *texture )
{
   unsigned int i;

   if(!atlas)
      return( RETURN_FAIL );
   if(!texture)
      return( RETURN_FAIL );

   /* don't add if we already have this texture */
   if(atlas->rect)
   {
      i = 0;
      for(; i != atlas->num_textures; ++i)
      {
         if(atlas->rect[i].texture == texture)
         {
            dlFreeTexture( texture );
            return( RETURN_OK );
         }
      }
   }

   dlSetAlloc( ALLOC_ATLAS );

   /* alloc */
   atlas->num_textures++;
   if(!atlas->rect)
      atlas->rect = dlCalloc( atlas->num_textures, sizeof(dlAtlasRect) );
   else
      atlas->rect = dlRealloc( atlas->rect, atlas->num_textures - 1,
                               atlas->num_textures, sizeof(dlAtlasRect) );

   /* check success */
   if(!atlas->rect)
      return( RETURN_FAIL );

   /* assign */
   atlas->rect[ atlas->num_textures - 1 ].texture = texture;
   atlas->rect[ atlas->num_textures - 1 ].index   = atlas->num_textures - 1;

   return( RETURN_OK );
}

/* free texture */
int dlAtlasFreeTexture( dlAtlas *atlas, dlTexture *texture )
{
   unsigned int i, found;
   dlAtlasRect *tmp;

   if(!atlas)
      return( RETURN_FAIL );
   if(!atlas->rect)
      return( RETURN_FAIL );
   if(!texture)
      return( RETURN_FAIL );

   dlSetAlloc( ALLOC_ATLAS );

   /* allocate tmp list */
   tmp = dlCalloc( atlas->num_textures, sizeof(dlAtlasRect) );
   if(!tmp)
      return( RETURN_FAIL );

   /* add everything expect the one we are looking for to tmp list */
   i = 0;
   found = 0;
   for(; i != atlas->num_textures; ++i)
   {
      if( atlas->rect[i].texture != texture )
      {
         tmp[++found] = atlas->rect[i];
      }
   }

   if(found)
   {
      /* resize list */
      tmp = dlRealloc( tmp, atlas->num_textures, found, sizeof(dlAtlasRect) );
      if(!tmp)
         return( RETURN_FAIL );
   }
   else
   {
      dlFree( tmp, atlas->num_textures * sizeof(dlAtlasRect) );
      tmp = NULL;
   }

   /* ok, free the old list */
   dlFree( atlas->rect, atlas->num_textures * sizeof(dlAtlasRect) );
   dlFreeTexture( texture );

   /* use the new list and new count */
   atlas->rect           = tmp;
   atlas->num_textures   = found;

   return( RETURN_OK );
}

/* reference textures */
dlAtlasRect* dlAtlasRefTextures( dlAtlas *atlas )
{
   unsigned int i;

   if(!atlas)
      return( NULL );
   if(!atlas->rect)
      return( NULL );

   /* reference all */
   i = 0;
   for(; i != atlas->num_textures; ++i)
   {
      dlRefTexture( atlas->rect[i].texture );
   }

   return( atlas->rect );
}

/* free textures */
int dlAtlasFreeTextures( dlAtlas *atlas )
{
   unsigned int i;

   if(!atlas)
      return( RETURN_FAIL );
   if(!atlas->rect)
      return( RETURN_FAIL );

   /* free all */
   i = 0;
   for(; i != atlas->num_textures; ++i)
   {
      if( dlFreeTexture( atlas->rect[i].texture ) == RETURN_OK )
         atlas->rect[i].texture = NULL;
   }

   /* free */
   dlSetAlloc( ALLOC_ATLAS );
   dlFree( atlas->rect, atlas->num_textures * sizeof(dlAtlasRect) );
   atlas->rect         = NULL;
   atlas->num_textures = 0;

   return( RETURN_OK );
}

/* pack textures into atlas */
dlTexture* dlAtlasPack( dlAtlas *atlas, int pow2, int border )
{
   unsigned int i;
   int width  = 0;
   int height = 0;

   dlTexturePacker *tp;
   dlTexture *texture = NULL;

   SDL_Surface *bitmap;
   SDL_Surface *surface, *oldsurface;
   unsigned char *data;

   Uint32 rmask, gmask, bmask, amask;

#if SDL_BYTEORDER == SDL_BIG_ENDIAN
   rmask = 0xff000000; gmask = 0x00ff0000; bmask = 0x0000ff00; amask = 0x000000ff;
#else
   rmask = 0x000000ff; gmask = 0x0000ff00; bmask = 0x00ff0000; amask = 0xff000000;
#endif

   if(!atlas)
      return( NULL );
   if(!atlas->rect)
      return( NULL );
   if(!atlas->num_textures)
      return( NULL );

   /* create texture packer */
   tp = dlNewTexturePacker();

   /* set count */
   dlTexturePackerSetCount( tp, atlas->num_textures );

   /* add textures */
   i = 0;
   for(; i != atlas->num_textures; ++i)
   {
      dlTexturePackerAdd( tp,
         atlas->rect[i].texture->width, atlas->rect[i].texture->height );
   }

   /* pack */
   dlTexturePackerPack( tp, &width, &height, pow2, border );

   /* create surface where to blit */
   bitmap = SDL_CreateRGBSurface( SDL_SWSURFACE, width, height, 24, rmask, gmask, bmask, amask );

   /* cycle again and get packed info */
   i = 0;
   for(; i != atlas->num_textures; ++i)
   {
      atlas->rect[i].packed.rotated =
         dlTexturePackerGetLocation( tp, atlas->rect[i].index,
            &atlas->rect[i].packed.x1, &atlas->rect[i].packed.y1,
            &atlas->rect[i].packed.x2, &atlas->rect[i].packed.y2 );

      printf("%dx%d+%d,%d - %d\n", atlas->rect[i].packed.x2, atlas->rect[i].packed.y2,
            atlas->rect[i].packed.x1, atlas->rect[i].packed.y1,
            atlas->rect[i].packed.rotated );

      /* create surface from texture */
      surface = SDL_CreateRGBSurfaceFrom( atlas->rect[i].texture->data,
                                          atlas->rect[i].texture->width,
                                          atlas->rect[i].texture->height,
                                          atlas->rect[i].texture->channels != 4 ? 24 : 32,
                                          atlas->rect[i].texture->width *
                                          atlas->rect[i].texture->channels,
                                          rmask,
                                          gmask,
                                          bmask,
                                          atlas->rect[i].texture->channels != 4 ? 0 : amask );

      /* we might need to rotate */
      if( atlas->rect[i].packed.rotated )
      {
         oldsurface = surface;
         surface = rotozoomSurface( oldsurface, 90, 1, 0);
         SDL_FreeSurface( oldsurface );
      }

      struct SDL_Rect src;
      src.x = atlas->rect[i].packed.x1;
      src.y = atlas->rect[i].packed.y1;
      //src.y = height - (atlas->rect[i].packed.y1 + atlas->rect[i].packed.y2);
      src.w = atlas->rect[i].packed.x2;
      src.h = atlas->rect[i].packed.y2;

      /* blit and free */
      SDL_BlitSurface( surface, NULL, bitmap, &src );
      SDL_FreeSurface( surface );
   }

   /* copy data from blitted surface */
   data = malloc( width * height * 3  );
   memcpy( data, bitmap->pixels, width * height * 3 );

   /* remove old if there is */
   dlFreeTexture( atlas->texture );

   /* create texture from it */
   texture = dlNewTexture( NULL, 0 );
   if( dlTextureCreate( texture, data, width, height, 3,
            SOIL_FLAG_POWER_OF_TWO    |
            SOIL_FLAG_NTSC_SAFE_RGB   |
            SOIL_FLAG_COMPRESS_TO_DXT |
            SOIL_FLAG_MIPMAPS )
         != RETURN_OK )
   {
      dlFreeTexture( texture );
      texture = NULL;
   }

   /* and free the blitted surface */
   SDL_FreeSurface( bitmap );

   /* free packer */
   dlFreeTexturePacker( tp );

   /* assign and return */
   atlas->texture = texture;
   return( texture );
}

/* return packed area */
dlAtlasArea* dlAtlasGetPacked( dlAtlas *atlas, dlTexture *texture )
{
   unsigned int i;

   if(!atlas)
      return( NULL );
   if(!atlas->rect)
      return( NULL );
   if(!texture)
      return( NULL );

   i = 0;
   for(; i != atlas->num_textures; ++i)
   {
      if( atlas->rect[i].texture == texture )
      {
         return( &atlas->rect[i].packed );
      }
   }

   return( NULL );
}

/* return transformed coords */
int dlAtlasGetTransformed( dlAtlas *atlas, dlTexture *texture, kmVec2 *coord )
{
   float atlasWidth;
   float atlasHeight;

   float width;
   float height;

   float x;
   float y;

   dlAtlasArea *packed;

   if(!atlas)
      return( RETURN_FAIL );
   if(!atlas->texture)
      return( RETURN_FAIL );
   if(!texture)
      return( RETURN_FAIL );

   packed = dlAtlasGetPacked( atlas, texture );
   if(!packed)
      return( RETURN_FAIL );

   atlasWidth  = atlas->texture->width;
   atlasHeight = atlas->texture->height;

   width  = packed->x2 / atlasWidth;
   height = packed->y2 / atlasHeight;
   x      = packed->x1 / atlasWidth;
   y      = packed->y1 / atlasHeight;

   if( packed->rotated )
   {
      //printf("%f, %f\n", coord->x, coord->y);
      kmVec2 center;
      center.x = 0.5f; center.y = 0.5f;

      kmVec2Rotate( coord, -90, &center );
      //printf("%f, %f\n", coord->x, coord->y);
      //puts("");
   }

   coord->x *= width;
   coord->x += x;
   coord->y *= height;
   coord->y += y;

   return( RETURN_OK );
}
