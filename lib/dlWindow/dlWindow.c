#include <SDL/SDL.h>
#include <SDL/SDL_syswm.h>
#include <stdarg.h>

#include "dlTypes.h"

#ifdef PANDORA
#  if !defined(GLES1) && !defined(GLES2)
#     error "Pandora detected, but no GLES selected"
#  endif
#endif

#if defined(GLES1) || defined(GLES2)
    #include <EGL/egl.h>
    #include <X11/Xlib.h>
#endif

#define LOGERR(e,m)       dlWindowSetError("["e"] "m);
#define LOGERRP(e,m,...)  dlWindowSetErrorP("["e"] "m"\n",##__VA_ARGS__);

static struct SDL_Surface* SDLWindow = NULL;
static char   DLWINERR[256];

#if defined(GLES1) || defined(GLES2)
   static EGLDisplay g_eglDisplay;
   static EGLContext g_eglContext;
   static EGLSurface g_eglSurface;

   static Display* x11Disp = NULL;

   #define g_totalConfigsIn 10
#endif

/* TO-DO:
 * Add dlCreateWindowEx, for more controlled window creation with params passed as struct.
 */

/* Get error string */
const char* dlWindowGetError(void)
{
   return( DLWINERR );
}

static void dlWindowSetError( const char *err )
{
   strcpy( DLWINERR, err );
}

static void dlWindowSetErrorP( const char *err, ... )
{
   va_list args;
   va_start(args, err);
   vsnprintf( DLWINERR, 256, err, args );
   va_end(args);
}

void dlCloseWindow(void);

/* Window create */
int dlCreateWindow(int window_width, int window_height, int bitsperpixel, unsigned int flags )
{
   uint8_t rgb[4];

   /* null error string */
   memset( DLWINERR, 0, 256 );

   switch(bitsperpixel)
   {
      case 8:
         rgb[0] = 2;
         rgb[1] = 3;
         rgb[2] = 3;
         rgb[3] = 8;
         break;
      case 15:
      case 16:
         rgb[0] = 5;
         rgb[1] = 5;
         rgb[2] = 5;
         rgb[3] = 16;
         break;
      default:
         rgb[0] = 8;
         rgb[1] = 8;
         rgb[2] = 8;
         rgb[3] = 24;
         break;
   }

   /* Close existing window */
   if(SDLWindow)
   {
      SDL_FreeSurface(SDLWindow);
      dlCloseWindow();
   }

#if defined(GLES1) || defined(GLES2)
   /* OpenGL GLES */
   x11Disp        = NULL;
   g_eglSurface   = 0;
   g_eglContext   = 0;
   g_eglDisplay   = 0;

   x11Disp        = XOpenDisplay(0);
   if(!x11Disp)
   {
      dlCloseWindow();
      LOGERR("WINDOW", "Failed to open X display");
      return( RETURN_FAIL );
   }

   g_eglDisplay = eglGetDisplay((EGLNativeDisplayType)x11Disp);
   if(g_eglDisplay == EGL_NO_DISPLAY)
   {
      dlCloseWindow();
      LOGERR("WINDOW", "Failed to get EGL display");
      return( RETURN_FAIL );
   }

   if(!eglInitialize(g_eglDisplay, NULL, NULL))
   {
      dlCloseWindow();
      LOGERR("WINDOW", "Failed to initialize EGL");
      return( RETURN_FAIL );
   }
#else
   /* Desktop OpenGL */
   SDL_GL_SetAttribute( SDL_GL_RED_SIZE,   rgb[0] );
   SDL_GL_SetAttribute( SDL_GL_GREEN_SIZE, rgb[1] );
   SDL_GL_SetAttribute( SDL_GL_BLUE_SIZE,  rgb[2] );
   SDL_GL_SetAttribute( SDL_GL_DEPTH_SIZE, rgb[3] );
#endif

   /* Create SDL Window */
   SDLWindow = SDL_SetVideoMode(window_width, window_height, bitsperpixel, flags );
   if(!SDLWindow)
   {
      dlCloseWindow();
      LOGERR("WINDOW", "SDL_SetVideoMode failed");
      return( RETURN_FAIL );
   }

#if defined(GLES1) || defined(GLES2)
   /* OpenGL GLES */
   static const EGLint s_configAttribs[] =
   {
      EGL_RED_SIZE,           rgb[0],
      EGL_GREEN_SIZE,         rgb[1],
      EGL_BLUE_SIZE,          rgb[2],
      EGL_DEPTH_SIZE,         rgb[3],
      EGL_SURFACE_TYPE,       EGL_WINDOW_BIT,
      #ifdef GLES1
      EGL_RENDERABLE_TYPE,    EGL_OPENGL_ES_BIT,
      #endif
      #ifdef GLES2
      EGL_RENDERABLE_TYPE,    EGL_OPENGL_ES2_BIT,
      #endif
      EGL_NONE
   };
   EGLint numConfigsOut = 0;

   EGLConfig g_allConfigs[g_totalConfigsIn];
   EGLConfig g_eglConfig;

   /* Choose configuration */
   if(eglChooseConfig(g_eglDisplay, s_configAttribs, g_allConfigs, g_totalConfigsIn, &numConfigsOut) != EGL_TRUE
      || numConfigsOut == 0)
   {
      dlCloseWindow();
      LOGERR("WINDOW", "Could not find suitable EGL configuration");
      return( RETURN_FAIL );
   }

   /* Bind GLES API */
   g_eglConfig = g_allConfigs[0];
   eglBindAPI(EGL_OPENGL_ES_API);
   #ifdef GLES1
   EGLint contextParams[] = {EGL_CONTEXT_CLIENT_VERSION, 1, EGL_NONE};
   #else
   EGLint contextParams[] = {EGL_CONTEXT_CLIENT_VERSION, 2, EGL_NONE};
   #endif

   /* Create EGL Context */
   g_eglContext = eglCreateContext(g_eglDisplay, g_eglConfig, EGL_NO_CONTEXT, contextParams);
   if(g_eglContext == EGL_NO_CONTEXT)
   {
      dlCloseWindow();
      LOGERR("WINDOW", "Failed to create EGL context");
      return( RETURN_FAIL );
   }

   /* Get window manager information */
   SDL_SysWMinfo sysInfo;
   SDL_VERSION(&sysInfo.version);
   if(SDL_GetWMInfo(&sysInfo) <= 0)
   {
      dlCloseWindow();
      LOGERR("WINDOW", "SDL_GetWMInfo failed");
      return( RETURN_FAIL );
   }

   /* Create surface */
   g_eglSurface = eglCreateWindowSurface(g_eglDisplay, g_eglConfig, (EGLNativeWindowType)sysInfo.info.x11.window, NULL);
   if(g_eglSurface == EGL_NO_SURFACE)
   {
      dlCloseWindow();
      LOGERR("WINDOW", "Failed to create EGL surface");
      return( RETURN_FAIL );
   }

   /* Make EGL current */
   if(eglMakeCurrent(g_eglDisplay, g_eglSurface, g_eglSurface, g_eglContext) == EGL_FALSE)
   {
      dlCloseWindow();
      LOGERR("WINDOW", "Failed to make EGL current");
      return( RETURN_FAIL );
   }
#else
   /* Desktop OpenGL */
#endif

   /* Set default caption */
   SDL_WM_SetCaption( "OGLWindow", NULL );
   LOGERRP("WINDOW", "%dx%dx%d created", window_width, window_height, bitsperpixel);

   return( RETURN_OK );
}

/* Get window SDL surface */
struct SDL_Surface *dlGetSDLSurface(void)
{
   return( SDLWindow );
}

/* Close window */
void dlCloseWindow(void)
{
#if defined(GLES1) || defined(GLES2)
   if(g_eglDisplay)
   {
      eglMakeCurrent(g_eglDisplay, NULL, NULL, EGL_NO_CONTEXT);
      if(g_eglContext)
         eglDestroyContext(g_eglDisplay, g_eglContext);
      if(g_eglSurface)
         eglDestroySurface(g_eglDisplay, g_eglSurface);
      eglTerminate(g_eglDisplay);

      g_eglSurface = 0;
      g_eglContext = 0;
      g_eglDisplay = 0;
   }

   if(x11Disp)
      XCloseDisplay(x11Disp);
   x11Disp = NULL;
#else
   /* Desktop OpenGL */
#endif
   LOGERR("WINDOW", "closed");
}

/* Swap buffers */
void dlSwapBuffers(void)
{
#if defined(GLES1) || defined(GLES2)
   eglSwapBuffers(g_eglDisplay, g_eglSurface);
#else
   SDL_GL_SwapBuffers();
#endif
}
