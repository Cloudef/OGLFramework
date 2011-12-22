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
   float x = 0;
   dlCamera *camera;
   dlObject *obj, *obj2, *obj3;
   dlTexture *texture;

   /* FPS Counter */
   unsigned int   now          = 0;
   unsigned int   last         = 0;
   unsigned int   frameCounter = 0;
   unsigned int   FPS          = 0;
   unsigned int   fpsDelay     = 0;
   float          duration     = 0;
   float          delta        = 0;

   char           WIN_TITLE[LINE_MAX];

   /* init debug channels */
   dlDEBINIT(argc, argv);

   if(SDL_Init(   SDL_INIT_VIDEO    ) != 0)
      cleanup(EXIT_FAILURE);

   if(dlCreateWindow( WINDOW_WIDTH, WINDOW_HEIGHT, WINDOW_BITS, WINDOW_FLAGS ) != 0)
      cleanup(EXIT_FAILURE);

   if(dlCreateDisplay( WINDOW_WIDTH, WINDOW_HEIGHT, DL_RENDER_OGL140 ) != 0)
      cleanup(EXIT_FAILURE);

   /* create camera */
   camera = dlNewCamera();
   if(!camera)
      cleanup(EXIT_FAILURE);

   texture = dlNewTexture( "model/test.png", SOIL_FLAG_DEFAULTS );
   if(!texture)
      cleanup(EXIT_FAILURE);

   obj = dlNewPlane( 0.1, 0.1, 1 );
   if(!obj)
      cleanup(EXIT_FAILURE);

   /* Add texture to plane
    * note: This steals the reference of texture object, so you don't have to free it.
    * If you want to still have reference to the texture after object dies, use glRefTexture( texture ); */
   obj->material = dlNewMaterialFromTexture( texture );
   if(!obj->material)
      cleanup(EXIT_FAILURE);

   obj2 = dlCopyObject( obj );
   if(!obj2)
      cleanup(EXIT_FAILURE);

   obj3 = dlCopyObject( obj );
   if(!obj3)
      cleanup(EXIT_FAILURE);

   /* Move objects a bit */
   dlMoveObjectf( obj, -10, 0, 0 );
   dlMoveObjectf( obj2, 10, 0, 0 );

   /* make center object transparent */
   obj3->material->flags = DL_MATERIAL_ALPHA;

   /* startup graph */
   dlMemoryGraph();

   /* wait for escape key */
   while(!dlKeyPress(SDLK_ESCAPE))
   {
      last  = now;
      now   = SDL_GetTicks();
      delta = (now - last) / 1000.0f;

      /* sets this as active camera */
      dlCameraRender( camera );
      keyHandle();

      x += 1.0f;
      dlRotateObjectf( obj3, 0, x, 0 );

      dlDraw( obj  );
      dlDraw( obj2 );
      dlDraw( obj3 );

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
   dlFreeObject(obj);
   dlFreeObject(obj2);
   dlFreeObject(obj3);
   dlFreeCamera(camera);

   cleanup(EXIT_SUCCESS);
   return(EXIT_SUCCESS);
}
