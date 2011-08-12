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
   float weight;
} glVertexWeight;

/* bone */
typedef struct glBone_t
{
   char *name;

   glVertexWeight *weight;
   unsigned int num_weights;
   kmMat4 offsetMatrix;

   unsigned int refCounter;
} glBone;

glBone* glNewBone(void);
glBone* glRefBone( glBone* );
int glFreeBone( glBone* );

#ifdef __cplusplus
}
#endif

#endif /* GL_BONE_H */
