#ifndef DL_CORE_H
#define DL_CORE_H

#include <stdint.h>
#include <stdio.h>

#include "dlSceneobject.h"
#include "dlCamera.h"

#ifdef __cplusplus
extern "C" {
#endif

/* renderer description enums */
typedef enum
{
   DL_RENDER_DEFAULT,
   DL_RENDER_OGL3,
   DL_RENDER_OGL140
} dleRenderer;

/* render mode enums, VBO or vertex array */
typedef enum
{
   DL_MODE_VBO,
   DL_MODE_VERTEX_ARRAY
} dleRenderMode;

/* struct for renderer info */
typedef void drawPtr( dlObject* );
typedef struct
{
   dleRenderer    id;
   dleRenderMode  mode;
   drawPtr       *draw;
   const char    *string;

   kmMat4         projection;
   dlCamera      *camera;
} dlRenderInfo;

/* version info */
typedef struct
{
   const char  *vendor;
   uint8_t     major;
   uint8_t     minor;
   uint8_t     patch;
} dlVersionInfo;

/* display info */
typedef struct
{
   int width;
   int height;
} dlDisplayInfo;

/* Opendl information */
typedef struct
{
   int maxLights;
   int maxClipPlanes;
   int subPixelBits;
   int maxTextureSize;
   int maxTextureUnits;
} dlInfo;

/* core struct
 * should store everything important,
 * that can be accessed anywhere */
typedef struct
{
  dlRenderInfo    render;
  dlVersionInfo   version;
  dlDisplayInfo   display;
  dlInfo          info;

  FILE            *log;
  uint8_t         disableLog;
  uint8_t         disableOut;

  const char      *extensions;
} dlCoreConfig;

/* extern this */
extern dlCoreConfig _dlCore;

#ifdef __cplusplus
}
#endif

#endif /* DL_CORE_H */

/* EoF */
