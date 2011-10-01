#ifndef DL_LOGFILE_WRAPPER
#define DL_LOGFILE_WRAPPER

#include <stdarg.h>
#include "logfile.h"

#ifdef __cplusplus
extern "C" {
#endif

void dlLogOpen(void);
void dlPrint(const char* output, ...);
void dlPuts(const char* output);
void dlLogClose(void);

#ifdef __cplusplus
}
#endif

#endif /* DL_LOGFILE_WRAPPER */
