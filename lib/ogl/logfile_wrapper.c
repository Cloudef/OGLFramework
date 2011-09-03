#include <stdarg.h>

#include "config.h"
#include "core.h"
#include "logfile.h"

/* open log */
void glLogOpen(void)
{
   if(!GL_LOG_FILE)
      return;

   if(_glCore.disableLog)
      return;

   if(_glCore.log) logClose( _glCore.log );
   _glCore.log = logOpen( GL_LOG_FILE  );
}

/* printf */
void glPrint(const char *output, ... )
{
   va_list args;

   va_start(args, output);
   if(_glCore.disableOut)
      logPrintfv( _glCore.log, output, args );
   else
      logPrintv( _glCore.log, output, args );
   va_end(args);
}

/* puts */
void glPuts(const char *output)
{
   if(_glCore.disableOut)
      logPutsf( _glCore.log, output );
   else
      logPuts( _glCore.log, output );
}

/* close log */
void glLogClose(void)
{
   if(_glCore.log)
      logClose( _glCore.log );

   _glCore.log = NULL;
}

/* enable disable log */
void glDisableLog( int ok )
{
   _glCore.disableLog = ok;

   if(ok)
      glLogOpen();
   else
      glLogClose();
}

/* enable printing to console */
void glDisableOut( int ok )
{
   _glCore.disableOut = ok;
}
