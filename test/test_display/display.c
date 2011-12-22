#include <stdio.h>
#include <SDL/SDL.h>

#include "DL/dl.h"
#include "DL/dlWindow.h"
#include "DL/dlInput.h"
#include "logfile.h"

#if defined(GLES1) || defined(GLES2)
#  define WINDOW_FLAGS  SDL_RESIZABLE
#  define WINDOW_WIDTH  800
#  define WINDOW_HEIGHT 480
#  define WINDOW_BITS   16
#else
#  define WINDOW_FLAGS  SDL_OPENGL | SDL_DOUBLEBUF | SDL_RESIZABLE
#  define WINDOW_WIDTH  800
#  define WINDOW_HEIGHT 480
#  define WINDOW_BITS   32
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
            dlWindowSetMode(event.resize.w, event.resize.h,
                            WINDOW_BITS, WINDOW_FLAGS);
            dlSetResolution(event.resize.w,event.resize.h);
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
   /* init debug channels */
   dlDEBINIT(argc, argv);

   if(SDL_Init(   SDL_INIT_VIDEO    ) != 0)
      cleanup(EXIT_FAILURE);

   if(dlCreateWindow( WINDOW_WIDTH, WINDOW_HEIGHT, WINDOW_BITS, WINDOW_FLAGS ) != 0)
      cleanup(EXIT_FAILURE);

   if(dlCreateDisplay( WINDOW_WIDTH, WINDOW_HEIGHT, DL_RENDER_OGL140 ) != 0)
      cleanup(EXIT_FAILURE);

   /* startup graph */
   dlMemoryGraph();

   /* wait for escape key */
   while(!dlKeyPress(SDLK_ESCAPE))
   {
      keyHandle();

      /* It's good idea to do this when we are not drawing anything */
      glFlush();
      glFinish();

      /* Actual swap and clear */
      dlSwapBuffers();
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
   }

   cleanup(EXIT_SUCCESS);
   return(EXIT_SUCCESS);
}
