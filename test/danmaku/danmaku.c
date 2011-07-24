/*
 *
 * Uses ripped images from Touhou 12, so not for public
 *
 */

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

typedef struct
{
   glObject *obj;
   glTexture *texture;

   kmVec2 *coords;

   unsigned int start;
   unsigned int frame;
   unsigned int end;

   int speed;
   int delay;
   int time;
} boss_t;

static void play_boss( boss_t *boss )
{
   boss->time += boss->speed;
   if(boss->time > boss->delay)
   {
      if(++boss->frame==boss->end)
         boss->frame = boss->start;

      memcpy( boss->obj->vbo->uvw[0].coords, boss->coords,
              boss->obj->vbo->uvw[0].c_num * sizeof(kmVec2) );

      puts("FRAME");
      printf("%d\n", boss->frame);

      glShiftObject( boss->obj, 0, 96, 128, boss->frame );

      boss->time = 0;
   }
}

int main( int argc, char **argv )
{
   unsigned int x;

   /* bosses */
   boss_t hijiri;
   hijiri.start = 0;
   hijiri.frame = 0;
   hijiri.end   = 4;
   hijiri.speed = 1;
   hijiri.delay = 4;
   hijiri.time  = 0;

   glTexture *bg_hijiri_tex, *bg2_hijiri_tex;
   glObject *bg_hijiri, *bg2_hijiri;

   float SIZE;
   float scale;

   /* FPS Counter */
   unsigned int   now          = 0;
   unsigned int   last         = 0;
   unsigned int   frameCounter = 0;
   unsigned int   FPS          = 0;
   unsigned int   fpsDelay     = 0;
   float          duration     = 0;
   float          delta        = 0;

   /* Timing */
   const unsigned int TICKS_PER_SECOND = 25;
   const unsigned int SKIP_TICKS       = 1000 / TICKS_PER_SECOND;
   const unsigned int MAX_FRAMESKIP    = 5;

   unsigned int   next_tick      = 0;
   unsigned int   game_loop      = 0;
   float          interpolation  = 0;

   char           WIN_TITLE[LINE_MAX];

   unsigned int   flags = SDL_OPENGL; //| SDL_FULLSCREEN;
   int            width = 2720;
   int            height= 1024;
   int            bits  = 32;
#if defined(GLES1) || defined(GLES2)
   flags = SDL_RESIZABLE;
   width = 800;
   height= 480;
   bits  = 16;
#endif

   SIZE     = 0.15;

   if(SDL_Init(   SDL_INIT_VIDEO    ) != 0)
      cleanup(EXIT_FAILURE);

   if(glCreateWindow( width, height, bits, flags ) != 0)
      cleanup(EXIT_FAILURE);

   if(glCreateDisplay( width, height, GL_RENDER_DEFAULT ) != 0)
      cleanup(EXIT_FAILURE);

   srand( SDL_GetTicks() );

   /* 2D Orthographic projection */
   kmMat4 ortho;
   kmMat4OrthographicProjection( &ortho, 0, width, 0, height, -1, 1 );
   glSetProjection( ortho );

   hijiri.obj = glNewPlane( SIZE * 8, SIZE * 10, 1 );
   if(!hijiri.obj)
      cleanup(EXIT_FAILURE);

   hijiri.texture = glNewTexture( "texture/stage06e01.png", SOIL_FLAG_DEFAULTS );
   if(!hijiri.texture)
      cleanup(EXIT_FAILURE);

   glObjectAddTexture( hijiri.obj, 0, hijiri.texture );
   hijiri.obj->material->flags = GL_MATERIAL_ALPHA;

   hijiri.coords = malloc( hijiri.obj->vbo->uvw[0].c_num * sizeof(kmVec2) );
   memcpy( hijiri.coords, hijiri.obj->vbo->uvw[0].coords,
           hijiri.obj->vbo->uvw[0].c_num * sizeof(kmVec2) );

   glShiftObject( hijiri.obj, 0, 96, 128, 6 );

   bg_hijiri = glNewPlane( SIZE * 34, SIZE * 24, 1 );
   if(!bg_hijiri)
      cleanup(EXIT_FAILURE);

   bg_hijiri_tex = glNewTexture( "texture/stage06e02.png", SOIL_FLAG_DEFAULTS );
   if(!bg_hijiri_tex)
      cleanup(EXIT_FAILURE);

   glObjectAddTexture( bg_hijiri, 0, bg_hijiri_tex );
   glShiftObject( bg_hijiri, 0, 385, 256, 0 );

   bg2_hijiri = glNewPlane( SIZE * 24, SIZE * 24, 1 );
   if(!bg2_hijiri)
      cleanup(EXIT_FAILURE);

   bg2_hijiri_tex = glNewTexture( "texture/stage06e03.png", SOIL_FLAG_DEFAULTS );
   if(!bg2_hijiri_tex)
      cleanup(EXIT_FAILURE);

   glObjectAddTexture( bg2_hijiri, 0, bg2_hijiri_tex );

   puts("");
   printf("Alloc : %.4f MB\n", (float)_glCore.memory / 1048576);
   puts("");

   /* must be done after SDL_init */
   next_tick = SDL_GetTicks();

   /* wait for escape key */
   while(!keyPress(SDLK_ESCAPE))
   {
      last  = now;
      now   = SDL_GetTicks();
      delta = (now - last) / 1000.0f;

      game_loop = 0;
      while( SDL_GetTicks() > next_tick && game_loop < MAX_FRAMESKIP )
      {
         keyHandle();

         play_boss( &hijiri );

         scale += 0.12f;
         glScaleObjectf( bg_hijiri, 100 + sin(scale) * 2, 100 + cos(scale) * 2, 100 );
         glScaleObjectf( bg2_hijiri, 100 + sin(scale) * 5, 100 + sin(scale) * 5, 100 );

         glPositionObjectf( hijiri.obj, width / 4, height - 300 + sin(scale) * 6, 0 );
         glPositionObjectf( bg_hijiri,  width / 4, height - 300 + sin(scale) * 6, 0 );
         glPositionObjectf( bg2_hijiri, width / 4, height - 300 + sin(scale) * 6, 0 );

         next_tick += SKIP_TICKS;
         ++game_loop;
      }

      /* render interpolation */
      interpolation = (SDL_GetTicks() + SKIP_TICKS - next_tick) / (float)SKIP_TICKS;

      glDraw( bg_hijiri );
      glDraw( bg2_hijiri );
      glDraw( hijiri.obj );

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

   glFreeObject(hijiri.obj);
   glFreeObject(bg_hijiri);
   glFreeObject(bg2_hijiri);

   cleanup(EXIT_SUCCESS);
   return(EXIT_SUCCESS);
}
