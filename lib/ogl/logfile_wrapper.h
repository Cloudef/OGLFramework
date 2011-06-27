#ifndef GL_LOGFILE_WRAPPER
#define GL_LOGFILE_WRAPPER

#include <stdarg.h>
#include "logfile.h"

#ifdef __cplusplus
extern "C" {
#endif

void glLogOpen(void);
void glPrint(const char* output, ...);
void glPuts(const char* output);
void glLogClose(void);

#ifdef __cplusplus
}
#endif

#endif /* GL_LOGFILE_WRAPPER */
