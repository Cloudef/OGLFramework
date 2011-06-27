#include <stdio.h>
#include "core.h"
#include "types.h"
#include "render/render.h"
#include "logfile_wrapper.h"

/* init/deinit texture cache */
#include "texture.h"

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
glCoreConfig _glCore;

/* Set projection */
void glSetProjection( kmMat4 projection )
{
   _glCore.render.projection = projection;
}

/* Get projection */
kmMat4 glGetProjection( void )
{
   return( _glCore.render.projection );
}

/* Set render mode */
void glSetRenderMode( gleRenderMode mode )
{
#if defined(GLES1) || defined(GLES2)
   _glCore.render.mode = mode;
#else
   if( mode == GL_MODE_VBO )
   {
      if( GL_ARB_vertex_buffer_object )
      {
         _glCore.render.mode = mode;
         logBlue(); glPuts("-!- GL_MODE_VBO"); logNormal();
      }
      else
      {
         _glCore.render.mode = GL_MODE_VERTEX_ARRAY;
         logBlue(); glPuts("-!- VBO rendering method is not avaible on this hardware"); logNormal();
      }
   }
   else
   {
      _glCore.render.mode = mode;
      logBlue(); glPuts("-!- GL_MODE_VERTEX_ARRAY"); logNormal();
   }
#endif
}

/* Get OpenGL information */
static int glGetInfo( void )
{
   char *GL_ver;
   int num_parsed;

   /* get OGL extensions */
   _glCore.extensions  = (char*)glGetString( GL_EXTENSIONS );
   if(!_glCore.extensions)
   {
      logRed(); glPrint("[GL] Failed to retive extensions, maybe no context?"); logNormal();
      return(RETURN_FAIL);
   }

   /* get OGL version */
   GL_ver                   = (char*)glGetString(GL_VERSION);
   _glCore.version.vendor   = (char*)glGetString(GL_VENDOR);

   num_parsed = sscanf(GL_ver, "%u.%u.%u",
         &_glCore.version.major,
         &_glCore.version.minor,
         &_glCore.version.patch);

   if (num_parsed == 1) {
      _glCore.version.minor = 0;
      _glCore.version.patch = 0;
   } else if (num_parsed == 2) {
      _glCore.version.patch = 0;
   }

   glGetIntegerv (GL_MAX_LIGHTS,                   &_glCore.info.maxLights);
   glGetIntegerv (GL_MAX_CLIP_PLANES,              &_glCore.info.maxClipPlanes);
   glGetIntegerv (GL_SUBPIXEL_BITS,                &_glCore.info.subPixelBits);
   glGetIntegerv (GL_MAX_TEXTURE_SIZE,             &_glCore.info.maxTextureSize);
   glGetIntegerv (GL_MAX_TEXTURE_UNITS,            &_glCore.info.maxTextureUnits);

   return( RETURN_OK );
}

/* Output OpenGL information */
static void glOutputInfo( void )
{
   logGreen();
   glPrint("[Renderer %s]\n",                   _glCore.render.string);
   glPrint("[OpenGL %u.%u.%u]\n",               _glCore.version.major,
                                                _glCore.version.minor,
                                                _glCore.version.patch);
   if(_glCore.version.vendor) {
      glPrint( "[GPU %s]\n",                    _glCore.version.vendor);
   }
   glPuts("");

   logWhite();
   glPrint("-!- Bits per pixel: %d\n\n",         _glCore.info.subPixelBits);

   glPrint("-!- Maximum 2D texture size: %d\n",  _glCore.info.maxTextureSize);
   glPrint("-!- Maximum lights: %d\n",           _glCore.info.maxLights);
   glPrint("-!- Maximum clipping planes: %d\n",  _glCore.info.maxClipPlanes);
   glPrint("-!- Maximum texture units: %d\n",    _glCore.info.maxTextureUnits);

   glPuts("");
   logYellow(); glPuts("[EXTENSIONS]");
   logWhite();  glPrint("%s\n\n", _glCore.extensions); logNormal();
}

/* creates virtual display and inits OGL Framework */
int glCreateDisplay(int display_width, int display_height, gleRenderer renderer)
{
   /* NULL these */
   _glCore.render.draw     = NULL;
   _glCore.render.string   = NULL;

   /* Open log */
   glLogOpen();

   /* just for temporary */
   renderer = GL_RENDER_OGL140;

#if !defined(GLES1) && !defined(GLES2)
   /* initialize GLEW if on correct platform */
   glClear( GL_COLOR_BUFFER_BIT );
   if(GLEW_OK != glewInit())
   {
      logRed(); glPrint("[GL] There is no OpenGL context"); logNormal();
      return( RETURN_FAIL );
   }
   logGreen(); glPuts("[GLEW] OK"); logNormal();
#endif

   /* check display resolution */
   if(display_width > 0 && display_height > 0)
   {
      _glCore.display.width   = display_width;
      _glCore.display.height  = display_height;
   } else {
      logRed(); glPuts("[GL] Incorrect display width or height"); logNormal();
      return( RETURN_FAIL );
   }

   /* get OpenGL information */
   if(glGetInfo() != RETURN_OK)
      return( RETURN_FAIL );

   /* assign correct renderer */
#if !defined(GLES1) && !defined(GLES2)
   if(renderer == GL_RENDER_DEFAULT)
   {
      if(_glCore.version.major >= 3)
      {
         /* OGL 3 */
      }
      else if(_glCore.version.major >= 1 &&
              _glCore.version.minor >= 4)
      {
         /* OGL 140 */
         if(glOGL140() != 0)
            return(RETURN_FAIL);
      }
      else
      {
         logBlue(); glPuts("-!- Unknown OpenGL version, trying to use OGL 3.1+]"); logNormal();
         /* OGL 3 */
      }
   }
   else
   {
      /*
      if(renderer == eOGL3)
      */

      if(renderer == GL_RENDER_OGL140)
      {
         if(glOGL140() != 0)
            return(RETURN_FAIL);
      }

      /*
      if(renderer == eNULL)
      */
   }
#else
#  ifdef GLES1
      /* GLES 1.0 */
      if(glOGL140() != 0)
         return(RETURN_FAIL);
#  else
      /* GLES 2.0 */
#  endif
#endif

   /* renderer fail ? */
   if(!_glCore.render.draw)
   {
      logRed(); glPuts("[GL] There is no renderer"); logNormal();
      return(RETURN_FAIL);
   }

   /* Init texture cache */
   if(glTextureInitCache() != RETURN_OK)
   {
      logRed(); glPuts("[GL] Failed to init texture cache"); logNormal();
      return(RETURN_FAIL);
   }

   /* Output openGL information */
   glOutputInfo();
   glPuts( "");
   logGreen(); glPuts("[GL] Created"); logNormal();

   /* Default to VBO */
   glSetRenderMode( GL_MODE_VBO );

   return( RETURN_OK );
}

/* frees virtual display and deinits OGL framework */
int glFreeDisplay( void )
{
   logRed(); glPuts("[GL] Destroyed"); logNormal();

   /* Deinit texture cache */
   glTextureFreeCache();

   /* close log */
   glLogClose( );

   return( RETURN_OK );
}

/* EoF */
