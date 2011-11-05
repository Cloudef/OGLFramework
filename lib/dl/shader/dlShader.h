#ifndef DL_SHADER_H
#define DL_SHADER_H

#include "kazmath/kazmath.h"

#define DL_VERTEX_ATTRIB 0
#define DL_COORD_ATTRIB  1
#define DL_NORMAL_ATTRIB 2
#define DL_COLOR_ATTRIB  3

#ifdef __cplusplus
extern "C" {
#endif

typedef struct dlShaderType_t
{
   const char     *value;
   unsigned int   dlType;
} dlShaderType;

typedef struct dlShaderUniform_t
{
   unsigned int      object;
   const char        *name;
   struct dlShader_t *shader;
} dlShaderUniform;

typedef struct dlShader_t
{
   unsigned int      object;
   const char        *file;
   dlShaderUniform   *uniforms;
   unsigned int      uniformCount;

   dlShaderUniform   *projection, *view;
} dlShader;

dlShader* dlNewShader( const char *file );
int dlFreeShader( dlShader *shader );

void dlBindShader( dlShader *shader );
void dlShaderUniformMatrix4( dlShaderUniform *uniform, kmMat4 *mat  );
dlShaderUniform* dlShaderGetUniform( dlShader *shader, const char *name );

#ifdef __cplusplus
}
#endif

#endif /* DL_SHADER_H */
