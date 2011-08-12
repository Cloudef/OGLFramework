#ifndef GL_ANIM_EVALUATOR_H
#define GL_ANIM_EVALUATOR_H

#include "anim.h"

#ifdef __cplusplus
extern "C" {
#endif

/* struct to store current animation and last state */
typedef struct glAnimTick_t
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

#ifdef __cplusplus
}
#endif

#endif /* GL_ANIM_EVALUATOR_H */
