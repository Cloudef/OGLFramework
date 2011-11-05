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
   /* Allocate material object */
   dlSetAlloc( ALLOC_MATERIAL );
   dlMaterial* object = (dlMaterial*)dlCalloc( 1, sizeof(dlMaterial) );
   if(!object)
      return( NULL );

   /* Nullify pointers */
   object->texture = NULL;

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

/* Allocate new material from texture */
dlMaterial* dlNewMaterialFromTexture( dlTexture *texture )
{
   dlMaterial *object = dlNewMaterial();
   if(!object)
      return(NULL);

   object->texture = texture;

   return(object);
}

dlMaterial* dlNewMaterialWithTexture( const char *texture, unsigned int flags )
{
   dlTexture  *object;
   dlMaterial *material;

   object = dlNewTexture( texture, flags );
   if(!object)
      return(NULL);

   material = dlNewMaterialFromTexture( object );
   if(!material)
      dlFreeTexture(object);

   return(material);
}

/* Copy material object */
dlMaterial* dlCopyMaterial( dlMaterial *src )
{
   dlMaterial *object;

   /* Fuuuuuuuuu--- We have non valid object */
   if(!src) return( NULL );

   /* Allocate scene object */
   dlSetAlloc( ALLOC_MATERIAL );
   object = (dlMaterial*)dlCalloc( 1, sizeof(dlMaterial) );
   if(!object)
      return( NULL );

   /* Copy data */
   object->texture = dlCopyTexture(src->texture);

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
   object->texture = dlRefTexture( object->texture );

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

   /* Free texture */
   if(dlFreeTexture( object->texture ) == RETURN_OK)
      object->texture = NULL;

   /* There is still references to this object alive */
   if(--object->refCounter != 0) return( RETURN_NOTHING );

   dlSetAlloc( ALLOC_MATERIAL );

   logRed();
   dlPuts("[F:MATERIAL]");
   logNormal();

   /* Free scene object */
   dlFree( object, sizeof(dlMaterial) );
   return( RETURN_OK );
}

/* Add texture, steals reference */
int dlMaterialAddTexture( dlMaterial *object,
                          dlTexture *texture )
{
   if(!object)
      return( RETURN_FAIL );

   object->texture = texture;

   return( RETURN_OK );
}

/* Free texture */
int dlMaterialFreeTexture( dlMaterial *object )
{
   if(!object)
      return( RETURN_FAIL );

   if( dlFreeTexture( object->texture ) == RETURN_OK )
      object->texture = NULL;

   return( RETURN_OK );
}

/* EoF */
