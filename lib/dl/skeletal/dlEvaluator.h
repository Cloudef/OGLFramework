#ifndef DL_ANIM_EVALUATOR_H
#define DL_ANIM_EVALUATOR_H

#include "dlAnim.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct dlAnimTickOldNode_t
{
   /* old times */
   float translationTime, rotationTime, scalingTime;

   /* pointer to keys */
   dlVectorKey  **translation;
   dlQuatKey    **rotation;
   dlVectorKey  **scaling;

   struct dlAnimTickOldNode_t *next;
} dlAnimTickOldNode;

/* struct to store current animation and last state */
typedef struct dlAnimTick_t
{
   /* animation and history */
   dlAnim *anim;
   dlAnimTickOldNode   *oldNode;

   /* old time */
   float oldTime;
} dlAnimTick;

/* allocate new animation handler */
dlAnimTick* dlNewAnimTick( dlAnim* );

/* free animation handler */
int dlFreeAnimTick( dlAnimTick* );

/* advance animation */
void dlAdvanceAnimTick( dlAnimTick*, float );

#ifdef __cplusplus
}
#endif

#endif /* DL_ANIM_EVALUATOR_H */
