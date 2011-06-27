#include <SDL/SDL.h>
#include <SDL/SDL_syswm.h>

#include "types.h"
#include "logfile.h"

#if defined(GLES1) || defined(GLES2)
    #include <EGL/egl.h>
    #include <X11/Xlib.h>
#endif

static struct SDL_Surface* SDLWindow = NULL;

#if defined(GLES1) || defined(GLES2)
   static EGLDisplay g_eglDisplay;
   static  EGLConfig  g_eglConfig;
   static EGLContext g_eglContext;
   static EGLSurface g_eglSurface;

   static Display* x11Disp = NULL;

   #define g_totalConfigsIn 10
#endif

int glCreateWindow(int window_width, int window_height, int bitsperpixel, unsigned int flags )
{
   FILE *logFile;

   if(SDLWindow)
      return( RETURN_NOTHING );

   /* open log */
   logFile = logOpen( "window.log" );
   logRed(); /* next are only error messages */
#if defined(GLES1) || defined(GLES2)
   x11Disp        = NULL;
   g_eglSurface   = 0;
   g_eglContext   = 0;
   g_eglDisplay   = 0;

   x11Disp        = XOpenDisplay(0);
   if(!x11Disp)
   {
      logPuts(logFile,"[WINDOW] Failed to open X display");
      return( RETURN_FAIL );
   }

   g_eglDisplay = eglGetDisplay((EGLNativeDisplayType)x11Disp);
   if(g_eglDisplay == EGL_NO_DISPLAY)
   {
      logPuts(logFile,"[WINDOW] Failed to get EGL display");
      return( RETURN_FAIL );
   }

   if(!eglInitialize(g_eglDisplay, NULL, NULL))
   {
      logPuts(logFile,"[WINDOW] Failed to initialize EGL");
      return( RETURN_FAIL );
   }

   /* Force 16 for now */
   bitsperpixel = 16;
#endif

   /* Create SDL Window */
   SDLWindow = SDL_SetVideoMode(window_width, window_height, bitsperpixel, flags );
   if(!SDLWindow)
   {
      logPuts(logFile,"[WINDOW] SDL_SetVideoMode failed");
      return( RETURN_FAIL );
   }

#if defined(GLES1) || defined(GLES2)
   static const EGLint s_configAttribs[] =
   {
      EGL_RED_SIZE,           5,
      EGL_GREEN_SIZE,         6,
      EGL_BLUE_SIZE,          5,
      EGL_DEPTH_SIZE,         16,
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

   if(eglChooseConfig(g_eglDisplay, s_configAttribs, g_allConfigs, g_totalConfigsIn, &numConfigsOut) != EGL_TRUE ||
         numConfigsOut == 0)
   {
      logPuts(logFile,"[WINDOW] Could not find suitable EGL configuration");
      return( RETURN_FAIL );
   }

   logGreen(); logPrint(logFile,"[WINDOW] Found %d suitable configurations\n", numConfigsOut); logRed();
   g_eglConfig = g_allConfigs[0];
   eglBindAPI(EGL_OPENGL_ES_API);
   #ifdef GLES1
   EGLint contextParams[] = {EGL_CONTEXT_CLIENT_VERSION, 1, EGL_NONE};
   #else
   EGLint contextParams[] = {EGL_CONTEXT_CLIENT_VERSION, 2, EGL_NONE};
   #endif

   g_eglContext = eglCreateContext(g_eglDisplay, g_eglConfig, EGL_NO_CONTEXT, contextParams);
   if(g_eglContext == EGL_NO_CONTEXT)
   {
      logPuts(logFile,"[WINDOW] Failed to create EGL context");
      return( RETURN_FAIL );
   }

   SDL_SysWMinfo sysInfo;
   SDL_VERSION(&sysInfo.version);
   if(SDL_GetWMInfo(&sysInfo) <= 0)
   {
      logPuts(logFile,"[WINDOW] SDL_GetWMInfo failed");
      return( RETURN_FAIL );
   }

   g_eglSurface = eglCreateWindowSurface(g_eglDisplay, g_eglConfig, (EGLNativeWindowType)sysInfo.info.x11.window, NULL);
   if(g_eglSurface == EGL_NO_SURFACE)
   {
      logPuts(logFile,"[WINDOW] Failed to create EGL surface");
      return( RETURN_FAIL );
   }

   if(eglMakeCurrent(g_eglDisplay, g_eglSurface, g_eglSurface, g_eglContext) == EGL_FALSE)
   {
      logPuts(logFile,"[WINDOW] Failed to make EGL current");
      return( RETURN_FAIL );
   }
#endif

   SDL_WM_SetCaption( "OGLWindow", NULL );
   logGreen();
   logPuts(logFile,"[WINDOW] OK");
   logNormal();

   /* close log */
   logClose( logFile );

   return( RETURN_OK );
}

void glCloseWindow()
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

	if (x11Disp)
		XCloseDisplay(x11Disp);
	x11Disp = NULL;
   #endif
}

void glSwapBuffers()
{
#if defined(GLES1) || defined(GLES2)
   eglSwapBuffers(g_eglDisplay, g_eglSurface);
#else
	SDL_GL_SwapBuffers();
#endif
}

/* EoF */
