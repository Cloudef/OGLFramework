#include <stdio.h>
#include <SDL/SDL.h>

#include "DL/dl.h"
#include "DL/dlWindow.h"
#include "DL/dlInput.h"
#include "logfile.h"

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
   unsigned int   flags = SDL_SWSURFACE | SDL_OPENGL | SDL_RESIZABLE;
   int            width = 800;
   int            height= 480;
   int            bits  = 32;
#if defined(GLES1) || defined(GLES2)
   flags = SDL_RESIZABLE;
   width = 800;
   height= 480;
   bits  = 16;
#endif

   /* init debug channels */
   dlDEBINIT(argc, argv);

   if(SDL_Init(   SDL_INIT_VIDEO    ) != 0)
      cleanup(EXIT_FAILURE);

   if(dlCreateWindow( width, height, bits, flags ) != 0)
      cleanup(EXIT_FAILURE);

   if(dlCreateDisplay( width, height, DL_RENDER_DEFAULT ) != 0)
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
