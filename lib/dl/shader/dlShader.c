#include <malloc.h>
#include <string.h>

#include "dlFramework.h"
#include "dlShader.h"
#include "dlAlloc.h"
#include "dlLog.h"
#include "dlTypes.h"
#include "dlConfig.h"

#ifdef __WIN32__
   #define LINE_MAX 1024
#else
   #include <limits.h>
#endif

#ifdef GLES1
#  include <GLES/gl.h>
#elif GLES2
#  include <GLES2/gl.h>
#else
#  include <GL/glew.h>
#  include <GL/gl.h>
#endif

#define DL_DEBUG_CHANNEL "SHADER"

#if SHADER_SUPPORT

#define MAX_SHADER_SIZE   1024 * 10 /* 10240 characters */

#define VERTEX_SHADER   "VERTEX_SHADER"
#define FRAGMENT_SHADER "FRAGMENT_SHADER"

#define DL_IN_VERTEX  "DL_IN_VERTEX"
#define DL_OUT_VERTEX "DL_OUT_VERTEX"
#define DL_IN_COORD   "DL_IN_COORD"
#define DL_OUT_COORD  "DL_OUT_COORD"
#define DL_IN_NORMAL  "DL_IN_NORMAL"
#define DL_OUT_NORMAL "DL_OUT_NORMAL"
#define DL_IN_COLOR   "DL_IN_COLOR"
#define DL_OUT_COLOR  "DL_OUT_COLOR"

#define DL_POSITION   "DL_POSITION"
#define DL_FRAGMENT   "DL_FRAGMENT"

#define DL_POSITION_CONV "gl_Position"
#define DL_FRAGMENT_CONV "gl_FragColor"

#define DL_PROJECTION "DL_PROJECTION"
#define DL_VIEW       "DL_VIEW"

#define DL_TEXTURE    "DL_TEXTURE"

static const dlShaderType uniformTypes[] =
{
   /* glsl type */      /* GL type */

   /* float && float vecs */
   { "float",           GL_FLOAT,               },
   { "vec2",            GL_FLOAT_VEC2           },
   { "vec3",            GL_FLOAT_VEC3           },
   { "vec4",            GL_FLOAT_VEC4           },

   /* double && double vecs */
   { "double",          GL_DOUBLE               },
   { "dvec2",           GL_DOUBLE_VEC2          },
   { "dvec3",           GL_DOUBLE_VEC3          },
   { "dvec4",           GL_DOUBLE_VEC4          },

   /* int && int vecs */
   { "int",             GL_INT                  },
   { "ivec2",           GL_INT_VEC2             },
   { "ivec3",           GL_INT_VEC3             },
   { "ivec4",           GL_INT_VEC4             },

   /* uint && uint vecs */
   { "unsigned int",    GL_UNSIGNED_INT         },
   { "uvec2",           GL_UNSIGNED_INT_VEC2    },
   { "uvec3",           GL_UNSIGNED_INT_VEC3    },
   { "uvec4",           GL_UNSIGNED_INT_VEC4    },

   /* bool && bool vecs */
   { "bool",            GL_BOOL                 },
   { "bvec2",           GL_BOOL_VEC2            },
   { "bvec3",           GL_BOOL_VEC3            },
   { "bvec4",           GL_BOOL_VEC4            },

   /* float matrices */
   { "mat2",            GL_FLOAT_MAT2           },
   { "mat3",            GL_FLOAT_MAT3           },
   { "mat4",            GL_FLOAT_MAT4           },
   { "mat2x3",          GL_FLOAT_MAT2x3         },
   { "mat2x4",          GL_FLOAT_MAT2x4         },
   { "mat3x2",          GL_FLOAT_MAT3x2         },
   { "mat3x4",          GL_FLOAT_MAT3x4         },
   { "mat4x2",          GL_FLOAT_MAT4x2         },
   { "mat4x3",          GL_FLOAT_MAT4x3         },

   /* double matrices */
   { "dmat2",           GL_DOUBLE_MAT2          },
   { "dmat3",           GL_DOUBLE_MAT3          },
   { "dmat4",           GL_DOUBLE_MAT4          },
   { "dmat2x3",         GL_DOUBLE_MAT2x3        },
   { "dmat2x4",         GL_DOUBLE_MAT2x4        },
   { "dmat3x2",         GL_DOUBLE_MAT3x2        },
   { "dmat3x4",         GL_DOUBLE_MAT3x4        },
   { "dmat4x2",         GL_DOUBLE_MAT4x2        },
   { "dmat4x3",         GL_DOUBLE_MAT4x3        },

   /* samplers */
   { "sampler1D",             GL_SAMPLER_1D                    },
   { "sampler2D",             GL_SAMPLER_2D                    },
   { "sampler3D",             GL_SAMPLER_3D                    },
   { "samplerCube",           GL_SAMPLER_CUBE                  },
   { "sampler1DShadow",       GL_SAMPLER_1D_SHADOW             },
   { "sampler2DShadow",       GL_SAMPLER_2D_SHADOW             },
   { "sampler1DArray",        GL_SAMPLER_1D_ARRAY              },
   { "sampler2DArray",        GL_SAMPLER_2D_ARRAY              },
   { "sampler1DArrayShadow",  GL_SAMPLER_1D_ARRAY_SHADOW       },
   { "sampler2DArrayShadow",  GL_SAMPLER_2D_ARRAY_SHADOW       },
   { "sampler2DMS",           GL_SAMPLER_2D_MULTISAMPLE        },
   { "sampler2DMSArray",      GL_SAMPLER_2D_MULTISAMPLE_ARRAY  },
   { "samplerCubeShadow",     GL_SAMPLER_CUBE_SHADOW           },
   { "samplerBuffer",         GL_SAMPLER_BUFFER                },
   { "sampler2DRect",         GL_SAMPLER_2D_RECT               },
   { "sampler2DRectShadow",   GL_SAMPLER_2D_RECT_SHADOW        },

   /* int samplers */
   { "isampler1D",            GL_INT_SAMPLER_1D                      },
   { "isampler2D",            GL_INT_SAMPLER_2D                      },
   { "isampler3D",            GL_INT_SAMPLER_3D                      },
   { "isamplerCube",          GL_INT_SAMPLER_CUBE                    },
   { "isampler1DArray",       GL_INT_SAMPLER_1D_ARRAY                },
   { "isampler2DArray",       GL_INT_SAMPLER_2D_ARRAY                },
   { "isampler2DMS",          GL_INT_SAMPLER_2D_MULTISAMPLE          },
   { "isampler2DMSArray",     GL_INT_SAMPLER_2D_MULTISAMPLE_ARRAY    },
   { "isamplerBuffer",        GL_INT_SAMPLER_BUFFER                  },
   { "isampler2DRect",        GL_INT_SAMPLER_2D_RECT                 },

   /* unsigned int samplers */
   { "usampler1D",            GL_UNSIGNED_INT_SAMPLER_1D                      },
   { "usampler2D",            GL_UNSIGNED_INT_SAMPLER_2D                      },
   { "usampler3D",            GL_UNSIGNED_INT_SAMPLER_3D                      },
   { "usamplerCube",          GL_UNSIGNED_INT_SAMPLER_CUBE                    },
   { "usampler1DArray",       GL_UNSIGNED_INT_SAMPLER_1D_ARRAY                },
   { "usampler2DArray",       GL_UNSIGNED_INT_SAMPLER_2D_ARRAY                },
   { "usampler2DMS",          GL_UNSIGNED_INT_SAMPLER_2D_MULTISAMPLE          },
   { "usampler2DMSArray",     GL_UNSIGNED_INT_SAMPLER_2D_MULTISAMPLE_ARRAY    },
   { "usamplerBuffer",        GL_UNSIGNED_INT_SAMPLER_BUFFER                  },
   { "usampler2DRect",        GL_UNSIGNED_INT_SAMPLER_2D_RECT                 },

   { NULL, 0 } /* END */
};

/* reads file */
static char* getFile( const char *file )
{
   FILE *f;
   size_t size;
   char *data;
   CALL("%s", file);

   f = fopen( file, "r" );
   if(!f) return( NULL );

   fseek(f, 0, SEEK_END); size = ftell(f);
   fseek(f, 0, SEEK_SET);

   if(size > MAX_SHADER_SIZE)
   {
      fclose(f);
      LOGERRP("Shader file is too big [ %llu > %llu ]", size, MAX_SHADER_SIZE);

      RET("%p", NULL);
      return( NULL );
   }

   data = malloc( size );
   fread(data, size, 1, f); fclose(f);

   data[ size-1 ] = '\0';

   RET("%s", data);
   return( data );
}

/*! appends char array to char array, if str == NULL, allocates new char array.
 * str == char array which to append, str2 == char array that gets appended */
static char* append( char *str, char *str2 )
{
   char *data = NULL, *ostr = NULL;
   CALL("%s, %s", str, str2);

   if(!str2) { RET("%s", str); return( str ); }
   if(str)
   { ostr = strdup(str); data = realloc( str, strlen(str) + strlen(str2) + 1 ); }
   if(!data)
   {
      if(str) data = malloc( strlen(str) + strlen(str2) + 1 );
      else    data = malloc( strlen(str2) + 1 );
      if(!data) { RET("%s", str);  return( str ); }

      if(str)
         strcpy( data, str );
   }
   str = data;

   if(ostr)
   { strcpy( str + strlen(ostr), str2 ); free(ostr); }
   else
      strcpy( str, str2 );

   RET("%s", str);
   return( str );
}

/*! replaces string in string */
static char *str_replace(const char *s, const char *old, const char *new)
{
  size_t slen = strlen(s)+1;
  char *cout=0, *p=0, *tmp=NULL;
  CALL("%s, %s, %s", s, old, new);

  cout=malloc(slen); p=cout;
  if( !p )
  { RET("%p", NULL); return( NULL ); }
  while( *s )
    if( !strncmp(s, old, strlen(old)) )
    {
      p  -= (intptr_t)cout;
      cout= realloc(cout, slen += strlen(new)-strlen(old) );
      tmp = strcpy(p=cout+(intptr_t)p, new);
      p  += strlen(tmp);
      s  += strlen(old);
    }
    else
     *p++=*s++;

  *p=0;
  RET("%s", cout);
  return( cout );
}

/* link shader program */
static int link_shader( dlShader *shader )
{
   GLint    status = 0, logLen = 0;
   GLsizei  length = 0;
   char     *log;
   CALL("%p", shader);

   glLinkProgram( shader->object );
   glGetProgramiv( shader->object, GL_LINK_STATUS, &status );
   if(!status)
   {
      LOGERR("Program failed to link");
      glGetProgramiv( shader->object, GL_INFO_LOG_LENGTH, &logLen );

      log = malloc( logLen );
      glGetProgramInfoLog( shader->object, logLen, &length, log );
      logWhite(); dlPuts( log ); logNormal(); /* prints error */
      free( log );

      RET("%d", RETURN_FAIL);
      return( RETURN_FAIL );
   }

   RET("%d", RETURN_OK);
   return( RETURN_OK );
}

/* process shader */
static int process_shader( dlShader *shader )
{
   unsigned int i;
   GLint uniformCount = 0, maxLen = 0, size = 0;
   GLenum type;
   char *buffer;
   CALL("%p", shader);

   glGetProgramiv( shader->object, GL_ACTIVE_UNIFORMS, &uniformCount );
   glGetProgramiv( shader->object, GL_ACTIVE_UNIFORM_MAX_LENGTH, &maxLen );

   if( !uniformCount || !maxLen )
   {
      LOGERR("Failed to retieve uniform information");

      RET("%d", RETURN_FAIL);
      return( RETURN_FAIL );
   }

   shader->uniformCount = (unsigned int)uniformCount;
   shader->uniforms     = dlCalloc( uniformCount, sizeof( dlShaderUniform ) );
   if(!shader->uniforms)
   {
      LOGERR("Failed to alloc uniform list");

      RET("%d", RETURN_FAIL);
      return( RETURN_FAIL );
   }

   buffer = malloc(maxLen);
   if(!buffer)
   {
      LOGERR("Failed to alloc uniform name buffer");

      RET("%d", RETURN_FAIL);
      return( RETURN_FAIL );
   }

   i = 0;
   for( ; i != uniformCount; ++i )
   {
      glGetActiveUniform( shader->object, i, maxLen, 0, &size, &type, buffer );
      dlPuts( buffer );
      shader->uniforms[i].name   = strdup(buffer);
      shader->uniforms[i].object = glGetUniformLocation( shader->object, buffer );
      shader->uniforms[i].shader = shader;
   }
   free(buffer);

   /* Assing attribute locations */
   glBindAttribLocation(shader->object, DL_VERTEX_ATTRIB, DL_IN_VERTEX);
   glBindAttribLocation(shader->object, DL_COORD_ATTRIB , DL_IN_COORD);
   glBindAttribLocation(shader->object, DL_NORMAL_ATTRIB, DL_IN_NORMAL);

   /* Assing projection && view */
   shader->projection = dlShaderGetUniform( shader, DL_PROJECTION );
   shader->view       = dlShaderGetUniform( shader, DL_VIEW );

   RET("%d", RETURN_OK);
   return( RETURN_OK );
}

static unsigned int process_vertex_GLSL( char *data )
{
   char *data2 = NULL, *tmp;
   unsigned int vertexShader;
   CALL("%s", data);

   /* vertex shader */
   data2 = append( data2, "#define "VERTEX_SHADER" 1\n" );
   /* if shader->state.projection
    * { */
   data2 = append( data2, "uniform mat4 "DL_PROJECTION";\n" );
   data2 = append( data2, "uniform mat4 "DL_VIEW";\n" );
   /* } */
   /* if shader->state.vertex
    * { */
   data2 = append( data2, "in vec3 "DL_IN_VERTEX";\n" );
   /* } */
   /* if shader->state.texture
    * {
    * data2 = append( data2, "in vec3 "DL_IN_COORD";\n" );
    * data2 = append( data2, "out vec3 "DL_OUT_COORD";\n" );
    * } */

   /* SHADER CODE */
   data2 = append( data2, data );

   tmp = str_replace( data2, DL_POSITION, DL_POSITION_CONV );
   if(tmp)
   { free(data2); data2 = tmp; }

   vertexShader = glCreateShader(GL_VERTEX_SHADER);
   glShaderSource(vertexShader, 1, (const GLchar**)&data2, NULL);
   glCompileShader(vertexShader);
   free(data2); data2 = NULL;

   RET("%u", vertexShader);
   return( vertexShader );
}

static unsigned int process_fragment_GLSL( char *data )
{
   char *data2 = NULL, *tmp;
   unsigned int fragmentShader;
   CALL("%s", data);

   /* fragment shader */
   data2 = append( data2, "#define "FRAGMENT_SHADER" 1\n" );

   /* if shader->state.texture
    * {
    * data2 = append( data2, "in vec3 "DL_IN_COORD";\n" );
    * data2 = append( data2, "uniform sampler2D "DL_TEXTURE"0;\n" );
    * } */

   /* SHADER CODE */
   data2 = append( data2, data );

   tmp = str_replace( data2, DL_FRAGMENT, DL_FRAGMENT_CONV );
   if(tmp)
   { free(data2); data2 = tmp; }

   fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
   glShaderSource(fragmentShader, 1, (const GLchar**)&data2, NULL);
   glCompileShader(fragmentShader);
   free(data2); data2 = NULL;

   RET("%u", fragmentShader);
   return( fragmentShader );
}

/* allocate new shader */
dlShader* dlNewShader( const char *file )
{
   dlShader *shader;
   char *data = NULL;
   unsigned int vertexShader, fragmentShader;
   CALL("%s", file);

   data = getFile( file );
   if(!data) { RET("%p", NULL); return( NULL ); }

   /* process GLSL code */
   vertexShader   = process_vertex_GLSL( data );
   fragmentShader = process_fragment_GLSL( data );

   /* free data */
   free(data);

   /* allocate shader */
   dlSetAlloc( ALLOC_SHADER );
   shader = dlCalloc( 1, sizeof( dlShader ) );
   if(!shader)
   {
      glDeleteShader(vertexShader);
      glDeleteShader(fragmentShader);

      RET("%p", NULL);
      return( NULL );
   }
   shader->uniforms  = NULL;
   shader->projection= NULL;
   shader->view      = NULL;
   shader->file      = strdup(file);

   /* create shader program */
   shader->object = glCreateProgram();
   glAttachShader( shader->object, vertexShader   );
   glAttachShader( shader->object, fragmentShader );

   /* we don't need these anymore */
   //glDeleteShader(vertexShader);
   //glDeleteShader(fragmentShader);

   /* link shader */
   if(link_shader( shader ) != RETURN_OK)
   { dlFreeShader( shader ); RET("%p", NULL); return( NULL ); }

   /* get uniform amount */
   if(process_shader( shader ) != RETURN_OK)
   { dlFreeShader( shader ); RET("%p", NULL); return( NULL ); }

   LOGOK("NEW");

   RET("%p", shader);
   return( shader );
}

/* free shader */
int dlFreeShader( dlShader *shader )
{
   unsigned int i;
   CALL("%p", shader);

   if(!shader)
   { RET("%d", RETURN_NOTHING); return( RETURN_NOTHING ); }

   dlSetAlloc( ALLOC_SHADER );

   /* free uniform list */
   if( shader->uniforms )
   {
      i = 0;
      for(; i != shader->uniformCount; ++i)
         if(shader->uniforms[i].name) free( (void*)shader->uniforms[i].name );

      dlFree( shader->uniforms,
      sizeof( dlShaderUniform ) * shader->uniformCount );
   }
   free( (void*)shader->file );

   /* delete program */
   glDeleteProgram( shader->object );

   LOGFREE("FREE");

   /* free shader */
   dlFree( shader, sizeof( dlShader ) );

   RET("%d", RETURN_OK);
   return( RETURN_OK );
}

/* bind shader */
static GLuint _DL_BIND_SHADER = 0;
void dlBindShader( dlShader *shader )
{
   CALL("%p", shader);

   if(!shader && _DL_BIND_SHADER)
   {
      glUseProgram( 0 ); _DL_BIND_SHADER = 0;
      return;
   }

   if( _DL_BIND_SHADER == shader->object )
      return;

   dlSetShader( shader );
   glUseProgram( shader->object );
   _DL_BIND_SHADER = shader->object;
}

/* bind using ID */
void dlBindShaderi( GLuint shader )
{
   CALL("%u", shader);

   if( _DL_BIND_SHADER == shader )
      return;

   glUseProgram( shader );
   _DL_BIND_SHADER = shader;
}

/* set shader uniform */
void dlShaderUniformMatrix4( dlShaderUniform *uniform, kmMat4 *mat  )
{
   CALL("%p, %p", uniform, mat);

   if(!uniform) return;
   glUniformMatrix4fv( uniform->object, 1, GL_FALSE, &mat->mat[0] );
}

/* get uniform */
dlShaderUniform* dlShaderGetUniform( dlShader *shader, const char *name )
{
   unsigned int i = 0;
   CALL("%p, %s", shader, name);

   for(; i != shader->uniformCount; ++i)
   {
      if(strcmp( name, shader->uniforms[i].name) == 0)
      { RET("%p", &shader->uniforms[i]); return( &shader->uniforms[i] ); }
   }

   RET("%p", NULL);
   return( NULL );
}

#else  /* SHADER SUPPORT == 1 */

/* allocate new shader */
dlShader* dlNewShader( const char *file )
{
   CALL("%s", file);
   RET("%p", NULL);
   return( NULL );
}

/* free shader */
int dlFreeShader( dlShader *shader )
{
   CALL("%p", shader);
   RET("%p", NULL);
   return( RETURN_OK );
}

/* bind shader */
void dlBindShader( dlShader *shader )
{
   CALL("%p", shader);
   return;
}

/* bind using ID */
void dlBindShaderi( GLuint shader )
{
   CALL("%u", shader);
   return;
}

void dlShaderUniformMatrix4( dlShaderUniform *uniform, kmMat4 *mat )
{
   CALL("%p, %p", uniform, mat);
   return;
}

dlShaderUniform* dlShaderGetUniform( dlShader *shader, const char *name )
{
   CALL("%p, %s", shader, name);
   RET("%p", NULL);
   return( NULL );
}

#endif /* SHADER SUPPORT == 0 */
