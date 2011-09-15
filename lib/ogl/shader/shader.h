#ifndef GL_SHADER_H
#define GL_SHADER_H

typedef struct glShaderType_t
{
   char           *value;
   unsigned int   glType;
} glShaderType;

typedef struct glShaderUniform_t
{
   unsigned int   object;
   const char     *name;
} glShaderUniform;

typedef struct glShader_t
{
   unsigned int object;
   char              *file;
   glShaderUniform   *uniforms;
   unsigned int      uniformCount;
} glShader;

glShader* glNewShader( const char *file );
int glFreeShader( glShader *shader );

#endif /* GL_SHADER_H */
