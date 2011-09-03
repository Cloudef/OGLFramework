#include <malloc.h>
#include "anim.h"
#include "alloc.h"
#include "types.h"

/* new anim */
glAnim* glNewAnim(void)
{
   glSetAlloc( ALLOC_ANIM );
   glAnim *anim = glCalloc( 1, sizeof(glAnim) );
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
   glNodeAnim  *node, *next;
   glVectorKey *vkey, *nextvkey;
   glQuatKey   *qkey, *nextqkey;

   if(!anim)
      return( RETURN_NOTHING );

   if(--anim->refCounter!=0)
      return( RETURN_NOTHING );

   glSetAlloc( ALLOC_ANIM );

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
      { nextvkey = vkey->next; glFree(vkey, sizeof(glVectorKey)); vkey = nextvkey; }
      node->translation = NULL;

      /* free rotation */
      qkey = node->rotation;
      while(qkey)
      { nextqkey = qkey->next; glFree(qkey, sizeof(glQuatKey)); qkey = nextqkey; }
      node->translation = NULL;

      /* free scaling */
      vkey = node->scaling;
      while(vkey)
      { nextvkey = vkey->next; glFree(vkey, sizeof(glVectorKey)); vkey = nextvkey; }
      node->scaling = NULL;

      /* free node */
      next = node->next;
      glFree(node, sizeof(glNodeAnim));
      node = next;
   }
   anim->node = NULL;

   /* free object */
   glFree( anim, sizeof(glAnim) );
   anim = NULL;
   return( RETURN_OK );
}

/* add new node to animation */
glNodeAnim* glAnimAddNode( glAnim *anim )
{
   glNodeAnim *node, **ptr;

   /* not valid animation */
   if(!anim)
      return( NULL );

   /* find next empty node */
   ptr = &anim->node;
   node = anim->node;
   for(; node; node = node->next)
      ptr = &node->next;

   /* allocate new node */
   glSetAlloc( ALLOC_ANIM );
   *ptr = glCalloc( 1, sizeof(glNodeAnim) );
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
glVectorKey* glNodeAddTranslationKey(glNodeAnim *node, kmVec3 value, float time )
{
   glVectorKey *key, **ptr;

   /* not valid animation */
   if(!node)
      return( NULL );

   /* find next empty node */
   ptr = &node->translation;
   key = node->translation;
   for(; key; key = key->next)
      ptr = &key->next;

   /* allocate new node */
   glSetAlloc( ALLOC_ANIM );
   *ptr = glCalloc( 1, sizeof(glVectorKey) );
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
glQuatKey* glNodeAddRotationKey(glNodeAnim *node, kmQuaternion value, float time)
{
   glQuatKey *key, **ptr;

   /* not valid animation */
   if(!node)
      return( NULL );

   /* find next empty node */
   ptr = &node->rotation;
   key = node->rotation;
   for(; key; key = key->next)
      ptr = &key->next;


   /* allocate new node */
   glSetAlloc( ALLOC_ANIM );
   *ptr = glCalloc( 1, sizeof(glQuatKey) );
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
glVectorKey* glNodeAddScalingKey(glNodeAnim *node, kmVec3 value, float time)
{
   glVectorKey *key, **ptr;

   /* not valid animation */
   if(!node)
      return( NULL );

   /* find next empty node */
   ptr = &node->scaling;
   key = node->scaling;
   for(; key; key = key->next)
      ptr = &key->next;

   /* allocate new node */
   glSetAlloc( ALLOC_ANIM );
   *ptr = glCalloc( 1, sizeof(glVectorKey) );
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
