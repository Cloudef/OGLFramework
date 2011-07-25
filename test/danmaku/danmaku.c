/*
 *
 * Uses ripped images from Touhou 12, so media files are not in git
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

   int width, height;

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

      glShiftObject( boss->obj, 0, boss->width, boss->height, boss->frame );

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

   hijiri.width  = 96;
   hijiri.height = 128;

   boss_t nue;
   nue.start = 0;
   nue.frame = 0;
   nue.end   = 4;
   nue.speed = 1;
   nue.delay = 5;
   nue.time  = 0;

   nue.width  = 96;
   nue.height = 104;

   boss_t reimu;
   reimu.start = 0;
   reimu.frame = 0;
   reimu.end   = 8;
   reimu.speed = 1;
   reimu.delay = 4;
   reimu.time  = 0;

   reimu.width  = 32;
   reimu.height = 48;

   boss_t marisa;
   marisa.start = 0;
   marisa.frame = 0;
   marisa.end   = 8;
   marisa.speed = 1;
   marisa.delay = 4;
   marisa.time  = 0;

   marisa.width  = 32;
   marisa.height = 48;

   glTexture *bg_hijiri_tex, *bg2_hijiri_tex;
   glObject *bg_hijiri, *bg2_hijiri;

   glTexture *bg_nue_tex;
   glObject *bg_nue;

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

   unsigned int   flags = SDL_OPENGL; // | SDL_FULLSCREEN;
   int            width = 1024;
   int            height= 768;
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

   glShiftObject( hijiri.obj, 0, hijiri.width, hijiri.height, 6 );

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

   nue.obj = glNewPlane( SIZE * 8, SIZE * 10, 1 );
   if(!nue.obj)
      cleanup(EXIT_FAILURE);

   nue.texture = glNewTexture( "texture/stg7enm.png", SOIL_FLAG_DEFAULTS );
   if(!nue.texture)
      cleanup(EXIT_FAILURE);

   glObjectAddTexture( nue.obj, 0, nue.texture );
   nue.coords = malloc( nue.obj->vbo->uvw[0].c_num * sizeof(kmVec2) );
   memcpy( nue.coords, nue.obj->vbo->uvw[0].coords,
           nue.obj->vbo->uvw[0].c_num * sizeof(kmVec2) );

   glShiftObject( nue.obj, 0, nue.width, nue.height, 0 );

   bg_nue = glNewPlane( SIZE * 24, SIZE * 24, 1 );
   if(!bg_nue)
      cleanup(EXIT_FAILURE);

   bg_nue_tex = glNewTexture( "texture/etama2.png", SOIL_FLAG_DEFAULTS );
   if(!bg_nue_tex)
      cleanup(EXIT_FAILURE);

   glObjectAddTexture( bg_nue, 0, bg_nue_tex );
   glOffsetObjectTexture( bg_nue, 0, 128, 49, 128, 127 );

   reimu.obj = glNewPlane( SIZE * 2, SIZE * 3, 1 );
   if(!reimu.obj)
      cleanup(EXIT_FAILURE);

   reimu.texture = glNewTexture( "texture/pl00.png", SOIL_FLAG_DEFAULTS );
   if(!reimu.texture)
      cleanup(EXIT_FAILURE);

   glObjectAddTexture( reimu.obj, 0, reimu.texture );
   reimu.obj->material->flags = GL_MATERIAL_ALPHA;

   reimu.coords = malloc( reimu.obj->vbo->uvw[0].c_num * sizeof(kmVec2) );
   memcpy( reimu.coords, reimu.obj->vbo->uvw[0].coords,
           reimu.obj->vbo->uvw[0].c_num * sizeof(kmVec2) );

   glShiftObject( reimu.obj, 0, reimu.width, reimu.height, 6 );

   marisa.obj = glNewPlane( SIZE * 2, SIZE * 3, 1 );
   if(!marisa.obj)
      cleanup(EXIT_FAILURE);

   marisa.texture = glNewTexture( "texture/pl01.png", SOIL_FLAG_DEFAULTS );
   if(!marisa.texture)
      cleanup(EXIT_FAILURE);

   glObjectAddTexture( marisa.obj, 0, marisa.texture );
   marisa.obj->material->flags = GL_MATERIAL_ALPHA;

   marisa.coords = malloc( marisa.obj->vbo->uvw[0].c_num * sizeof(kmVec2) );
   memcpy( marisa.coords, marisa.obj->vbo->uvw[0].coords,
           marisa.obj->vbo->uvw[0].c_num * sizeof(kmVec2) );

   glShiftObject( marisa.obj, 0, marisa.width, marisa.height, 6 );

   glPositionObjectf( reimu.obj, width / 4, 120, 0 );
   glPositionObjectf( marisa.obj, width - width / 4, 160, 0 );

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
         play_boss( &nue );
         play_boss( &reimu );
         play_boss( &marisa );

         scale += 0.12f;
         glScaleObjectf( bg_hijiri, 100 + sin(scale) * 2, 100 + cos(scale) * 2, 100 );
         glScaleObjectf( bg2_hijiri, 100 + sin(scale) * 5, 100 + sin(scale) * 5, 100 );

         glPositionObjectf( hijiri.obj, width / 4, height - 320 + sin(scale) * 6, 0 );
         glPositionObjectf( bg_hijiri,  width / 4, height - 320 + sin(scale) * 6, 0 );
         glPositionObjectf( bg2_hijiri, width / 4, height - 320 + sin(scale) * 6, 0 );

         glPositionObjectf( bg_nue, width - width / 4, height - 220, 0 );
         glPositionObjectf( nue.obj, width - width / 4, height - 220 + cos(scale) * 6, 0 );

         glScaleObjectf( bg_nue, 100 + sin(scale), 100 + sin(scale), 100 );
         glRotateObjectf( bg_nue, 0, 0, scale );

         next_tick += SKIP_TICKS;
         ++game_loop;
      }

      /* render interpolation */
      interpolation = (SDL_GetTicks() + SKIP_TICKS - next_tick) / (float)SKIP_TICKS;

      glDraw( bg_hijiri );
      glDraw( bg2_hijiri );
      glDraw( hijiri.obj );

      glDraw( bg_nue );
      glDraw( nue.obj );

      glDraw( reimu.obj );
      glDraw( marisa.obj );

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

   glFreeObject(nue.obj);
   glFreeObject(bg_nue);

   glFreeObject(reimu.obj);
   glFreeObject(marisa.obj);

   free(hijiri.coords);
   free(nue.coords);
   free(reimu.coords);
   free(marisa.coords);

   cleanup(EXIT_SUCCESS);
   return(EXIT_SUCCESS);
}
