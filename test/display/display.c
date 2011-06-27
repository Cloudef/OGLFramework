#include <stdio.h>
#include <SDL/SDL.h>

#include "ogl/ogl.h"
#include "ogl/window.h"
#include "input.h"

static void keyHandle( void )
{
   SDL_Event event;

   while(SDL_PollEvent(&event))
   {
      switch (event.type) {
         case SDL_KEYDOWN:
            keyAdd(event.key.keysym.sym);
         break;  /* SDL_KEYDOWN */
         case SDL_KEYUP:
            keyDel(event.key.keysym.sym);
         break;  /* SDL_KEYUP */
         case SDL_VIDEOEXPOSE:
         break;  /* SDL_VIDEOEXPOSE */
	      case SDL_VIDEORESIZE:
		   break;  /* SDL_VIDEORESIZE */
         case SDL_QUIT:
            keyAdd(SDLK_ESCAPE);
		   break;  /* SDL_QUIT */
      }
   }
}

static void cleanup( int ret )
{
   glFreeDisplay();
   glCloseWindow();
   SDL_Quit();

   puts("");
   printf("Exit : %lu\n", _glCore.memory);
   puts("");

   exit( ret );
}

int main( int argc, char **argv )
{
   glObject *obj;

   unsigned int   flags = SDL_OPENGL | SDL_RESIZABLE;
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

   if(glCreateWindow( width, height, bits, flags ) != 0)
      cleanup(EXIT_FAILURE);

   if(glCreateDisplay( width, height, GL_RENDER_DEFAULT ) != 0)
      cleanup(EXIT_FAILURE);

   obj = glNewObject();
   if(!obj)
      cleanup(EXIT_FAILURE);

   puts("");
   printf("Alloc : %lu\n", _glCore.memory);
   puts("");

   glFreeObject(obj);

   /* wait for escape key */
   while(!keyPress(SDLK_ESCAPE))
   {
      keyHandle();
      glSwapBuffers();
   }

   cleanup(EXIT_SUCCESS);
   return(EXIT_SUCCESS);
}
