#include <malloc.h>
#include "dlAnim.h"
#include "dlAlloc.h"
#include "dlTypes.h"

/* new anim */
dlAnim* dlNewAnim(void)
{
   dlSetAlloc( ALLOC_ANIM );
   dlAnim *anim = dlCalloc( 1, sizeof(dlAnim) );
   if(!anim)
      return( NULL );

   /* NULL */
   anim->name     = NULL;
   anim->node     = NULL;

   /* defaults */
   anim->ticksPerSecond = 25.f;
   anim->duration       = 0.0f;

   /* increase ref */
   anim->refCounter++;

   return( anim );
}

/* ref anim */
dlAnim* dlRefAnim( dlAnim *anim )
{
   if(!anim)
      return( NULL );

   /* increase ref */
   anim->refCounter++;
   return( anim );
}

/* free anim */
int dlFreeAnim( dlAnim *anim )
{
   dlNodeAnim  *node, *next;
   dlVectorKey *vkey, *nextvkey;
   dlQuatKey   *qkey, *nextqkey;

   if(!anim)
      return( RETURN_NOTHING );

   if(--anim->refCounter!=0)
      return( RETURN_NOTHING );

   dlSetAlloc( ALLOC_ANIM );

   /* free strdupped name */
   if(anim->name) free( anim->name );
   anim->name = NULL;

   /* free all nodes */
   node = anim->node;
   while(node)
   {
      /* free translation */
      vkey = node->translation;
      while(vkey)
      { nextvkey = vkey->next; dlFree(vkey, sizeof(dlVectorKey)); vkey = nextvkey; }
      node->translation = NULL;

      /* free rotation */
      qkey = node->rotation;
      while(qkey)
      { nextqkey = qkey->next; dlFree(qkey, sizeof(dlQuatKey)); qkey = nextqkey; }
      node->translation = NULL;

      /* free scaling */
      vkey = node->scaling;
      while(vkey)
      { nextvkey = vkey->next; dlFree(vkey, sizeof(dlVectorKey)); vkey = nextvkey; }
      node->scaling = NULL;

      /* free node */
      next = node->next;
      dlFree(node, sizeof(dlNodeAnim));
      node = next;
   }
   anim->node = NULL;

   /* free object */
   dlFree( anim, sizeof(dlAnim) );
   anim = NULL;
   return( RETURN_OK );
}

/* add new node to animation */
dlNodeAnim* dlAnimAddNode( dlAnim *anim )
{
   dlNodeAnim *node, **ptr;

   /* not valid animation */
   if(!anim)
      return( NULL );

   /* find next empty node */
   ptr = &anim->node;
   node = anim->node;
   for(; node; node = node->next)
      ptr = &node->next;

   /* allocate new node */
   dlSetAlloc( ALLOC_ANIM );
   *ptr = dlCalloc( 1, sizeof(dlNodeAnim) );
   if(!*ptr)
      return( NULL );

   /* null next */
   (*ptr)->next = NULL;

   /* null */
   (*ptr)->translation = NULL;
   (*ptr)->rotation    = NULL;
   (*ptr)->scaling     = NULL;

   (*ptr)->bone        = NULL;

   (*ptr)->num_translation = 0;
   (*ptr)->num_rotation    = 0;
   (*ptr)->num_scaling     = 0;

   /* success */
   return( *ptr );
}

/* add new translation key to node */
dlVectorKey* dlNodeAddTranslationKey(dlNodeAnim *node, kmVec3 value, float time )
{
   dlVectorKey *key, **ptr;

   /* not valid animation */
   if(!node)
      return( NULL );

   /* find next empty node */
   ptr = &node->translation;
   key = node->translation;
   for(; key; key = key->next)
      ptr = &key->next;

   /* allocate new node */
   dlSetAlloc( ALLOC_ANIM );
   *ptr = dlCalloc( 1, sizeof(dlVectorKey) );
   if(!*ptr)
      return( NULL );

   /* assing values */
   (*ptr)->value = value;
   (*ptr)->time  = time;

   /* null next */
   (*ptr)->next = NULL;

   /* increase count */
   node->num_translation++;

   /* success */
   return( *ptr );
}

/* add new rotation key to node */
dlQuatKey* dlNodeAddRotationKey(dlNodeAnim *node, kmQuaternion value, float time)
{
   dlQuatKey *key, **ptr;

   /* not valid animation */
   if(!node)
      return( NULL );

   /* find next empty node */
   ptr = &node->rotation;
   key = node->rotation;
   for(; key; key = key->next)
      ptr = &key->next;


   /* allocate new node */
   dlSetAlloc( ALLOC_ANIM );
   *ptr = dlCalloc( 1, sizeof(dlQuatKey) );
   if(!*ptr)
      return( NULL );

   /* assing values */
   (*ptr)->value = value;
   (*ptr)->time  = time;

   /* null next */
   (*ptr)->next = NULL;

   /* increase count */
   node->num_rotation++;

   /* success */
   return( *ptr );
}

/* add new scale key to node */
dlVectorKey* dlNodeAddScalingKey(dlNodeAnim *node, kmVec3 value, float time)
{
   dlVectorKey *key, **ptr;

   /* not valid animation */
   if(!node)
      return( NULL );

   /* find next empty node */
   ptr = &node->scaling;
   key = node->scaling;
   for(; key; key = key->next)
      ptr = &key->next;

   /* allocate new node */
   dlSetAlloc( ALLOC_ANIM );
   *ptr = dlCalloc( 1, sizeof(dlVectorKey) );
   if(!*ptr)
      return( NULL );

   /* assing values */
   (*ptr)->value = value;
   (*ptr)->time  = time;

   /* null next */
   (*ptr)->next = NULL;

   /* increase count */
   node->num_scaling++;

   /* success */
   return( *ptr );
}
