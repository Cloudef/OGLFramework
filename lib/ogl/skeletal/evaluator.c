#include "evaluator.h"
#include "alloc.h"
#include "types.h"

#include <assert.h>

glAnimTick* glNewAnimTick( glAnim *anim )
{
   glNodeAnim           *node;
   glAnimTickOldNode    **oldNode;

   glVectorKey          *vkey;
   glQuatKey            *qkey;
   GL_NODE_TYPE         count;

   if(!anim)
      return( NULL );

	/* Allocate animation handler object */
	glAnimTick *animTick = (glAnimTick*)glCalloc( 1, sizeof(glAnimTick) );
   if(!animTick)
      return( NULL );

   /* assign animation */
   animTick->anim          = anim;
   animTick->oldTime       = 0.0f;

   /* null */
   animTick->oldNode    = NULL;

   node      = anim->node;
   oldNode   = &animTick->oldNode;
   for(; node; node = node->next)
   {
      *oldNode = glCalloc( 1, sizeof(glAnimTickOldNode) );
      if(!*oldNode)
      { glFreeAnimTick( animTick ); return( NULL ); }

      /* store translations to pointer array */
      if(node->translation)
      {
         (*oldNode)->translation = glCalloc( node->num_translation, sizeof(glVectorKey*) );
         if(!(*oldNode)->translation)
         { glFreeAnimTick( animTick ); return( NULL ); }

         vkey = node->translation; count = 0;
         for(; vkey; vkey = vkey->next)
            (*oldNode)->translation[count++] = vkey;
      }

      /* store rortations to pointer array */
      if(node->rotation)
      {
         (*oldNode)->rotation = glCalloc( node->num_rotation, sizeof(glQuatKey*) );
         if(!(*oldNode)->rotation)
         { glFreeAnimTick( animTick ); return( NULL ); }

         qkey = node->rotation; count = 0;
         for(; qkey; qkey = qkey->next)
            (*oldNode)->rotation[count++] = qkey;
      }

      /* store scalings to pointer array */
      if(node->scaling)
      {
         (*oldNode)->scaling = glCalloc( node->num_scaling, sizeof(glVectorKey*) );
         if(!(*oldNode)->scaling)
         { glFreeAnimTick( animTick ); return( NULL ); }

         vkey = node->scaling; count = 0;
         for(; vkey; vkey = vkey->next)
            (*oldNode)->scaling[count++] = vkey;
      }

      oldNode = &(*oldNode)->next;
   }

   /* no animations, pointless */
   if(!anim->node)
   { glFreeAnimTick( animTick ); return( NULL ); }

   /* return */
   return( animTick );
}

int glFreeAnimTick( glAnimTick *animTick )
{
   glAnimTickOldNode    *oldNode, *nextOldNode;
   glNodeAnim           *node;

   /* invalid object */
   if(!animTick)
      return( RETURN_NOTHING );

   /* we should have animation */
   if(animTick->anim)
   {

      /* free nodes */
      oldNode = animTick->oldNode;
      node    = animTick->anim->node;
      while(oldNode)
      {
         /* free stored pointers */
         glFree( oldNode->translation,
                 node->num_translation * sizeof(glVectorKey*) );
         glFree( oldNode->rotation,
                 node->num_rotation    * sizeof(glQuatKey*)   );
         glFree( oldNode->scaling,
                 node->num_scaling     * sizeof(glVectorKey*) );

         node = node->next;

         /* free node */
         nextOldNode = oldNode->next;
         glFree( oldNode, sizeof(glAnimTickOldNode) );
         oldNode = nextOldNode;
      }
      animTick->oldNode = NULL;
   }

   /* free ticker */
   glFree( animTick, sizeof(glAnimTick) );
   animTick = NULL;
   return( RETURN_OK );
}

void glAdvanceAnimTick( glAnimTick *animTick, float pTime )
{
   glAnim               *anim;
   glNodeAnim           *node;
   glAnimTickOldNode    *oldNode;
   unsigned int   frame, nextFrame;
   glVectorKey    *vkey, *nextvKey;
   glQuatKey      *qkey, *nextqKey;
   kmVec3         presentTranslation, presentScaling;
   kmQuaternion   presentRotation;
   float          diffTime, factor;
   float          ticksPerSecond;

   /* get glAnim */
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
			if( diffTime < 0.0)
				diffTime += anim->duration;
			if( diffTime > 0)
			{
			   factor            = (time - vkey->time) / diffTime;
				presentTranslation.x = vkey->value.x + (nextvKey->value.x - vkey->value.x) * factor;
				presentTranslation.y = vkey->value.y + (nextvKey->value.y - vkey->value.z) * factor;
            presentTranslation.z = vkey->value.z + (nextvKey->value.z - vkey->value.z) * factor;
			} else
			{
				presentTranslation = vkey->value;
			}

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
			if( diffTime < 0.0f)
				diffTime += anim->duration;
			if( diffTime > 0.0f)
			{
				factor   = (time - qkey->time) / diffTime;
            kmQuaternionSlerp( &presentRotation,
								&qkey->value,
								&nextqKey->value,
								factor);
            				presentRotation = qkey->value;
			} else
			{
				presentRotation = qkey->value;
			}

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
		kmMat4 *mat = &node->bone->transformMatrix;
      *mat = node->bone->relativeMatrix;

      mat->mat[0] = 1.0f - 2.0f * (presentRotation.y * presentRotation.y +
                                   presentRotation.z * presentRotation.z);
      mat->mat[1] = 2.0f        * (presentRotation.x * presentRotation.y -
                                   presentRotation.z * presentRotation.w);
      mat->mat[2] = 2.0f        * (presentRotation.x * presentRotation.z +
                                   presentRotation.y * presentRotation.w);

      mat->mat[4] = 2.0f        * (presentRotation.x * presentRotation.y +
                                   presentRotation.z * presentRotation.w);
      mat->mat[5] = 1.0f - 2.0f * (presentRotation.x * presentRotation.x +
                                   presentRotation.z * presentRotation.z);
      mat->mat[6] = 2.0f        * (presentRotation.y * presentRotation.z -
                                   presentRotation.x * presentRotation.w);

      mat->mat[8] = 2.0f        * (presentRotation.x * presentRotation.z -
                                   presentRotation.y * presentRotation.w);
      mat->mat[9] = 2.0f        * (presentRotation.y * presentRotation.z +
                                   presentRotation.x * presentRotation.w);
      mat->mat[10]= 1.0f - 2.0f * (presentRotation.x * presentRotation.x +
                                   presentRotation.y * presentRotation.y);


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
