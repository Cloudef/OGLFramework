#include <malloc.h>
#include <string.h>

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

#ifdef GLES2
#  include <GLES2/gl.h>
#else
#  include <GL/glew.h>
#  include <GL/gl.h>
#endif

#if SHADER_SUPPORT

#define MAX_SHADER_SIZE   1024 * 10 /* 10240 characters */

#define VERTEX_SHADER   "VERTEX_SHADER"
#define FRAGMENT_SHADER "FRAGMENT_SHADER"

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

   f = fopen( file, "r" );
   if(!f) return( NULL );

   fseek(f, 0, SEEK_END); size = ftell(f);
   fseek(f, 0, SEEK_SET);

   if(size > MAX_SHADER_SIZE)
   {
      fclose(f);
      logRed(); dlPuts("Shader file is too big."); logNormal();
      return( NULL );
   }

   data = malloc( size );
   fread(data, size, 1, f); fclose(f);

   data[ size-1 ] = '\0';

   return( data );
}

/*! appends char array to char array, if str == NULL, allocates new char array.
 * str == char array which to append, str2 == char array that gets appended */
static char* append( char *str, char *str2 )
{
   char *data = NULL;
   char *ostr = NULL;

   if(!str2) return( str );
   if(str)
   { ostr = strdup(str); data = realloc( str, strlen(str) + strlen(str2) + 1 ); }
   if(!data)
   {
      if(str) data = malloc( strlen(str) + strlen(str2) + 1 );
      else    data = malloc( strlen(str2) + 1 );
      if(!data) return( str );

      if(str)
         strcpy( data, str );
   }
   str = data;

   if(ostr)
   { strcpy( str + strlen(ostr), str2 ); free(ostr); }
   else
      strcpy( str, str2 );

   return( str );
}

/*! replaces string in string */
static char *str_replace(const char *s, const char *old, const char *new)
{
  size_t slen = strlen(s)+1;
  char *cout=0, *p=0, *tmp=NULL; cout=malloc(slen); p=cout;
  if( !p )
    return 0;
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
  return cout;
}

/* link shader program */
static int link_shader( dlShader *shader )
{
   GLint    status = 0, logLen = 0;
   GLsizei  length = 0;
   char     *log;

   glLinkProgram( shader->object );
   glGetProgramiv( shader->object, GL_LINK_STATUS, &status );
   if(!status)
   {
      logRed(); dlPuts("[SHADER] Program failed to link"); logNormal();
      glGetProgramiv( shader->object, GL_INFO_LOG_LENGTH, &logLen );

      log = malloc( logLen );
      glGetProgramInfoLog( shader->object, logLen, &length, log );
      logWhite(); dlPuts( log ); logNormal(); /* prints error */
      free( log );

      return( RETURN_FAIL );
   }

   return( RETURN_OK );
}

/* process shader */
static int process_shader( dlShader *shader )
{
   unsigned int i;
   GLint uniformCount = 0, maxLen = 0, size = 0;
   GLenum type;
   char *buffer;

   glGetProgramiv( shader->object, GL_ACTIVE_UNIFORMS, &uniformCount );
   glGetProgramiv( shader->object, GL_ACTIVE_UNIFORM_MAX_LENGTH, &maxLen );

   if( !uniformCount || !maxLen )
   {
      logRed(); dlPuts("[SHADER] Failed to retieve uniform information"); logNormal();
      return( RETURN_FAIL );
   }

   shader->uniformCount = (unsigned int)uniformCount;
   shader->uniforms     = dlCalloc( uniformCount, sizeof( dlShaderUniform ) );
   if(!shader->uniforms)
   {
      logRed(); dlPuts("[SHADER] Failed to alloc uniform list"); logNormal();
      return( RETURN_FAIL );
   }

   buffer = malloc(maxLen);
   if(!buffer)
   {
      logRed(); dlPuts("[SHADER] Failed to alloc uniform name buffer"); logNormal();
      return( RETURN_FAIL );
   }

   i = 0;
   for( ; i != uniformCount; ++i )
   {
      glGetActiveUniform( shader->object, i, maxLen, 0, &size, &type, buffer );
      dlPuts( buffer );
      shader->uniforms[i].name = strdup(buffer);
   }
   free(buffer);

   return( RETURN_OK );
}

static unsigned int process_vertex_GLSL( char *data )
{
   char *data2 = NULL;
   unsigned int vertexShader;

   /* vertex shader */
   data2 = append( data2, "#define "VERTEX_SHADER" 1\n" );
   data2 = append( data2, data );

   vertexShader = glCreateShader(GL_VERTEX_SHADER);
   glShaderSource(vertexShader, 1, (const GLchar**)&data2, NULL);
   glCompileShader(vertexShader);
   free(data2); data2 = NULL;

   return( vertexShader );
}

static unsigned int process_fragment_GLSL( char *data )
{
   char *data2 = NULL, *tmp;
   unsigned int fragmentShader;

   /* fragment shader */
   data2 = append( data2, "#define "FRAGMENT_SHADER" 1\n" );
   data2 = append( data2, data );

   tmp = str_replace( data2, "FRAGMENT_COLOR", "gl_FragColor" );
   if(tmp)
   { free(data2); data2 = tmp; }

   fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
   glShaderSource(fragmentShader, 1, (const GLchar**)&data2, NULL);
   glCompileShader(fragmentShader);
   free(data2); data2 = NULL;

   return( fragmentShader );
}

/* allocate new shader */
dlShader* dlNewShader( const char *file )
{
   dlShader *shader;
   char *data = NULL;
   unsigned int vertexShader, fragmentShader;

   data = getFile( file );
   if(!data) return( NULL );

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
      return( NULL );
   }
   shader->uniforms  = NULL;
   shader->file      = strdup(file);

   /* create shader program */
   shader->object = glCreateProgram();
   glAttachShader( shader->object, vertexShader   );
   glAttachShader( shader->object, fragmentShader );

   /* we don't need these anymore */
   glDeleteShader(vertexShader);
   glDeleteShader(fragmentShader);

   /* link shader */
   if(link_shader( shader ) != RETURN_OK)
   { dlFreeShader( shader ); return( NULL ); }

   /* get uniform amount */
   if(process_shader( shader ) != RETURN_OK)
   { dlFreeShader( shader ); return( NULL ); }

   return( shader );
}

/* free shader */
int dlFreeShader( dlShader *shader )
{
   unsigned int i;

   if(!shader) return( RETURN_NOTHING );

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

   /* free shader */
   dlFree( shader, sizeof( dlShader ) );
   return( RETURN_OK );
}

/* bind shader */
static GLuint _DL_BIND_SHADER = 0;
void dlBindShader( dlShader *shader )
{
   if(!shader && _DL_BIND_SHADER)
   {
      glUseProgram( 0 ); _DL_BIND_SHADER = 0;
      return;
   }

   if( _DL_BIND_SHADER == shader->object )
      return;

   glUseProgram( shader->object );
   _DL_BIND_SHADER = shader->object;
}

/* bind using ID */
void dlBindShaderi( GLuint shader )
{
   if( _DL_BIND_SHADER == shader )
      return;

   glUseProgram( shader );
   _DL_BIND_SHADER = shader;
}

#else  /* SHADER SUPPORT == 1 */

/* allocate new shader */
dlShader* dlNewShader( const char *file )
{
   return( NULL );
}

/* free shader */
int dlFreeShader( dlShader *shader )
{
   return( NULL );
}

/* bind shader */
void dlBindShader( dlShader *shader )
{
   return;
}

/* bind using ID */
void dlBindShaderi( GLuint shader )
{
   return;
}

#endif /* SHADER SUPPORT == 0 */
