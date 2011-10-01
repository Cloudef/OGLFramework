#include "dlAlloc.h"
#include "dlTypes.h"
#include "dlCore.h"
#include "dlTexture.h"
#include "dlLog.h"

#ifdef GLES2
#  include <GLES2/gl2.h>
#elif  GLES1
#  include <GLES/gl.h>
#  include <GLES/glext.h>
#else
#  include <GL/glew.h>
#  include <GL/gl.h>
#endif

/* Allocate material object */
dlMaterial* dlNewMaterial( void )
{
   unsigned int i;

   /* Allocate material object */
   dlSetAlloc( ALLOC_MATERIAL );
   dlMaterial* object = (dlMaterial*)dlCalloc( 1, sizeof(dlMaterial) );
   if(!object)
      return( NULL );

   /* Nullify pointers */
   object->texture = dlCalloc( _dlCore.info.maxTextureUnits, sizeof(dlTexture*) );
   if(!object->texture)
   {
      dlFree( object, sizeof(dlMaterial) );
      return( NULL );
   }

   i = 0;
   while( i != _dlCore.info.maxTextureUnits )
   {
      object->texture[i] = NULL;
      ++i;
   }

   /* default blend modes */
   object->blend1 = GL_SRC_ALPHA;
   object->blend2 = GL_ONE_MINUS_SRC_ALPHA;

   logGreen();
   dlPuts("[A:MATERIAL]");
   logNormal();

   /* Increase ref counter */
   object->refCounter++;

   /* Return the instance */
   return( object );
}

/* Copy material object */
dlMaterial* dlCopyMaterial( dlMaterial *src )
{
   unsigned int i;
   dlMaterial *object;

   /* Fuuuuuuuuu--- We have non valid object */
   if(!src) return( NULL );

   /* Allocate scene object */
   dlSetAlloc( ALLOC_MATERIAL );
   object = (dlMaterial*)dlCalloc( 1, sizeof(dlMaterial) );
   if(!object)
      return( NULL );

   /* Copy data */
   object->texture = dlCopy( src->texture,
         _dlCore.info.maxTextureUnits * sizeof(dlTexture*) );
   if(!object->texture)
   {
      dlFree( object, sizeof(dlMaterial) );
      return( NULL );
   }

   i = 0;
   while( i != _dlCore.info.maxTextureUnits )
   {
      object->texture[i] = dlCopyTexture(src->texture[i]);
      ++i;
   }

   object->blend1 = src->blend1;
   object->blend2 = src->blend2;
   object->flags  = src->flags;

   logYellow();
   dlPuts("[C:MATERIAL]");
   logNormal();

   /* Increase ref counter */
   object->refCounter++;

   /* Return the instance */
   return( object );
}

/* Reference material object */
dlMaterial* dlRefMaterial( dlMaterial *src )
{
   dlMaterial* object;

   /* Fuuuuuuuuu--- We have non valid object */
   if(!src) return( NULL );

   /* Point magic */
   object                      = src;

   /* Ref textures */
   object->texture = dlMaterialRefTextures( object );

   logYellow();
   dlPuts("[R:MATERIAL]");
   logNormal();

   /* Increase ref counter */
   object->refCounter++;

   /* Return the instance */
   return( object );
}

/* Free material object */
int dlFreeMaterial( dlMaterial *object )
{
   /* Fuuuuuuuuu--- We have non valid object */
   if(!object) return( RETURN_NOTHING );

   /* Free textures */
   dlMaterialFreeTexturesAll( object );

   /* There is still references to this object alive */
   if(--object->refCounter != 0) return( RETURN_NOTHING );

   dlSetAlloc( ALLOC_MATERIAL );

   /* Free texture array */
   dlFree( object->texture,
           _dlCore.info.maxTextureUnits * sizeof(dlTexture*) );
   object->texture = NULL;

   logRed();
   dlPuts("[F:MATERIAL]");
   logNormal();

   /* Free scene object */
   dlFree( object, sizeof(dlMaterial) );
   return( RETURN_OK );
}

/* Add texture, steals reference */
int dlMaterialAddTexture( dlMaterial *object,
                          unsigned int index,
                          dlTexture *texture )
{
   if(!object)
      return( RETURN_FAIL );

   if(index > _dlCore.info.maxTextureUnits)
      return( RETURN_FAIL );

   object->texture[index] = texture;

   return( RETURN_OK );
}

/* Free texture */
int dlMaterialFreeTexture( dlMaterial *object,
                           unsigned int index )
{
   if(!object)
      return( RETURN_FAIL );

   if(index > _dlCore.info.maxTextureUnits)
      return( RETURN_FAIL );

   if(!object->texture[index])
      return( RETURN_OK );

   if( dlFreeTexture( object->texture[index] ) == RETURN_OK )
      object->texture[index] = NULL;

   return( RETURN_OK );
}

/* Free all textures */
int dlMaterialFreeTexturesAll( dlMaterial *object )
{
   unsigned int i;

   if(!object)
      return( RETURN_FAIL );

   i = 0;
   while( i != _dlCore.info.maxTextureUnits )
   {
      if( dlMaterialFreeTexture( object, i ) != RETURN_OK )
         return( RETURN_FAIL );
      ++i;
   }

   return( RETURN_OK );
}

/* reference textures */
dlTexture** dlMaterialRefTextures( dlMaterial *object )
{
   unsigned int i;

   if(!object)
      return( NULL );

   i = 0;
   while( i != _dlCore.info.maxTextureUnits )
   {
      if(object->texture[i])
         dlRefTexture( object->texture[i] );
      ++i;
   }

   return( object->texture );
}

/* EoF */
