#ifndef DL_H
#define DL_H

#ifdef GLES1
#  include <GLES/gl.h>
#elif GLES2
#  include <GLES2/gl2.h>
#else
#  include "GL/glew.h"
#  include "GL/gl.h"
#endif

#include "kazmath/kazmath.h"
#include "dlConfig.h"
#include "dlFramework.h"
#include "dlSceneobject.h"
#include "dlLog.h"
#include "skeletal/dlEvaluator.h"
#include "shader/dlShader.h"

#endif /* DL_H */
