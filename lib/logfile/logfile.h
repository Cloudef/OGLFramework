#ifndef LOGFILE_H
#define LOGFILE_H

#include "stdio.h"

#ifdef __cplusplus
extern "C" {
#endif

/* logging */
FILE* logOpen( const char *path );
void logPrint( FILE*, const char *output, ... );
void logPrintf( FILE*, const char* output, ... );
void logPrintv( FILE*, const char *output, va_list args );
void logPrintfv( FILE*, const char *output, va_list args );
void logPuts( FILE*, const char *output );
void logPutsf( FILE*, const char *output );
void logClose( FILE* );

/* colors */
void logRed(void);
void logGreen( void );
void logYellow( void );
void logBlue( void );
void logWhite( void );
void logNormal( void );

#ifdef __cplusplus
}
#endif

#endif /* LOGFILE_H */

/* EoF */
