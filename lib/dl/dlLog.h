#ifndef DL_LOGFILE_WRAPPER
#define DL_LOGFILE_WRAPPER

#include <stdarg.h>
#include "logfile.h"

/* TO-DO: add log channels */
#define TRACE()           #ifdef DEBUG LOGWARNP("CALL", "%s()", __FUNCTION__);                        #endif
#define CALL(a,...)       #ifdef DEBUG LOGWARNP("CALL", "%s("a")", __FUNCTION__, ##__VA_ARGS__);      #endif
#define RET(a,...)        #ifdef DEBUG LOGWARNP("RET",  "%s => ("a")", __FUNCTION__, ##__VA_ARGS__);  #endif
#define LOGERR(e,m)       logRed();    dlPuts("["e"] "m);                     logNormal();
#define LOGERRP(e,m,...)  logRed();    dlPrint("["e"] "m"\n",##__VA_ARGS__);  logNormal();
#define LOGWARN(e,m)      logYellow(); dlPuts("["e"] "m);                     logNormal();
#define LOGWARNP(e,m,...) logYellow(); dlPrint("["e"] "m"\n",##__VA_ARGS__);  logNormal();
#define LOGINFO(e,m)      logBlue();   dlPuts("["e"] "m);                     logNormal();
#define LOGINFOP(e,m,...) logBlue();   dlPrint("["e"] "m"\n",##__VA_ARGS__);  logNormal();
#define LOGOK(e,m)        logGreen();  dlPuts("["e"] "m);                     logNormal();
#define LOGOKP(e,m,...)   logGreen();  dlPrint("["e"] "m"\n",##__VA_ARGS__);  logNormal();

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
