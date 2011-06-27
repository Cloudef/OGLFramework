#include <stdio.h>
#include <stdarg.h>

/* open log */
FILE* logOpen( const char *path )
{
   FILE *log_file;

   log_file = fopen( path, "w" );
   if(!log_file)
   {
      printf("could not open %s for writing\n", path);
      return( NULL );
   }

   fputs("--------- LOG STARTED ---------", log_file);
   fputs("\n\n", log_file);

   fflush( log_file  );
   return( log_file );
}

/* print to log and stdout */
void logPrint( FILE *log_file, const char *output, ... )
{
   va_list args;
   va_start(args, output);
   vfprintf( stdout, output, args );
   va_end(args);

   if(!log_file)
      return;

   va_start(args, output);
   vfprintf( log_file, output, args );
   va_end(args);

   fflush( log_file );
}

/* print to log  */
void logPrintf( FILE *log_file, const char *output, ... )
{
   va_list args;
   if(!log_file)
      return;

   va_start(args, output);
   vfprintf( log_file, output, args );
   va_end(args);
}

/* print to log and stdout using va_list */
void logPrintv( FILE *log_file, const char *output, va_list args )
{
   va_list args2;
#ifdef __va_copy
   __va_copy(args2, args);
#else
   args2 = args;
#endif

   vfprintf( stdout, output, args );

   if(!log_file)
      return;

   vfprintf( log_file, output, args2 );

   fflush( log_file );
}

/* print to log using va_list */
void logPrintfv( FILE *log_file, const char *output, va_list args )
{
   if(!log_file)
      return;

   vfprintf( log_file, output, args );

   fflush( log_file );
}

/* puts to log and stdout */
void logPuts( FILE *log_file, const char *output )
{
   puts( output );

   if(!log_file)
      return;

   fputs( output, log_file );
   fputs( "\n", log_file );

   fflush( log_file );
}

/* puts to log */
void logPutsf( FILE *log_file, const char *output )
{
   if(!log_file)
      return;

   fputs( output, log_file );
   fputs( "\n", log_file );

   fflush( log_file );
}

/* log close */
void logClose( FILE *log_file )
{
   if(!log_file)
      return;

   fputs("\n", log_file);
   fputs("--------- LOG CLOSED ---------", log_file);
   fclose( log_file );
   log_file = NULL;
}

/* EoF */
