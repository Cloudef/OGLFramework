#include <stdio.h>
#include <malloc.h>

#include "texture.h"
#include "alloc.h"
#include "types.h"
#include "core.h"
#include "import/import.h"
#include "logfile_wrapper.h"

#include "SOIL.h"

#ifdef GLES2
#	include <GLES2/gl2.h>
#elif  GLES1
#  include <GLES/gl.h>
#  include <GLES/glext.h>
#else
#	include <GL/glew.h>
#  include <GL/gl.h>
#endif

/* texture cache object */
glTextureCache GL_TEXTURE_CACHE;

/* Allocate texture
 * Takes filename as argument, pass NULL to use user data */
glTexture* glNewTexture( const char *file, unsigned int flags )
{
   glTexture *obj;

   obj = glTextureCheckCache( file );
   if(obj)  return( obj );

   /* Allocate texture */
   obj = (glTexture*)glCalloc( 1, sizeof(glTexture) );
   if(!obj)
      return( NULL );

   /* GL Texture object */
   obj->object = 0;
   obj->file   = NULL;

   /* If file is passed, then try import it */
   if( file )
   {
      /* copy filename */
      obj->file   = strdup(file);

      /* default flags if not any specified */
      if((flags & SOIL_FLAG_DEFAULTS))
         flags += SOIL_FLAG_POWER_OF_TWO        |
                  SOIL_FLAG_MIPMAPS             |
                  SOIL_FLAG_NTSC_SAFE_RGB       |
                  SOIL_FLAG_INVERT_Y            |
                  SOIL_FLAG_COMPRESS_TO_DXT;

      /* import image */
      if(glImportImage( obj, file, flags ) != RETURN_OK)
      {
         glFreeTexture(obj);
         return( NULL );
      }
      glTextureAddCache( obj );

      logGreen();
      glPrint("[TEXTURE] %dx%d\n", obj->width, obj->height);
      logNormal();
   }

   /* Increase ref counter */
   obj->refCounter++;

   return( obj );
}

/* Copy texture */
glTexture* glCopyTexture( glTexture *src )
{
   glTexture *obj;

   /* Fuuuuuuuuuu--- We have non valid object */
   if(!src) return( NULL );

   /* Allocate texture */
   obj = (glTexture*)glCalloc( 1, sizeof(glTexture) );
   if(!obj)
      return( NULL );

   /* GL Texture object */
   obj->object = src->object;
   obj->file   = strdup(src->file);

   /* Increase ref counter */
   obj->refCounter++;

   /* Return texture */
   return( obj );
}

/* Reference texture */
glTexture* glRefTexture( glTexture *src )
{
   glTexture *obj;

   /* non valid */
   if(!src)
      return( NULL );

   /*  pointer to pointer */
   obj = src;

   /* Increase ref counter */
   obj->refCounter++;

   /* Return reference */
   return( obj );
}

/* Free texture */
int glFreeTexture( glTexture *obj )
{
   /* non valid */
   if(!obj)
      return( RETURN_NOTHING );

	/* There is still references to this object alive */
	if(--obj->refCounter != 0) return( RETURN_NOTHING );

   logRed();
   glPrint("[TEXTURE] %dx%d\n", obj->width, obj->height);
   logNormal();

   /* remove from cache */
   glTextureRemoveCache( obj );

   /* delete OGL texture if there is one */
   if(obj->object)   glDeleteTextures( 1, &obj->object );
   if(obj->data)     free(obj->data);
   if(obj->file)     free(obj->file);

   /* free */
   glFree( obj, sizeof( glTexture ) );
   return( RETURN_OK );
}

/* Create GL Texture manually.
 * Flags are SOIL flags */
int glTextureCreate( glTexture *texture, unsigned char *data,
      int width, int height, int channels, unsigned int flags )
{
   if(!texture)
      return( RETURN_FAIL );

   /* Create GL texture */
   if(texture->object)  glDeleteTextures( 1, &texture->object );
   if(texture->data)    free(texture->data);

   texture->object =
   SOIL_create_OGL_texture(
         data, width, height, channels,
         0,
         flags );

   texture->width    = width;
   texture->height   = height;
   texture->channels = channels;
   texture->data     = data;

   if(!texture->object)
      return( RETURN_FAIL );

   logGreen();
   glPrint("[TEXTURE] %dx%d\n", texture->width, texture->height);
   logNormal();

   return( RETURN_OK );
}

/* Save texture to file in TGA format */
int glTextureSave( glTexture *texture, const char *path )
{
   if(!texture)
      return( RETURN_FAIL );

	if(!SOIL_save_image
	(
		path,
		SOIL_SAVE_TYPE_TGA,
		texture->width, texture->height, texture->channels,
		texture->data
	))
      return( RETURN_FAIL );

   return( RETURN_OK );
}

/* check if texture is in chache
 * returns reference if found */
glTexture* glTextureCheckCache( const char *file )
{
   unsigned int i;

   if(!file)
      return( NULL );

   i = 0;
   for(; i != GL_TEXTURE_CACHE.num_textures; ++i)
   {
      if( strcmp( GL_TEXTURE_CACHE.texture[i]->file,
                  file ) == 0 )
      {
         return( glRefTexture( GL_TEXTURE_CACHE.texture[i] ) );
      }
   }

   return( NULL );
}

/* add to texture cache */
int glTextureAddCache( glTexture *texture )
{
   if(!texture)
      return( RETURN_FAIL );
   if(!texture->file)
      return( RETURN_FAIL );

   /* alloc */
   GL_TEXTURE_CACHE.num_textures++;
   if(!GL_TEXTURE_CACHE.texture)
      GL_TEXTURE_CACHE.texture = glCalloc( GL_TEXTURE_CACHE.num_textures,
                                           sizeof(glTexture*) );
   else
      GL_TEXTURE_CACHE.texture = glRealloc( GL_TEXTURE_CACHE.texture,
                                            GL_TEXTURE_CACHE.num_textures - 1,
                                            GL_TEXTURE_CACHE.num_textures, sizeof(glTexture*) );

   /* check success */
   if(!GL_TEXTURE_CACHE.texture)
      return( RETURN_FAIL );

   /* assign */
   GL_TEXTURE_CACHE.texture[ GL_TEXTURE_CACHE.num_textures - 1 ] = texture;
   return( RETURN_OK );
}

/* remove texture from cache */
int glTextureRemoveCache( glTexture *texture )
{
   unsigned int i, found;
   glTexture **tmp;

   if(!texture)
      return( RETURN_FAIL );
   if(!texture->file)
      return( RETURN_FAIL );

   if(!GL_TEXTURE_CACHE.num_textures)
      return( RETURN_FAIL );

   tmp = glCalloc( GL_TEXTURE_CACHE.num_textures, sizeof(glTexture*) );
   if(!tmp)
      return( RETURN_FAIL );

   /* add everything expect the one we are looking for to tmp list */
   i = 0;
   found = 0;
   for(; i != GL_TEXTURE_CACHE.num_textures; ++i)
   {
      if( GL_TEXTURE_CACHE.texture[i] == texture )
      {
         tmp[found++] = GL_TEXTURE_CACHE.texture[i];
      }
   }

   if(found)
   {
      /* resize list */
      tmp = glRealloc( tmp, GL_TEXTURE_CACHE.num_textures, found, sizeof(glTexture*) );
      if(!tmp)
         return( RETURN_FAIL );
   }
   else
   {
      /* free */
      glFree( tmp,  GL_TEXTURE_CACHE.num_textures * sizeof(glTexture*) );
      tmp = NULL;
   }

   /* ok, free the old list */
   glFree( GL_TEXTURE_CACHE.texture, GL_TEXTURE_CACHE.num_textures * sizeof(glTexture*) );

   /* use the new list and new count */
   GL_TEXTURE_CACHE.texture      = tmp;
   GL_TEXTURE_CACHE.num_textures = found;

   return( RETURN_OK );
}

/* Init texture cache */
int glTextureInitCache( void )
{
   /* nullify */
   GL_TEXTURE_CACHE.texture      = NULL;
   GL_TEXTURE_CACHE.num_textures = 0;

   return( RETURN_OK );
}

/* Free texture cache */
int glTextureFreeCache( void )
{
   if(!GL_TEXTURE_CACHE.texture)
      return( RETURN_OK );

   /* free */
   glFree( GL_TEXTURE_CACHE.texture, GL_TEXTURE_CACHE.num_textures * sizeof(glTexture*) );
   GL_TEXTURE_CACHE.texture = NULL;

   return( RETURN_OK );
}
