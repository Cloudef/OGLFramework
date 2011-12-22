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

#define DL_DEBUG_CHANNEL "MATERIAL"

/* Allocate material object */
dlMaterial* dlNewMaterial( void )
{
   TRACE();

   /* Allocate material object */
   dlSetAlloc( ALLOC_MATERIAL );
   dlMaterial* object = (dlMaterial*)dlCalloc( 1, sizeof(dlMaterial) );
   if(!object)
   { RET("%p", NULL); return( NULL ); }

   /* Nullify pointers */
   object->texture = NULL;

   /* default blend modes */
   object->blend1 = GL_SRC_ALPHA;
   object->blend2 = GL_ONE_MINUS_SRC_ALPHA;

   LOGOK("NEW");

   /* Increase ref counter */
   object->refCounter++;

   /* Return the instance */
   RET("%p", object);
   return( object );
}

/* Allocate new material from texture */
dlMaterial* dlNewMaterialFromTexture( dlTexture *texture )
{
   dlMaterial *object;
   CALL("%p", texture);

   object = dlNewMaterial();
   if(!object)
   { RET("%p", NULL); return(NULL); }

   object->texture = texture;

   RET("%p", object);
   return(object);
}

dlMaterial* dlNewMaterialFromImage( const char *texture, unsigned int flags )
{
   dlTexture  *object;
   dlMaterial *material;
   CALL("%p, %u", texture, flags);

   object = dlNewTexture( texture, flags );
   if(!object)
   { RET("%p", NULL); return(NULL); }

   material = dlNewMaterialFromTexture( object );
   if(!material)
      dlFreeTexture(object);

   RET("%p", material);
   return(material);
}

/* Copy material object */
dlMaterial* dlCopyMaterial( dlMaterial *src )
{
   dlMaterial *object;
   CALL("%p", src);

   /* Fuuuuuuuuu--- We have non valid object */
   if(!src) { RET("%p", NULL); return( NULL ); }

   /* Allocate scene object */
   dlSetAlloc( ALLOC_MATERIAL );
   object = (dlMaterial*)dlCalloc( 1, sizeof(dlMaterial) );
   if(!object)
   { RET("%p", NULL); return( NULL ); }

   /* Copy data */
   object->texture = dlCopyTexture(src->texture);

   object->blend1 = src->blend1;
   object->blend2 = src->blend2;
   object->flags  = src->flags;

   LOGWARN("COPY");

   /* Increase ref counter */
   object->refCounter++;

   /* Return the instance */
   RET("%p", object);
   return( object );
}

/* Reference material object */
dlMaterial* dlRefMaterial( dlMaterial *src )
{
   dlMaterial* object;
   CALL("%p", src);

   /* Fuuuuuuuuu--- We have non valid object */
   if(!src) { RET("%p", NULL); return( NULL ); }

   /* Point magic */
   object = src;

   /* Ref textures */
   object->texture = dlRefTexture( object->texture );

   LOGWARN("REFERENCE");

   /* Increase ref counter */
   object->refCounter++;

   /* Return the instance */
   RET("%p", object);
   return( object );
}

/* Free material object */
int dlFreeMaterial( dlMaterial *object )
{
   CALL("%p", object);

   /* Fuuuuuuuuu--- We have non valid object */
   if(!object) { RET("%d", RETURN_NOTHING); return( RETURN_NOTHING ); }

   /* Free texture */
   if(dlFreeTexture( object->texture ) == RETURN_OK)
      object->texture = NULL;

   /* There is still references to this object alive */
   if(--object->refCounter != 0) { RET("%d", RETURN_NOTHING); return( RETURN_NOTHING ); }

   dlSetAlloc( ALLOC_MATERIAL );

   LOGFREE("FREE");

   /* Free scene object */
   dlFree( object, sizeof(dlMaterial) );

   RET("%d", RETURN_OK);
   return( RETURN_OK );
}

/* Add texture, steals reference */
int dlMaterialAddTexture( dlMaterial *object,
                          dlTexture *texture )
{
   CALL("%p, %p", object, texture);

   if(!object)
   { RET("%d", RETURN_FAIL); return( RETURN_FAIL ); }

   if(object->texture) dlFreeTexture(object->texture);
   object->texture = texture;

   RET("%d", RETURN_OK);
   return( RETURN_OK );
}

/* Free texture */
int dlMaterialFreeTexture( dlMaterial *object )
{
   CALL("%p", object);

   if(!object)
   { RET("%d", RETURN_FAIL); return( RETURN_FAIL ); }

   if( dlFreeTexture( object->texture ) == RETURN_OK )
      object->texture = NULL;

   RET("%d", RETURN_OK);
   return( RETURN_OK );
}
