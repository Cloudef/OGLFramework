#include <stdio.h>
#include <limits.h>
#include <SDL/SDL.h>

#include "DL/dl.h"
#include "DL/dlWindow.h"
#include "DL/dlInput.h"
#include "logfile.h"

#include "kazmath/kazmath.h"

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
   dlObject *obj = NULL;
#if WITH_PMD
   dlObject  *mokou, *reisen, *kaguya;
   dlObject  *plane;
#elif WITH_ASSIMP
   dlTexture *texture;
   dlObject  *obj2, *obj3;
#elif WITH_OPENCTM

#endif

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

   camera = dlNewCamera();
   if(!camera)
      cleanup(EXIT_FAILURE);

#if WITH_PMD
   dlPositionCameraf( camera, 0,10,45 );
   dlTargetCameraf( camera, 0,10,0 );

   plane = dlNewPlane( 0.08, 0.08, 0 );
   if(!plane)
      cleanup(EXIT_FAILURE);

   mokou = dlNewStaticModel( "model/Mokou/Mokou A.pmd" );
   if(!mokou)
      cleanup(EXIT_FAILURE);

   reisen = dlNewStaticModel( "model/Reisen/Reisen.pmd" );
   if(!reisen)
      cleanup(EXIT_FAILURE);

   kaguya = dlNewStaticModel( "model/Kaguya/Kaguya.pmd" );
   if(!kaguya)
      cleanup(EXIT_FAILURE);

   dlScaleObjectf( mokou,  1, 1, 1 );
   dlScaleObjectf( reisen, 1, 1, 1 );
   dlScaleObjectf( kaguya, 1, 1, 1 );

   dlPositionObjectf( plane, 10.35,-1,10 );

   /* mokou by default */
   obj = mokou;
   plane->material = dlNewMaterialFromTexture( dlRefTexture( obj->material->texture ) );
#elif WITH_ASSIMP
   dlPositionCameraf( camera, 0,8,45 );
   dlTargetCameraf( camera, 0,8,0 );

   obj = dlNewDynamicModel( "model/lovely.b3d" );
   if(!obj)
      cleanup(EXIT_FAILURE);

   obj->material = dlNewMaterialFromImage( "model/npc_1.tga",
                                            SOIL_FLAG_DEFAULTS        |
                                            SOIL_FLAG_TEXTURE_REPEATS );
   if(!obj->material)
      cleanup(EXIT_FAILURE);

   dlScaleObjectf( obj, 0.35, 0.35, 0.35 );

   obj2 = dlCopyObject( obj );
   if(!obj2)
      cleanup(EXIT_FAILURE);

   obj3 = dlCopyObject( obj );
   if(!obj3)
      cleanup(EXIT_FAILURE);

   dlMoveObjectf( obj2,   15, 0, 0 );
   dlMoveObjectf( obj3,  -15, 0, 0 );
   dlRotateObjectf( obj2, 0,  0, 0 );
   dlRotateObjectf( obj3, 0, 180, 0 );
#elif WITH_OPENCTM
   dlPositionCameraf( camera, 0,0,15 );

   obj = dlNewStaticModel( "model/raf22031.ctm" );
   if(!obj)
      cleanup(EXIT_FAILURE);

   dlScaleObjectf( obj, 3, 3, 3 );
#endif

   /* here if no import was done */
   if(!obj)
      cleanup(EXIT_FAILURE);

   /* Startup graph */
   dlMemoryGraph();

   /* wait for escape key */
   while(!dlKeyPress(SDLK_ESCAPE))
   {
      last  = now;
      now   = SDL_GetTicks();
      delta = (now - last) / 1000.0f;

      dlCameraRender( camera );
      keyHandle();

      x += 0.01f;
#if WITH_PMD
      x += 0.08f;

      if(dlKeyPress(SDLK_1))
         dlSetRenderMode( DL_MODE_VERTEX_ARRAY );
      if(dlKeyPress(SDLK_2))
         dlSetRenderMode( DL_MODE_VBO );
      if(dlKeyPress(SDLK_3))
      {
         if(obj == kaguya)
            obj = mokou;
         else if(obj == mokou)
            obj = reisen;
         else
            obj = kaguya;

         dlFreeTexture( plane->material->texture );
         plane->material->texture = dlRefTexture( obj->material->texture );
      }

      dlPositionObjectf( obj, 0, 0, 0 );
      dlRotateObjectf( obj, 0, x, 0 );
      dlDraw( obj );

      dlRotateObjectf( obj, 0, 0, 0 );
      dlPositionObjectf( obj, 15, 0, 0 );
      dlDraw( obj );

      dlRotateObjectf( obj, 0, 180, 0 );
      dlPositionObjectf( obj, -15, 0, 0 );
      dlDraw( obj );

      if(dlKeyHold(SDLK_4))
         dlDraw( plane );
#elif WITH_ASSIMP
      dlRotateObjectf( obj, 0, x, 0 );
      dlDraw( obj2 );
      dlDraw( obj3 );
      dlDraw( obj );
      dlObjectDrawSkeleton( obj );

      dlObjectTick( obj,  x / 10.0f );
      dlObjectCalculateAABB( obj );
      dlObjectTick( obj2, x / 5.0f );
      dlObjectCalculateAABB( obj2 );
      dlObjectTick( obj3, x );
      dlObjectCalculateAABB( obj3 );

#elif WITH_OPENCTM
      dlRotateObjectf( obj, -90, 0, x );
      dlDraw( obj );
#endif

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

#if WITH_PMD
   dlFreeObject(plane);
   dlFreeObject(mokou);
   dlFreeObject(reisen);
   dlFreeObject(kaguya);
#elif WITH_ASSIMP
   dlFreeObject(obj);
   dlFreeObject(obj2);
   dlFreeObject(obj3);
#elif WITH_OPENCTM
   dlFreeObject(obj);
#endif
   dlFreeCamera(camera);

   cleanup(EXIT_SUCCESS);
   return(EXIT_SUCCESS);
}
