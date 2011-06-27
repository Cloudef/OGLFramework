/* Example Loop */
#if 0
void keyHandle()
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
#endif

#ifndef INPUT_H
#define INPUT_H

#ifdef __cplusplus
extern "C" {
#endif

int   keyInit(void);

void  keyAdd(int key);
void  keyDel(int key);

int   keyHold(int key);
int   keyPress(int key);

#ifdef __cplusplus
}
#endif

#endif /* INPUT_H */

/* EoF */
