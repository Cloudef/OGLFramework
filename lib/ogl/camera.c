#include "camera.h"
#include "types.h"
#include "alloc.h"

/* Allocate camera */
glCamera* glNewCamera( void )
{
	/* Allocate camera */
	glCamera *object = (glCamera*)glCalloc( 1, sizeof(glCamera) );
	if(!object)
      return( NULL );

	/* Increase ref counter */
	object->refCounter++;

	/* Return the instance */
	return( object );
}

/* Copy camera */
glCamera* glCopyCamera( glCamera *src )
{
   glCamera *object;

   /* Fuuuuuuuuu--- We have non valid object */
	if(!src) return( NULL );

	/* Allocate scene object */
	object = (glCamera*)glCalloc( 1, sizeof(glCamera) );
	if(!object)
      return( NULL );

	/* Increase ref counter */
	object->refCounter++;

	/* Return the instance */
	return( object );
}

/* Reference camera */
glCamera* glRefCamera( glCamera *src )
{
   glCamera* object;

	/* Fuuuuuuuuu--- We have non valid object */
	if(!src) return( NULL );

	/* Point magic */
	object                      = src;

	/* Increase ref counter */
	object->refCounter++;

	/* Return the instance */
	return( object );
}

/* Free camera */
int glFreeCamera( glCamera *object )
{
   unsigned int i;

	/* Fuuuuuuuuu--- We have non valid object */
   if(!object) return( RETURN_NOTHING );

	/* There is still references to this object alive */
	if(--object->refCounter != 0) return( RETURN_NOTHING );

	/* Free camera */
   glFree( object, sizeof(glCamera) );
   return( RETURN_OK );
}

/* EoF */
