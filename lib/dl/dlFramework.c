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

#define DL_DEBUG_CHANNEL "GL"

/* init here */
dlCoreConfig _dlCore;

/* Set projection */
void dlSetProjection( kmMat4 projection )
{
   TRACE();

   _dlCore.render.projection = projection;
   _dlCore.render.camera     = NULL; /* we use own projection */
}

/* Get projection */
kmMat4 dlGetProjection( void )
{
   TRACE();
   return( _dlCore.render.projection );
}

/* Set active camera */
void dlSetCamera( dlCamera *camera )
{
   CALL("%p", camera);

   _dlCore.render.camera = camera;

   if(camera) /* camera can also be set NULL */
      _dlCore.render.projection = camera->matrix; /* we use camera's projection */
}

/* Get active camera */
dlCamera* dlGetCamera( void )
{
   TRACE();
   RET("%p", _dlCore.render.camera);
   return( _dlCore.render.camera );
}

/* Set active shader */
void dlSetShader( dlShader *shader )
{
   CALL("%p", shader);
   _dlCore.render.shader = shader;
}

/* Get active shader */
dlShader* dlGetShader( void )
{
   TRACE();
   RET("%p", _dlCore.render.shader);
   return( _dlCore.render.shader );
}

/* Changes internal resolution.
 * NOTE: Any cameras not in use need to be set manually */
void dlSetResolution( int x, int y )
{
   dlCamera *camera;
   CALL("%d, %d", x, y);

   _dlCore.display.width  = x;
   _dlCore.display.height = y;

   /* Check if we have active camera */
   if(!(camera = dlGetCamera()))
      return;

   /* Hmm.. This camera seems to have viewcuts, we propably don't want to resize this */
   if(camera->viewCut.x != 0 || camera->viewCut.y != 0)
      return;

   dlCameraSetView( camera, 0, 0, x, y );
}

/* Set render mode */
void dlSetRenderMode( dleRenderMode mode )
{
   CALL("%d", mode);

#if defined(GLES1) || defined(GLES2)
   _dlCore.render.mode = mode;
#else
   if( mode == DL_MODE_VBO )
   {
      if( GL_ARB_vertex_buffer_object )
      {
         _dlCore.render.mode = mode;
         LOGINFO("DL_MODE_VBO");
      }
      else
      {
         _dlCore.render.mode = DL_MODE_VERTEX_ARRAY;
         LOGINFO("VBO rendering method is not avaible on this hardware");
      }
   }
   else
   {
      _dlCore.render.mode = mode;
      LOGINFO("DL_MODE_VERTEX_ARRAY");
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
   TRACE();

   /* get ogl extensions */
   _dlCore.extensions  = (char*)glGetString( GL_EXTENSIONS );
   if(!_dlCore.extensions)
   {
      LOGERR("Failed to retive extensions, maybe no context?");
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

   RET("%d", RETURN_OK);
   return( RETURN_OK );
}

/* Output OpenGL information */
static void dlOutputInfo( void )
{
   TRACE();

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
   CALL("%d, %d, %d", display_width, display_height, renderer);

   /* NULL these */
   _dlCore.render.draw     = NULL;
   _dlCore.render.string   = NULL;
   _dlCore.render.camera   = NULL;
   _dlCore.render.shader   = NULL;

   /* Open log */
   dlLogOpen();

#if !defined(GLES1) && !defined(GLES2)
   /* initialize GLEW if on correct platform */
   glClear( GL_COLOR_BUFFER_BIT );
   if(GLEW_OK != glewInit())
   {
      LOGERR("There is no OpenGL context");
      return( RETURN_FAIL );
   }
   LOGOK("GLEW OK");
#endif

   /* check display resolution */
   if(display_width > 0 && display_height > 0)
   {
      _dlCore.display.width   = display_width;
      _dlCore.display.height  = display_height;
   } else {
      LOGERRP("Incorrect display: %dx%d", display_width, display_height);
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
         LOGINFO("Unknown OpenGL version, trying to use OGL 3.1+");
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
      LOGERR("There is no renderer");
      return(RETURN_FAIL);
   }

   /* Init texture cache */
   if(dlTextureInitCache() != RETURN_OK)
   {
      LOGERR("Failed to init texture cache");
      return(RETURN_FAIL);
   }

   /* Output opengl information */
   dlOutputInfo(); dlPuts("");
   LOGOKP("%dx%d created",
          _dlCore.display.width, _dlCore.display.height);

   /* Default to VBO */
   dlSetRenderMode( DL_MODE_VBO );

   RET("%d", RETURN_OK);
   return( RETURN_OK );
}

/* frees virtual display and deinits dl framework */
int dlFreeDisplay( void )
{
   TRACE();

   /* Deinit texture cache */
   dlTextureFreeCache();

   LOGFREE("Destroyed");

   /* close log */
   dlLogClose( );

   RET("%d", RETURN_OK);
   return( RETURN_OK );
}
