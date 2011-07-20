#include <stdio.h>
#include <limits.h>
#include <SDL/SDL.h>

#include "ogl/ogl.h"
#include "ogl/window.h"
#include "input.h"

#ifdef WIN32
#define LINE_MAX 254
#endif

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

   exit( ret );
}

int main( int argc, char **argv )
{
   float x = 0;
   glCamera *camera;
   glObject *obj, *obj2, *obj3;
   glTexture *texture;

   /* FPS Counter */
   unsigned int   now          = SDL_GetTicks();
   unsigned int   last         = 0;
   unsigned int   frameCounter = 0;
   unsigned int   FPS          = 0;
   unsigned int   fpsDelay     = 0;
   float          duration     = 0;
   float          delta        = 0;

   char           WIN_TITLE[LINE_MAX];

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

   camera = glNewCamera();
   if(!camera)
      cleanup(EXIT_FAILURE);

   /* Sets this as active camera */
   glCameraRender( camera );

   texture = glNewTexture( "model/test.png", 0 );
   if(!texture)
      cleanup(EXIT_FAILURE);

   obj = glNewPlane( 0.005, 0.005, 1 );
   if(!obj)
      cleanup(EXIT_FAILURE);

   /* Add texture to plane
    * note: This steals the reference of texture object, so you don't have to free it.
    * If you want to still have reference to the texture after object dies, use glRefTexture( texture ); */
   glObjectAddTexture( obj, 0, texture );

   obj2 = glCopyObject( obj );
   if(!obj2)
      cleanup(EXIT_FAILURE);

   obj3 = glCopyObject( obj );
   if(!obj3)
      cleanup(EXIT_FAILURE);

   /* Move objects a bit */
   glMoveObjectf( obj, -0.5, 0, 0 );
   glMoveObjectf( obj2, 0.5, 0, 0 );

   puts("");
   printf("Alloc : %.4f MB\n", (float)_glCore.memory / 1048576);
   puts("");

   /* wait for escape key */
   while(!keyPress(SDLK_ESCAPE))
   {
      last  = now;
      now   = SDL_GetTicks();
      delta = (now - last) / 1000.0f;

      keyHandle();

      x += 1.0f;
      glRotateObjectf( obj3, 0, x, 0 );

      glDraw( obj  );
      glDraw( obj2 );
      glDraw( obj3 );

      glSwapBuffers();
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
   glFreeObject(obj);
   glFreeObject(obj2);
   glFreeObject(obj3);
   glFreeCamera(camera);

   cleanup(EXIT_SUCCESS);
   return(EXIT_SUCCESS);
}
