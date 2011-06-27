#ifndef FRAMEWORK_H
#define FRAMEWORK_H

#include "core.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Enable disable logging */
void glDisableLog( int );
void glDisableOut( int );

/* Set render mode */
void glSetRenderMode( gleRenderMode );

/* Create display */
int glCreateDisplay(int, int, gleRenderer);
int glFreeDisplay( void );

/* Projection */
void glSetProjection( kmMat4 projection );
kmMat4 glGetProjection( void );

#ifdef __cplusplus
}
#endif

#endif /* FRAMEWORK_H */

/* EOF */
