#ifndef GL_BONE_H
#define GL_BONE_H

#include <stdint.h>
#include "kazmath/kazmath.h"
#include "../config.h"

/* weight */
typedef struct
{
   unsigned int vertex;
   float weight;
} glVertexWeight;

/* bone */
typedef struct
{
   char *name;

   glVertexWeight *weight;
   GL_NODE_TYPE num_weights;
   kmMat4 offsetMatrix;

   unsigned int refCounter;
} glBone;

glBone* glNewBone(void);
glBone* glRefBone( glBone* );
int glFreeBone( glBone* );

#endif /* GL_BONE_H */
