#ifndef DL_ANIMATOR_H
#define DL_ANIMAOTR_H

#include "dlAnim.h"
#include "dlBone.h"
#include "dlEvaluator.h"
#include "../dlVbo.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct dlAnimator_t
{
   /* bones && anim */
   dlAnim      *anim;
   dlBone      *bone;
   dlAnimTick  *tick;

   /* current animation */
   dlAnim      *current;

   /* ref counter */
   unsigned int refCounter;
} dlAnimator;

dlAnimator* dlNewAnimator(void);
dlAnimator* dlCopyAnimator( dlAnimator* );
dlAnimator* dlRefAnimator( dlAnimator* );
int dlFreeAnimator( dlAnimator* );

dlAnim* dlAnimatorAddAnim( dlAnimator* );
dlAnim* dlAnimatorRefAnims( dlAnimator* );

dlBone* dlAnimatorAddBone( dlAnimator* );
dlBone* dlAnimatorRefBones( dlAnimator* );
dlBone* dlAnimatorGetBone( dlAnimator*, const char* );

void dlAnimatorTick( dlAnimator*, float );
void dlAnimatorUpdate( dlAnimator* );
void dlAnimatorSetAnim( dlAnimator*, DL_NODE_TYPE );
void dlAnimatorCalculateGlobalTransformations( dlAnimator* );

#ifdef __cplusplus
}
#endif

#endif /* DL_ANIMATOR_H */
