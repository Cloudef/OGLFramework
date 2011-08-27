#ifndef GL_ANIM_H
#define GL_ANIM_H

#include <stdint.h>

#include "../config.h"
#include "kazmath/kazmath.h"
#include "bone.h"

#ifdef __cplusplus
extern "C" {
#endif

/* translation key */
typedef struct glVectorKey_t
{
   float time;
   kmVec3 value;
   struct glVectorKey_t *next;
} glVectorKey;

/* rotation key */
typedef struct glQuactKey_t
{
   float time;
   kmQuaternion value;
   struct glQuactKey_t *next;
} glQuatKey;

/* animation node */
typedef struct glNodeAnim_t
{
   /* animation node keys */
   glVectorKey *translation;
   glQuatKey   *rotation;
   glVectorKey *scaling;

   /* counts */
   GL_NODE_TYPE num_translation;
   GL_NODE_TYPE num_rotation;
   GL_NODE_TYPE num_scaling;

   /* affected bone */
   glBone *bone;

   struct glNodeAnim_t *next;
} glNodeAnim;

/* animation */
typedef struct glAnim_t
{
   /* strdupped */
   char *name;

   /* animation node */
   glNodeAnim *node;

   /* ticks && duration */
   float ticksPerSecond;
   float duration;

   /* next animation */
   struct glAnim_t *next;

   /* reference counted */
   unsigned int refCounter;
} glAnim;

glAnim* glNewAnim(void);
glAnim* glRefAnim(glAnim*);
int     glFreeAnim(glAnim*);

glNodeAnim* glAnimAddNode( glAnim *node );

glVectorKey* glNodeAddTranslationKey(glNodeAnim*, kmVec3, float);
glQuatKey*   glNodeAddRotationKey(glNodeAnim*, kmQuaternion, float);
glVectorKey* glNodeAddScalingKey(glNodeAnim*, kmVec3, float);

#ifdef __cplusplus
}
#endif

#endif /* GL_ANIM_H */
