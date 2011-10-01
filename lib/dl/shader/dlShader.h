#ifndef DL_SHADER_H
#define DL_SHADER_H

typedef struct dlShaderType_t
{
   const char     *value;
   unsigned int   dlType;
} dlShaderType;

typedef struct dlShaderUniform_t
{
   unsigned int   object;
   const char     *name;
} dlShaderUniform;

typedef struct dlShader_t
{
   unsigned int      object;
   const char        *file;
   dlShaderUniform   *uniforms;
   unsigned int      uniformCount;
} dlShader;

dlShader* dlNewShader( const char *file );
int dlFreeShader( dlShader *shader );

void dlBindShader( dlShader *shader );

#endif /* DL_SHADER_H */
