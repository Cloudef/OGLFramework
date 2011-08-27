#ifndef GL_ANIM_EVALUATOR_H
#define GL_ANIM_EVALUATOR_H

#include "anim.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct glAnimTickOldNode_t
{
   /* old times */
   float translationTime, rotationTime, scalingTime;

   /* pointer to keys */
   glVectorKey  **translation;
   glQuatKey    **rotation;
   glVectorKey  **scaling;

   struct glAnimTickOldNode_t *next;
} glAnimTickOldNode;

/* struct to store current animation and last state */
typedef struct glAnimTick_t
{
   /* animation and history */
   glAnim *anim;
   glAnimTickOldNode   *oldNode;

   /* old time */
   float oldTime;
} glAnimTick;

/* allocate new animation handler */
glAnimTick* glNewAnimTick( glAnim* );

/* free animation handler */
int glFreeAnimTick( glAnimTick* );

/* advance animation */
void glAdvanceAnimTick( glAnimTick*, float );

#ifdef __cplusplus
}
#endif

#endif /* GL_ANIM_EVALUATOR_H */
