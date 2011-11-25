#include <malloc.h>
#include "dlAnim.h"
#include "dlAlloc.h"
#include "dlTypes.h"
#include "dlLog.h"

#define DL_DEBUG_CHANNEL "ANIM"

/* new anim */
dlAnim* dlNewAnim(void)
{
   TRACE();

   dlSetAlloc( ALLOC_ANIM );
   dlAnim *anim = dlCalloc( 1, sizeof(dlAnim) );
   if(!anim)
   { RET("%p", NULL); return( NULL ); }

   /* NULL */
   anim->name     = NULL;
   anim->node     = NULL;

   /* defaults */
   anim->ticksPerSecond = 25.f;
   anim->duration       = 0.0f;

   LOGOK("NEW");

   /* increase ref */
   anim->refCounter++;

   RET("%p", anim);
   return( anim );
}

/* ref anim */
dlAnim* dlRefAnim( dlAnim *anim )
{
   CALL("%p", anim);

   if(!anim)
   { RET("%p", NULL); return( NULL ); }

   LOGWARN("REFERENCE");

   /* increase ref */
   anim->refCounter++;

   RET("%p", anim);
   return( anim );
}

/* free anim */
int dlFreeAnim( dlAnim *anim )
{
   dlNodeAnim  *node, *next;
   dlVectorKey *vkey, *nextvkey;
   dlQuatKey   *qkey, *nextqkey;
   CALL("%p", anim);

   if(!anim)
   { RET("%d", RETURN_NOTHING); return( RETURN_NOTHING ); }

   if(--anim->refCounter!=0)
   { RET("%d", RETURN_NOTHING); return( RETURN_NOTHING ); }

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

   LOGFREE("FREE");

   /* free object */
   dlFree( anim, sizeof(dlAnim) );
   anim = NULL;

   RET("%d", RETURN_OK);
   return( RETURN_OK );
}

/* add new node to animation */
dlNodeAnim* dlAnimAddNode( dlAnim *anim )
{
   dlNodeAnim *node, **ptr;
   CALL("%p", anim);

   /* not valid animation */
   if(!anim)
   { RET("%p", NULL); return( NULL ); }

   /* find next empty node */
   ptr = &anim->node;
   node = anim->node;
   for(; node; node = node->next)
      ptr = &node->next;

   /* allocate new node */
   dlSetAlloc( ALLOC_ANIM );
   *ptr = dlCalloc( 1, sizeof(dlNodeAnim) );
   if(!*ptr)
   { RET("%p", NULL); return( NULL ); }

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
   RET("%p", *ptr);
   return( *ptr );
}

/* add new translation key to node */
dlVectorKey* dlNodeAddTranslationKey(dlNodeAnim *node, kmVec3 *value, float time )
{
   dlVectorKey *key, **ptr;
   CALL("%p, vec3[%f, %f, %f], %f", node, value->x, value->y, value->z, time);

   /* not valid animation */
   if(!node)
   { RET("%p", NULL); return( NULL ); }

   /* find next empty node */
   ptr = &node->translation;
   key = node->translation;
   for(; key; key = key->next)
      ptr = &key->next;

   /* allocate new node */
   dlSetAlloc( ALLOC_ANIM );
   *ptr = dlCalloc( 1, sizeof(dlVectorKey) );
   if(!*ptr)
   { RET("%p", NULL); return( NULL ); }

   /* assing values */
   (*ptr)->value = *value;
   (*ptr)->time  = time;

   /* null next */
   (*ptr)->next = NULL;

   /* increase count */
   node->num_translation++;

   /* success */
   RET("%p", *ptr);
   return( *ptr );
}

/* add new rotation key to node */
dlQuatKey* dlNodeAddRotationKey(dlNodeAnim *node, kmQuaternion *value, float time)
{
   dlQuatKey *key, **ptr;
   CALL("%p, quat[%f, %f, %f, %f], %f", node,
         value->x, value->y, value->z, value->w, time);

   /* not valid animation */
   if(!node)
   { RET("%p", NULL); return( NULL ); }

   /* find next empty node */
   ptr = &node->rotation;
   key = node->rotation;
   for(; key; key = key->next)
      ptr = &key->next;

   /* allocate new node */
   dlSetAlloc( ALLOC_ANIM );
   *ptr = dlCalloc( 1, sizeof(dlQuatKey) );
   if(!*ptr)
   { RET("%p", NULL); return( NULL ); }

   /* assing values */
   (*ptr)->value = *value;
   (*ptr)->time  = time;

   /* null next */
   (*ptr)->next = NULL;

   /* increase count */
   node->num_rotation++;

   /* success */
   RET("%p", *ptr);
   return( *ptr );
}

/* add new scale key to node */
dlVectorKey* dlNodeAddScalingKey(dlNodeAnim *node, kmVec3 *value, float time)
{
   dlVectorKey *key, **ptr;
   CALL("%p, vec3[%f, %f, %f], %f", node,
         value->x, value->y, value->z, time);

   /* not valid animation */
   if(!node)
   { RET("%p", NULL); return( NULL ); }

   /* find next empty node */
   ptr = &node->scaling;
   key = node->scaling;
   for(; key; key = key->next)
      ptr = &key->next;

   /* allocate new node */
   dlSetAlloc( ALLOC_ANIM );
   *ptr = dlCalloc( 1, sizeof(dlVectorKey) );
   if(!*ptr)
   { RET("%p", NULL); return( NULL ); }

   /* assing values */
   (*ptr)->value = *value;
   (*ptr)->time  = time;

   /* null next */
   (*ptr)->next = NULL;

   /* increase count */
   node->num_scaling++;

   /* success */
   RET("%p", *ptr);
   return( *ptr );
}
