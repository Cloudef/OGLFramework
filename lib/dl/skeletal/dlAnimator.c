#include <malloc.h>

#include "dlAnimator.h"
#include "dlAlloc.h"
#include "dlTypes.h"
#include "dlLog.h"

/* new animator */
dlAnimator* dlNewAnimator( void )
{
   dlAnimator *object;

   /* create new animator */
   dlSetAlloc( ALLOC_ANIMATOR );
   object = (dlAnimator*)dlCalloc( 1, sizeof(dlAnimator) );
   if(!object)
      return( NULL );

   /* null */
   object->anim = NULL;
   object->bone = NULL;
   object->tick = NULL;

   object->current   = NULL;

   logGreen();
   dlPuts("[A:ANIMATOR]");
   logNormal();

   object->refCounter++;
   return( object );
}

/* copy animator */
dlAnimator* dlCopyAnimator( dlAnimator *src )
{
   dlAnimator *object;

   /* invalid source */
   if(!src) return( NULL );

   /* allocate object */
   dlSetAlloc( ALLOC_ANIMATOR );
   object = (dlAnimator*)dlCalloc( 1, sizeof(dlAnimator) );
   if(!object)
      return( NULL );

   /* null */
   object->tick    = NULL;
   object->current = NULL;

   /* reference animations */
   object->anim                  = dlAnimatorRefAnims( src );
   object->bone                  = dlAnimatorRefBones( src );

   if(src->current)
   {
      object->tick      = dlNewAnimTick( src->current );
      object->current   = src->current;
   }

   logYellow();
   dlPuts("[C:ANIMATOR]");
   logNormal();

   object->refCounter++;
   return( object );
}

dlAnimator* dlRefAnimator( dlAnimator *src )
{
   dlAnimator *object;

   /* invalid source */
   if(!src)
      return( NULL );

   /* point */
   object = src;

   /* reference animations */
   object->anim                  = dlAnimatorRefAnims( src );
   object->bone                  = dlAnimatorRefBones( src );

   logYellow();
   dlPuts("[R:ANIMATOR]");
   logNormal();

   object->refCounter++;
   return( object );
}

int dlFreeAnimator( dlAnimator *object )
{
   dlBone *bone, *nextbone;
   dlAnim *anim, *nextanim;

   /* invalid object */
   if(!object)
      return( RETURN_NOTHING );

   if(object->refCounter - 1 == 0)
   {
      /* free animation ticker */
      dlFreeAnimTick( object->tick );
      object->tick = NULL;
   }

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

   if(--object->refCounter!=0)
      return( RETURN_NOTHING );

   dlSetAlloc( ALLOC_ANIMATOR );

   logRed();
   dlPuts("[F:ANIMATOR]");
   logNormal();

   /* free object */
   dlFree( object, sizeof(dlAnimator) );
   object = NULL;
   return( RETURN_OK );
}

/* Change animation */
void dlAnimatorSetAnim( dlAnimator *object, DL_NODE_TYPE index )
{
   dlAnim *node;
   DL_NODE_TYPE i;

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

   /* invalid object */
   if(!object)
      return( NULL );

   /* find empty slot */
   ptr  = &object->bone;
   bone = object->bone;
   for(; bone; bone = bone->next)
      ptr = &bone->next;

   /* assign new bone */
   *ptr = dlNewBone();
   if(!*ptr)
      return( NULL );

   /* null next */
   (*ptr)->next   = NULL;
   (*ptr)->parent = NULL;

   return( *ptr );
}

/* Reference all bones */
dlBone* dlAnimatorRefBones( dlAnimator *object )
{
   dlBone *bone;

   if(!object)
      return( NULL );
   if(!object->bone)
      return( NULL );

   /* reference everything */
   bone = object->bone;
   for(; bone; bone = bone->next)
      dlRefBone( bone );

   return( object->bone );
}

/* Get bone by name */
dlBone* dlAnimatorGetBone( dlAnimator *object, const char *name )
{
   dlBone *bone;

   if(!object || !name)
      return( NULL );

   /* find */
   bone = object->bone;
   for(; bone; bone = bone->next)
   { if(bone->name) if(strcmp( bone->name, name ) == 0) return( bone ); }

   /* return */
   return( NULL );
}

/* Add animation */
dlAnim* dlAnimatorAddAnim( dlAnimator *object )
{
   dlAnim *anim, **ptr;

   /* invalid object */
   if(!object)
      return( NULL );

   /* find empty slot */
   ptr  = &object->anim;
   anim = object->anim;
   for(; anim; anim = anim->next)
      ptr = &anim->next;

   /* assign new animation */
   *ptr = dlNewAnim();
   if(!*ptr)
      return( NULL );

   /* null next */
   (*ptr)->next = NULL;

   return( *ptr );
}

/* Reference all animations */
dlAnim* dlAnimatorRefAnims( dlAnimator *object )
{
   dlAnim *anim;

   if(!object)
      return( NULL );
   if(!object->anim)
      return( NULL );

   /* reference everything */
   anim = object->anim;
   for(; anim; anim = anim->next)
      dlRefAnim( anim );

   return( object->anim );
}

/* Calculate global transformations */
void dlAnimatorCalculateGlobalTransformations( dlAnimator *object )
{
   dlBone *parent, *bone;
   kmMat4 globalMat;

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
