#include <malloc.h>

#include "dlAnimator.h"
#include "dlAlloc.h"
#include "dlTypes.h"
#include "dlLog.h"

#define DL_DEBUG_CHANNEL "ANIMATOR"

/* new animator */
dlAnimator* dlNewAnimator( void )
{
   dlAnimator *object;
   TRACE();

   /* create new animator */
   dlSetAlloc( ALLOC_ANIMATOR );
   object = (dlAnimator*)dlCalloc( 1, sizeof(dlAnimator) );
   if(!object)
   { RET("%p", NULL); return( NULL ); }

   /* null */
   object->anim = NULL;
   object->bone = NULL;
   object->tick = NULL;

   object->current = NULL;

   LOGOK("NEW");
   object->refCounter++;

   RET("%p", object);
   return( object );
}

/* copy animator */
dlAnimator* dlCopyAnimator( dlAnimator *src )
{
   dlAnimator *object;
   CALL("%p", src);

   /* invalid source */
   if(!src) { RET("%p", NULL); return( NULL ); }

   /* allocate object */
   dlSetAlloc( ALLOC_ANIMATOR );
   object = (dlAnimator*)dlCalloc( 1, sizeof(dlAnimator) );
   if(!object)
   { RET("%p", NULL); return( NULL ); }

   /* null */
   object->tick    = NULL;
   object->current = NULL;

   /* reference animations */
   object->anim   = dlAnimatorRefAnims( src );
   object->bone   = dlAnimatorRefBones( src );

   if(src->current)
   {
      object->tick      = dlNewAnimTick( src->current );
      object->current   = src->current;
   }

   LOGWARN("COPY");
   object->refCounter++;

   RET("%p", object);
   return( object );
}

dlAnimator* dlRefAnimator( dlAnimator *src )
{
   dlAnimator *object;
   CALL("%p", src);

   /* invalid source */
   if(!src)
   { RET("%p", NULL); return( NULL ); }

   /* point */
   object = src;

   /* reference animations */
   object->anim   = dlAnimatorRefAnims( src );
   object->bone   = dlAnimatorRefBones( src );

   LOGWARN("REFERENCE");
   object->refCounter++;

   RET("%p", object);
   return( object );
}

int dlFreeAnimator( dlAnimator *object )
{
   dlBone *bone, *nextbone;
   dlAnim *anim, *nextanim;
   CALL("%p", object);

   /* invalid object */
   if(!object)
   { RET("%d", RETURN_NOTHING); return( RETURN_NOTHING ); }

   /* free bones */
   bone = object->bone;
   while(bone)
   { nextbone = bone->next;
     if(dlFreeBone(bone) == RETURN_OK) object->bone = NULL; /* truly freed */
     bone = nextbone; }

   /* free animations */
   anim = object->anim;
   while(anim)
   { nextanim = anim->next;
     if(dlFreeAnim(anim) == RETURN_OK) object->anim = NULL; /* truly freed */
     anim = nextanim; }

   if(--object->refCounter!=0) { RET("%d", RETURN_NOTHING); return( RETURN_NOTHING ); }

   /* free animation ticker */
   dlFreeAnimTick( object->tick );
   object->tick = NULL;

   dlSetAlloc( ALLOC_ANIMATOR );

   LOGFREE("FREE");

   /* free object */
   dlFree( object, sizeof(dlAnimator) );
   object = NULL;

   RET("%d", RETURN_OK);
   return( RETURN_OK );
}

/* Change animation */
void dlAnimatorSetAnim( dlAnimator *object, DL_NODE_TYPE index )
{
   dlAnim *node;
   DL_NODE_TYPE i;
   CALL("%p, %d", object, index);

   /* invalid object */
   if(!object)
      return;

   /* get animation */
   node = object->anim; i = 0;
   for(; i != index && node; node = node->next)
      i++;

   if(node == object->current)
      return;

   /* set animation */
   dlFreeAnimTick( object->tick );
   object->tick      = dlNewAnimTick( node );
   object->current   = node;
}

/* Tick animation */
void dlAnimatorTick( dlAnimator *object, float time )
{
   CALL("%p, %f", object, time);

   if(!object)
      return;
   if(!object->tick)
      return;

   /* Advance tick */
   dlAdvanceAnimTick( object->tick, time );
   dlAnimatorCalculateGlobalTransformations( object );
}

/* Add new bone */
dlBone* dlAnimatorAddBone( dlAnimator *object )
{
   dlBone *bone, **ptr;
   CALL("%p", object);

   /* invalid object */
   if(!object)
   { RET("%p", NULL); return( NULL ); }

   /* find empty slot */
   ptr  = &object->bone;
   bone = object->bone;
   for(; bone; bone = bone->next)
      ptr = &bone->next;

   /* assign new bone */
   *ptr = dlNewBone();
   if(!*ptr)
   { RET("%p", NULL); return( NULL ); }

   /* null next */
   (*ptr)->next   = NULL;
   (*ptr)->parent = NULL;

   RET("%p", *ptr);
   return( *ptr );
}

/* Reference all bones */
dlBone* dlAnimatorRefBones( dlAnimator *object )
{
   dlBone *bone;
   CALL("%p", object);

   if(!object)
   { RET("%p", NULL); return( NULL ); }
   if(!object->bone)
   { RET("%p", NULL); return( NULL ); }

   /* reference everything */
   bone = object->bone;
   for(; bone; bone = bone->next)
      dlRefBone( bone );

   RET("%p", object->bone);
   return( object->bone );
}

/* Get bone by name */
dlBone* dlAnimatorGetBone( dlAnimator *object, const char *name )
{
   dlBone *bone;
   CALL("%p, %s", object, name);

   if(!object || !name)
   { RET("%p", NULL); return( NULL ); }

   /* find */
   bone = object->bone;
   for(; bone; bone = bone->next)
   {
      if(bone->name) if(strcmp( bone->name, name ) == 0)
      { RET("%p", bone); return( bone ); }
   }

   /* return */
   RET("%p", NULL);
   return( NULL );
}

/* Add animation */
dlAnim* dlAnimatorAddAnim( dlAnimator *object )
{
   dlAnim *anim, **ptr;
   CALL("%p", object);

   /* invalid object */
   if(!object)
   { RET("%p", NULL); return( NULL ); }

   /* find empty slot */
   ptr  = &object->anim;
   anim = object->anim;
   for(; anim; anim = anim->next)
      ptr = &anim->next;

   /* assign new animation */
   *ptr = dlNewAnim();
   if(!*ptr)
   { RET("%p", NULL); return( NULL ); }

   /* null next */
   (*ptr)->next = NULL;

   RET("%p", NULL);
   return( *ptr );
}

/* Reference all animations */
dlAnim* dlAnimatorRefAnims( dlAnimator *object )
{
   dlAnim *anim;
   CALL("%p", object);

   if(!object)
   { RET("%p", NULL); return( NULL ); }
   if(!object->anim)
   { RET("%p", NULL); return( NULL ); }

   /* reference everything */
   anim = object->anim;
   for(; anim; anim = anim->next)
      dlRefAnim( anim );

   RET("%p", object->anim);
   return( object->anim );
}

/* Calculate global transformations */
void dlAnimatorCalculateGlobalTransformations( dlAnimator *object )
{
   dlBone *parent, *bone;
   kmMat4 globalMat;
   CALL("%p", object);

   if(!object)
      return;

   bone = object->bone;
   for(; bone; bone = bone->next)
   {
      parent    = bone;
      globalMat = bone->offsetMatrix;
      for(; parent; parent = parent->parent)
         kmMat4Multiply( &globalMat, &globalMat, &parent->relativeMatrix );
      bone->globalMatrix = globalMat;
   }
}
