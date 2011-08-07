#include "alloc.h"
#include "types.h"
#include "core.h"
#include "texture.h"

#ifdef GLES2
#	include <GLES2/gl2.h>
#elif  GLES1
#  include <GLES/gl.h>
#  include <GLES/glext.h>
#else
#	include <GL/glew.h>
#  include <GL/gl.h>
#endif

/* Allocate material object */
glMaterial* glNewMaterial( void )
{
   unsigned int i;

	/* Allocate material object */
	glMaterial* object = (glMaterial*)glCalloc( 1, sizeof(glMaterial) );
	if(!object)
      return( NULL );

	/* Nullify pointers */
   object->texture = glCalloc( _glCore.info.maxTextureUnits, sizeof(glTexture*) );
   if(!object->texture)
   {
      glFree( object, sizeof(glMaterial) );
      return( NULL );
   }

   i = 0;
   while( i != _glCore.info.maxTextureUnits )
   {
      object->texture[i] = NULL;
      ++i;
   }

   /* default blend modes */
   object->blend1 = GL_SRC_ALPHA;
   object->blend2 = GL_ONE_MINUS_SRC_ALPHA;

	/* Increase ref counter */
	object->refCounter++;

	/* Return the instance */
	return( object );
}

/* Copy material object */
glMaterial* glCopyMaterial( glMaterial *src )
{
   unsigned int i;
   glMaterial *object;

   /* Fuuuuuuuuu--- We have non valid object */
	if(!src) return( NULL );

	/* Allocate scene object */
	object = (glMaterial*)glCalloc( 1, sizeof(glMaterial) );
	if(!object)
      return( NULL );

	/* Copy data */
   object->texture = glCopy( src->texture,
                     _glCore.info.maxTextureUnits * sizeof(glTexture*) );
   if(!object->texture)
   {
      glFree( object, sizeof(glMaterial) );
      return( NULL );
   }

   i = 0;
   while( i != _glCore.info.maxTextureUnits )
   {
      object->texture[i] = glCopyTexture(src->texture[i]);
      ++i;
   }

   object->blend1 = src->blend1;
   object->blend2 = src->blend2;
   object->flags  = src->flags;

	/* Increase ref counter */
	object->refCounter++;

	/* Return the instance */
	return( object );
}

/* Reference material object */
glMaterial* glRefMaterial( glMaterial *src )
{
   glMaterial* object;

	/* Fuuuuuuuuu--- We have non valid object */
	if(!src) return( NULL );

	/* Point magic */
	object                      = src;

   /* Ref textures */
   object->texture = glMaterialRefTextures( object );

	/* Increase ref counter */
	object->refCounter++;

	/* Return the instance */
	return( object );
}

/* Free material object */
int glFreeMaterial( glMaterial *object )
{
	/* Fuuuuuuuuu--- We have non valid object */
   if(!object) return( RETURN_NOTHING );

	/* Free textures */
   glMaterialFreeTexturesAll( object );

	/* There is still references to this object alive */
	if(--object->refCounter != 0) return( RETURN_NOTHING );

   /* Free texture array */
   glFree( object->texture,
           _glCore.info.maxTextureUnits * sizeof(glTexture*) );
   object->texture = NULL;

	/* Free scene object */
   glFree( object, sizeof(glMaterial) );
   return( RETURN_OK );
}

/* Add texture, steals reference */
int glMaterialAddTexture( glMaterial *object,
                          unsigned int index,
                          glTexture *texture )
{
   if(!object)
      return( RETURN_FAIL );

   if(index > _glCore.info.maxTextureUnits)
      return( RETURN_FAIL );

   object->texture[index] = texture;

   return( RETURN_OK );
}

/* Free texture */
int glMaterialFreeTexture( glMaterial *object,
                           unsigned int index )
{
   if(!object)
      return( RETURN_FAIL );

   if(index > _glCore.info.maxTextureUnits)
      return( RETURN_FAIL );

   if(!object->texture[index])
      return( RETURN_OK );

   if( glFreeTexture( object->texture[index] ) == RETURN_OK )
      object->texture[index] = NULL;

   return( RETURN_OK );
}

/* Free all textures */
int glMaterialFreeTexturesAll( glMaterial *object )
{
   unsigned int i;

   if(!object)
      return( RETURN_FAIL );

   i = 0;
   while( i != _glCore.info.maxTextureUnits )
   {
      if( glMaterialFreeTexture( object, i ) != RETURN_OK )
         return( RETURN_FAIL );
      ++i;
   }

   return( RETURN_OK );
}

/* reference textures */
glTexture** glMaterialRefTextures( glMaterial *object )
{
   unsigned int i;

   if(!object)
      return( NULL );

   i = 0;
   while( i != _glCore.info.maxTextureUnits )
   {
      if(object->texture[i])
         glRefTexture( object->texture[i] );
      ++i;
   }

   return( object->texture );
}

/* EoF */
