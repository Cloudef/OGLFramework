#ifndef FRAMEWORK_H
#define FRAMEWORK_H

#include "dlCore.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Enable disable logging */
void dlDisableLog( int );
void dlDisableOut( int );

/* Set render mode */
void dlSetRenderMode( dleRenderMode );

/* Create display */
int dlCreateDisplay(int, int, dleRenderer);
int dlFreeDisplay( void );

/* Projection */
void dlSetProjection( kmMat4 projection );
kmMat4 dlGetProjection( void );

/* Active camera */
void dlSetCamera( dlCamera *camera );
dlCamera* dlGetCamera( void );

/* Active shader */
void dlSetShader( dlShader *shader );
dlShader* dlGetShader( void );

/* Output memory graph */
void dlMemoryGraph( void );

#ifdef __cplusplus
}
#endif

#endif /* FRAMEWORK_H */
