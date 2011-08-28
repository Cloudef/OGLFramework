#ifndef GL_BONE_H
#define GL_BONE_H

#include <stdint.h>
#include "kazmath/kazmath.h"
#include "../config.h"

#ifdef __cplusplus
extern "C" {
#endif

/* weight */
typedef struct glVertexWeight_t
{
   unsigned int vertex;
   float value;
   struct glVertexWeight_t *next;
} glVertexWeight;

/* bone */
typedef struct glBone_t
{
   /* strdupped name */
   char *name;

   /* next && parent */
   struct glBone_t *parent;
   struct glBone_t *next;

   /* weights */
   glVertexWeight *weight;
   kmMat4         offsetMatrix;
   kmMat4         globalMatrix;
   kmMat4         relativeMatrix;

   unsigned int refCounter;
} glBone;

glBone* glNewBone(void);
glBone* glRefBone( glBone* );
int glFreeBone( glBone* );

int glBoneAddChild( glBone*, glBone* );
glVertexWeight *glBoneAddWeight( glBone*, unsigned int, float );

#ifdef __cplusplus
}
#endif

#endif /* GL_BONE_H */
