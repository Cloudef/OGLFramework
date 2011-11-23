#include <stdio.h>
#include <limits.h>
#include <SDL/SDL.h>

#include "DL/dl.h"
#include "DL/dlWindow.h"
#include "DL/dlInput.h"
#include "logfile.h"

#ifdef WIN32
#define LINE_MAX 256
#endif

static void keyHandle( void )
{
   SDL_Event event;

   while(SDL_PollEvent(&event))
   {
      switch (event.type) {
         case SDL_KEYDOWN:
            dlKeyAdd(event.key.keysym.sym);
         break;  /* SDL_KEYDOWN */
         case SDL_KEYUP:
            dlKeyDel(event.key.keysym.sym);
         break;  /* SDL_KEYUP */
         case SDL_VIDEOEXPOSE:
         break;  /* SDL_VIDEOEXPOSE */
         case SDL_VIDEORESIZE:
         break;  /* SDL_VIDEORESIZE */
         case SDL_QUIT:
            dlKeyAdd(SDLK_ESCAPE);
         break;  /* SDL_QUIT */
      }
   }
}

static void cleanup( int ret )
{
   dlFreeDisplay();
   dlCloseWindow();
   SDL_Quit();

   /* exit graph */
   dlMemoryGraph();

   exit( ret );
}

int main( int argc, char **argv )
{
   dlCamera *camera;
   dlShader *shader;
   dlObject *object;

   /* FPS Counter */
   unsigned int   now          = 0;
   unsigned int   last         = 0;
   unsigned int   frameCounter = 0;
   unsigned int   FPS          = 0;
   unsigned int   fpsDelay     = 0;
   float          duration     = 0;
   float          delta        = 0;

   char           WIN_TITLE[LINE_MAX];

   unsigned int   flags = SDL_OPENGL | SDL_DOUBLEBUF | SDL_RESIZABLE;
   int            width = 800;
   int            height= 480;
   int            bits  = 32;
#if defined(GLES1) || defined(GLES2)
   flags = SDL_RESIZABLE;
   width = 800;
   height= 480;
   bits  = 16;
#endif

   if(SDL_Init(   SDL_INIT_VIDEO    ) != 0)
      cleanup(EXIT_FAILURE);

   if(dlCreateWindow( width, height, bits, flags ) != 0)
   {
      logRed();
      puts(dlWindowGetError());
      logNormal();
      cleanup(EXIT_FAILURE);
   } else {
      logGreen();
      puts(dlWindowGetError());
      logNormal();
   }

   if(dlCreateDisplay( width, height, DL_RENDER_OGL3 ) != 0)
      cleanup(EXIT_FAILURE);

   /* create camera */
   camera = dlNewCamera();
   if(!camera)
      cleanup(EXIT_FAILURE);

   /* sets this as active camera */
   dlCameraRender( camera );

   /* create test plane */
   object = dlNewPlane( 0.005, 0.005, 1 );
   if(!object)
      cleanup(EXIT_FAILURE);

   /* create shader */
   shader = dlNewShader( "shader/ogl3.shd" );
   if(!shader)
      cleanup(EXIT_FAILURE);

   /* bind */
   dlBindShader(shader);

   /* startup graph */
   dlMemoryGraph();

   /* wait for escape key */
   while(!dlKeyPress(SDLK_ESCAPE))
   {
      last  = now;
      now   = SDL_GetTicks();
      delta = (now - last) / 1000.0f;

      keyHandle();

      dlDraw( object );

      dlSwapBuffers();
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

      if(fpsDelay < SDL_GetTicks())
      {
         if(duration > 0.0f)
            FPS = (float)frameCounter / duration;

         sprintf(WIN_TITLE, "OpenGL [FPS: %d]", FPS);
         SDL_WM_SetCaption( WIN_TITLE, NULL );

         frameCounter = 0; fpsDelay = now + 1000; duration = 0;
      }

      ++frameCounter;
      duration += delta;
   }
   dlFreeCamera( camera );
   dlFreeObject( object );
   dlFreeShader( shader );

   cleanup(EXIT_SUCCESS);
   return(EXIT_SUCCESS);
}
