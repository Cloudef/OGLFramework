#include "bone.h"
#include "alloc.h"
#include "types.h"
#include <malloc.h>

/* new bone */
glBone* glNewBone(void)
{
   glBone *bone = glCalloc( 1, sizeof(glBone) );
   if(!bone)
      return( NULL );

   /* null */
   bone->name   = NULL;
   bone->parent = NULL;
   bone->next   = NULL;

#if 0
   /* child */
   bone->child  = NULL;
   bone->num_child = 0;
#endif

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
   glVertexWeight *weight, *next;

   if(!bone)
      return( RETURN_NOTHING );

   if(--bone->refCounter!=0)
      return( RETURN_NOTHING );

   /* free strdupped name */
   if(bone->name) free(bone->name);
   bone->name = NULL;

   /* free weights */
   weight = bone->weight;
   while(weight)
   { next = weight->next; glFree(weight, sizeof(glVertexWeight)); weight = next; }
   bone->weight = NULL;

#if 0
   /* free child list */
   glFree( bone->child, bone->num_child * sizeof(glBone*) );
   bone->child = NULL;
#endif

   /* free bone */
   glFree( bone, sizeof(glBone) ); bone = NULL;
   return( RETURN_OK );
}

/* add weight to bone */
glVertexWeight* glBoneAddWeight( glBone *bone, unsigned int vertex, float value )
{
   glVertexWeight *weight, **ptr;

   if(!bone)
      return( NULL );

   /* find empty slot */
   ptr = &bone->weight;
   weight = bone->weight;
   for(; weight; weight = weight->next)
      ptr = &weight->next;

   /* add weight */
   *ptr = glCalloc( 1, sizeof(glVertexWeight) );
   if(!*ptr)
      return( NULL );

   /* assign values */
   (*ptr)->vertex = vertex;
   (*ptr)->value  = value;

   /* null next */
   (*ptr)->next = NULL;

   return( *ptr );
}

/* add child bone to list */
int glBoneAddChild( glBone *bone, glBone *child )
{
   if(!bone || !child)
      return( RETURN_FAIL );

#if 0 /* child code */
   /* resize array */
   bone->num_child++;
   if(bone->child)
      bone->child = glRealloc( bone->child, bone->num_child - 1, bone->num_child, sizeof(glBone*) );
   else
      bone->child = glCalloc( bone->num_child, sizeof(glBone*) );

   /* fail */
   if(!bone->child)
      return( RETURN_FAIL );

   /* add child */
   bone->child[bone->num_child - 1] = child;
#endif
   child->parent = bone;

   return( RETURN_OK );
}
