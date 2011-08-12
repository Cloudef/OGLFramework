#include "bone.h"
#include "alloc.h"
#include "types.h"

/* new bone */
glBone* glNewBone(void)
{
   glBone *bone = glCalloc( 1, sizeof(glBone) );
   if(!bone)
      return( NULL );

   /* inc ref */
   bone->refCounter++;

   return( bone );
}

/* refence bone */
glBone* glRefBone( glBone *bone )
{
   if(!bone)
      return( NULL );

   /* increase ref */
   bone->refCounter++;
   return( bone );
}

/* free bone */
int glFreeBone( glBone *bone )
{
   if(!bone)
      return( RETURN_NOTHING );

   if(--bone->refCounter!=0)
      return( RETURN_NOTHING );

   if(bone->name) free(bone->name);
   bone->name = NULL;

   glFree( bone->weight, bone->num_weights * sizeof(glVertexWeight) );
   bone->weight = NULL;

   glFree( bone, sizeof(glBone) ); bone = NULL;
   return( RETURN_OK );
}
