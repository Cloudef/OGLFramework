#include <stdio.h>
#include <limits.h>
#include <SDL/SDL.h>

#include "ogl/ogl.h"
#include "ogl/window.h"
#include "input.h"

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

   /* exit graph */
   glMemoryGraph();

   exit( ret );
}

int main( int argc, char **argv )
{
   float x = 0;

   glCamera *camera;
   glObject *obj = NULL;
#if WITH_PMD
   glObject  *mokou, *reisen, *kaguya;
   glObject  *plane;
#elif WITH_ASSIMP
   glTexture *texture;
   glObject  *obj2, *obj3;
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

   if(glCreateWindow( width, height, bits, flags ) != 0)
      cleanup(EXIT_FAILURE);

   if(glCreateDisplay( width, height, GL_RENDER_DEFAULT ) != 0)
      cleanup(EXIT_FAILURE);

   camera = glNewCamera();
   if(!camera)
      cleanup(EXIT_FAILURE);

#if WITH_PMD
   glPositionCameraf( camera, 0,0,2 );

   plane = glNewPlane( 0.02, 0.02, 1 );
   if(!plane)
      cleanup(EXIT_FAILURE);

   glPositionObjectf( plane, 0.021, -0.01, 0.5f );
   glScaleObjectf( plane, 1, 1, 1 );

   mokou = glNewStaticModel( "model/Mokou/Mokou A.pmd" );
   if(!mokou)
      cleanup(EXIT_FAILURE);

   reisen = glNewStaticModel( "model/Reisen/Reisen.pmd" );
   if(!reisen)
      cleanup(EXIT_FAILURE);

   kaguya = glNewStaticModel( "model/Kaguya/Kaguya.pmd" );
   if(!kaguya)
      cleanup(EXIT_FAILURE);

   glScaleObjectf( reisen, 0.002, 0.002, 0.002 );
   glPositionObjectf( reisen, 0, -0.02, 0 );

   glScaleObjectf( mokou, 0.002, 0.002, 0.002 );
   glPositionObjectf( mokou, 0, -0.02, 0 );

   glScaleObjectf( kaguya, 0.002, 0.002, 0.002 );
   glPositionObjectf( kaguya, 0, -0.02, 0 );

   /* mokou by default */
   obj = mokou;
   glObjectAddTexture( plane, 0, glRefTexture( obj->material->texture[0] ) );
#elif WITH_ASSIMP
   glPositionCameraf( camera, 0,0,15 );

   obj = glNewDynamicModel( "model/lovely.b3d" );
   if(!obj)
      cleanup(EXIT_FAILURE);

   texture = glNewTexture( "model/npc_1.tga",
                  SOIL_FLAG_DEFAULTS        |
                  SOIL_FLAG_TEXTURE_REPEATS );
   if(!texture)
      cleanup(EXIT_FAILURE);

   glObjectAddTexture( obj, 0, texture );

   glScaleObjectf( obj, 0.005, 0.005, 0.005 );
   glPositionObjectf( obj, 0, -0.13, 0 );

   obj2 = glCopyObject( obj );
   if(!obj2)
      cleanup(EXIT_FAILURE);

   obj3 = glCopyObject( obj );
   if(!obj3)
      cleanup(EXIT_FAILURE);

   glMoveObjectf( obj2, 0.22, 0, 0 );
   glMoveObjectf( obj3, -0.22, 0, 0 );
   glRotateObjectf( obj2, 0, 0, 0 );
   glRotateObjectf( obj3, 0, 180, 0 );
#elif WITH_OPENCTM
   glPositionCameraf( camera, 0,0,8 );

   obj = glNewStaticModel( "model/raf22031.ctm" );
   if(!obj)
      cleanup(EXIT_FAILURE);

   glScaleObjectf( obj, 0.04, 0.04, 0.04 );
#endif

   /* here if no import was done */
   if(!obj)
      cleanup(EXIT_FAILURE);

   /* Sets this as active camera */
   glCameraRender( camera );

   /* Startup graph */
   glMemoryGraph();

   /* wait for escape key */
   while(!keyPress(SDLK_ESCAPE))
   {
      last  = now;
      now   = SDL_GetTicks();
      delta = (now - last) / 1000.0f;

      keyHandle();

      x += 0.01f;
#if WITH_PMD
      x += 0.08f;

      if(keyPress(SDLK_1))
         glSetRenderMode( GL_MODE_VERTEX_ARRAY );
      if(keyPress(SDLK_2))
         glSetRenderMode( GL_MODE_VBO );
      if(keyPress(SDLK_3))
      {
         if(obj == kaguya)
            obj = mokou;
         else if(obj == mokou)
            obj = reisen;
         else
            obj = kaguya;

         glObjectFreeTexture( plane, 0 );
         glObjectAddTexture( plane, 0, glRefTexture( obj->material->texture[0] ) );
      }

      glPositionObjectf( obj, 0, -0.02, 0 );
      glRotateObjectf( obj, 0, x, 0 );
      glDraw( obj );

      glRotateObjectf( obj, 0, 0, 0 );
      glPositionObjectf( obj, 0.03, -0.02, 0 );
      glDraw( obj );

      glRotateObjectf( obj, 0, 180, 0 );
      glPositionObjectf( obj, -0.03, -0.02, 0 );
      glDraw( obj );

      if(keyHold(SDLK_4))
         glDraw( plane );
#elif WITH_ASSIMP
      glRotateObjectf( obj, 0, x, 0 );
      glDraw( obj2 );
      glDraw( obj3 );
      glDraw( obj );
      glObjectDrawSkeleton( obj );

      glObjectTick( obj,  x / 10.0f );
      glObjectCalculateAABB( obj );
      glObjectTick( obj2, x / 5.0f );
      glObjectCalculateAABB( obj2 );
      glObjectTick( obj3, x );
      glObjectCalculateAABB( obj3 );

#elif WITH_OPENCTM
      glRotateObjectf( obj, -90, 0, x );
      glDraw( obj );
#endif

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

#if WITH_PMD
   glFreeObject(plane);
   glFreeObject(reisen);
   glFreeObject(mokou);
   glFreeObject(kaguya);
#elif WITH_ASSIMP
   glFreeObject(obj);
   glFreeObject(obj2);
   glFreeObject(obj3);
#elif WITH_OPENCTM
   glFreeObject(obj);
#endif
   glFreeCamera(camera);

   cleanup(EXIT_SUCCESS);
   return(EXIT_SUCCESS);
}
