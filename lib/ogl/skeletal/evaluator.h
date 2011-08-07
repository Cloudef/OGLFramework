#ifndef GL_ANIM_EVALUATOR_H
#define GL_ANIM_EVALUATOR_H

#include "anim.h"

/* struct to store current animation and last state */
typedef struct
{
   glAnim *mAnim;
   float   lastTime;
   kmVec3 *lastPosition;
   kmMat4 *transform;
} glAnimTick;

/* allocate new animation handler */
glAnimTick* glNewAnimTick( glAnim *mAnim );

/* free animation handler */
int glFreeAnimTick( glAnimTick *mAnimTick );

/* advance animation */
void glAdvanceAnimTick( glAnimTick *mAnimTick, float pTime );

#endif /* GL_ANIM_EVALUATOR_H */
