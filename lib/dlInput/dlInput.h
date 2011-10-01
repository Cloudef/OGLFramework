/* Example Loop */
#if 0
void dlKeyHandle()
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
#endif

#ifndef DL_INPUT_H
#define DL_INPUT_H

#ifdef __cplusplus
extern "C" {
#endif

int   dlKeyInit(void);

void  dlKeyAdd(int key);
void  dlKeyDel(int key);

int   dlKeyHold(int key);
int   dlKeyPress(int key);

#ifdef __cplusplus
}
#endif

#endif /* DL_INPUT_H */

/* EoF */
