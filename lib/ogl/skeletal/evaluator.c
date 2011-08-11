#include "evaluator.h"
#include "alloc.h"
#include "types.h"

glAnimTick* glNewAnimTick( glAnim *mAnim )
{
   if(!mAnim)
      return( NULL );

	/* Allocate animation handler object */
	glAnimTick *mAnimTick = (glAnimTick*)glCalloc( 1, sizeof(glAnimTick) );
   if(!mAnimTick)
      return( NULL );

   /* assign animation */
   mAnimTick->mAnim           = mAnim;

   mAnimTick->lastPosition  = glCalloc( mAnim->num_skeletal, sizeof(kmVec3) );
   if(!mAnimTick->lastPosition)
   {
      glFree( mAnimTick, sizeof(glAnimTick) );
      return( NULL );
   }
   mAnimTick->transform     = glCalloc( mAnim->num_skeletal, sizeof(kmMat4) );
   if(!mAnimTick->transform)
   {
      glFree( mAnimTick->lastPosition, sizeof(kmVec3) * mAnim->num_skeletal );
      glFree( mAnimTick, sizeof(glAnimTick) );
      return( NULL );
   }

   /* return */
   return( mAnimTick );
}

int glFreeAnimTick( glAnimTick *mAnimTick )
{
   if(!mAnimTick)
      return( RETURN_NOTHING );

   glFree( mAnimTick->lastPosition, sizeof(kmVec3) * mAnimTick->mAnim->num_skeletal );
   mAnimTick->lastPosition = NULL;

   glFree( mAnimTick->transform, sizeof(kmMat4) * mAnimTick->mAnim->num_skeletal );
   mAnimTick->transform = NULL;

   glFree( mAnimTick, sizeof(glAnimTick) );
   mAnimTick = NULL;

   return( RETURN_OK );
}

void glAdvanceAnimTick( glAnimTick *mAnimTick, float pTime )
{
   glAnim         *mAnim;
   unsigned int   a, frame, nextFrame;
   glVectorKey    *vkey, *nextvKey;
   glQuatKey      *qkey, *nextqKey;
   kmVec3         presentPosition, presentScaling;
   kmQuaternion   presentRotation;
   float          diffTime, factor;
   glNodeAnim     *channel;

   /* get glAnim */
   mAnim = mAnimTick->mAnim;

	float ticksPerSecond = mAnim->ticksPerSecond != 0.0 ? mAnim->ticksPerSecond : 25.0;
	pTime *= ticksPerSecond;

	/* map into anim's duration */
	float time = 0.0f;
	if( mAnim->duration > 0.0)
		time = fmod( pTime, mAnim->duration);

	/* calculate the transformations for each animation channel */
	a = 0;
   for(; a < mAnim->num_skeletal; ++a)
	{
		channel = &mAnim->skeletal[a];

		/* ******** Position **** */
      presentPosition.x = 0;
      presentPosition.y = 0;
      presentPosition.z = 0;

		if( channel->num_position > 0)
		{
			frame = (time >= mAnimTick->lastTime) ? mAnimTick->lastPosition[a].x : 0;
			while( frame < channel->num_position - 1)
			{
				if( time < channel->positionKey[frame+1].time)
					break;
				++frame;
			}

			/* interpolate between this frame's value and next frame's value */
			nextFrame   = (frame + 1) % channel->num_position;
			vkey        = &channel->positionKey[frame];
		   nextvKey    = &channel->positionKey[nextFrame];
			diffTime    = nextvKey->time - vkey->time;
			if( diffTime < 0.0)
				diffTime += mAnim->duration;
			if( diffTime > 0)
			{
			   factor            = (time - vkey->time) / diffTime;
				presentPosition.x = vkey->value.x + (nextvKey->value.x - vkey->value.x) * factor;
				presentPosition.y = vkey->value.y + (nextvKey->value.y - vkey->value.z) * factor;
            presentPosition.z = vkey->value.z + (nextvKey->value.z - vkey->value.z) * factor;
			} else
			{
				presentPosition = vkey->value;
			}

			mAnimTick->lastPosition[a].x = frame;
		}

		/* ******** Rotation ******** */
      presentRotation.w = 1;
      presentRotation.x = 0;
      presentRotation.y = 0;
      presentRotation.z = 0;

		if( channel->num_rotation > 0)
		{
			frame = (time >= mAnimTick->lastTime) ? mAnimTick->lastPosition[a].y : 0;
			while( frame < channel->num_rotation - 1)
			{
				if( time < channel->rotationKey[frame+1].time)
					break;
				++frame;
			}

			/* interpolate between this frame's value and next frame's value */
			nextFrame   = (frame + 1) % channel->num_rotation;
			qkey        = &channel->rotationKey[frame];
			nextqKey    = &channel->rotationKey[nextFrame];
			diffTime    = nextqKey->time - qkey->time;
			if( diffTime < 0.0)
				diffTime += mAnim->duration;
			if( diffTime > 0)
			{
				factor   = (time - qkey->time) / diffTime;
				/* aiQuaternion::Interpolate( presentRotation, key.value, nextKey.value, factor); */
			} else
			{
				presentRotation = qkey->value;
			}

			mAnimTick->lastPosition[a].y = frame;
		}

		/* ******** Scaling ********** */
		presentScaling.x = 1;
      presentScaling.y = 1;
      presentScaling.z = 1;

		if( channel->num_scale > 0)
		{
			frame = (time >= mAnimTick->lastTime) ? mAnimTick->lastPosition[a].z : 0;
			while( frame < channel->num_scale - 1)
			{
				if( time < channel->scaleKey[frame+1].time)
					break;
				++frame;
			}

			/* TODO: (thom) interpolation maybe? This time maybe even logarithmic, not linear */
			presentScaling                = channel->scaleKey[frame].value;
			mAnimTick->lastPosition[a].z  = frame;
		}

		// build a transformation matrix from it
		kmMat4 mat = mAnimTick->transform[a];
	   kmMat4RotationQuaternion( &mat, &presentRotation );
		mat.mat[0] *= presentScaling.x; mat.mat[5] *= presentScaling.x; mat.mat[9] *= presentScaling.x;
		mat.mat[1] *= presentScaling.y; mat.mat[6] *= presentScaling.y; mat.mat[10] *= presentScaling.y;
		mat.mat[3] *= presentScaling.z; mat.mat[7] *= presentScaling.z; mat.mat[11] *= presentScaling.z;
		mat.mat[4] = presentPosition.x; mat.mat[8] = presentPosition.y; mat.mat[12] = presentPosition.z;
	}

	mAnimTick->lastTime = time;
}
