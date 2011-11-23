#include <stdarg.h>

#include "dlConfig.h"
#include "dlCore.h"
#include "logfile.h"

#define DL_LOG_FORMAT "[%s] "
#define LENGTH(X) (sizeof X / sizeof X[0])

typedef struct DEBCHAN
{
   char    channel[24];
   uint8_t active;
} DEBCHAN;

/* list all debug channels */
static DEBCHAN DEBUG_CHANNELS[] =
{
   /* CORE => */
   { "ALLOC",           0 },
   { "ATLAS",           0 },
   { "CAMERA",          0 },
   { "GL",              0 },
   { "IBO",             0 },
   { "LOG",             0 },
   { "MATERIAL",        0 },
   { "SCENEOBJECT",     0 },
   { "TEXTURE",         0 },
   { "VBO",             0 },
   /* <= */

   /* ANIMATION => */
   { "ANIM",            0 },
   { "ANIMATOR",        0 },
   { "BONE",            0 },
   { "EVALUATOR",       0 },
   /* <= */

   /* SHADER => */
   { "SHADER",          0 },
   /* <= */

   /* RENDER => */
   { "OGL140",          0 },
   { "OGL3",            0 },
   /* <= */

   /* IMPORT => */
   { "IMPORT",          0 },
   { "IMPORT_ASSIMP",   0 },
   { "IMPORT_OCTM",     0 },
   { "IMPORT_PMD",      0 },
   /* <= */

   /* GEOMETRY => */
   { "DYNAMICMODEL",    0 },
   { "PLANE",           0 },
   { "SPRITE",          0 },
   { "STATICMODEL",     0 },
   /* <= */

   /* TRACING */
   { "TRACE",           0 },
};

/* do tracing? */
int dlDEBTRACE(void)
{
   if(DEBUG_CHANNELS[ LENGTH(DEBUG_CHANNELS) - 1 ].active) return(1);
   return(0);
}

/* check if channel is active */
int dlDEBCHAN(const char *channel)
{
   unsigned int i;

   i = 0;
   for(; i != LENGTH(DEBUG_CHANNELS); ++i)
      if(!strcmp(channel, DEBUG_CHANNELS[i].channel) &&
         DEBUG_CHANNELS[i].active) return(1);

   return(0);
}

/* active a debug channel */
void dlDEBADD(const char *channel)
{
   unsigned int i;

   i = 0;
   for(; DEBUG_CHANNELS[i].channel; ++i)
      if(!strcmp(channel, DEBUG_CHANNELS[i].channel))
      { DEBUG_CHANNELS[i].active = 1; return; }

   return;
}

/* deactive a debug channel */
void dlDEBRM(const char *channel)
{
   unsigned int i;

   i = 0;
   for(; DEBUG_CHANNELS[i].channel; ++i)
      if(!strcmp(channel, DEBUG_CHANNELS[i].channel))
      { DEBUG_CHANNELS[i].active = 0; return; }

   return;
}

/* open log */
void dlLogOpen(void)
{
   if(!DL_LOG_FILE)
      return;

   if(_dlCore.disableLog)
      return;

   if(_dlCore.log) logClose( _dlCore.log );
   _dlCore.log = logOpen( DL_LOG_FILE  );
}

/* debug print */
void dlDPrint(const char *channel, const char *output, ... )
{
   va_list args;

   if(!dlDEBCHAN(channel)) return;

   if(_dlCore.disableOut)
   {
      logPrintf( _dlCore.log, DL_LOG_FORMAT, channel );

      va_start(args, output);
      logPrintfv( _dlCore.log, output, args );
      va_end(args);
   }
   else
   {
      logPrint( _dlCore.log, DL_LOG_FORMAT, channel );

      va_start(args, output);
      logPrintv( _dlCore.log, output, args );
      va_end(args);
   }
}

/* printf */
void dlPrint(const char *output, ... )
{
   va_list args;

   if(_dlCore.disableOut)
   {
      va_start(args, output);
      logPrintfv( _dlCore.log, output, args );
      va_end(args);
   }
   else
   {
      va_start(args, output);
      logPrintv( _dlCore.log, output, args );
      va_end(args);
   }
}

/* debug puts */
void dlDPuts(const char *channel, const char *output)
{
   if(!dlDEBCHAN(channel)) return;

   if(_dlCore.disableOut)
   {
      logPrintf( _dlCore.log, DL_LOG_FORMAT, channel );
      logPutsf( _dlCore.log, output );
   }
   else
   {
      logPrint( _dlCore.log, DL_LOG_FORMAT, channel );
      logPuts( _dlCore.log, output );
   }
}

/* puts */
void dlPuts(const char *output)
{
   if(_dlCore.disableOut)
      logPutsf( _dlCore.log, output );
   else
      logPuts( _dlCore.log, output );
}

/* close log */
void dlLogClose(void)
{
   if(_dlCore.log)
      logClose( _dlCore.log );

   _dlCore.log = NULL;
}

/* enable disable log */
void dlDisableLog( int ok )
{
   _dlCore.disableLog = ok;

   if(ok)
      dlLogOpen();
   else
      dlLogClose();
}

/* enable printing to console */
void dlDisableOut( int ok )
{
   _dlCore.disableOut = ok;
}
