#ifndef GL_ANIM_H
#define GL_ANIM_H

#include <stdint.h>

#include "../config.h"
#include "kazmath/kazmath.h"

#ifdef __cplusplus
extern "C" {
#endif

/* translation key */
typedef struct glVectorKey_t
{
   float time;
   kmVec3 value;
} glVectorKey;

/* rotation key */
typedef struct glQuactKey_t
{
   float time;
   kmQuaternion value;
} glQuatKey;

/* animation node */
typedef struct glNodeAnim_t
{
   /* translation keys */
   glVectorKey *positionKey;
   GL_NODE_TYPE num_position;

   /* rotation keys */
   glQuatKey *rotationKey;
   GL_NODE_TYPE num_rotation;

   /* scaling keys */
   glVectorKey *scaleKey;
   GL_NODE_TYPE num_scale;

} glNodeAnim;

/* animation */
typedef struct glAnim_t
{
   /* strdupped */
   char *name;

   glNodeAnim *skeletal;
   GL_NODE_TYPE num_skeletal;

#if USE_KEYFRAME_ANIMATION
   glMeshAnim *keyFrame
   GL_NODE_TYPE num_keyframe;
#endif

   /* ticks && duration */
   float ticksPerSecond;
   float duration;

   unsigned int refCounter;
} glAnim;

glAnim* glNewAnim(void);
glAnim* glRefAnim(glAnim*);
int     glFreeAnim(glAnim*);

#ifdef __cplusplus
}
#endif

#endif /* GL_ANIM_H */