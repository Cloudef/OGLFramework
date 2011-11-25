#include "dlBone.h"
#include "dlAlloc.h"
#include "dlTypes.h"
#include "dlLog.h"
#include <malloc.h>

#define DL_DEBUG_CHANNEL "BONE"

/* new bone */
dlBone* dlNewBone(void)
{
   TRACE();

   dlSetAlloc( ALLOC_BONE );
   dlBone *bone = dlCalloc( 1, sizeof(dlBone) );
   if(!bone)
   { RET("%p", NULL); return( NULL ); }

   /* null */
   bone->name   = NULL;
   bone->parent = NULL;
   bone->next   = NULL;

#if 0
   /* child */
   bone->child  = NULL;
   bone->num_child = 0;
#endif

   LOGWARN("NEW");

   /* inc ref */
   bone->refCounter++;

   RET("%p", bone);
   return( bone );
}

/* refence bone */
dlBone* dlRefBone( dlBone *bone )
{
   CALL("%p", bone);

   if(!bone)
   { RET("%p", NULL); return( NULL ); }

   LOGWARN("REFERENCE");

   /* increase ref */
   bone->refCounter++;

   RET("%p", bone);
   return( bone );
}

/* free bone */
int dlFreeBone( dlBone *bone )
{
   dlVertexWeight *weight, *next;
   CALL("%p", bone);

   if(!bone)
   { RET("%d", RETURN_NOTHING); return( RETURN_NOTHING ); }

   if(--bone->refCounter!=0) { RET("%d", RETURN_NOTHING); return( RETURN_NOTHING ); }

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

   LOGFREE("FREE");

   /* free bone */
   dlFree( bone, sizeof(dlBone) ); bone = NULL;

   RET("%d", RETURN_OK);
   return( RETURN_OK );
}

/* add weight to bone */
dlVertexWeight* dlBoneAddWeight( dlBone *bone, unsigned int vertex, float value )
{
   dlVertexWeight *weight, **ptr;
   CALL("%p, %u, %f", bone, vertex, value);

   if(!bone)
   { RET("%p", NULL); return( NULL ); }

   /* find empty slot */
   ptr = &bone->weight;
   weight = bone->weight;
   for(; weight; weight = weight->next)
      ptr = &weight->next;

   /* add weight */
   dlSetAlloc( ALLOC_BONE );
   *ptr = dlCalloc( 1, sizeof(dlVertexWeight) );
   if(!*ptr)
   { RET("%p", NULL); return( NULL ); }

   /* assign values */
   (*ptr)->vertex = vertex;
   (*ptr)->value  = value;

   /* null next */
   (*ptr)->next = NULL;

   RET("%p", *ptr);
   return( *ptr );
}

/* add child bone to list */
int dlBoneAddChild( dlBone *bone, dlBone *child )
{
   CALL("%p, %p", bone, child);

   if(!bone || !child)
   { RET("%d", RETURN_FAIL); return( RETURN_FAIL ); }

#if 0 /* child code */
   /* resize array */
   bone->num_child++;
   if(bone->child)
      bone->child = dlRealloc( bone->child, bone->num_child - 1, bone->num_child, sizeof(dlBone*) );
   else
      bone->child = dlCalloc( bone->num_child, sizeof(dlBone*) );

   /* fail */
   if(!bone->child)
   { RET("%d", RETURN_FAIL); return( RETURN_FAIL ); }

   /* add child */
   bone->child[bone->num_child - 1] = child;
#endif
   child->parent = bone;

   RET("%d", RETURN_OK);
   return( RETURN_OK );
}
