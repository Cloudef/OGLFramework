#include <malloc.h>
#include "anim.h"
#include "alloc.h"
#include "types.h"

/* new anim */
glAnim* glNewAnim(void)
{
   glAnim *anim = glCalloc( 1, sizeof(glAnim) );
   if(!anim)
      return( NULL );

   /* NULL */
   anim->name     = NULL;
   anim->skeletal = NULL;
#if USE_KEYFRAME_ANIMATION
   anim->keyFrame = NULL;
#endif

   /* increase ref */
   anim->refCounter++;

   return( anim );
}

/* ref anim */
glAnim* glRefAnim( glAnim *anim )
{
   if(!anim)
      return( NULL );

   /* increase ref */
   anim->refCounter++;
   return( anim );
}

/* free anim */
int glFreeAnim( glAnim *anim )
{
   unsigned int i;
   if(!anim)
      return( RETURN_NOTHING );

   if(--anim->refCounter!=0)
      return( RETURN_NOTHING );

   if(anim->name) free( anim->name );
   anim->name = NULL;

   i = 0;
   for(; i != anim->num_skeletal; ++i)
   {
      glFree( anim->skeletal[i].positionKey,
              anim->skeletal[i].num_position * sizeof(glVectorKey) );
      glFree( anim->skeletal[i].rotationKey,
              anim->skeletal[i].num_rotation * sizeof(glQuatKey) );
      glFree( anim->skeletal[i].scaleKey,
              anim->skeletal[i].num_scale * sizeof(glVectorKey) );
   }

   glFree( anim->skeletal, anim->num_skeletal * sizeof(glNodeAnim) );
   anim->skeletal = NULL;
#if USE_KEYFRAME_ANIMATION
   glFree( anim->keyFrame, anim->num_keyframe * sizeof(glMeshAnim) );
   anim->keyFrame = NULL;
#endif
   glFree( anim, sizeof(glAnim) );
   anim = NULL;

   return( RETURN_OK );
}
