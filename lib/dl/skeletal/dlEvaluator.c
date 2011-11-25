#include "dlEvaluator.h"
#include "dlAlloc.h"
#include "dlTypes.h"
#include "dlLog.h"

#include <assert.h>

#define DL_DEBUG_CHANNEL "EVALUATOR"

dlAnimTick* dlNewAnimTick( dlAnim *anim )
{
   dlNodeAnim           *node;
   dlAnimTickOldNode    **oldNode;

   dlVectorKey          *vkey;
   dlQuatKey            *qkey;
   DL_NODE_TYPE         count;

   CALL("%p", anim);

   if(!anim)
   { RET("%p", NULL); return( NULL ); }

	/* Allocate animation handler object */
   dlSetAlloc( ALLOC_EVALUATOR );
	dlAnimTick *animTick = (dlAnimTick*)dlCalloc( 1, sizeof(dlAnimTick) );
   if(!animTick)
   { RET("%p", NULL); return( NULL ); }

   /* assign animation */
   animTick->anim       = anim;
   animTick->oldTime    = 0.0f;

   /* null */
   animTick->oldNode    = NULL;

   node      = anim->node;
   oldNode   = &animTick->oldNode;
   for(; node; node = node->next)
   {
      *oldNode = dlCalloc( 1, sizeof(dlAnimTickOldNode) );
      if(!*oldNode)
      {
         dlFreeAnimTick( animTick );

         RET("%p", NULL);
         return( NULL );
      }

      /* store translations to pointer array */
      if(node->translation)
      {
         (*oldNode)->translation = dlCalloc( node->num_translation, sizeof(dlVectorKey*) );
         if(!(*oldNode)->translation)
         {
            dlFreeAnimTick( animTick );

            RET("%p", NULL);
            return( NULL );
         }

         vkey = node->translation; count = 0;
         for(; vkey; vkey = vkey->next)
            (*oldNode)->translation[count++] = vkey;
      }

      /* store rortations to pointer array */
      if(node->rotation)
      {
         (*oldNode)->rotation = dlCalloc( node->num_rotation, sizeof(dlQuatKey*) );
         if(!(*oldNode)->rotation)
         {
            dlFreeAnimTick( animTick );

            RET("%p", NULL);
            return( NULL );
         }

         qkey = node->rotation; count = 0;
         for(; qkey; qkey = qkey->next)
            (*oldNode)->rotation[count++] = qkey;
      }

      /* store scalings to pointer array */
      if(node->scaling)
      {
         (*oldNode)->scaling = dlCalloc( node->num_scaling, sizeof(dlVectorKey*) );
         if(!(*oldNode)->scaling)
         {
            dlFreeAnimTick( animTick );

            RET("%p", NULL);
            return( NULL );
         }

         vkey = node->scaling; count = 0;
         for(; vkey; vkey = vkey->next)
            (*oldNode)->scaling[count++] = vkey;
      }

      oldNode = &(*oldNode)->next;
   }

   /* no animations, pointless */
   if(!anim->node)
   {
      dlFreeAnimTick( animTick );

      RET("%p", NULL);
      return( NULL );
   }

   LOGOK("NEW");

   /* return */
   RET("%p", animTick);
   return( animTick );
}

int dlFreeAnimTick( dlAnimTick *animTick )
{
   dlAnimTickOldNode    *oldNode, *nextOldNode;
   dlNodeAnim           *node;
   CALL("%p", animTick);

   /* invalid object */
   if(!animTick)
   { RET("%d", RETURN_NOTHING); return( RETURN_NOTHING ); }

   dlSetAlloc( ALLOC_EVALUATOR );

   /* we should have animation */
   if(animTick->anim)
   {
      /* free nodes */
      oldNode = animTick->oldNode;
      node    = animTick->anim->node;
      while(oldNode)
      {
         /* free stored pointers */
         dlFree( oldNode->translation,
                 node->num_translation * sizeof(dlVectorKey*) );
         dlFree( oldNode->rotation,
                 node->num_rotation    * sizeof(dlQuatKey*)   );
         dlFree( oldNode->scaling,
                 node->num_scaling     * sizeof(dlVectorKey*) );

         node = node->next;

         /* free node */
         nextOldNode = oldNode->next;
         dlFree( oldNode, sizeof(dlAnimTickOldNode) );
         oldNode = nextOldNode;
      }
      animTick->oldNode = NULL;
   }

   LOGFREE("FREE");

   /* free ticker */
   dlFree( animTick, sizeof(dlAnimTick) );
   animTick = NULL;

   RET("%d", RETURN_OK);
   return( RETURN_OK );
}

void dlAdvanceAnimTick( dlAnimTick *animTick, float pTime )
{
   dlAnim               *anim;
   dlNodeAnim           *node;
   dlAnimTickOldNode    *oldNode;
   unsigned int   frame, nextFrame;
   dlVectorKey    *vkey, *nextvKey;
   dlQuatKey      *qkey, *nextqKey;
   kmVec3         presentTranslation, presentScaling;
   kmQuaternion   presentRotation;
   float          diffTime, factor;
   float          ticksPerSecond;

   CALL("%p, %f", animTick, pTime);

   /* get dlAnim */
   anim = animTick->anim;

   ticksPerSecond = anim->ticksPerSecond != 0.0 ? anim->ticksPerSecond : 25.0f;
   pTime *= ticksPerSecond;

   /* map into anim's duration */
   float time = 0.0f;
   if( anim->duration > 0.0)
      time = fmod( pTime, anim->duration);

   /* calculate the transformations for each animation channel */
   node     = anim->node;
   oldNode  = animTick->oldNode;
   while(node && oldNode)
   {
      /* ******** Position **** */
      presentTranslation.x = 0;
      presentTranslation.y = 0;
      presentTranslation.z = 0;

      if(node->translation)
      {
         frame = (time >= animTick->oldTime) ? oldNode->translationTime : 0;
         while( frame < node->num_translation - 1)
         {
            if( time < oldNode->translation[frame+1]->time)
               break;
            ++frame;
         }

         /* interpolate between this frame's value and next frame's value */
         nextFrame   = (frame + 1) % node->num_translation;
         vkey        = oldNode->translation[frame];
         nextvKey    = oldNode->translation[nextFrame];
         diffTime    = nextvKey->time - vkey->time;
#if 1
         if( diffTime < 0.0)
            diffTime += anim->duration;
         if( diffTime > 0)
         {
            factor            = (time - vkey->time) / diffTime;
            presentTranslation.x = vkey->value.x + (nextvKey->value.x - vkey->value.x) * factor;
            presentTranslation.y = vkey->value.y + (nextvKey->value.y - vkey->value.y) * factor;
            presentTranslation.z = vkey->value.z + (nextvKey->value.z - vkey->value.z) * factor;
         } else
         {
            presentTranslation = vkey->value;
         }
#else
         presentTranslation = vkey->value;
#endif

         oldNode->translationTime = frame;
      }

      /* ******** Rotation ******** */
      presentRotation.w = 0;
      presentRotation.x = 0;
      presentRotation.y = 0;
      presentRotation.z = 0;

      if(node->rotation)
      {
         frame = (time >= animTick->oldTime) ? oldNode->rotationTime : 0;
         while( frame < node->num_rotation - 1)
         {
            if( time < oldNode->rotation[frame+1]->time)
               break;
            ++frame;
         }

         /* interpolate between this frame's value and next frame's value */
         nextFrame   = (frame + 1) % node->num_rotation;
         qkey        = oldNode->rotation[frame];
         nextqKey    = oldNode->rotation[nextFrame];
         diffTime    = nextqKey->time - qkey->time;

#if 1
         if( diffTime < 0.0f)
            diffTime += anim->duration;
         if( diffTime > 0.0f)
         {
            factor   = (time - qkey->time) / diffTime;
            kmQuaternionSlerp( &presentRotation,
                  &qkey->value,
                  &nextqKey->value,
                  factor);
         } else
         {
            presentRotation = qkey->value;
         }
#else
         presentRotation = qkey->value;
#endif

         oldNode->rotationTime = frame;
      }

      /* ******** Scaling ********** */
      presentScaling.x = 1;
      presentScaling.y = 1;
      presentScaling.z = 1;

      if(node->scaling)
      {
         frame = (time >= animTick->oldTime) ? oldNode->scalingTime : 0;
         while( frame < node->num_scaling - 1)
         {
            if( time < oldNode->scaling[frame+1]->time)
               break;
            ++frame;
         }

         /* TODO: (thom) interpolation maybe? This time maybe even logarithmic, not linear */
         presentScaling        = oldNode->scaling[frame]->value;
         oldNode->scalingTime  = frame;
      }

      // build a transformation matrix from it
      kmMat4 *mat = &node->bone->relativeMatrix;
      kmMat4RotationQuaternion( mat, &presentRotation );

      mat->mat[0] *= presentScaling.x; mat->mat[4] *= presentScaling.x; mat->mat[8] *= presentScaling.x;
      mat->mat[1] *= presentScaling.y; mat->mat[5] *= presentScaling.y; mat->mat[9] *= presentScaling.y;
      mat->mat[2] *= presentScaling.z; mat->mat[6] *= presentScaling.z; mat->mat[10] *= presentScaling.z;
      mat->mat[3] = presentTranslation.x; mat->mat[7] = presentTranslation.y; mat->mat[11] = presentTranslation.z;

      node      = node->next;
      oldNode   = oldNode->next;
   }

   /* old time */
   animTick->oldTime = time;
}
