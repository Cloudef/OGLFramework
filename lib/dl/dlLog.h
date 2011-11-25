#ifndef DL_LOGFILE_WRAPPER
#define DL_LOGFILE_WRAPPER

#include <stdarg.h>
#include "logfile.h"

/* TO-DO: add log channels */
#define TRACE()         if(dlDEBTRACE()) { LOGWARNP("%s()", __FUNCTION__); }
#define CALL(a,...)     if(dlDEBTRACE()) { LOGWARNP("%s("a")", __FUNCTION__, ##__VA_ARGS__); }
#define RET(a,...)      if(dlDEBTRACE()) { LOGWARNP("%s => ("a")", __FUNCTION__, ##__VA_ARGS__); }
#define LOGERR(m)       logRed();    dlPuts(m);                                       logNormal();
#define LOGERRP(m,...)  logRed();    dlPrint(m"\n",##__VA_ARGS__);                    logNormal();
#define LOGFREE(m)      logRed();    dlDPuts(DL_DEBUG_CHANNEL,m);                     logNormal();
#define LOGFREEP(m,...) logRed();    dlDPrint(DL_DEBUG_CHANNEL,m"\n",##__VA_ARGS__);  logNormal();
#define LOGWARN(m)      logYellow(); dlDPuts(DL_DEBUG_CHANNEL,m);                     logNormal();
#define LOGWARNP(m,...) logYellow(); dlDPrint(DL_DEBUG_CHANNEL,m"\n",##__VA_ARGS__);  logNormal();
#define LOGINFO(m)      logBlue();   dlDPuts(DL_DEBUG_CHANNEL,m);                     logNormal();
#define LOGINFOP(m,...) logBlue();   dlDPrint(DL_DEBUG_CHANNEL,m"\n",##__VA_ARGS__);  logNormal();
#define LOGOK(m)        logGreen();  dlDPuts(DL_DEBUG_CHANNEL,m);                     logNormal();
#define LOGOKP(m,...)   logGreen();  dlDPrint(DL_DEBUG_CHANNEL,m"\n",##__VA_ARGS__);  logNormal();

#ifdef __cplusplus
extern "C" {
#endif

int dlDEBTRACE(void);
int dlDEBCHAN(const char *channel);
void dlDEBADD(const char *channel);
void dlDEBRM(const char *channel);
void dlDEBINIT(int argc, const char **argv);

void dlLogOpen(void);
void dlDPrint(const char *channel, const char *output, ...);
void dlDPuts(const char *channel, const char *output);
void dlPrint(const char *output, ...);
void dlPuts(const char *output);
void dlLogClose(void);

#ifdef __cplusplus
}
#endif

#endif /* DL_LOGFILE_WRAPPER */
