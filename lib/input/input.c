#include <SDL/SDL.h>
#include "types.h"

static int _keyPress[SDLK_LAST];
static int _keyHold[SDLK_LAST];

int keyInit(void)
{
   int x = 0;
   for(;x < SDLK_LAST; ++x)
   {
      _keyPress[x] = 0;
      _keyHold[x]  = 0;
   }

   return(RETURN_OK);
}

void keyAdd(SDLKey key)
{
   _keyPress[key]	= 1;
   _keyHold[key]	= 1;
}

void keyDel(SDLKey key)
{
   _keyPress[key]	= 0;
   _keyHold[key]	= 0;
}

int keyHold(SDLKey key)
{
   return( _keyHold[key] );
}

int keyPress(SDLKey key)
{
   int ret = _keyPress[key];
   _keyPress[key] = 0;

   return( ret );
}

/* EoF */
