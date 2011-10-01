#include "dlBone.h"
#include "dlAlloc.h"
#include "dlTypes.h"
#include <malloc.h>

/* new bone */
dlBone* dlNewBone(void)
{
   dlSetAlloc( ALLOC_BONE );
   dlBone *bone = dlCalloc( 1, sizeof(dlBone) );
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
dlBone* dlRefBone( dlBone *bone )
{
   if(!bone)
      return( NULL );

   /* increase ref */
   bone->refCounter++;
   return( bone );
}

/* free bone */
int dlFreeBone( dlBone *bone )
{
   dlVertexWeight *weight, *next;

   if(!bone)
      return( RETURN_NOTHING );

   if(--bone->refCounter!=0)
      return( RETURN_NOTHING );

   dlSetAlloc( ALLOC_BONE );

   /* free strdupped name */
   if(bone->name) free(bone->name);
   bone->name = NULL;

   /* free weights */
   weight = bone->weight;
   while(weight)
   { next = weight->next; dlFree(weight, sizeof(dlVertexWeight)); weight = next; }
   bone->weight = NULL;

#if 0
   /* free child list */
   dlFree( bone->child, bone->num_child * sizeof(dlBone*) );
   bone->child = NULL;
#endif

   /* free bone */
   dlFree( bone, sizeof(dlBone) ); bone = NULL;
   return( RETURN_OK );
}

/* add weight to bone */
dlVertexWeight* dlBoneAddWeight( dlBone *bone, unsigned int vertex, float value )
{
   dlVertexWeight *weight, **ptr;

   if(!bone)
      return( NULL );

   /* find empty slot */
   ptr = &bone->weight;
   weight = bone->weight;
   for(; weight; weight = weight->next)
      ptr = &weight->next;

   /* add weight */
   dlSetAlloc( ALLOC_BONE );
   *ptr = dlCalloc( 1, sizeof(dlVertexWeight) );
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
int dlBoneAddChild( dlBone *bone, dlBone *child )
{
   if(!bone || !child)
      return( RETURN_FAIL );

#if 0 /* child code */
   /* resize array */
   bone->num_child++;
   if(bone->child)
      bone->child = dlRealloc( bone->child, bone->num_child - 1, bone->num_child, sizeof(dlBone*) );
   else
      bone->child = dlCalloc( bone->num_child, sizeof(dlBone*) );

   /* fail */
   if(!bone->child)
      return( RETURN_FAIL );

   /* add child */
   bone->child[bone->num_child - 1] = child;
#endif
   child->parent = bone;

   return( RETURN_OK );
}
