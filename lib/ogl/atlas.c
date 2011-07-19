#include <SDL/SDL.h>

#include "packer/texturepacker.h"

#include "atlas.h"
#include "types.h"
#include "alloc.h"
#include "sceneobject.h"
#include "framework.h"

#include "SOIL.h"
#include "SDL/SDL_rotozoom.h"

/* allocate new atlas */
glAtlas* glNewAtlas(void)
{
   glAtlas *atlas;

   atlas = glCalloc( 1, sizeof(glAtlas) );
   if(!atlas)
      return( NULL );

   /* nullify */
   atlas->rect    = NULL;
   atlas->texture = NULL;

   atlas->refCounter++;

   return( atlas );
}

/* reference atlas */
glAtlas* glRefAtlas( glAtlas *src )
{
   glAtlas *atlas;

   /* non valid */
   if(!src)
      return( NULL );

   /* point */
   atlas = src;

   /* ref textures */
   atlas->rect    = glAtlasRefTextures( atlas );
   atlas->texture = glRefTexture( atlas->texture );

   atlas->refCounter++;
   return( atlas );
}

/* free atlas */
int glFreeAtlas( glAtlas *atlas )
{
   unsigned int i;

   /* non valid */
   if(!atlas)
      return( RETURN_FAIL );

   /* Free as in, decrease reference on childs */
   i = 0;
   for(; i != atlas->num_textures; ++i)
   {
      if(glFreeTexture( atlas->rect[i].texture ) == RETURN_OK)
         atlas->rect[i].texture = NULL;
   }

   /* Free combined atlas texture */
   glFreeTexture( atlas->texture );

   /* still hold reference? */
   if(--atlas->refCounter != 0)
      return( RETURN_NOTHING );

   /* free everything */
   glFree( atlas->rect, atlas->num_textures * sizeof(glAtlasRect) );
   atlas->rect          = NULL;
   atlas->num_textures  = 0;

   glFree( atlas, sizeof(glAtlas) );
   return( RETURN_OK );
}

/* add texture */
int glAtlasAddTexture( glAtlas *atlas, glTexture *texture )
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
            glFreeTexture( texture );
            return( RETURN_OK );
         }
      }
   }

   /* alloc */
   atlas->num_textures++;
   if(!atlas->rect)
      atlas->rect = glCalloc( atlas->num_textures, sizeof(glAtlasRect) );
   else
      atlas->rect = glRealloc( atlas->rect, atlas->num_textures - 1,
                              atlas->num_textures, sizeof(glAtlasRect) );

   /* check success */
   if(!atlas->rect)
      return( RETURN_FAIL );

   /* assign */
   atlas->rect[ atlas->num_textures - 1 ].texture = texture;
   atlas->rect[ atlas->num_textures - 1 ].index   = atlas->num_textures - 1;

   return( RETURN_OK );
}

/* free texture */
int glAtlasFreeTexture( glAtlas *atlas, glTexture *texture )
{
   unsigned int i, found;
   glAtlasRect *tmp;

   if(!atlas)
      return( RETURN_FAIL );
   if(!atlas->rect)
      return( RETURN_FAIL );
   if(!texture)
      return( RETURN_FAIL );

   /* allocate tmp list */
   tmp = glCalloc( atlas->num_textures, sizeof(glAtlasRect) );
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
      tmp = glRealloc( tmp, atlas->num_textures, found, sizeof(glAtlasRect) );
      if(!tmp)
         return( RETURN_FAIL );
   }
   else
   {
      glFree( tmp, atlas->num_textures * sizeof(glAtlasRect) );
      tmp = NULL;
   }

   /* ok, free the old list */
   glFree( atlas->rect, atlas->num_textures * sizeof(glAtlasRect) );
   glFreeTexture( texture );

   /* use the new list and new count */
   atlas->rect           = tmp;
   atlas->num_textures   = found;

   return( RETURN_OK );
}

/* reference textures */
glAtlasRect* glAtlasRefTextures( glAtlas *atlas )
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
      glRefTexture( atlas->rect[i].texture );
   }

   return( atlas->rect );
}

/* free textures */
int glAtlasFreeTextures( glAtlas *atlas )
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
      if( glFreeTexture( atlas->rect[i].texture ) == RETURN_OK )
         atlas->rect[i].texture = NULL;
   }

   /* free */
   glFree( atlas->rect, atlas->num_textures * sizeof(glAtlasRect) );
   atlas->rect         = NULL;
   atlas->num_textures = 0;

   return( RETURN_OK );
}

/* pack textures into atlas */
glTexture* glAtlasPack( glAtlas *atlas, int pow2, int border )
{
   unsigned int i;

   int oldWidth, oldHeight;
   int width  = 0;
   int height = 0;

   glTexturePacker *tp;
   glTexture *texture = NULL;

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
   tp = glNewTexturePacker();

   /* set count */
   glTexturePackerSetCount( tp, atlas->num_textures );

   /* add textures */
   i = 0;
   for(; i != atlas->num_textures; ++i)
   {
      glTexturePackerAdd( tp,
            atlas->rect[i].texture->width, atlas->rect[i].texture->height );
   }

   /* pack */
   glTexturePackerPack( tp, &width, &height, pow2, border );

   /* create surface where to blit */
   bitmap = SDL_CreateRGBSurface( SDL_SWSURFACE, width, height, 24, rmask, gmask, bmask, amask );

   /* cycle again and get packed info */
   i = 0;
   for(; i != atlas->num_textures; ++i)
   {
      atlas->rect[i].packed.rotated =
         glTexturePackerGetLocation( tp, atlas->rect[i].index,
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
   glFreeTexture( atlas->texture );

   /* create texture from it */
   texture = glNewTexture( NULL, 0 );
   if( glTextureCreate( texture, data, width, height, 3,
            SOIL_FLAG_POWER_OF_TWO    |
            SOIL_FLAG_NTSC_SAFE_RGB   |
            SOIL_FLAG_COMPRESS_TO_DXT |
            SOIL_FLAG_MIPMAPS )
         != RETURN_OK )
   {
      glFreeTexture( texture );
      texture = NULL;
   }

   /* and free the blitted surface */
   SDL_FreeSurface( bitmap );

   /* debug */
   printf("%dx%d\n", width, height);

   /* free packer */
   glFreeTexturePacker( tp );

   /* assign and return */
   atlas->texture = texture;
   return( texture );
}

/* return packed area */
glAtlasArea* glAtlasGetPacked( glAtlas *atlas, glTexture *texture )
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
int glAtlasGetTransformed( glAtlas *atlas, glTexture *texture, kmVec2 *coord )
{
   float atlasWidth;
   float atlasHeight;

   float width;
   float height;

   float x;
   float y;

   glAtlasArea *packed;

   if(!atlas)
      return( RETURN_FAIL );
   if(!atlas->texture)
      return( RETURN_FAIL );
   if(!texture)
      return( RETURN_FAIL );

   packed = glAtlasGetPacked( atlas, texture );
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
