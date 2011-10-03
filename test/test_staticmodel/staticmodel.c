#include <stdio.h>
#include <limits.h>
#include <SDL/SDL.h>

#include "DL/dl.h"
#include "DL/dlWindow.h"
#include "DL/dlInput.h"

#include "kazmath/kazmath.h"

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

   unsigned int   flags = SDL_OPENGL | SDL_RESIZABLE;
   int            width = 800;
   int            height= 512;
   int            bits  = 32;
#if defined(GLES1) || defined(GLES2)
   flags = SDL_RESIZABLE;
   width = 600;
   height= 320;
   bits  = 16;
#endif

   if(SDL_Init(   SDL_INIT_VIDEO    ) != 0)
      cleanup(EXIT_FAILURE);

   if(dlCreateWindow( width, height, bits, flags ) != 0)
      cleanup(EXIT_FAILURE);

   if(dlCreateDisplay( width, height, DL_RENDER_DEFAULT ) != 0)
      cleanup(EXIT_FAILURE);

   camera = dlNewCamera();
   if(!camera)
      cleanup(EXIT_FAILURE);

#if WITH_PMD
   dlPositionCameraf( camera, 0,0,2 );

   plane = dlNewPlane( 0.02, 0.02, 1 );
   if(!plane)
      cleanup(EXIT_FAILURE);

   dlPositionObjectf( plane, 0.021, -0.01, 0.5f );
   dlScaleObjectf( plane, 1, 1, 1 );

   mokou = dlNewStaticModel( "model/Mokou/Mokou A.pmd" );
   if(!mokou)
      cleanup(EXIT_FAILURE);

   /*
   reisen = dlNewStaticModel( "model/Reisen/Reisen.pmd" );
   if(!reisen)
      cleanup(EXIT_FAILURE);

   kaguya = dlNewStaticModel( "model/Kaguya/Kaguya.pmd" );
   if(!kaguya)
      cleanup(EXIT_FAILURE);
   */
   dlScaleObjectf( mokou, 0.002, 0.002, 0.002 );
   dlPositionObjectf( mokou, 0, -0.02, 0 );

   /*
   dlScaleObjectf( reisen, 0.002, 0.002, 0.002 );
   dlPositionObjectf( reisen, 0, -0.02, 0 );

   dlScaleObjectf( kaguya, 0.002, 0.002, 0.002 );
   dlPositionObjectf( kaguya, 0, -0.02, 0 );
   */

   /* mokou by default */
   obj = mokou;
   dlObjectAddTexture( plane, 0, dlRefTexture( obj->material->texture[0] ) );
#elif WITH_ASSIMP
   dlPositionCameraf( camera, 0,0,15 );

   obj = dlNewDynamicModel( "model/lovely.b3d" );
   if(!obj)
      cleanup(EXIT_FAILURE);

   texture = dlNewTexture( "model/npc_1.tga",
                  SOIL_FLAG_DEFAULTS        |
                  SOIL_FLAG_TEXTURE_REPEATS );
   if(!texture)
      cleanup(EXIT_FAILURE);

   dlObjectAddTexture( obj, 0, texture );

   dlScaleObjectf( obj, 0.005, 0.005, 0.005 );
   dlPositionObjectf( obj, 0, -0.13, 0 );

   obj2 = dlCopyObject( obj );
   if(!obj2)
      cleanup(EXIT_FAILURE);

   obj3 = dlCopyObject( obj );
   if(!obj3)
      cleanup(EXIT_FAILURE);

   dlMoveObjectf( obj2, 0.22, 0, 0 );
   dlMoveObjectf( obj3, -0.22, 0, 0 );
   dlRotateObjectf( obj2, 0, 0, 0 );
   dlRotateObjectf( obj3, 0, 180, 0 );
#elif WITH_OPENCTM
   dlPositionCameraf( camera, 0,0,8 );

   obj = dlNewStaticModel( "model/raf22031.ctm" );
   if(!obj)
      cleanup(EXIT_FAILURE);

   dlScaleObjectf( obj, 0.04, 0.04, 0.04 );
#endif

   /* here if no import was done */
   if(!obj)
      cleanup(EXIT_FAILURE);

   /* Sets this as active camera */
   dlCameraRender( camera );

   /* Startup graph */
   dlMemoryGraph();

   /* wait for escape key */
   while(!dlKeyPress(SDLK_ESCAPE))
   {
      last  = now;
      now   = SDL_GetTicks();
      delta = (now - last) / 1000.0f;

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

         dlObjectFreeTexture( plane, 0 );
         dlObjectAddTexture( plane, 0, dlRefTexture( obj->material->texture[0] ) );
      }

      dlPositionObjectf( obj, 0, -0.02, 0 );
      dlRotateObjectf( obj, 0, x, 0 );
      dlDraw( obj );

      dlRotateObjectf( obj, 0, 0, 0 );
      dlPositionObjectf( obj, 0.03, -0.02, 0 );
      dlDraw( obj );

      dlRotateObjectf( obj, 0, 180, 0 );
      dlPositionObjectf( obj, -0.03, -0.02, 0 );
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
   /*
   dlFreeObject(reisen);
   dlFreeObject(kaguya);
   */
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
