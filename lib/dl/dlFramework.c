#include <stdio.h>
#include "dlCore.h"
#include "dlTypes.h"
#include "render/dlRender.h"
#include "dlLog.h"

/* init/deinit texture cache */
#include "dlTexture.h"

#ifdef GLES2
#	include <GLES2/gl2.h>
#elif  GLES1
#  include <GLES/gl.h>
#  include <GLES/glext.h>
#else
#	include <GL/glew.h>
#  include <GL/gl.h>
#endif

/* init here */
dlCoreConfig _dlCore;

/* Set projection */
void dlSetProjection( kmMat4 projection )
{
   _dlCore.render.projection = projection;
   _dlCore.render.camera     = NULL; /* we use own projection */
}

/* Get projection */
kmMat4 dlGetProjection( void )
{
   return( _dlCore.render.projection );
}

/* Set active camera */
void dlSetCamera( dlCamera *camera )
{
   _dlCore.render.camera     = camera;

   if(camera) /* camera can also be set NULL */
      _dlCore.render.projection = camera->matrix; /* we use camera's projection */
}

/* Get active camera */
dlCamera* dlGetCamera( void )
{
   return( _dlCore.render.camera );
}

/* Set active shader */
void dlSetShader( dlShader *shader )
{
   _dlCore.render.shader = shader;
}

/* Get active shader */
dlShader* dlGetShader( void )
{
   return( _dlCore.render.shader );
}

/* Set render mode */
void dlSetRenderMode( dleRenderMode mode )
{
#if defined(GLES1) || defined(GLES2)
   _dlCore.render.mode = mode;
#else
   if( mode == DL_MODE_VBO )
   {
      if( GL_ARB_vertex_buffer_object )
      {
         _dlCore.render.mode = mode;
         logBlue(); dlPuts("-!- DL_MODE_VBO"); logNormal();
      }
      else
      {
         _dlCore.render.mode = DL_MODE_VERTEX_ARRAY;
         logBlue(); dlPuts("-!- VBO rendering method is not avaible on this hardware"); logNormal();
      }
   }
   else
   {
      _dlCore.render.mode = mode;
      logBlue(); dlPuts("-!- DL_MODE_VERTEX_ARRAY"); logNormal();
   }
#endif
}

/* Get OpenGL information */
static int dlGetInfo( void )
{
#if !defined(GLES1) && !defined(GLES2)
   char *gl_ver;
   int num_parsed;
#endif

   /* get ogl extensions */
   _dlCore.extensions  = (char*)glGetString( GL_EXTENSIONS );
   if(!_dlCore.extensions)
   {
      logRed(); dlPrint("[GL] Failed to retive extensions, maybe no context?"); logNormal();
      return(RETURN_FAIL);
   }

#if !defined(GLES1) && !defined(GLES2)
   /* get ogl version */
   gl_ver                   = (char*)glGetString(GL_VERSION);
   _dlCore.version.vendor   = (char*)glGetString(GL_VENDOR);

   num_parsed = sscanf(gl_ver, "%u.%u.%u",
         (unsigned int*)&_dlCore.version.major,
         (unsigned int*)&_dlCore.version.minor,
         (unsigned int*)&_dlCore.version.patch);

   if (num_parsed == 1) {
      _dlCore.version.minor = 0;
      _dlCore.version.patch = 0;
   } else if (num_parsed == 2) {
      _dlCore.version.patch = 0;
   }
#endif

   glGetIntegerv (GL_MAX_LIGHTS,                   &_dlCore.info.maxLights);
   glGetIntegerv (GL_MAX_CLIP_PLANES,              &_dlCore.info.maxClipPlanes);
   glGetIntegerv (GL_SUBPIXEL_BITS,                &_dlCore.info.subPixelBits);
   glGetIntegerv (GL_MAX_TEXTURE_SIZE,             &_dlCore.info.maxTextureSize);
   glGetIntegerv (GL_MAX_TEXTURE_UNITS,            &_dlCore.info.maxTextureUnits);

   return( RETURN_OK );
}

/* Output OpenGL information */
static void dlOutputInfo( void )
{
   logGreen();
   dlPrint("[Renderer %s]\n",                   _dlCore.render.string);

#if !defined(GLES1) && !defined(GLES2)
   dlPrint("[OpenGL %u.%u.%u]\n",               _dlCore.version.major,
                                                _dlCore.version.minor,
                                                _dlCore.version.patch);
#else
#  ifdef GLES1
      dlPuts("[GLES 1.1]");
#  else
      dlPuts("[GLES 2.0]");
#  endif
#endif

   if(_dlCore.version.vendor) {
      dlPrint( "[GPU %s]\n",                    _dlCore.version.vendor);
   }
   dlPuts("");

   logWhite();
   dlPrint("-!- Bits per pixel: %d\n\n",         _dlCore.info.subPixelBits);

   dlPrint("-!- Maximum 2D texture size: %d\n",  _dlCore.info.maxTextureSize);
   dlPrint("-!- Maximum lights: %d\n",           _dlCore.info.maxLights);
   dlPrint("-!- Maximum clipping planes: %d\n",  _dlCore.info.maxClipPlanes);
   dlPrint("-!- Maximum texture units: %d\n",    _dlCore.info.maxTextureUnits);

   dlPuts("");
   logYellow(); dlPuts("[EXTENSIONS]");
   logWhite();  dlPrint("%s\n\n", _dlCore.extensions); logNormal();
}

/* creates virtual display and inits dl Framework */
int dlCreateDisplay(int display_width, int display_height, dleRenderer renderer)
{
   /* NULL these */
   _dlCore.render.draw     = NULL;
   _dlCore.render.string   = NULL;
   _dlCore.render.camera   = NULL;
   _dlCore.render.shader   = NULL;

   /* Open log */
   dlLogOpen();

#if !defined(GLES1) && !defined(GLES2)
   /* initialize dlEW if on correct platform */
   glClear( GL_COLOR_BUFFER_BIT );
   if(GLEW_OK != glewInit())
   {
      logRed(); dlPrint("[GL] There is no OpenGL context"); logNormal();
      return( RETURN_FAIL );
   }
   logGreen(); dlPuts("[GLEW] OK"); logNormal();
#endif

   /* check display resolution */
   if(display_width > 0 && display_height > 0)
   {
      _dlCore.display.width   = display_width;
      _dlCore.display.height  = display_height;
   } else {
      logRed(); dlPuts("[GL] Incorrect display width or height"); logNormal();
      return( RETURN_FAIL );
   }

   /* get OpenGL information */
   if(dlGetInfo() != RETURN_OK)
      return( RETURN_FAIL );

   /* assign correct renderer */
#if !defined(GLES1) && !defined(GLES2)
   if(renderer == DL_RENDER_DEFAULT)
   {
      if(_dlCore.version.major >= 3)
      {
         /* OGL 3 */
         if(dlOGL3() != RETURN_OK)
            return(RETURN_FAIL);
      }
      else if(_dlCore.version.major >= 1 &&
              _dlCore.version.minor >= 4)
      {
         /* OGL 140 */
         if(dlOGL140() != RETURN_OK)
            return(RETURN_FAIL);
      }
      else
      {
         logBlue(); dlPuts("-!- Unknown OpenGL version, trying to use OGL 3.1+]"); logNormal();
         /* OGL 3 */
         if(dlOGL3() != RETURN_OK)
            return(RETURN_FAIL);
      }
   }
   else
   {
      if(renderer == DL_RENDER_OGL3)
      {
         if(dlOGL3() != RETURN_OK)
            return(RETURN_FAIL);
      }

      if(renderer == DL_RENDER_OGL140)
      {
         if(dlOGL140() != RETURN_OK)
            return(RETURN_FAIL);
      }

      /*
      if(renderer == eNULL)
      */
   }
#else
#  ifdef GLES1
      /* GLES 1.0 */
      if(dlOGL140() != RETURN_OK)
         return(RETURN_FAIL);
#  else
      /* GLES 2.0 */
      if(dlOGL3() != RETURN_OK)
         return(RETURN_FAIL);
#  endif
#endif

   /* renderer fail ? */
   if(!_dlCore.render.draw)
   {
      logRed(); dlPuts("[GL] There is no renderer"); logNormal();
      return(RETURN_FAIL);
   }

   /* Init texture cache */
   if(dlTextureInitCache() != RETURN_OK)
   {
      logRed(); dlPuts("[GL] Failed to init texture cache"); logNormal();
      return(RETURN_FAIL);
   }

   /* Output opengl information */
   dlOutputInfo();
   dlPuts( "");
   logGreen(); dlPuts("[GL] Created"); logNormal();

   /* Default to VBO */
   dlSetRenderMode( DL_MODE_VBO );

   return( RETURN_OK );
}

/* frees virtual display and deinits dl framework */
int dlFreeDisplay( void )
{
   logRed(); dlPuts("[GL] Destroyed"); logNormal();

   /* Deinit texture cache */
   dlTextureFreeCache();

   /* close log */
   dlLogClose( );

   return( RETURN_OK );
}

/* EoF */
