#ifndef DL_BONE_H
#define DL_BONE_H

#include <stdint.h>
#include "kazmath/kazmath.h"
#include "../dlConfig.h"

#ifdef __cplusplus
extern "C" {
#endif

/* weight */
typedef struct dlVertexWeight_t
{
   unsigned int vertex;
   float value;
   struct dlVertexWeight_t *next;
} dlVertexWeight;

/* bone */
typedef struct dlBone_t
{
   /* strdupped name */
   char *name;

   /* next && parent */
   struct dlBone_t *parent;
   struct dlBone_t *next;

   /* weights */
   dlVertexWeight *weight;
   kmMat4         offsetMatrix;
   kmMat4         globalMatrix;
   kmMat4         relativeMatrix;

   unsigned int refCounter;
} dlBone;

dlBone* dlNewBone(void);
dlBone* dlRefBone( dlBone* );
int dlFreeBone( dlBone* );

int dlBoneAddChild( dlBone*, dlBone* );
dlVertexWeight *dlBoneAddWeight( dlBone*, unsigned int, float );

#ifdef __cplusplus
}
#endif

#endif /* DL_BONE_H */
