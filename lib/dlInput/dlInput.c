#include <SDL/SDL.h>
#include "dlTypes.h"

static int _keyPress[SDLK_LAST];
static int _keyHold[SDLK_LAST];

int dlKeyInit(void)
{
   int x = 0;
   for(;x < SDLK_LAST; ++x)
   {
      _keyPress[x] = 0;
      _keyHold[x]  = 0;
   }

   return(RETURN_OK);
}

void dlKeyAdd(SDLKey key)
{
   _keyPress[key]	= 1;
   _keyHold[key]	= 1;
}

void dlKeyDel(SDLKey key)
{
   _keyPress[key]	= 0;
   _keyHold[key]	= 0;
}

int dlKeyHold(SDLKey key)
{
   return( _keyHold[key] );
}

int dlKeyPress(SDLKey key)
{
   int ret = _keyPress[key];
   _keyPress[key] = 0;

   return( ret );
}

/* EoF */
