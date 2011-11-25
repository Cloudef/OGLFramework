#ifndef DL_ANIM_H
#define DL_ANIM_H

#include <stdint.h>

#include "../dlConfig.h"
#include "kazmath/kazmath.h"
#include "dlBone.h"

#ifdef __cplusplus
extern "C" {
#endif

/* translation key */
typedef struct dlVectorKey_t
{
   float time;
   kmVec3 value;
   struct dlVectorKey_t *next;
} dlVectorKey;

/* rotation key */
typedef struct dlQuactKey_t
{
   float time;
   kmQuaternion value;
   struct dlQuactKey_t *next;
} dlQuatKey;

/* animation node */
typedef struct dlNodeAnim_t
{
   /* animation node keys */
   dlVectorKey *translation;
   dlQuatKey   *rotation;
   dlVectorKey *scaling;

   /* counts */
   DL_NODE_TYPE num_translation;
   DL_NODE_TYPE num_rotation;
   DL_NODE_TYPE num_scaling;

   /* affected bone */
   dlBone *bone;

   struct dlNodeAnim_t *next;
} dlNodeAnim;

/* animation */
typedef struct dlAnim_t
{
   /* strdupped */
   char *name;

   /* animation node */
   dlNodeAnim *node;

   /* ticks && duration */
   float ticksPerSecond;
   float duration;

   /* next animation */
   struct dlAnim_t *next;

   /* reference counted */
   unsigned int refCounter;
} dlAnim;

dlAnim* dlNewAnim(void);
dlAnim* dlRefAnim(dlAnim*);
int     dlFreeAnim(dlAnim*);

dlNodeAnim* dlAnimAddNode( dlAnim *node );

dlVectorKey* dlNodeAddTranslationKey(dlNodeAnim*, kmVec3*, float);
dlQuatKey*   dlNodeAddRotationKey(dlNodeAnim*, kmQuaternion*, float);
dlVectorKey* dlNodeAddScalingKey(dlNodeAnim*, kmVec3*, float);

#ifdef __cplusplus
}
#endif

#endif /* DL_ANIM_H */
