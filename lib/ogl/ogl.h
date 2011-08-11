#ifndef OGL_H
#define OGL_H

#ifdef GLES2
#	include <GLES2/gl2.h>
#else
#	include "GL/glew.h"
#	include "GL/gl.h"
#endif

#include "kazmath/kazmath.h"
#include "config.h"
#include "framework.h"
#include "sceneobject.h"
#include "skeletal/evaluator.h"

#endif /* OGL_H */

/* EoF */
