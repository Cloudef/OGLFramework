#ifndef GL_CORE_H
#define GL_CORE_H

#include <stdint.h>
#include <stdio.h>

#include "sceneobject.h"
#include "camera.h"

#ifdef __cplusplus
extern "C" {
#endif

/* renderer description enums */
typedef enum
{
   GL_RENDER_DEFAULT,
   GL_RENDER_OGL3,
   GL_RENDER_OGL140
} gleRenderer;

/* render mode enums, VBO or vertex array */
typedef enum
{
   GL_MODE_VBO,
   GL_MODE_VERTEX_ARRAY
} gleRenderMode;

/* struct for renderer info */
typedef void drawPtr( glObject* );
typedef struct
{
   gleRenderer    id;
   gleRenderMode  mode;
   drawPtr       *draw;
   const char    *string;

   kmMat4         projection;
   glCamera      *camera;
} glRenderInfo;

/* version info */
typedef struct
{
   const char  *vendor;
   uint8_t     major;
   uint8_t     minor;
   uint8_t     patch;
} glVersionInfo;

/* display info */
typedef struct
{
   int width;
   int height;
} glDisplayInfo;

/* OpenGL information */
typedef struct
{
   int maxLights;
   int maxClipPlanes;
   int subPixelBits;
   int maxTextureSize;
   int maxTextureUnits;
} glInfo;

/* core struct
 * should store everything important,
 * that can be accessed anywhere */
typedef struct
{
  glRenderInfo    render;
  glVersionInfo   version;
  glDisplayInfo   display;
  glInfo          info;

  FILE            *log;
  uint8_t         disableLog;
  uint8_t         disableOut;

  const char      *extensions;
} glCoreConfig;

/* extern this */
extern glCoreConfig _glCore;

#ifdef __cplusplus
}
#endif

#endif /* GL_CORE_H */

/* EoF */
