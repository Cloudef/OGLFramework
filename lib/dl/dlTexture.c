#include <stdio.h>
#include <malloc.h>

#include "dlTexture.h"
#include "dlAlloc.h"
#include "dlTypes.h"
#include "dlCore.h"
#include "import/dlImport.h"
#include "dlLog.h"

#include "SOIL.h"

#ifdef GLES2
#  include <GLES2/gl2.h>
#elif  GLES1
#  include <GLES/gl.h>
#  include <GLES/glext.h>
#else
#  include <GL/glew.h>
#  include <GL/gl.h>
#endif

/* Allocate texture
 * Takes filename as argument, pass NULL to use user data */
dlTexture* dlNewTexture( const char *file, unsigned int flags )
{
   dlTexture *obj;

   /* check if texture is in cache */
   obj = dlTextureCheckCache( file );
   if(obj)  return( obj );

   /* Allocate texture */
   dlSetAlloc( ALLOC_TEXTURE );
   obj = (dlTexture*)dlCalloc( 1, sizeof(dlTexture) );
   if(!obj)
      return( NULL );

   /* dl Texture object */
   obj->object = 0;
   obj->file   = NULL;
   obj->data   = NULL;

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
      if(dlImportImage( obj, file, flags ) != RETURN_OK)
      {
         dlFreeTexture(obj);
         return( NULL );
      }

      obj->size = obj->width * obj->height * obj->channels;
#ifdef DEBUG
      dlFakeAlloc( obj->size );
#endif

      dlTextureAddCache( obj );

      logGreen();
      dlPrint("[A:TEXTURE] %dx%d %.2f MiB\n", obj->width, obj->height, (float)obj->size / 1048576);
      logNormal();
   }

   /* Increase ref counter */
   obj->refCounter++;

   return( obj );
}

/* Copy texture */
dlTexture* dlCopyTexture( dlTexture *src )
{
   dlTexture *obj;

   /* Fuuuuuuuuuu--- We have non valid object */
   if(!src) return( NULL );

   /* Allocate texture */
   dlSetAlloc( ALLOC_TEXTURE );
   obj = (dlTexture*)dlCalloc( 1, sizeof(dlTexture) );
   if(!obj)
      return( NULL );

   /* dl Texture object */
   obj->object = src->object;
   obj->file   = strdup(src->file);
   obj->data   = dlCopy(src->data, src->size);

   logYellow();
   dlPrint("[C:TEXTURE] %dx%d %.2f MiB\n", obj->width, obj->height, (float)obj->size / 1048576);
   logNormal();

   /* Increase ref counter */
   obj->refCounter++;

   /* Return texture */
   return( obj );
}

/* Reference texture */
dlTexture* dlRefTexture( dlTexture *src )
{
   dlTexture *obj;

   /* non valid */
   if(!src)
      return( NULL );

   /*  pointer to pointer */
   obj = src;

   logYellow();
   dlPrint("[R:TEXTURE] %dx%d %.2f MiB\n", obj->width, obj->height, (float)obj->size / 1048576);
   logNormal();

   /* Increase ref counter */
   obj->refCounter++;

   /* Return reference */
   return( obj );
}

/* Free texture */
int dlFreeTexture( dlTexture *obj )
{
   /* non valid */
   if(!obj)
      return( RETURN_NOTHING );

   /* There is still references to this object alive */
   if(--obj->refCounter != 0) return( RETURN_NOTHING );

   logRed();
   dlPrint("[F:TEXTURE] %dx%d %.2f MiB\n", obj->width, obj->height, (float)obj->size / 1048576);
   logNormal();

   /* remove from cache */
   dlTextureRemoveCache( obj );
   dlSetAlloc( ALLOC_TEXTURE );

   /* delete Odl texture if there is one */
   if(obj->object)   glDeleteTextures( 1, &obj->object );
   if(obj->data)     dlFree(obj->data, obj->size);
   if(obj->file)     free(obj->file);

   /* free */
   dlFree( obj, sizeof( dlTexture ) );
   return( RETURN_OK );
}

/* Create GL Texture manually.
 * Flags are SOIL flags */
int dlTextureCreate( dlTexture *texture, unsigned char *data,
      int width, int height, int channels, unsigned int flags )
{
   if(!texture)
      return( RETURN_FAIL );

   dlSetAlloc( ALLOC_TEXTURE );

   /* Create dl texture */
   if(texture->object)  glDeleteTextures( 1, &texture->object );
   if(texture->data)    dlFree(texture->data, texture->size);

   texture->object =
   SOIL_create_OGL_texture(
      data, width, height, channels,
      0,
      flags );

   texture->width    = width;
   texture->height   = height;
   texture->channels = channels;
   texture->data     = data;
   texture->size     = width * height * channels;

#ifdef DEBUG
   /* to keep with statistics */
   dlFakeAlloc( texture->size );
#endif

   if(!texture->object)
      return( RETURN_FAIL );

   logGreen();
   dlPrint("[A:TEXTURE] %dx%d %.2f MiB\n", texture->width, texture->height, (float)texture->size / 1048576);
   logNormal();

   return( RETURN_OK );
}

/* Save texture to file in TGA format */
int dlTextureSave( dlTexture *texture, const char *path )
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

/* bind texture */
static GLuint _DL_BIND_TEXTURE = 0;
void dlBindTexture( dlTexture *texture )
{
   if(!texture && _DL_BIND_TEXTURE)
   {
      glBindTexture( GL_TEXTURE_2D, 0); _DL_BIND_TEXTURE = 0;
      return;
   }

   if(_DL_BIND_TEXTURE == texture->object)
      return;

   glBindTexture( GL_TEXTURE_2D, texture->object );
   _DL_BIND_TEXTURE = texture->object;
}

/* bind using ID */
void dlBindTexturei( GLuint texture )
{
   if(_DL_BIND_TEXTURE == texture)
      return;

   glBindTexture( GL_TEXTURE_2D, texture );
   _DL_BIND_TEXTURE = texture;
}

/* --------------- TEXTURE CACHE --------------- */

/* texture cache object */
static dlTextureCache _DL_TEXTURE_CACHE;

/* check if texture is in chache
 * returns reference if found */
dlTexture* dlTextureCheckCache( const char *file )
{
   unsigned int i;

   if(!file)
      return( NULL );

   i = 0;
   for(; i != _DL_TEXTURE_CACHE.num_textures; ++i)
   {
      if( strcmp( _DL_TEXTURE_CACHE.texture[i]->file,
                  file ) == 0 )
      {
         return( dlRefTexture( _DL_TEXTURE_CACHE.texture[i] ) );
      }
   }

   return( NULL );
}

/* add to texture cache */
int dlTextureAddCache( dlTexture *texture )
{
   if(!texture)
      return( RETURN_FAIL );
   if(!texture->file)
      return( RETURN_FAIL );

   /* alloc */
   dlSetAlloc( ALLOC_TEXTURE_CACHE );
   _DL_TEXTURE_CACHE.num_textures++;
   if(!_DL_TEXTURE_CACHE.texture)
      _DL_TEXTURE_CACHE.texture = dlCalloc( _DL_TEXTURE_CACHE.num_textures,
                                           sizeof(dlTexture*) );
   else
      _DL_TEXTURE_CACHE.texture = dlRealloc( _DL_TEXTURE_CACHE.texture,
                                            _DL_TEXTURE_CACHE.num_textures - 1,
                                            _DL_TEXTURE_CACHE.num_textures, sizeof(dlTexture*) );

   /* check success */
   if(!_DL_TEXTURE_CACHE.texture)
      return( RETURN_FAIL );

   /* assign */
   _DL_TEXTURE_CACHE.texture[ _DL_TEXTURE_CACHE.num_textures - 1 ] = texture;
   return( RETURN_OK );
}

/* remove texture from cache */
int dlTextureRemoveCache( dlTexture *texture )
{
   unsigned int i, found;
   dlTexture **tmp;

   if(!texture)
      return( RETURN_FAIL );
   if(!texture->file)
      return( RETURN_FAIL );

   if(!_DL_TEXTURE_CACHE.num_textures)
      return( RETURN_FAIL );

   dlSetAlloc( ALLOC_TEXTURE_CACHE );
   tmp = dlCalloc( _DL_TEXTURE_CACHE.num_textures, sizeof(dlTexture*) );
   if(!tmp)
      return( RETURN_FAIL );

   /* add everything expect the one we are looking for to tmp list */
   i = 0;
   found = 0;
   for(; i != _DL_TEXTURE_CACHE.num_textures; ++i)
   {
      if( _DL_TEXTURE_CACHE.texture[i] == texture )
      {
         tmp[found++] = _DL_TEXTURE_CACHE.texture[i];
      }
   }

   if(found)
   {
      /* resize list */
      tmp = dlRealloc( tmp, _DL_TEXTURE_CACHE.num_textures, found, sizeof(dlTexture*) );
      if(!tmp)
         return( RETURN_FAIL );
   }
   else
   {
      /* free */
      dlFree( tmp,  _DL_TEXTURE_CACHE.num_textures * sizeof(dlTexture*) );
      tmp = NULL;
   }

   /* ok, free the old list */
   dlFree( _DL_TEXTURE_CACHE.texture, _DL_TEXTURE_CACHE.num_textures * sizeof(dlTexture*) );

   /* use the new list and new count */
   _DL_TEXTURE_CACHE.texture      = tmp;
   _DL_TEXTURE_CACHE.num_textures = found;

   return( RETURN_OK );
}

/* Init texture cache */
int dlTextureInitCache( void )
{
   /* nullify */
   _DL_TEXTURE_CACHE.texture      = NULL;
   _DL_TEXTURE_CACHE.num_textures = 0;

   return( RETURN_OK );
}

/* Free texture cache */
int dlTextureFreeCache( void )
{
   if(!_DL_TEXTURE_CACHE.texture)
      return( RETURN_OK );

   /* free */
   dlSetAlloc( ALLOC_TEXTURE_CACHE );
   dlFree( _DL_TEXTURE_CACHE.texture, _DL_TEXTURE_CACHE.num_textures * sizeof(dlTexture*) );
   _DL_TEXTURE_CACHE.texture = NULL;

   return( RETURN_OK );
}
