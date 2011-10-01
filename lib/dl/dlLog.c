#include <stdarg.h>

#include "dlConfig.h"
#include "dlCore.h"
#include "logfile.h"

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

/* printf */
void dlPrint(const char *output, ... )
{
   va_list args;

   va_start(args, output);
   if(_dlCore.disableOut)
      logPrintfv( _dlCore.log, output, args );
   else
      logPrintv( _dlCore.log, output, args );
   va_end(args);
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
